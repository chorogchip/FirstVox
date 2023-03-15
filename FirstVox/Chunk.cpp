#include "Chunk.h"

#include <cstdio>

#include "FirstVoxHeader.h"
#include "Perlin.h"
#include "Logger.h"
#include "VertexRenderer.h"
#include "VertexRenderer.h"


namespace vox::data
{
#define VERTEX_SPEC { sizeof( vox::ren::vertex::VertexChunk ), alignof( vox::ren::vertex::VertexChunk ) }

    Chunk::Chunk( vox::data::Vector4i cv ) :
        cv_( cv ), d_id_ {}, d_data_ {},
        vertex_buffer_temp_ { VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC },
        vertex_buffer_ {}, is_changed_( false )
    {}

    void Chunk::Load()
    {
        char file_name[256];
        sprintf_s( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d.chnk", this->cv_.m128i_i32[0], this->cv_.m128i_i32[1], this->cv_.m128i_i32[2]);
        FILE *fp;
        fopen_s( &fp, file_name, "rb" );
        if ( fp != nullptr )
        {
            unsigned version;
            fread( &version, sizeof( version ), 1, fp );
            if ( version <= 0xff'ff'ff'ff )
            {
                static_assert( sizeof( EBlockID ) == sizeof ( unsigned short ));

                int sz;
                fread( &sz, sizeof( sz ), 1, fp );
                unsigned short *const data = new unsigned short[sz];
                const unsigned short *p = data;
                fread( data, sizeof( unsigned short ), sz, fp );
                do
                {
                    unsigned short rd1 = *p++;
                    if ( rd1 == (unsigned short)-1 ) break;
                    unsigned short rd2 = *p++;
                    unsigned short rd3 = *p++;
                    this->SetBlock( (rd1 & 0xff00) >> 8, vox::consts::CHUNK_Y - 1, rd1 & 0xff,
                        vox::data::Block( (vox::data::EBlockID)rd2, rd3 ) );
                } while ( true );
                for ( int y = vox::consts::CHUNK_Y - 2; y >= 0; --y )
                {
                    for (int z = 0; z < vox::consts::CHUNK_Z; ++z)
                        for (int x = 0; x < vox::consts::CHUNK_X; ++x)
                        {
                            this->SetBlock( x, y, z, this->GetBlock( x, y + 1, z ) );
                        }
                    do
                    {
                        unsigned short rd1 = *p++;
                        if ( rd1 == (unsigned short)-1 ) break;
                        unsigned short rd2 = *p++;
                        unsigned short rd3 = *p++;
                        this->SetBlock( (rd1 & 0xff00) >> 8, y, rd1 & 0xff,
                                       vox::data::Block( (vox::data::EBlockID)rd2, rd3 ) );
                    } while ( true );
                }
                delete[] data;
            }
            fclose( fp );
            return;
        }

        static constexpr float multipliers_stone[3] = { 0.5f, 0.25f, 0.125f };
        vox::rand::perlin::PerlinGeneratorUnit perlin_stone{
            this->cv_.m128i_i32[0], this->cv_.m128i_i32[2], sizeof(multipliers_stone) / sizeof(float), multipliers_stone
        };
        static constexpr float multipliers_dirt[3] = { 0.375f, 0.25f, 0.125f };
        vox::rand::perlin::PerlinGeneratorUnit perlin_dirt{
            this->cv_.m128i_i32[0], this->cv_.m128i_i32[2], sizeof(multipliers_dirt) / sizeof(float), multipliers_dirt,
            0x12345678
        };

        float cxr = 1.0f / (float)vox::consts::CHUNK_X;
        float czr = 1.0f / (float)vox::consts::CHUNK_Z;

        for (int  iz = 0; iz < vox::consts::CHUNK_Z; ++iz)
            for ( int ix = 0; ix < vox::consts::CHUNK_X; ++ix )
            {
                const float sample_stone = perlin_stone.Sample(
                    (float)ix * cxr, (float)iz * czr
                );
                const float sample_dirt = perlin_dirt.Sample(
                    (float)ix * cxr, (float)iz * czr
                );

                const int rand_height_stone = 30 + (int)((float)vox::consts::MAP_Y * sample_stone * 0.1f);
                const int height_stone = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_stone ) );
                const int rand_height_dirt = rand_height_stone + 2 + (int)((float)vox::consts::MAP_Y * sample_dirt * 0.1f);
                const int height_dirt = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_dirt ) );

