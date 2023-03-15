#include "ChunkManager.h"

#include <list>
#include <vector>
#include <bit>
#include <thread>


#include "Macros.h"
#include "Logger.h"
#include "GameUtils.h"
#include "QueueFor1WorkerThread.h"
#include "Chunk.h"
#include "GameCore.h"
#include "VertexRenderer.h"

namespace vox::core::chunkmanager
{
    static constexpr int GAME_CHUNKS_ARR_WIDTH_LOG2 = 5;
    static constexpr int GAME_CHUNKS_ARR_WIDTH = 1 << GAME_CHUNKS_ARR_WIDTH_LOG2;
    static constexpr int GAME_CHUNKS_ARR_SZ = GAME_CHUNKS_ARR_WIDTH * GAME_CHUNKS_ARR_WIDTH;

    static int chunk_render_dist_;
    static vox::data::Vector4f previous_camera_pos_;

    /*
        청크들은 ChunkNode에 묶여 해쉬 배열로 관리된다.
        우선 해쉬의 버킷에 아무것도 들어있지 않을 수 있다.
        또한 다른 스레드에 해당 버킷에 해당하는 청크를 요청했을 경우가 있다. (이 경우를 위해 chunk pos를 chunknode로 옮김, chunk pos를 기록해놔야 돼서)
        그리고 다른 스레드가 해당 청크의 데이터를 다 읽어온 상태가 있다. chunkmanager에서는 이 상태가 있다면 즉시 다음 상태로 전환한다.
        또 청크들이 청크로더의 범위 안에 들어와 로딩된 상태가 있다.
        모든 청크로더가 로딩된 청크의 범위 밖에서 벗어나 
    */
    enum class EnumChunkStates : int
    {
        LOADING,
        LOAD_FINISHED,
        MESH_NEEDED,
        MESH_GENED,
    };
    struct ChunkNode
    {
        ChunkNode* next;
        uint64_t last_updated_gametick;
        EnumChunkStates state;
        vox::data::Chunk chunk;

        ChunkNode( vox::data::Vector4i cv ) :
            next{ nullptr },
            last_updated_gametick{ 0U },
            state{ EnumChunkStates::LOADING },
            chunk{ cv }
        { }

    };
    static ChunkNode* game_chunks_arr_[GAME_CHUNKS_ARR_SZ];

    static std::thread chunk_load_thread_;
    static volatile bool exit_chunk_load_thread = false;
    vox::data::QueueFor1WorkerThread<ChunkNode*, 256> chunk_load_job_queue_;
    vox::data::QueueFor1WorkerThread<ChunkNode*, 256> chunk_clear_job_queue_;

    struct ChunkNodeRenderInfo
    {
        ChunkNode* chunk_node;
        vox::data::EnumBitSide6 sides;
    };
    static std::vector<ChunkNodeRenderInfo> game_chunks_render_list_;

    struct ChunkLoaderDynamic
    {
        vox::data::Vector4f* p_pos;
        int load_distance;
    };
    struct ChunkLoaderStatic
    {
        vox::data::Vector4i pos;
        int load_distance;
    };
    static std::vector<ChunkLoaderDynamic> game_chunkloaders_dynamic_;
    static std::vector<ChunkLoaderStatic> game_chunkloaders_static_;
    static std::vector<vox::data::Vector4f*> game_chunkloaders_dynamic_remove_;
    static std::vector<ChunkLoaderStatic> game_chunkloaders_static_remove_;


    static FORCE_INLINE vox::data::Vector4i VEC_CALL GetChunkNumByBlockPos( vox::data::Vector4i pos )
    {
        static_assert(vox::consts::CHUNK_X_LOG2 == vox::consts::CHUNK_Z_LOG2);
        const auto sh_res = vox::data::vector::ShiftRightSignExtend( pos, vox::consts::CHUNK_X_LOG2 );
        const auto mask = vox::data::vector::Set( -1, 0, -1, 0 );
        return vox::data::vector::And( sh_res, mask );
    }

    static FORCE_INLINE vox::data::Vector4i VEC_CALL GetBlockRemPosByPos( vox::data::Vector4i pos )
    {
        const auto mask = vox::data::vector::Set( vox::consts::CHUNK_X - 1, -1, vox::consts::CHUNK_Z - 1, 0 );
        return vox::data::vector::And( pos, mask );
    }

