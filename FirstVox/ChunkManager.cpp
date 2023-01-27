#include "ChunkManager.h"

#include <list>
#include <bit>

#include "Macros.h"
#include "Logger.h"
#include "GameUtils.h"
#include "Chunk.h"
#include "GameCore.h"
#include "VertexRenderer.h"

namespace vox::core::chunkmanager
{
    static constexpr int GAME_CHUNKS_ARR_WIDTH_LOG2 = 32 - (int)std::countl_zero((unsigned)vox::consts::MAX_RENDER_DIST * 2U);
    static constexpr int GAME_CHUNKS_ARR_WIDTH = 1 << GAME_CHUNKS_ARR_WIDTH_LOG2;
    static constexpr int GAME_CHUNKS_ARR_SZ = GAME_CHUNKS_ARR_WIDTH * GAME_CHUNKS_ARR_WIDTH;

    static int render_chunk_dist_ = 5;
    static int update_chunk_dist_ = 6;
    static vox::data::Chunk* game_chunks_arr_[GAME_CHUNKS_ARR_SZ];
    static std::list<vox::data::Chunk**> game_chunks_render_list_;

    static FORCE_INLINE vox::data::Chunk*& VEC_CALL GetChunkByChunkNum( vox::data::Vector4i chunk_num )
    {
        alignas(16) int gc[4];
        vox::data::vector::Store( (vox::data::Vector4i*)gc, chunk_num );
        const int gcx = gc[0] & (GAME_CHUNKS_ARR_WIDTH - 1);
        const int gcz = gc[2] & (GAME_CHUNKS_ARR_WIDTH - 1);

        return game_chunks_arr_[(gcz << GAME_CHUNKS_ARR_WIDTH_LOG2) + gcx];
    }
    // assumes input block pos y is correct
    vox::data::Block* VEC_CALL GetBlockByPos( vox::data::Vector4i block_pos )
    {
        const auto cv = vox::gameutils::GetChunkNumByBlockPos( block_pos );
        auto& ch = GetChunkByChunkNum( cv );
        if ( vox::data::vector::Equal( cv, ch->GetChunkPos() ) && ch->IsRenderable() )
        {
            alignas(16) int bp[4];
            vox::data::vector::Store( (vox::data::Vector4i*)bp,
                vox::gameutils::GetBlockRemPosByPos( block_pos ) );
            return &ch->At( bp[0], bp[1], bp[2] );
        }
        return nullptr;
    }

    void VEC_CALL RebuildMeshByBlockPos( vox::data::Vector4i block_pos )
    {
        const auto cv = vox::gameutils::GetChunkNumByBlockPos( block_pos );
        alignas(16) int bp[4];
        vox::data::vector::Store( (vox::data::Vector4i*)bp,
            vox::gameutils::GetBlockRemPosByPos( block_pos ) );
        auto& ch = GetChunkByChunkNum( cv );
        if ( vox::data::vector::Equal( cv, ch->GetChunkPos() ) && ch->IsRenderable() )
        {
            vox::data::Chunk* adjacent_chunks[4];
            vox::data::Chunk* adadjacent_chunks[4];
            for ( int i = 0; i < 4; ++i )
            {
                const auto adjacent_cv = vox::data::vector::Add( cv, vox::data::DIRECTION4_V4I[i] );
                adjacent_chunks[i] = GetChunkByChunkNum( adjacent_cv );

                static constexpr int zzxx[4] = { 3, 3, 0, 0 };
                static constexpr int border[4] = { vox::consts::CHUNK_Z - 1, 0, vox::consts::CHUNK_X - 1, 0 };

                if ( bp[zzxx[i]] == border[i] && adjacent_chunks[i]->IsRenderable() )
                {
                    for ( int i = 0; i < 4; ++i )
                    {
                        const auto adadjacent_cv = vox::data::vector::Add( adjacent_cv, vox::data::DIRECTION4_V4I[i] );
                        adadjacent_chunks[i] = GetChunkByChunkNum( adadjacent_cv );
                    }
                    adjacent_chunks[i]->GenerateVertex(
                        adadjacent_chunks[0],
                        adadjacent_chunks[1],
                        adadjacent_chunks[2],
                        adadjacent_chunks[3]
                    );
                }
            }
            // TODO adjacent_chunks can be not loaded so nullptr,
            // this assume that chunks adjacent to renderable chunk must be already loaded
            // need to make sure that by making GenerateVertex in chunkmanager layer
            ch->GenerateVertex(
                adjacent_chunks[0],
                adjacent_chunks[1],
                adjacent_chunks[2],
                adjacent_chunks[3]
            );

        }
    }
    
