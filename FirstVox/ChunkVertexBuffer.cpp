#include "ChunkVertexBuffer.h"

#include <vector>

#include "Macros.h"
#include "Consts.h"
#include "Logger.h"
#include "VertexRenderer.h"

namespace vox::data
{
    ChunkVertexBuffer::ChunkVertexBuffer() :
        vertex_buffer_{
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}}, }
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vox::ren::vertex::CreateVertexBuffer(
                &(this->vertex_buffer_[i].front().vertex_buffer),
                vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
            );
        }
    }

    ChunkVertexBuffer::~ChunkVertexBuffer()
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            const int unit_count = (int)this->vertex_buffer_[i].size();
            for ( int j = 0; j < unit_count; ++j )
            {
                vox::ren::vertex::ReleaseVertexBuffer( this->vertex_buffer_[i][j].vertex_buffer );
            }
        }
    }

    void ChunkVertexBuffer::GenerateVertex(
        vox::data::Vector4i cv,
        vox::data::Block d[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
        vox::data::Block( *d_front )[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
        vox::data::Block( *d_back )[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
        vox::data::Block( *d_right )[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
        vox::data::Block( *d_left )[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X]
    )
    {
        static std::vector<vox::ren::vertex::VertexChunk> vertex_temp[(size_t)vox::data::EnumSide::MAX_COUNT];

        for ( int iy = 0; iy < vox::consts::CHUNK_Y; ++iy )
            for ( int iz = 0; iz < vox::consts::CHUNK_Z; ++iz )
                for ( int ix = 0; ix < vox::consts::CHUNK_X; ++ix )
                    if ( auto id = d[iy][iz][ix].id; id != EBlockID::AIR )
                    {
                        auto& tp = vox::data::GetTexturePos( id );
                        const float tpx = tp.x;
                        const float tpz = tp.z;

                        if ( vox::data::IsFullBlock( id ) )
                        {
                            if ( iy == vox::consts::CHUNK_Y - 1 || !vox::data::IsFullBlock( d[iy + 1][iz][ix].id ) )
                            {
                                for ( int i = 0; i < 6; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[(int)vox::data::EnumSide::UP].push_back( vc );
                                }
                            }
                            if ( iy == 0 || !vox::data::IsFullBlock( d[iy - 1][iz][ix].id ) )
                            {
                                for ( int i = 6; i < 12; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[(int)vox::data::EnumSide::DOWN].push_back( vc );
                                }
                            }
                            if ( iz == vox::consts::CHUNK_Z - 1 )
                            {
                                if ( !vox::data::IsFullBlock((*d_front)[iy][0][ix].id))
                                    for ( int i = 12; i < 18; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_temp[(int)vox::data::EnumSide::FRONT].push_back( vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( d[iy][iz + 1][ix].id ) )
                            {
                                for ( int i = 12; i < 18; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[(int)vox::data::EnumSide::FRONT].push_back( vc );
                                }
                            }
                            if ( iz == 0 )
                            {
                                if ( !vox::data::IsFullBlock((*d_back)[iy][vox::consts::CHUNK_Z - 1][ix].id) )
                                    for ( int i = 18; i < 24; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_temp[(int)vox::data::EnumSide::BACK].push_back( vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( d[iy][iz - 1][ix].id ) )
                            {
                                for ( int i = 18; i < 24; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[(int)vox::data::EnumSide::BACK].push_back( vc );
                                }
                            }
                            if ( ix == vox::consts::CHUNK_X - 1 )
                            {
                                if ( !vox::data::IsFullBlock((*d_right)[iy][iz][0].id) )
                                    for ( int i = 24; i < 30; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_temp[(int)vox::data::EnumSide::RIGHT].push_back( vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( d[iy][iz][ix + 1].id ) )
                            {
                                for ( int i = 24; i < 30; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[(int)vox::data::EnumSide::RIGHT].push_back( vc );
                                }
                            }
                            if ( ix == 0 )
                            {
                                if ( !vox::data::IsFullBlock((*d_left)[iy][iz][vox::consts::CHUNK_X - 1].id) )
                                    for ( int i = 30; i < 36; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_temp[(int)vox::data::EnumSide::LEFT].push_back( vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( d[iy][iz][ix - 1].id ) )
                            {
                                for ( int i = 30; i < 36; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[(int)vox::data::EnumSide::LEFT].push_back( vc );
                                }
                            }
                        }
                        else
                        {
                            // generate all side
                            for ( int i = 0; i < 6; ++i )
                                for ( int j = 0; j < 6; ++j )
                                {
                                    vox::ren::vertex::VertexChunk vc{ vox::ren::vertex::VERTICES_BLOCK[i * 6 + j] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_temp[i].push_back(vc);
                                }
                        }
                    }

        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            const int vertices_count = (int)vertex_temp[i].size();
            const int unit_count = (int)(
                (unsigned)(vertices_count + vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE - 1)
                / (unsigned)vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
            );
            const int cur_unit_count = (int)this->vertex_buffer_[i].size();

            for ( int j = cur_unit_count; j < unit_count; ++j )
            {
                this->vertex_buffer_[i].push_back( vox::data::ChunkVertexUnit{ nullptr, 0 } );
                vox::ren::vertex::CreateVertexBuffer(
                    &(this->vertex_buffer_[i].back().vertex_buffer),
                    vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
                );
            }

            for ( int j = 0; j < unit_count - 1; ++j )
            {
                vox::ren::vertex::MapVertex(
                    this->vertex_buffer_[i][j].vertex_buffer,
                    &vertex_temp[i][j * vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE],
                    vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
                );
                this->vertex_buffer_[i][j].size = vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE;
            }
            const int other_vertices_cnt = (unit_count - 1) * vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE;
            const int remain_vertices_cnt = vertices_count - other_vertices_cnt;
            if ( unit_count > 0 )
            {
                vox::ren::vertex::MapVertex(
                    this->vertex_buffer_[i][unit_count - 1].vertex_buffer,
                    &vertex_temp[i][other_vertices_cnt],
                    remain_vertices_cnt
                );
                this->vertex_buffer_[i][unit_count - 1].size = remain_vertices_cnt;
            }

            const int new_unit_count = (int)this->vertex_buffer_[i].size();
            for ( int j = unit_count; j < new_unit_count; ++j )
            {
                this->vertex_buffer_[i][j].size = 0;
            }
            vertex_temp[i].clear();
        }
    }

    void ChunkVertexBuffer::Render( vox::data::Vector4i cv, vox::data::EnumBitSide6 sides ) const
    {
        vox::data::Vector4f cvf = vox::data::vector::ConvertToVector4f( cv );
        vox::data::Vector4f multiplier = vox::data::vector::Set(
            (float)vox::consts::CHUNK_X,
            (float)vox::consts::CHUNK_Y,
            (float)vox::consts::CHUNK_Z,
            0.0f
        );
        alignas(16) float cvfs[4];
        vox::data::vector::Store( cvfs, vox::data::vector::Mul( cvf, multiplier ) );

        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
            if ( (int)sides & (1 << i) )
            {
                const int unit_count = (int)this->vertex_buffer_[i].size();
                for ( int j = 0; j < unit_count && this->vertex_buffer_[i][j].size; ++j )
                {
                    vox::ren::vertex::RenderChunk(
                        cvfs[0], cvfs[1], cvfs[2],
                        &this->vertex_buffer_[i][j].vertex_buffer,
                        this->vertex_buffer_[i][j].size
                    );
                }
            }
    }
}