    static FORCE_INLINE ChunkNode* VEC_CALL GetChunkNodeByChunkNum( vox::data::Vector4i chunk_num )
    {
        alignas(16) int gc[4];
        vox::data::vector::Store( (vox::data::Vector4i*)gc, chunk_num );
        const int gcx = gc[0] & (GAME_CHUNKS_ARR_WIDTH - 1);
        const int gcz = gc[2] & (GAME_CHUNKS_ARR_WIDTH - 1);

        ChunkNode* cur = game_chunks_arr_[(gcz << GAME_CHUNKS_ARR_WIDTH_LOG2) + gcx];
        while ( cur != nullptr && !vox::data::vector::Equal( cur->chunk.GetCV(), chunk_num) )
        {
            cur = cur->next;
        }
        return cur;
    }

    static FORCE_INLINE ChunkNode** VEC_CALL GetChunkNodeAddressByChunkNum( vox::data::Vector4i chunk_num )
    {
        alignas(16) int gc[4];
        vox::data::vector::Store( (vox::data::Vector4i*)gc, chunk_num );
        const int gcx = gc[0] & (GAME_CHUNKS_ARR_WIDTH - 1);
        const int gcz = gc[2] & (GAME_CHUNKS_ARR_WIDTH - 1);

        ChunkNode** cur = &game_chunks_arr_[(gcz << GAME_CHUNKS_ARR_WIDTH_LOG2) + gcx];
        while ( *cur != nullptr && !vox::data::vector::Equal( (*cur)->chunk.GetCV(), chunk_num) )
        {
            cur = &(*cur)->next;
        }
        return cur;
    }

    static FORCE_INLINE void VEC_CALL LoadChunksByChunkLoader( vox::data::Vector4i chunk_num, int load_dist )
    {
        const auto game_tick = vox::core::gamecore::GetGameTicks();
        const int load_dist_sq = load_dist * load_dist;

        for ( int dcz = -load_dist; dcz <= load_dist; ++dcz )
            for ( int dcx = -load_dist; dcx <= load_dist; ++dcx )
            {
                const int dist_sq = dcx * dcx + dcz * dcz;
                if ( dist_sq <= load_dist_sq )
                {
                    const vox::data::Vector4i dcv = vox::data::vector::Set( dcx, 0, dcz, 0 );
                    const vox::data::Vector4i cv = vox::data::vector::Add( chunk_num, dcv );
                    ChunkNode** ch = GetChunkNodeAddressByChunkNum( cv );
                    if ( *ch == nullptr )
                    {
                        *ch = new ChunkNode{ cv };
                        if ( chunk_load_job_queue_.IsFull() )
                        {
                            (*ch)->chunk.Load();
                            (*ch)->state = EnumChunkStates::LOAD_FINISHED;
                        }
                        else
                        {
                            chunk_load_job_queue_.Push( *ch );
                        }
                    }
                    (*ch)->last_updated_gametick = game_tick;
                }
            }
    }


    constexpr static int CHUNKS_DIV_CNT = 4;
    constexpr static int CHUNKS_DIV_SZ = GAME_CHUNKS_ARR_SZ / CHUNKS_DIV_CNT;
    static_assert(GAME_CHUNKS_ARR_SZ % CHUNKS_DIV_CNT == 0);

    static int chunk_index_begin = 0;
    static int chunk_index_end = CHUNKS_DIV_SZ;

    static FORCE_INLINE void UpdateChunksToClear()
    {
        const auto game_ticks = vox::core::gamecore::GetGameTicks();

        for ( int i = chunk_index_begin; i < chunk_index_end; ++i )
        {
            ChunkNode** pp_chunk = &game_chunks_arr_[i];
            ChunkNode *p_chunk = game_chunks_arr_[i];
            while ( p_chunk != nullptr )
            {
                if ( p_chunk->last_updated_gametick + 10 < game_ticks)
                {
                    ChunkNode *const p_chunk_to_delete = p_chunk;
                    *pp_chunk = p_chunk->next;
                    p_chunk = *pp_chunk;
                    if ( chunk_clear_job_queue_.IsFull() )
                    {
                        p_chunk_to_delete->chunk.Clear();
                        delete p_chunk_to_delete;
                    }
                    else
                    {
                        chunk_clear_job_queue_.Push( p_chunk_to_delete );
                    }
                }
                else
                {
                    pp_chunk = &(p_chunk->next);
                    p_chunk = p_chunk->next;
                }
            }
        }

        if ( chunk_index_end == GAME_CHUNKS_ARR_SZ )
        {
            chunk_index_begin = 0;
            chunk_index_end = CHUNKS_DIV_SZ;
        }
        else
        {
            chunk_index_begin += CHUNKS_DIV_SZ;
            chunk_index_end += CHUNKS_DIV_SZ;
        }
    }