    void Init()
    {
        const vox::data::Vector4i cam_pos = vox::core::gamecore::camera.position.ConvertToInts();
        const vox::data::Vector4i cam_chunk_num = vox::gameutils::GetChunkNumByBlockPos( cam_pos );
        const int rad_sq_ren = render_chunk_dist_ * render_chunk_dist_;
        const int rad_sq_upd = update_chunk_dist_ * update_chunk_dist_;

        for (int dcz = -update_chunk_dist_; dcz <= update_chunk_dist_; ++dcz)
            for ( int dcx = -update_chunk_dist_; dcx <= update_chunk_dist_; ++dcx )
            {
                const int dist_sq = dcx * dcx + dcz * dcz;
                if ( dist_sq <= rad_sq_upd )
                {
                    const vox::data::Vector4i dcv{ dcx, 0, dcz, 0 };
                    const vox::data::Vector4i cv = cam_chunk_num + dcv;
                    auto& chunk = GetChunkByChunkNum( cv );
                    chunk = new vox::data::Chunk{ cv };
                    game_chunks_render_list_.push_back( &chunk );
                }
            }

        for (int dcz = -render_chunk_dist_; dcz <= render_chunk_dist_; ++dcz)
            for ( int dcx = -render_chunk_dist_; dcx <= render_chunk_dist_; ++dcx )
            {
                const int dist_sq = dcx * dcx + dcz * dcz;
                if ( dist_sq <= rad_sq_ren )
                {
                    const vox::data::Vector4i dcv{ dcx, 0, dcz, 0 };
                    const vox::data::Vector4i cv = cam_chunk_num + dcv;
                    auto& chunk = GetChunkByChunkNum( cv );

                    vox::data::Chunk* adjacent_chunks[4];
                    for ( int i = 0; i < 4; ++i )
                    {
                        const vox::data::Vector4i adjacent_cv = cv + vox::data::DIRECTION4_V4I[i];
                        adjacent_chunks[i] = GetChunkByChunkNum( adjacent_cv );
                    }
                    chunk->GenerateVertex(
                        adjacent_chunks[0],
                        adjacent_chunks[1],
                        adjacent_chunks[2],
                        adjacent_chunks[3]
                    );
                }
            }
    }

    void Clean()
    {
        for ( int i = 0; i < GAME_CHUNKS_ARR_SZ; ++i )
        {
            // need to wait for chunk loading thread for EnumChunkStates::LOADING state
            auto& chunk = game_chunks_arr_[i];
            if ( chunk != nullptr )
            {
                delete chunk;
                chunk = nullptr;
            }
        }
        render_chunk_dist_ = 0;
        game_chunks_render_list_.clear();
    }

    int GetRenderChunkDist()
    {
        return render_chunk_dist_;
    }

    void SetRenderChunkDist( int render_chunk_dist )
    {

    }
    
    void Render()
    {
        const vox::data::Vector4i cam_pos{ vox::core::gamecore::camera.position.ConvertToInts() };
        const vox::data::Vector4i cam_chunk_num{ vox::gameutils::GetChunkNumByBlockPos( cam_pos ) };
        const int rad_sq_ren = render_chunk_dist_ * render_chunk_dist_;
        const int rad_sq_upd = update_chunk_dist_ * update_chunk_dist_;

        for (int dcz = -update_chunk_dist_; dcz <= update_chunk_dist_; ++dcz)
            for ( int dcx = -update_chunk_dist_; dcx <= update_chunk_dist_; ++dcx )
            {
                const int dist_sq = dcx * dcx + dcz * dcz;
                if ( dist_sq <= rad_sq_upd )
                {
                    const vox::data::Vector4i dcv{ dcx, 0, dcz, 0 };
                    const vox::data::Vector4i cv{ cam_chunk_num + dcv };
                    auto& chunk = GetChunkByChunkNum( cv );

                    if ( chunk == nullptr )
                    {
                        chunk = new vox::data::Chunk{ cv };
                        game_chunks_render_list_.push_back( &chunk );
                    }
                    if ( chunk->GetChunkPos() == cv && dist_sq <= rad_sq_ren )
                    {
                        if ( !chunk->IsRenderable() )
                        {
                            vox::data::Chunk* adjacent_chunks[4];
                            for ( int i = 0; i < 4; ++i )
                            {
                                auto adjacent_cv = cv + vox::data::DIRECTION4_V4I[i];
                                vox::data::Chunk*& adjacent_chunk = GetChunkByChunkNum( adjacent_cv );
                                if ( adjacent_chunk == nullptr )
                                {
                                    adjacent_chunk = new vox::data::Chunk{ adjacent_cv };
                                }
                                else if ( adjacent_chunk->GetChunkPos() != adjacent_cv )
                                {
                                    delete adjacent_chunk;
                                    adjacent_chunk = new vox::data::Chunk{ adjacent_cv };
                                }
                                adjacent_chunks[i] = adjacent_chunk;
                            }
                            chunk->GenerateVertex(
                                adjacent_chunks[0],
                                adjacent_chunks[1],
                                adjacent_chunks[2],
                                adjacent_chunks[3]
                            );
                        }
                        if ( chunk->IsRenderable() )
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

                            chunk->Render( (vox::data::EnumBitSide6)sides );
                        }
                    }
                }  // if dist_sq <= rad_sq_upd
            }  // foreach chunks
        for ( auto it = game_chunks_render_list_.begin(); it != game_chunks_render_list_.end(); )
        {
            auto& chunk = **it;
            const vox::data::Vector4i chunk_pos{ chunk->GetChunkPos() };
            const vox::data::Vector4i dcv{ chunk_pos - cam_chunk_num };
            int dci[4];
            dcv.ToInt4( dci );
            const int dist_sq = dci[0] * dci[0] + dci[3] * dci[3];

            if ( dist_sq > rad_sq_upd )
            {
                it = game_chunks_render_list_.erase( it );
                delete chunk;
                chunk = nullptr;
            }
            else
            {
                ++it;
            }
        }
    }  // void Render();

}  // namespace
