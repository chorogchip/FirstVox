#include "ChunkManager.h"

#include <list>
#include <vector>
#include <bit>
#include <thread>


#include "Macros.h"
#include "Logger.h"
#include "GameUtils.h"
#include "QueueFor1WorkerThread.h"
#include "Shapes.h"
#include "Chunk.h"
#include "GameCore.h"
#include "VertexRenderer.h"
#include "NetWorkManager.h"

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
        MESH_GENERATING,
        MESH_UNMAPPED,
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

        void ConstructForReuse( vox::data::Vector4i cv )
        {
            next = nullptr;
            last_updated_gametick = 0U;
            state = EnumChunkStates::LOADING;
            chunk.ConstructForReuse( cv );
        }
    };
    static ChunkNode* game_chunks_arr_[GAME_CHUNKS_ARR_SZ];
    vox::data::QueueFor1WorkerThread<ChunkNode*, 32> chunknode_reuse_buffer;

    static std::thread chunk_load_thread_;
    static volatile bool exit_chunk_load_thread = false;
    static vox::data::QueueFor1WorkerThread<ChunkNode*, 256> chunk_load_job_queue_;
    static vox::data::QueueFor1WorkerThread<ChunkNode*, 256> chunk_clear_job_queue_;
    static vox::data::QueueFor1WorkerThread<ChunkNode*, 256> chunk_vertex_job_queue_;
    static vox::data::QueueFor1WorkerThread<std::tuple<int,int,int,data::Block>, 256> packet_setblock_queue_;
    static vox::data::QueueFor1WorkerThread<std::tuple<int,int,int>, 256> packet_load_chunk_queue_;
    static vox::data::QueueFor1WorkerThread<std::tuple<int,int,int, ChunkNode*>, 256> packet_load_chunk_job_queue_;
    static std::vector<std::tuple<int,int,int,data::Block>> setblock_packet_store_;

    struct ChunkNodeRenderInfo
    {
        ChunkNode* chunk_node;
        vox::data::EnumBitSide6 sides;
    };
    static std::vector<ChunkNodeRenderInfo> game_chunks_render_list_;

    struct ChunkLoaderDynamic
    {
        vox::data::Entity* p_pos;
        int load_distance;
    };
    struct ChunkLoaderStatic
    {
        vox::data::Vector4i pos;
        int load_distance;
    };
    static std::vector<ChunkLoaderDynamic> game_chunkloaders_dynamic_;
    static std::vector<ChunkLoaderStatic> game_chunkloaders_static_;
    static std::vector<vox::data::Entity*> game_chunkloaders_dynamic_remove_;
    static std::vector<ChunkLoaderStatic> game_chunkloaders_static_remove_;

    unsigned char light_bfs_arr[vox::consts::CHUNK_BLOCKS_CNT * 12];
    unsigned d_lights[(vox::consts::CHUNK_X + 2) * vox::consts::CHUNK_Y * (vox::consts::CHUNK_Z + 2)];

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


    void ProcessSetBlockPacket(int x, int y, int z, data::Block block)
    {
        if (packet_setblock_queue_.IsFull())
            assert(0);
        packet_setblock_queue_.Push(std::make_tuple(x, y, z, block));
    }
    void ReplyLoadChunkPacket(int x, int y, int z)
    {
        if (packet_load_chunk_queue_.IsFull())
            assert(0);
        packet_load_chunk_queue_.Push(std::make_tuple(x, y, z));
    }

    static void ProcessLoadChunkPacket(int x, int y, int z, ChunkNode* cn)
    {
        net::DefPacket ret_packet;
        memset(&ret_packet, 0, sizeof(ret_packet));
        ret_packet.x = x;
        ret_packet.y = y;
        ret_packet.z = z;

        if (cn == nullptr)
        {
            FILE* fp = data::Chunk::GetFP(x, y, z, "rb");
            if (fp == nullptr)
            {
                goto GEN_CHUNK;
            }
            else
            {
                // frem storage
                fseek(fp, 0L, SEEK_END);
                size_t sz = ftell(fp);
                fseek(fp, 0L, SEEK_SET);
                unsigned char *data = new unsigned char[sz + sizeof(ret_packet)];
                ret_packet.flag = net::EnumPacketFlag::DATA_CHUNK;
                ret_packet.data = (void*)(sz + sizeof(ret_packet));
                memcpy(data, &ret_packet, sizeof(ret_packet));
                fread(data + sizeof(ret_packet), 1, sz, fp);
                fclose(fp);
                net::NMSendDefPacket((net::DefPacket*)data);
                delete[] data;
            }
        }
        else if (cn->chunk.IsChanged())
        {
            // from ram
            unsigned char* data;
            size_t sz = cn->chunk.GetStoringData(&data, sizeof(ret_packet));
            ret_packet.flag = net::EnumPacketFlag::DATA_CHUNK;
            ret_packet.data = (void*)(sz + sizeof(ret_packet));
            memcpy(data, &ret_packet, sizeof(ret_packet));
            net::NMSendDefPacket((net::DefPacket*)data);
            delete[] data;
        }
        else
        {
            goto GEN_CHUNK;
        }

GEN_CHUNK:
        ret_packet.flag = net::EnumPacketFlag::GEN_CHUNK;
        net::NMSendDefPacket(&ret_packet);
        return;
    }
    void ProcessGenChunkPacket(void* chunk_node_ptr)
    {
        ChunkNode* p_chunk_node = (ChunkNode*)chunk_node_ptr;
        p_chunk_node->chunk.Load( true );
        p_chunk_node->state = EnumChunkStates::LOAD_FINISHED;
    }
    void ProcessDataChunkPacket(void* chunk_node_ptr, void* data, size_t sz)
    {
        ChunkNode* p_chunk_node = (ChunkNode*)chunk_node_ptr;
        p_chunk_node->chunk.LoadFromData(data, sz);
        p_chunk_node->state = EnumChunkStates::LOAD_FINISHED;
    }

    static void VEC_CALL LoadChunksByChunkLoader( vox::data::Vector4i chunk_num, int load_dist )
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
                        if ( !chunknode_reuse_buffer.IsEmpty() )
                        {
                            *ch = chunknode_reuse_buffer.Pop();
                            (*ch)->ConstructForReuse( cv );
                        }
                        else
                        {
                            *ch = new ChunkNode( cv );
                        }

                        if (!net::NMIsClient())
                        {
                            if ( chunk_load_job_queue_.IsFull() )
                            {
                                (*ch)->chunk.Load();
                                (*ch)->chunk.SetBlocks(setblock_packet_store_);
                                (*ch)->state = EnumChunkStates::LOAD_FINISHED;
                            }
                            else
                            {
                                chunk_load_job_queue_.Push( *ch );
                            }
                        }
                        else
                        {
                            net::DefPacket def_packet;
                            def_packet.flag = net::EnumPacketFlag::LOAD_CHUNK;
                            def_packet.x = vox::data::vector::GetX( cv ); 
                            def_packet.y = vox::data::vector::GetY( cv ); 
                            def_packet.z = vox::data::vector::GetZ( cv ); 
                            def_packet.data = *ch;
                            net::NMSendDefPacket(&def_packet);
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
                    if ( p_chunk->state == EnumChunkStates::LOADING ||
                        p_chunk->state == EnumChunkStates::MESH_GENERATING )
                        continue;

                    ChunkNode *const p_chunk_to_delete = p_chunk;
                    *pp_chunk = p_chunk->next;
                    p_chunk = *pp_chunk;
                    if ( chunk_clear_job_queue_.IsFull() )
                    {
                        p_chunk_to_delete->chunk.Clear( false );
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
        //unsigned char* light_bfs_arr = (unsigned char*)malloc( sizeof( unsigned char ) * vox::consts::CHUNK_BLOCKS_CNT * 12 );
        //unsigned* d_lights = (unsigned*)malloc( sizeof( unsigned ) * (vox::consts::CHUNK_X + 2) * vox::consts::CHUNK_Y * (vox::consts::CHUNK_Z + 2) );

        while ( true )
        {
            if ( chunk_load_job_queue_.IsEmpty() &&
                chunk_clear_job_queue_.IsEmpty() &&
                chunk_vertex_job_queue_.IsEmpty() &&
                packet_load_chunk_job_queue_.IsEmpty() )
            {
                if ( exit_chunk_load_thread ) return;

                using namespace std::chrono_literals;
                std::this_thread::sleep_for(20ms);
            }
            else
            {

                const int sz_mesh = (int)chunk_vertex_job_queue_.Size();
                for ( int i = 0; i < sz_mesh; ++i )
                {
                    auto ch = chunk_vertex_job_queue_.Pop();

                    if ( ch->state != EnumChunkStates::MESH_GENERATING )
                        goto PASS_THIS_CHUNK;

                    vox::data::Chunk *adchs[9];
                    for ( int i = 0; i < 8; ++i )
                    {
                        const vox::data::Vector4i adjacent_cv = vox::data::vector::Add( ch->chunk.GetCV(), vox::data::DIRECTION8_V4I[i]);
                        ChunkNode *adch = GetChunkNodeByChunkNum( adjacent_cv );
                        if ( adch == nullptr || adch->state == EnumChunkStates::LOADING )
                        {
                            ch->state = EnumChunkStates::MESH_NEEDED;
                            goto PASS_THIS_CHUNK;
                        }
                        adchs[i] = &adch->chunk;
                    }
                    adchs[8] = &ch->chunk;
                    ch->chunk.GenerateVertex( adchs, light_bfs_arr, d_lights );

                    if ( ch->state != EnumChunkStates::MESH_GENERATING )
                        goto PASS_THIS_CHUNK;

                    // TODO bug : if chunk state modification code at SetBlock is executed on other thread at here, then mesh is not regenerated.
                    // and i think i have to prevent reordering and make chunk state safer by for example making it atomic
                    // but that bug don't terminates the program and just make 1 block invisible,
                    // and has very low possibility to occur, so i will care later

                    ch->state = EnumChunkStates::MESH_UNMAPPED;
PASS_THIS_CHUNK:;
                }

                const int sz_load = (int)chunk_load_job_queue_.Size();
                for (int i = 0; i < sz_load; ++i)
                {
                    ChunkNode *const p_chunk_node = chunk_load_job_queue_.Pop();
                    p_chunk_node->chunk.Load();
                    p_chunk_node->chunk.SetBlocks(setblock_packet_store_);
                    p_chunk_node->state = EnumChunkStates::LOAD_FINISHED;
                }

                const int sz_clear = (int)chunk_clear_job_queue_.Size();
                for ( int i = 0; i < sz_clear; ++i )
                {
                    ChunkNode *const p_chunk_node = chunk_clear_job_queue_.Pop();
                    p_chunk_node->chunk.Clear( true );
                    if ( !chunknode_reuse_buffer.IsFull() )
                        chunknode_reuse_buffer.Push( p_chunk_node );
                    else
                        delete p_chunk_node;
                }

                const int sz_ldnet = (int)packet_load_chunk_job_queue_.Size();
                for (int i = 0; i < sz_ldnet; ++i)
                {
                    auto [x, y, z, cn] = packet_load_chunk_job_queue_.Pop();
                    ProcessLoadChunkPacket(x, y, z, cn);
                }

            }
        }

        //free( d_lights );
        //free( light_bfs_arr );
    }

    int GetRenderChunkDist()
    {
        return chunk_render_dist_;
    }

    void SetRenderChunkDist( int render_chunk_dist )
    {
        if ( render_chunk_dist > vox::consts::MAX_RENDER_DIST )
            render_chunk_dist = vox::consts::MAX_RENDER_DIST;
        if ( render_chunk_dist < 2 )
            render_chunk_dist = 2;
        chunk_render_dist_ = render_chunk_dist;
    }

    void Init()
    {
        if (net::NMIsClient())
            SetRenderChunkDist(2);
        else
            SetRenderChunkDist(vox::consts::INIT_RENDER_DIST);

        if (!net::NMIsClient())
        {
            FILE* fp = nullptr;
            fopen_s(&fp, "GameData/SetBlockBuf", "rb");
            if (fp != nullptr)
            {
                size_t sz;
                fread(&sz, sizeof(sz), 1, fp);
                if (sz != 0)
                {
                    setblock_packet_store_.reserve(sz);
                    for (int i = 0; i < sz; ++i) setblock_packet_store_.emplace_back();
                    fread(&setblock_packet_store_[0], sizeof(setblock_packet_store_[0]), sz, fp);
                }
                fclose(fp);
            }
        }

        const vox::data::Vector4i cam_pos = vox::data::vector::ConvertToVector4i(
            vox::data::vector::Round( vox::core::gamecore::camera.entity.GetPositionVec() ) );
        const vox::data::Vector4i cam_chunk_num = GetChunkNumByBlockPos( cam_pos );
        
        RegisterDynamicChunkLoader( &vox::core::gamecore::camera.entity, GetRenderChunkDist() + 2);

        chunk_load_thread_ = std::thread(ChunkLoadThreadLoop);

        // size can be incremented during loop, need to modify when introduced multithread
        int dyn_i = 0, stt_i = 0;
        do
        {
            if ( dyn_i < game_chunkloaders_dynamic_.size() )
            {
                const vox::data::Vector4i loader_pos = vox::data::vector::ConvertToVector4i(
                    vox::data::vector::Round( game_chunkloaders_dynamic_[dyn_i].p_pos->GetPositionVec() )
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
        if (!net::NMIsClient())
        {
            FILE* fp = nullptr;
            fopen_s(&fp, "GameData/SetBlockBuf", "wb");
            const size_t sz = setblock_packet_store_.size();
            fwrite(&sz, sizeof(sz), 1, fp);
            if (sz != 0)
                fwrite(&setblock_packet_store_[0], sizeof(setblock_packet_store_[0]), sz, fp);
            fclose(fp);
        }


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
                cur->chunk.Clear( false );
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

        for (int i = 0, sz = (int)packet_setblock_queue_.Size(); i < sz; ++i )
        {
            auto o = packet_setblock_queue_.Pop();
            auto [x, y, z, blk] = o;
            const auto res = SetBlock( data::vector::Set(x, y, z, 0), blk, false );
            if (res == data::EnumActionResultSF::FAILED)
                setblock_packet_store_.push_back(o);
        }

        for (int i = 0, sz = (int)packet_load_chunk_queue_.Size(); i < sz; ++i)
        {
            if (packet_load_chunk_job_queue_.IsFull())
                assert(0);
            auto [x, y, z] = packet_load_chunk_queue_.Pop();
            ChunkNode* cn = GetChunkNodeByChunkNum(vox::data::vector::Set(x, y, z, 0));
            packet_load_chunk_job_queue_.Push(std::make_tuple(x, y, z, cn));
        }

        // size can be incremented during loop, need to modify when introduced multithread
        int dyn_i = 0, stt_i = 0;
        do
        {
            if ( dyn_i < game_chunkloaders_dynamic_.size() )
            {
                const vox::data::Vector4i loader_pos = vox::data::vector::ConvertToVector4i(
                    vox::data::vector::Round( game_chunkloaders_dynamic_[dyn_i].p_pos->GetPositionVec() )
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

        const auto cam_pos_f = vox::core::gamecore::camera.entity.GetPositionVec();
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


        vox::data::shapes::Plane frustum_planes[6];
        vox::core::gamecore::camera.GenerateViewFrustum( frustum_planes );

        for ( int dcz = -chunk_render_dist_; dcz <= chunk_render_dist_; ++dcz )
            for ( int dcx = -chunk_render_dist_; dcx <= chunk_render_dist_; ++dcx )
            {
                const int64_t dpx = (int64_t)((dcx << vox::consts::CHUNK_X_LOG2) + cdcx);
                const int64_t dpz = (int64_t)((dcz << vox::consts::CHUNK_Z_LOG2) + cdcz);
                const int64_t dist_sq = dpz * dpz + dpx * dpx;

                vox::data::Vector4i cv;
                ChunkNode* ch;
                bool is_chunk_in_frustum = true;

                // check chunk is in render dist
                if ( dist_sq > render_dist_sq )
                    goto DONT_RENDER_THIS_CHUNK;

                // check presence of chunk on memory
                cv = vox::data::vector::Add( cam_chunk_num, vox::data::vector::Set( dcx, 0, dcz, 0 ) );
                ch = GetChunkNodeByChunkNum( cv );
                if ( ch == nullptr )
                    goto DONT_RENDER_THIS_CHUNK;

                // view frustum culling
                {
                    // im busy abstraction will be done later

                    const float x_min = (float)(vox::data::vector::GetX( cv ) << vox::consts::CHUNK_X_LOG2);
                    const float x_max = x_min + (float)vox::consts::CHUNK_X;
                    const float y_min = 0.0f;
                    const float y_max = (float)vox::consts::CHUNK_Y;
                    const float z_min = (float)(vox::data::vector::GetZ( cv ) << vox::consts::CHUNK_Z_LOG2);
                    const float z_max = z_min + (float)vox::consts::CHUNK_Z;

                    for ( auto& plane : frustum_planes )
                    {
                        const float* plane_f = (const float*)&plane;

                        float far_xyz[3];

                        far_xyz[0] = plane_f[0] >= 0.0f ? x_max : x_min;
                        far_xyz[1] = plane_f[1] >= 0.0f ? y_max : y_min;
                        far_xyz[2] = plane_f[2] >= 0.0f ? z_max : z_min;

                        const float far_res = vox::utils::dot3( far_xyz, plane_f );
                        if ( far_res + plane_f[3] < 0.0f )
                        {
                            is_chunk_in_frustum = true;
                            break;
                        }
                    }

                }

                switch ( ch->state )
                {
                case EnumChunkStates::LOADING:
                default:
                    goto DONT_RENDER_THIS_CHUNK;

                case EnumChunkStates::LOAD_FINISHED:
                    if ( chunk_vertex_job_queue_.IsFull() )
                    {
                        if ( is_chunk_in_frustum )
                            goto GENERATE_VERTEX;
                        else
                            goto DONT_RENDER_THIS_CHUNK;
                    }
                    ch->state = EnumChunkStates::MESH_GENERATING;
                    chunk_vertex_job_queue_.Push( ch );
                    goto DONT_RENDER_THIS_CHUNK;

                case EnumChunkStates::MESH_GENERATING:
                    goto DONT_RENDER_THIS_CHUNK;

                case EnumChunkStates::MESH_UNMAPPED:
                    ch->chunk.MapTempVertexToBuffer();
                    ch->state = EnumChunkStates::MESH_GENED;
                    goto RENDER_THIS_CHUNK;

                case EnumChunkStates::MESH_NEEDED:
                    if ( is_chunk_in_frustum )
                    {
                        goto GENERATE_VERTEX;
                    }
                    goto DONT_RENDER_THIS_CHUNK;

                case EnumChunkStates::MESH_GENED:
                    goto RENDER_THIS_CHUNK;
                }
GENERATE_VERTEX:
                {
                    vox::data::Chunk *adchs[9];
                    for ( int i = 0; i < 8; ++i )
                    {
                        const vox::data::Vector4i adjacent_cv = vox::data::vector::Add( cv, vox::data::DIRECTION8_V4I[i] );
                        ChunkNode *adch = GetChunkNodeByChunkNum( adjacent_cv );
                        if ( adch == nullptr || adch->state == EnumChunkStates::LOADING )
                        {
                            goto DONT_RENDER_THIS_CHUNK;
                        }
                        adchs[i] = &adch->chunk;
                    }
                    adchs[8] = &ch->chunk;
                    ch->chunk.GenerateVertex( adchs, light_bfs_arr, d_lights );
                    ch->chunk.MapTempVertexToBuffer();
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
            }  // 2 for loop
    }

    void Render()
    {
        for ( auto o : game_chunks_render_list_ )
        {
            o.chunk_node->chunk.Render( o.sides );
        }
    }

    data::EnumActionResultSF VEC_CALL SetBlock(vox::data::Vector4i block_pos, vox::data::Block block, bool send_packet )
    {
        const auto cv = GetChunkNumByBlockPos( block_pos );
        ChunkNode* const ch = GetChunkNodeByChunkNum( cv );
        if ( ch == nullptr )
        {
            return data::EnumActionResultSF::FAILED;
        }
        else
        {
            alignas(16) int bp[4];
            vox::data::vector::Store( (vox::data::Vector4i*)bp, GetBlockRemPosByPos( block_pos ) );

            if ( (int)ch->state > (int)EnumChunkStates::MESH_NEEDED )
            {
                ch->state = EnumChunkStates::MESH_NEEDED;
            }
            for ( int i = 0; i < 8; ++i )
            {
                const vox::data::Vector4i adjacent_cv =
                    vox::data::vector::Add( cv, vox::data::DIRECTION8_V4I[i] );
                ChunkNode* const adch = GetChunkNodeByChunkNum( adjacent_cv );
                if ( adch != nullptr && (int)adch->state > (int)EnumChunkStates::MESH_NEEDED )
                {
                    adch->state = EnumChunkStates::MESH_NEEDED;
                }
            }
            ch->chunk.SetBlock( bp[0], bp[1], bp[2], block );
            ch->chunk.Touch();

            if (send_packet && net::NMHasConnection())
            {
                net::DefPacket def_packet;
                def_packet.flag = net::EnumPacketFlag::SET_BLOCK;
                def_packet.x = vox::data::vector::GetX( block_pos ); 
                def_packet.y = vox::data::vector::GetY( block_pos ); 
                def_packet.z = vox::data::vector::GetZ( block_pos );
                def_packet.data = (void*)(size_t)block.id;
                net::NMSendDefPacket(&def_packet);
            }

            return data::EnumActionResultSF::SUCCEED;
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

    void RegisterDynamicChunkLoader( vox::data::Entity* p_chunk_loader, int load_distance )
    {
        game_chunkloaders_dynamic_.emplace_back( p_chunk_loader, load_distance );
    }

    // this method cannot be called in init of some object, but in update or clean is ok
    void CleanDynamicChunkLoader( vox::data::Entity* p_chunk_loader )
    {
        game_chunkloaders_dynamic_remove_.push_back( p_chunk_loader );
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