    static void ChunkLoadThreadLoop()
    {
        while ( true )
        {
            if ( chunk_load_job_queue_.IsEmpty() && chunk_clear_job_queue_.IsEmpty() )
            {
                if ( exit_chunk_load_thread ) return;

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(20ms);
            }
            else
            {
                const int sz_load = (int)chunk_load_job_queue_.Size();
                for (int i = 0; i < sz_load; ++i)
                {
                    ChunkNode *const p_chunk_node = chunk_load_job_queue_.Pop();
                    p_chunk_node->chunk.Load();
                    p_chunk_node->state = EnumChunkStates::LOAD_FINISHED;
                }

                const int sz_clear = (int)chunk_clear_job_queue_.Size();
                for ( int i = 0; i < sz_clear; ++i )
                {
                    ChunkNode *const p_chunk_node = chunk_clear_job_queue_.Pop();
                    p_chunk_node->chunk.Clear();
                    delete p_chunk_node;
                }
            }
        }
    }

    int GetRenderChunkDist()
    {
        return chunk_render_dist_;
    }

    void SetRenderChunkDist( int render_chunk_dist )
    {
        if ( render_chunk_dist > vox::consts::MAX_RENDER_DIST )
            render_chunk_dist = vox::consts::MAX_RENDER_DIST;
        if ( render_chunk_dist < 1 )
            render_chunk_dist = 1;
        chunk_render_dist_ = render_chunk_dist;
    }

    void Init()
    {
        chunk_render_dist_ = vox::consts::INIT_RENDER_DIST;
        const vox::data::Vector4i cam_pos = vox::data::vector::ConvertToVector4i(
            vox::data::vector::Round( vox::core::gamecore::camera.position ) );
        const vox::data::Vector4i cam_chunk_num = GetChunkNumByBlockPos( cam_pos );
        
        RegisterDynamicChunkLoader( &vox::core::gamecore::camera.position, vox::consts::INIT_LOAD_DIST );

        chunk_load_thread_ = std::thread(ChunkLoadThreadLoop);

        // size can be incremented during loop, need to modify when introduced multithread
        int dyn_i = 0, stt_i = 0;
        do
        {
            if ( dyn_i < game_chunkloaders_dynamic_.size() )
            {
                const vox::data::Vector4i loader_pos = vox::data::vector::ConvertToVector4i(
                    vox::data::vector::Round( *game_chunkloaders_dynamic_[dyn_i].p_pos )
                );
                const vox::data::Vector4i loader_chunk_num = GetChunkNumByBlockPos( loader_pos );
                LoadChunksByChunkLoader( loader_chunk_num, game_chunkloaders_dynamic_[dyn_i].load_distance );

                ++dyn_i;
                continue;
            }
            else if ( stt_i < game_chunkloaders_static_.size() )
            {
                LoadChunksByChunkLoader( game_chunkloaders_static_[stt_i].pos, game_chunkloaders_static_[stt_i].load_distance);

                ++stt_i;
                continue;
            }
            else break;
        }
        while ( true );

    }

    void Clean()
    {
        exit_chunk_load_thread = true;
        chunk_load_thread_.join();

        game_chunks_render_list_.clear();

        game_chunkloaders_dynamic_.clear();
        game_chunkloaders_static_.clear();
        game_chunkloaders_dynamic_remove_.clear();
        game_chunkloaders_dynamic_remove_.clear();

        for ( int i = 0; i < GAME_CHUNKS_ARR_SZ; ++i )
        {
            ChunkNode* cur = game_chunks_arr_[i];
            while ( cur != nullptr )
            {
                ChunkNode* const tmp = cur->next;
                cur->chunk.Clear();
                delete cur;
                cur = tmp;
            }
            game_chunks_arr_[i] = nullptr;
        }
    }

