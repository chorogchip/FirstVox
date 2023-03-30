#include "Chunk.h"

#include <cstdio>

#include "FirstVoxHeader.h"
#include "Perlin.h"
#include "Logger.h"
#include "ChunkManager.h"
#include "VertexRenderer.h"
#include "VertexRenderer.h"

namespace vox::data
{
#define VERTEX_SPEC { sizeof( vox::ren::vertex::VertexChunk ), alignof( vox::ren::vertex::VertexChunk ) }

    Chunk::Chunk( vox::data::Vector4i cv ) :
        cv_( cv ), d_id_ {}, d_data_ {}, d_light_ {},
        vertex_buffer_temp_ { VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC },
        vertex_buffer_ {}, is_changed_( false )
    {}

    void Chunk::ConstructForReuse( vox::data::Vector4i cv )
    {
        vox::data::vector::Storeu( (int*)&cv_, cv );
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_temp_[i].clear();
        }
        is_changed_ = false;
    }

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

        memset( d_data_, 0, sizeof( d_data_ ) );
        memset( d_id_, 0, sizeof( d_id_ ) );

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

    void Chunk::Clear( bool to_retrieve_buffer )
    {
        if ( to_retrieve_buffer )
            vertex_buffer_.RetrieveBuffer();

        // start saving

        if ( !this->is_changed_ )
            return;

        char file_name[256];
        sprintf_s( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d.chnk", this->cv_.m128i_i32[0], this->cv_.m128i_i32[1], this->cv_.m128i_i32[2]);
        FILE *fp;
        fopen_s( &fp, file_name, "wb" );

        if ( fp == nullptr )
            return;

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

    void Chunk::GenerateVertex( Chunk* adj_chks[8] )
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_temp_[i].clear();
        }

        memset( d_light_, 0, sizeof( d_light_ ) );


        // mesh start

        for ( int iy = 0; iy < vox::consts::CHUNK_Y; ++iy )
            for ( int iz = 0; iz < vox::consts::CHUNK_Z; ++iz )
                for ( int ix = 0; ix < vox::consts::CHUNK_X; ++ix )
                {
                    const auto id = this->GetBlockId( ix, iy, iz );
                    if ( id != EBlockID::AIR )
                    {
                        unsigned adj_blocks_is_full = 0U;
                        unsigned adji = 0U;
                        for ( int dy = -1; dy <= 1; ++dy )
                            for ( int dz = 1; dz >= -1; --dz )
                                for ( int dx = -1; dx <= 1; ++dx )
                                {
                                    const int iiy = iy + dy;
                                    const int iiz = iz + dz;
                                    const int iix = ix + dx;

                                    unsigned is_full_dblock;
                                    if ( iiy < 0 || iiy >= vox::consts::CHUNK_Y )
                                    {
                                        is_full_dblock = 0U;
                                    }
                                    else if ( iiz >= 0 && iiz < vox::consts::CHUNK_Z && iix >= 0 && iix < vox::consts::CHUNK_X )
                                    {
                                        is_full_dblock = vox::data::IsFullBlock( this->GetBlockId( iix, iiy, iiz ) );
                                    }
                                    else
                                    {
                                        static_assert(vox::consts::CHUNK_Y == vox::consts::MAP_Y);
                                        const auto new_cv = vox::data::vector::Add( cv_,
                                            vox::data::vector::Set( iix >> vox::consts::CHUNK_X_LOG2, 0, iiz >> vox::consts::CHUNK_Z_LOG2, 0 )
                                        );
                                        for (int i = 0; i < 8; ++i )
                                            if ( vox::data::vector::Equal( new_cv, adj_chks[i]->cv_ ) )
                                            {
                                                is_full_dblock = vox::data::IsFullBlock(
                                                    adj_chks[i]->GetBlockId(
                                                        iix & (vox::consts::CHUNK_X - 1), iiy, iiz & (vox::consts::CHUNK_Z - 1)
                                                    ) );
                                                break;
                                            }
                                    }
                                    adj_blocks_is_full |= is_full_dblock << adji;
                                    ++adji;
                                }  // for y z x [-1, 1]

                        const unsigned tp = vox::data::GetTexturePos( id );
                        const unsigned dpos = ix << 24U | iy << 8U | iz << 0U;

                        if ( adj_blocks_is_full >> 13 & 0x1 )
                        {
                            static constexpr unsigned adj_ind_bits = 0b00'01100'01110'10000'01010'00100'10110U;
#define U4(x1, x2, x3, x4) (x1 | x2 << 8U | x3 << 16U | x4 << 24U)
                            static constexpr unsigned adjl_ind_bits[6] = { U4(19,21,25,23), U4( 7, 3, 1, 5), U4(19,11, 1, 9), U4(25,15, 7,17), U4(23,17, 5,11), U4(21, 9, 3,15) };
                            static constexpr unsigned adjm_ind_bits[6] = { U4(20,18,24,26), U4( 8, 6, 0, 2), U4(18,20, 2, 0), U4(26,24, 6, 8), U4(20,26, 8, 2), U4(24,18, 0, 6) };
                            static constexpr unsigned adjr_ind_bits[6] = { U4(23,19,21,25), U4( 5, 7, 3, 1), U4( 9,19,11, 1), U4(17,25,15, 7), U4(11,23,17, 5), U4(15,21, 9, 3) };
#undef U4
                            for ( unsigned side = 0; side < 6; ++side )
                            {
                                if ( adj_blocks_is_full >> ((adj_ind_bits >> side * 5U) & 0b11111) & 0x1 )
                                    continue;

                                unsigned vertex_ambient_level_bits = 0U;

                                unsigned darkest_index = 0U;
                                unsigned darkest_num = 0U;
                                unsigned darkest_level = 3U;

                                unsigned brightest_index = 0U;
                                //unsigned brightest_num = 0U;
                                unsigned brightest_level = 0U;

                                for ( unsigned vtx = 0U; vtx < 4U; ++vtx )
                                {
                                    const unsigned adjl = adj_blocks_is_full >> (adjl_ind_bits[side] >> (vtx << 3U) & 0xff) & 0x1;
                                    const unsigned adjm = adj_blocks_is_full >> (adjm_ind_bits[side] >> (vtx << 3U) & 0xff) & 0x1;
                                    const unsigned adjr = adj_blocks_is_full >> (adjr_ind_bits[side] >> (vtx << 3U) & 0xff) & 0x1;

                                    unsigned ambient_level = 0U;
                                    if ( !adjl || !adjr )
                                        ambient_level = 3 - adjl - adjm - adjr;

                                    vertex_ambient_level_bits |= ambient_level << (vtx << 1);

                                    if ( ambient_level < darkest_level )
                                        darkest_index = vtx,
                                        darkest_num = 1U,
                                        darkest_level = ambient_level;
                                    else if ( ambient_level == darkest_level )
                                        ++darkest_num;
                                    if ( ambient_level > brightest_level )  // not else for vtx == 0
                                        brightest_index = vtx,
                                        brightest_level = ambient_level;
                                }

                                unsigned vertex_select_index = 0x230120U;
                                
                                if ( darkest_num == 1 )  // this branch can be substituted with bit ops like below
                                    vertex_select_index ^= (darkest_index & 0x1) - 1 & (0x231130U ^ 0x230120U);
                                else
                                    vertex_select_index ^= (brightest_index & 0x1) - 1 & (0x231130U ^ 0x230120U);

                                for ( unsigned vtx = 0U; vtx < 6U; ++vtx )
                                {
                                    const unsigned ind = (vertex_select_index >> vtx * 4U) & 0xf;
                                    vox::ren::vertex::VertexChunk vc =
                                        vox::ren::vertex::VERTICES_BLOCK[side * 4U + ind];
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= (vertex_ambient_level_bits >> (ind * 2U)) & 0b11;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::UP].push_back( &vc );
                                }
                            }

                            /*
                            static constexpr signed char dir8_d[] = { 1, 1, 1, 0, -1, -1, -1, 0, 1, 1 };

                            if ( iy == vox::consts::CHUNK_Y - 1 )
                            {
                                for ( int i = 0; i < 6; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::UP].push_back( &vc );
                                }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix, iy + 1, iz ) ) )
                            {
                                static constexpr unsigned sel_left = 0x7531;
                                static constexpr unsigned sel_mid = 0x6420;
                                static constexpr unsigned sel_right = 0x5317;

                                unsigned adj_is_full = 0U;
                                for ( unsigned i = 0; i < 8; ++i )
                                {
                                    vox::data::EBlockID id;

                                    int iix = ix + dir8_d[i + 2];
                                    int iiz = iz + dir8_d[i];
                                    Chunk* ch_to_get = this;
                                    if ( iix < 0 && iiz < 0 )
                                        iix += vox::consts::CHUNK_X,
                                        iiz += vox::consts::CHUNK_Z,
                                        ch_to_get = adj_chks[4];
                                    else if ( iix < 0 && iiz == vox::consts::CHUNK_Z )
                                        iix += vox::consts::CHUNK_X,
                                        iiz -= vox::consts::CHUNK_Z,
                                        ch_to_get = adj_chks[2];
                                    else if ( iix == vox::consts::CHUNK_X && iiz < 0 )
                                        iix -= vox::consts::CHUNK_X,
                                        iiz += vox::consts::CHUNK_Z,
                                        ch_to_get = adj_chks[6];
                                    else if ( iix == vox::consts::CHUNK_X && iiz == vox::consts::CHUNK_Z )
                                        iix -= vox::consts::CHUNK_X,
                                        iiz -= vox::consts::CHUNK_Z,
                                        ch_to_get = adj_chks[0];
                                    else if ( iix < 0 )
                                        iix += vox::consts::CHUNK_X,
                                        ch_to_get = adj_chks[3];
                                    else if ( iix == vox::consts::CHUNK_X )
                                        iix -= vox::consts::CHUNK_X,
                                        ch_to_get = adj_chks[7];
                                    else if ( iiz < 0 )
                                        iiz += vox::consts::CHUNK_Z,
                                        ch_to_get = adj_chks[5];
                                    else if ( iiz == vox::consts::CHUNK_Z )
                                        iiz -= vox::consts::CHUNK_Z,
                                        ch_to_get = adj_chks[1];

                                    id = ch_to_get->GetBlockId( iix, iy + 1, iiz );

                                    adj_is_full |= vox::data::IsFullBlock( id ) << i;
                                }
                                unsigned vertex_ambient_level = 0U;
                                int darkest_index = 0;
                                int brightest_index = 0;
                                int darkest_num = 0;
                                int brightest_num = 0;
                                int darkest_level = 3;
                                int brightest_level = 0;

                                for ( int i = 0; i < 4; ++i )
                                {
                                    const unsigned is_left_full = (adj_is_full >> ((sel_left >> (i << 2)) & 0xf)) & 1;
                                    const unsigned is_mid_full = (adj_is_full >> ((sel_mid >> (i << 2)) & 0xf)) & 1;
                                    const unsigned is_right_full = (adj_is_full >> ((sel_right >> (i << 2)) & 0xf)) & 1;

                                    int ambient_level = 0;
                                    if ( !is_left_full || !is_right_full )
                                        ambient_level = 3 - is_left_full - is_mid_full - is_right_full;

                                    vertex_ambient_level |= ambient_level << (i << 1);

                                    if ( ambient_level < darkest_level )
                                        darkest_index = i, darkest_num = 1, darkest_level = ambient_level;
                                    else if ( ambient_level == darkest_level )
                                        ++darkest_num;
                                    if ( ambient_level > brightest_level )
                                        brightest_index = i, brightest_num = 1, brightest_level = ambient_level;
                                    else if ( ambient_level == brightest_level )
                                        ++brightest_num;
                                }

                                int triangle_div_way = 0;
                                if ( darkest_num == 1 )
                                    triangle_div_way = darkest_index & 1;
                                else
                                    triangle_div_way = brightest_index & 1;
                                
                                unsigned vertex_select_index = 0x231130U;
                                if ( triangle_div_way )
                                    vertex_select_index = 0x230120U;

                                for (int i = 0; i < 6; ++i)
                                {
                                    const unsigned ind = (vertex_select_index >> (i << 2)) & 0xf;
                                    vox::ren::vertex::VertexChunk vc =
                                        vox::ren::vertex::VERTICES_BLOCK[ind];
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= (vertex_ambient_level >> (ind << 1)) & 0b11;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::UP].push_back( &vc );
                                }
                            }
                            if ( iy == 0 || !vox::data::IsFullBlock( this->GetBlockId( ix, iy - 1, iz ) ) )
                            {
                                for ( int i = 6; i < 12; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::DOWN].push_back( &vc );
                                }
                            }
                            if ( iz == vox::consts::CHUNK_Z - 1 )
                            {
                                if ( !vox::data::IsFullBlock( adj_chks[1]->GetBlockId(ix, iy, 0)) )
                                    for ( int i = 12; i < 18; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.position += dpos;
                                        vc.texcoord += tp;
                                        vc.light |= 3;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::FRONT].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix, iy, iz + 1 ) ) )
                            {
                                for ( int i = 12; i < 18; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::FRONT].push_back( &vc );
                                }
                            }
                            if ( iz == 0 )
                            {
                                if ( !vox::data::IsFullBlock( adj_chks[5]->GetBlockId(ix, iy, vox::consts::CHUNK_Z - 1)) )
                                    for ( int i = 18; i < 24; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.position += dpos;
                                        vc.texcoord += tp;
                                        vc.light |= 3;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::BACK].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix, iy, iz - 1 ) ) )
                            {
                                for ( int i = 18; i < 24; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::BACK].push_back( &vc );
                                }
                            }
                            if ( ix == vox::consts::CHUNK_X - 1 )
                            {
                                if ( !vox::data::IsFullBlock( adj_chks[7]->GetBlockId(0, iy, iz)) )
                                    for ( int i = 24; i < 30; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.position += dpos;
                                        vc.texcoord += tp;
                                        vc.light |= 3;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::RIGHT].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix + 1, iy, iz ) ) )
                            {
                                for ( int i = 24; i < 30; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::RIGHT].push_back( &vc );
                                }
                            }
                            if ( ix == 0 )
                            {
                                if ( !vox::data::IsFullBlock( adj_chks[3]->GetBlockId(vox::consts::CHUNK_X - 1, iy, iz)) )
                                    for ( int i = 30; i < 36; ++i )
                                    {
                                        vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                        vc.position += dpos;
                                        vc.texcoord += tp;
                                        vc.light |= 3;
                                        vertex_buffer_temp_[(int)vox::data::EnumSide::LEFT].push_back( &vc );
                                    }
                            }
                            else if ( !vox::data::IsFullBlock( this->GetBlockId( ix - 1, iy, iz ) ) )
                            {
                                for ( int i = 30; i < 36; ++i )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[(int)vox::data::EnumSide::LEFT].push_back( &vc );
                                }
                            }*/
                        }  // if full block
                        else
                        {
                            // generate all side
                            for ( int i = 0; i < 6; ++i )
                                for ( int j = 0; j < 6; ++j )
                                {
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i * 4 + j] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[i].push_back( &vc );
                                }
                        }  // if not full block
                    }  // if not AIR
                }  // for y z x
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