                int iy = 0;
                for ( ; iy < rand_height_stone; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::COBBLESTONE ) );
                }
                for ( ; iy < rand_height_dirt; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::DIRT ) );
                }

            }
    }

    void Chunk::Clear()
    {
        // start saving

        if ( !this->is_changed_ ) return;
        char file_name[256];
        sprintf_s( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d.chnk", this->cv_.m128i_i32[0], this->cv_.m128i_i32[1], this->cv_.m128i_i32[2]);
        FILE *fp;
        fopen_s( &fp, file_name, "wb" );
        if ( fp == nullptr )
        {
            return;
        }
        fwrite( &vox::consts::GAME_VERSION, sizeof( vox::consts::GAME_VERSION ), 1, fp );

        std::vector<unsigned short> out;
        for (int z = 0; z < vox::consts::CHUNK_Z; ++z)
            for ( int x = 0; x < vox::consts::CHUNK_X; ++x )
            {
                const Block block = this->GetBlock( x, vox::consts::CHUNK_Y - 1, z );
                if ( block.id != vox::data::EBlockID::AIR || block.data != 0 )
                {
                    out.push_back( z | x << 8 );
                    out.push_back( (unsigned short)block.id );
                    out.push_back( block.data );
                }
            }
        out.push_back( -1 );
        for ( int y = vox::consts::CHUNK_Y - 2; y >= 0; --y )
        {
            for (int z = 0; z < vox::consts::CHUNK_Z; ++z)
                for ( int x = 0; x < vox::consts::CHUNK_X; ++x )
                {
                    const Block block = this->GetBlock( x, y, z );
                    const Block block_up = this->GetBlock( x, y + 1, z );
                    if ( block.id != block_up.id || block.data != block_up.data )
                    {
                        out.push_back( z | x << 8 );
                        out.push_back( (unsigned short)block.id );
                        out.push_back( block.data );
                    }
                }
            out.push_back( -1 );
        }
        int sz = (int)out.size();
        fwrite( &sz, sizeof( sz ), 1, fp );
        fwrite( &out[0], sizeof( unsigned short ), out.size(), fp );
        fclose( fp );
    }

    void Chunk::GenerateVertex( Chunk* front, Chunk* back, Chunk* right, Chunk* left )
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_temp_[i].clear();
        }

        for ( int iy = 0; iy < vox::consts::CHUNK_Y; ++iy )
        {
            for ( int iz = 0; iz < vox::consts::CHUNK_Z; ++iz )
            {
                for ( int ix = 0; ix < vox::consts::CHUNK_X; ++ix )
                {
                    if ( auto id = this->GetBlockId( ix, iy, iz ); id != EBlockID::AIR )
                    {
                        auto& tp = vox::data::GetTexturePos( id );
                        const float tpx = tp.x;
                        const float tpz = tp.z;

                        if ( vox::data::IsFullBlock( id ) )
                        {
                            if ( iy == vox::consts::CHUNK_Y - 1 || !vox::data::IsFullBlock( this->GetBlockId( ix, iy + 1, iz ) ) )
                            {
                                for ( int i = 0; i < 6; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::UP].push_back( &vc );
                                }
                            }
                            if ( iy == 0 || !vox::data::IsFullBlock( this->GetBlockId( ix, iy - 1, iz ) ) )
                            {
                                for ( int i = 6; i < 12; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::DOWN].push_back( &vc );
                                }
                            }
                            if ( iz == vox::consts::CHUNK_Z - 1 )
                            {
                                if ( !vox::data::IsFullBlock( front->GetBlockId( ix, iy, 0 ) ) )
                                    for ( int i = 12; i < 18; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::FRONT].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix, iy, iz + 1 ) ) )
                            {
                                for ( int i = 12; i < 18; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::FRONT].push_back( &vc );
                                }
                            }
                            if ( iz == 0 )
                            {
                                if ( !vox::data::IsFullBlock( back->GetBlockId( ix, iy, vox::consts::CHUNK_Z - 1 ) ) )
                                    for ( int i = 18; i < 24; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::BACK].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix, iy, iz - 1 ) ) )
                            {
                                for ( int i = 18; i < 24; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::BACK].push_back( &vc );
                                }
                            }
                            if ( ix == vox::consts::CHUNK_X - 1 )
                            {
                                if ( !vox::data::IsFullBlock( right->GetBlockId( 0, iy, iz ) ) )
                                    for ( int i = 24; i < 30; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::RIGHT].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix + 1, iy, iz ) ) )
                            {
                                for ( int i = 24; i < 30; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::RIGHT].push_back( &vc );
                                }
                            }
                            if ( ix == 0 )
                            {
                                if ( !vox::data::IsFullBlock( left->GetBlockId( vox::consts::CHUNK_X - 1, iy, iz ) ) )
                                    for ( int i = 30; i < 36; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.Pos.x += (float)ix;
                                        vc.Pos.y += (float)iy;
                                        vc.Pos.z += (float)iz;
                                        vc.Tex.x += tpx;
                                        vc.Tex.y += tpz;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::LEFT].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix - 1, iy, iz ) ) )
                            {
                                for ( int i = 30; i < 36; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::LEFT].push_back( &vc );
                                }
                            }
                        }  // if ( vox::data::IsFullBlock( id ) )
                        else
                        {
                            // generate all side
                            for ( int i = 0; i < 6; ++i )
                                for ( int j = 0; j < 6; ++j )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i * 6 + j] };
                                    vc.Pos.x += (float)ix;
                                    vc.Pos.y += (float)iy;
                                    vc.Pos.z += (float)iz;
                                    vc.Tex.x += tpx;
                                    vc.Tex.y += tpz;
                                    vertex_buffer_temp_[i].push_back( &vc );
                                }
                        }
                    }

                }  // for x
            }  // for z
        }  // for y

        this->MapTempVertexToBuffer();
    }

    void Chunk::MapTempVertexToBuffer()
    {
        this->vertex_buffer_.MapData( vertex_buffer_temp_ );
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_temp_[i].clear();
        }
    }

    void Chunk::Render( vox::data::EnumBitSide6 sides )
    {
        this->vertex_buffer_.Render( this->cv_, sides );
    }

    void Chunk::Touch()
    {
        this->is_changed_ = true;
    }
}