    void Update()
    {
        for ( int i = 0, sz = (int)game_chunkloaders_dynamic_remove_.size(); i < sz; ++i )
            for ( auto it = game_chunkloaders_dynamic_.begin(), it_end = game_chunkloaders_dynamic_.end(); it != it_end; ++it )
                if ( it->p_pos == game_chunkloaders_dynamic_remove_[i] )
                {
                    game_chunkloaders_dynamic_.erase( it );
                    break;
                }
        game_chunkloaders_dynamic_remove_.clear();

        for ( int i = 0, sz = (int)game_chunkloaders_static_remove_.size(); i < sz; ++i )
            for ( auto it = game_chunkloaders_static_.begin(), it_end = game_chunkloaders_static_.end(); it != it_end; ++it )
                if ( vox::data::vector::Equal( it->pos, game_chunkloaders_static_remove_[i].pos ) &&
                    it->load_distance == game_chunkloaders_static_remove_[i].load_distance )
                {
                    game_chunkloaders_static_.erase( it );
                    break;
                }
        game_chunkloaders_static_remove_.clear();


        // size can be incremented during loop, need to modify when introduced multithread
        int dyn_i = 0, stt_i = 0;
        do
        {
            if ( dyn_i < game_chunkloaders_dynamic_.size() )
            {
                const vox::data::Vector4i loader_pos = vox::data::vector::ConvertToVector4i(
                    vox::data::vector::Round( *game_chunkloaders_dynamic_[dyn_i].p_pos )
                );
                const vox::data::Vector4i loader_chunk_num = GetChunkNumByBlockPos( loader_pos );
                LoadChunksByChunkLoader( loader_chunk_num, game_chunkloaders_dynamic_[dyn_i].load_distance );

                ++dyn_i;
                continue;
            }
            else if ( stt_i < game_chunkloaders_static_.size() )
            {
                LoadChunksByChunkLoader( game_chunkloaders_static_[stt_i].pos, game_chunkloaders_static_[stt_i].load_distance );

                ++stt_i;
                continue;
            }
            else break;
        } while ( true );

        UpdateChunksToClear();

        const auto cam_pos_f = vox::core::gamecore::camera.position;
        const auto cam_pos = vox::data::vector::ConvertToVector4i( vox::data::vector::Round( cam_pos_f ) );
        const auto cam_chunk_num = GetChunkNumByBlockPos( cam_pos );

        const int cdcx = (vox::data::vector::GetX( cam_chunk_num ) << vox::consts::CHUNK_X_LOG2) +
            vox::consts::CHUNK_X / 2 - vox::data::vector::GetX( cam_pos );
        const int cdcz = (vox::data::vector::GetZ( cam_chunk_num ) << vox::consts::CHUNK_Z_LOG2) +
            vox::consts::CHUNK_Z / 2 - vox::data::vector::GetZ( cam_pos );

        game_chunks_render_list_.clear();

        static_assert(vox::consts::CHUNK_X_LOG2 == vox::consts::CHUNK_Z_LOG2);
        const int64_t render_dist_sq = ((int64_t)chunk_render_dist_ * chunk_render_dist_) <<
            (int64_t)(vox::consts::CHUNK_X_LOG2 * 2);
        for ( int dcz = -chunk_render_dist_; dcz <= chunk_render_dist_; ++dcz )
            for ( int dcx = -chunk_render_dist_; dcx <= chunk_render_dist_; ++dcx )
            {
                const int64_t dpx = (int64_t)((dcx << vox::consts::CHUNK_X_LOG2) + cdcx);
                const int64_t dpz = (int64_t)((dcz << vox::consts::CHUNK_Z_LOG2) + cdcz);
                const int64_t dist_sq = dpz * dpz + dpx * dpx;

                if ( dist_sq <= render_dist_sq )
                {
                    const auto cv = vox::data::vector::Add( cam_chunk_num, vox::data::vector::Set( dcx, 0, dcz, 0 ) );
                    ChunkNode *ch = GetChunkNodeByChunkNum( cv );
                    if ( ch != nullptr )
                    {
                        switch ( ch->state )
                        {
                        case EnumChunkStates::LOADING:
                        default:
                            goto DONT_RENDER_THIS_CHUNK;

                        case EnumChunkStates::LOAD_FINISHED:
                        case EnumChunkStates::MESH_NEEDED:
                            goto GENERATE_VERTEX;

                        case EnumChunkStates::MESH_GENED:
                            goto RENDER_THIS_CHUNK;
                        }
GENERATE_VERTEX:
                        {
                            vox::data::Chunk *adchs[4];
                            for ( int i = 0; i < 4; ++i )
                            {
                                const vox::data::Vector4i adjacent_cv = vox::data::vector::Add( cv, vox::data::DIRECTION4_V4I[i] );
                                ChunkNode *adch = GetChunkNodeByChunkNum( adjacent_cv );
                                if ( adch == nullptr || adch->state == EnumChunkStates::LOADING )
                                {
                                    goto DONT_RENDER_THIS_CHUNK;
                                }
                                adchs[i] = &adch->chunk;
                            }
                            ch->chunk.GenerateVertex( adchs[0], adchs[1], adchs[2], adchs[3] );
                            ch->state = EnumChunkStates::MESH_GENED;
                        }
RENDER_THIS_CHUNK:
                        {
                            int sides = (int)vox::data::EnumBitSide6::FULL_VALUE;
                            if ( dcz > 0 )
                            {
                                sides -= (int)vox::data::EnumBitSide6::FRONT;
                            }
                            else if ( dcz < 0 )
                            {
                                sides -= (int)vox::data::EnumBitSide6::BACK;
                            }
                            if ( dcx > 0 )
                            {
                                sides -= (int)vox::data::EnumBitSide6::RIGHT;
                            }
                            else if ( dcx < 0 )
                            {
                                sides -= (int)vox::data::EnumBitSide6::LEFT;
                            }

                            game_chunks_render_list_.emplace_back( ch, (vox::data::EnumBitSide6)sides );
                        }
DONT_RENDER_THIS_CHUNK:;
                    }  // if chunk is not nullptr
                }  // if dist is in render dist
            }  // 2 for loop
    }  // if to_make_render_list_again

    void Render()
    {
        for ( auto o : game_chunks_render_list_ )
        {
            o.chunk_node->chunk.Render( o.sides );
        }
    }

    void VEC_CALL SetBlock(vox::data::Vector4i block_pos, vox::data::Block block)
    {
        const auto cv = GetChunkNumByBlockPos( block_pos );
        ChunkNode* const ch = GetChunkNodeByChunkNum( cv );
        if ( ch != nullptr )
        {
            alignas(16) int bp[4];
            vox::data::vector::Store( (vox::data::Vector4i*)bp, GetBlockRemPosByPos( block_pos ) );

            if ( ch->state == EnumChunkStates::MESH_GENED )
            {
                ch->state = EnumChunkStates::MESH_NEEDED;
            }
            static constexpr int zzxx[4] = { 2, 2, 0, 0 };
            static constexpr int border[4] = { vox::consts::CHUNK_Z - 1, 0, vox::consts::CHUNK_X - 1, 0 };
            for ( int i = 0; i < 4; ++i )
                if ( bp[zzxx[i]] == border[i] )
                {
                    const vox::data::Vector4i adjacent_cv =
                        vox::data::vector::Add( cv, vox::data::DIRECTION4_V4I[i] );
                    ChunkNode* const adch = GetChunkNodeByChunkNum( adjacent_cv );
                    if ( adch != nullptr && adch->state == EnumChunkStates::MESH_GENED )
                    {
                        adch->state = EnumChunkStates::MESH_NEEDED;
                    }
                }
            ch->chunk.SetBlock( bp[0], bp[1], bp[2], block );
            ch->chunk.Touch();
        }
    }

    vox::data::Block VEC_CALL GetBlock( vox::data::Vector4i block_pos )
    {
        const auto cv = GetChunkNumByBlockPos( block_pos );
        const ChunkNode* const ch = GetChunkNodeByChunkNum( cv );
        if ( ch != nullptr )
        {
            alignas(16) int bp[4];
            vox::data::vector::Store( (vox::data::Vector4i*)bp,
                GetBlockRemPosByPos( block_pos ) );
            return ch->chunk.GetBlock( bp[0], bp[1], bp[2] );
        }
        return vox::data::Block( vox::data::EBlockID::MAX_COUNT );
    }

    void RegisterDynamicChunkLoader( vox::data::Vector4f* p_chunk_loader_pos, int load_distance )
    {
        game_chunkloaders_dynamic_.emplace_back( p_chunk_loader_pos, load_distance );
    }

    // this method cannot be called in init of some object, but in update or clean is ok
    void CleanDynamicChunkLoader( vox::data::Vector4f* p_chunk_loader_pos )
    {
        game_chunkloaders_dynamic_remove_.push_back( p_chunk_loader_pos );
    }

    void VEC_CALL RegisterStaticChunkLoader( vox::data::Vector4i chunk_loader_pos, int load_distance )
    {
        game_chunkloaders_static_.emplace_back( chunk_loader_pos, load_distance );
    }

    // this method cannot be called in init of some object, but in update or clean is ok
    void VEC_CALL CleanStaticChunkLoader( vox::data::Vector4i chunk_loader_pos, int load_distance )
    {
        game_chunkloaders_static_remove_.emplace_back( chunk_loader_pos, load_distance );
    }

}  // namespace
