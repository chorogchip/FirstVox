#include "Chunk.h"

#include <cstdio>
#include <queue>
#include <cstdlib>

#include "FirstVoxHeader.h"
#include "Perlin.h"
#include "Logger.h"
#include "ChunkManager.h"
#include "VertexRenderer.h"
#include "VertexRenderer.h"
#include "GameCore.h"
#include "GlobalPerlinField.h"
#include "GlobalUniformField.h"
#include "Biones.h"

namespace vox::data
{
#define VERTEX_SPEC { sizeof( vox::ren::vertex::VertexChunk ), alignof( vox::ren::vertex::VertexChunk ) }

    Chunk::Chunk( vox::data::Vector4i cv ) :
        cv_( cv ), d_id_ {}, /*d_data_ {},*/
        vertex_buffer_temp_ { VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC, VERTEX_SPEC },
        vertex_buffer_ {}, is_changed_( false ),
        light_infos_ {}
    {}

    void Chunk::SetBlock( int x, int y, int z, vox::data::Block block )
    {
        const int ind = GetInd( x, y, z );
        this->d_id_[ind] = block.id;
        //this->d_data_[ind] = block.data;
        /*
        if ( block.id != vox::data::EBlockID::AIR )
        {
            max_block_y_ = std::max( max_block_y_, (unsigned char)y );
        }
        
        if ( block.id == vox::data::EBlockID::DIAMOND_ORE )
        {
            const auto lit_type = (vox::data::lightinfos::EnumLightType)(
                (vox::core::gamecore::GetGameTicks() + x + y + z) % 7);
            for ( auto& lin : light_infos_ )
                if ( lin.type == lit_type )
                {
                    lin.instances.push_back(
                        { (unsigned)x << 16U | (unsigned)z << 8U | (unsigned)y, 63 } );
                    goto FIN_INSERT_LIGHT;
                }

            light_infos_.push_back( vox::data::lightinfos::LightTypesInfo {
                { { (unsigned)x << 16U | (unsigned)z << 8U | (unsigned)y, 63 } }, lit_type } );

FIN_INSERT_LIGHT:;
        }*/
    }

    void Chunk::ConstructForReuse( vox::data::Vector4i cv )
    {
        vox::data::vector::Storeu( (int*)&cv_, cv );
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_temp_[i].clear();
        }
        light_infos_.clear();
        is_changed_ = false;
    }

    void Chunk::LoadFromData(void* data, size_t size)
    {
        unsigned version = *(unsigned *)data;
        data = (unsigned *)data + 1;
        if ( version <= 0xff'ff'ff'ff )
        {
            static_assert( sizeof( EBlockID ) == sizeof ( unsigned short ));

            int sz = *(int *)data;
            data = (int *)data + 1;
            const unsigned short *p = (unsigned short *)data;
            do
            {
                unsigned short rd1 = *p++;
                if ( rd1 == (unsigned short)-1 ) break;
                unsigned short rd2 = *p++;
                //unsigned short rd3 = *p++;
                this->SetBlock( (rd1 & 0xff00) >> 8, vox::consts::CHUNK_Y - 1, rd1 & 0xff,
                    vox::data::Block( (vox::data::EBlockID)rd2/*, rd3*/ ) );
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
                    //unsigned short rd3 = *p++;
                    this->SetBlock( (rd1 & 0xff00) >> 8, y, rd1 & 0xff,
                        vox::data::Block( (vox::data::EBlockID)rd2/*, rd3*/ ) );
                } while ( true );
            }
        }
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
                    //unsigned short rd3 = *p++;
                    this->SetBlock( (rd1 & 0xff00) >> 8, vox::consts::CHUNK_Y - 1, rd1 & 0xff,
                        vox::data::Block( (vox::data::EBlockID)rd2/*, rd3*/ ) );
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
                        //unsigned short rd3 = *p++;
                        this->SetBlock( (rd1 & 0xff00) >> 8, y, rd1 & 0xff,
                                       vox::data::Block( (vox::data::EBlockID)rd2/*, rd3*/ ) );
                    } while ( true );
                }
                delete[] data;
            }
            fclose( fp );
            return;
        }

        //memset( d_data_, 0, sizeof( d_data_ ) );
        memset( d_id_, 0, sizeof( d_id_ ) );
        /*
        static constexpr float multipliers_blk_type[5] = { 0.0f, 0.0f, 0.125f, 0.25f, 0.5f };
        vox::rand::perlin::PerlinGeneratorUnit perlin_blk_type{
            this->cv_.m128i_i32[0], this->cv_.m128i_i32[2], sizeof(multipliers_blk_type) / sizeof(float), multipliers_blk_type,
        };

        static constexpr float multipliers_height[3] = { 0.375f/2.0f, 0.25f/2.0f, 0.125/2.0f };
        vox::rand::perlin::PerlinGeneratorUnit perlin_height{
            this->cv_.m128i_i32[0], this->cv_.m128i_i32[2], sizeof(multipliers_height) / sizeof(float), multipliers_height,
            0x12345678
        };
        static constexpr float multipliers_grass[3] = { 0.375f, 0.25f, 0.125f };
        vox::rand::perlin::PerlinGeneratorUnit perlin_grass{
            this->cv_.m128i_i32[0], this->cv_.m128i_i32[2], sizeof(multipliers_grass) / sizeof(float), multipliers_grass,
                0x12345678
        };
        */
        float cxr = 1.0f / (float)vox::consts::CHUNK_X;
        float czr = 1.0f / (float)vox::consts::CHUNK_Z;
        
        const int cpx = data::vector::GetX(cv_) * consts::CHUNK_X;
        const int cpz = data::vector::GetZ(cv_) * consts::CHUNK_Z;


        static rand::GlobalPerlinField gpf_height(123456789U, 1024, 1.0f);
        static rand::GlobalPerlinField gpf_temperature(987654321U, 1024, 1.0f);

        static rand::GlobalUniformField guf_blk_type{13243546U};
        static rand::GlobalUniformField guf_object{928374652U};

        static rand::GlobalPerlinField gpf_world_height1(23456781U, 256, 1.0f);
        static rand::GlobalPerlinField gpf_world_height2(34567812U, 128, 0.75f);
        static rand::GlobalPerlinField gpf_world_height3(45678123U,  64, 0.5f);
        static rand::GlobalPerlinField gpf_world_height4(56781234U,  32, 0.125f);
        static rand::GlobalPerlinField gpf_world_height5(67812345U,  16, 0.0625f);
        static rand::GlobalPerlinField gpf_world_height6(78123456U,   8, 0.03125f);
        static rand::GlobalPerlinField gpf_cover_block(15263748U, 32, 1.0f);

        for (int  iz = 0; iz < vox::consts::CHUNK_Z; ++iz)
            for ( int ix = 0; ix < vox::consts::CHUNK_X; ++ix )
            {
                /*
                const float sample_blk_type = perlin_blk_type.Sample(
                    (float)ix * cxr, (float)iz * czr
                );
                const float sample_height = perlin_height.Sample(
                    (float)ix * cxr, (float)iz * czr
                );
                const float sample_grass = perlin_grass.Sample(
                    (float)ix * cxr, (float)iz * czr
                );*/
#if 0
                const int rand_height_stone = 20 + (int)((float)vox::consts::MAP_Y * sample_stone * 0.4f);
                const int height_stone = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_stone ) );

                const int rand_height_dirt = 30 + (int)((float)vox::consts::MAP_Y * sample_dirt * 0.05f);
                const int height_dirt = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_dirt ) );

                const int rand_height_grass = 1 + height_dirt + (int)((float)vox::consts::MAP_Y * sample_grass * 0.025f);
                const int height_grass = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_grass ) );

                int iy = 0;
                for ( ; iy <= height_dirt; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::DIRT ) );
                }
                for ( ; iy <= height_grass; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::GRASS ) );
                }
                iy = 0;
                for ( ; iy <= height_stone; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::STONE ) );
                }
#elif 0
                const int rand_height_stone = 30 + (int)((float)vox::consts::MAP_Y * sample_stone * 0.025f);
                const int height_stone = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_stone ) );

                const int rand_height_dirt = 10 + (int)((float)vox::consts::MAP_Y * sample_dirt * 0.5f);
                const int height_dirt = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_dirt ) );

                const int rand_height_grass = 1 + height_dirt + (int)((float)vox::consts::MAP_Y * sample_grass * 0.025f);
                const int height_grass = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_grass ) );

                int iy = 0;
                iy = 0;
                for ( ; iy <= height_stone; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::SAND ) );
                }
                iy = 0;
                for ( ; iy <= height_dirt; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::STONE ) );
                }
#elif 0

                const int rand_height_stone = 20 + (int)((float)vox::consts::MAP_Y * sample_stone * 0.4f);
                const int height_stone = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_stone ) );

                const int rand_height_dirt = 30 + (int)((float)vox::consts::MAP_Y * sample_dirt * 0.05f);
                const int height_dirt = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_dirt ) );

                const int rand_height_grass = 1 + height_dirt + (int)((float)vox::consts::MAP_Y * sample_grass * 0.025f);
                const int height_grass = std::max( 1, std::min( vox::consts::MAP_Y, rand_height_grass ) );

                int iy = 0;
                for ( ; iy <= height_dirt; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::DIRT ) );
                }
                for ( ; iy <= height_grass; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( vox::data::EBlockID::GRASS ) );
                }
                if (std::rand() % 1000 == 0)
                {
                    int hei = 5 + std::rand() % 10;
                    for (int iyy = 0; iyy < hei; ++iyy)
                    {
                       this->SetBlock( ix, iy + iyy, iz, Block( vox::data::EBlockID::TREE ) );
                    }
                    int rad = hei/3 + std::rand() % 5;
                    for (int i1 = -rad; i1 <= rad; ++i1)
                        for (int i2 = -rad; i2 <= rad; ++i2)
                            for (int i3 = -rad; i3 <= rad; ++i3)
                                if ( std::rand() % (rad * rad * 2) >= i1 * i1 + i2 * i2 + i3 * i3 )
                                    this->SetBlock( ix + i1, iy + hei + i2, iz + i3, Block( vox::data::EBlockID::LEAF ) );
                }


#elif 1

                float biome_weights[6];
                wrd::GetBiomeFraction(biome_weights,
                    gpf_height.Sample(cpx + ix, cpz + iz),
                    gpf_temperature.Sample(cpx + ix, cpz + iz));
                
                const float acc = guf_blk_type.Sample(cpx + ix, cpz + iz);
                float sum = 0.0f;
                int blk_id = 0;
                for (; blk_id < 6; ++blk_id)
                {
                    sum += biome_weights[blk_id];
                    if (sum >= acc) break;
                }
                data::EBlockID blk;
                data::EBlockID cover_blk;
                float cover_threshold;
                switch(blk_id)
                {
                case 0:
                    blk = data::EBlockID::SAND;
                    cover_blk = data::EBlockID::SAND;
                    cover_threshold = -1.0f;
                    break;
                case 1:
                    blk = data::EBlockID::RED_SAND;
                    cover_blk = data::EBlockID::DIRT;
                    cover_threshold = 0.4f;
                    break;
                case 2:
                    blk = data::EBlockID::DIRT;
                    cover_blk = data::EBlockID::GRASS;
                    cover_threshold = -0.2f;
                    break;
                case 3:
                    blk = data::EBlockID::COBBLESTONE;
                    cover_blk = data::EBlockID::DIRT;
                    cover_threshold = 0.1f;
                    break;
                case 4:
                    blk = data::EBlockID::DIRT;
                    cover_blk = data::EBlockID::SNOW;
                    cover_threshold = -0.4f;
                    break;
                case 5: case 6:
                    blk = data::EBlockID::COBBLESTONE;
                    cover_blk = data::EBlockID::SNOW;
                    cover_threshold = -0.1f;
                    break;
                }

                float real_sample_height =
                      gpf_world_height1.Sample( cpx + ix, cpz + iz )
                    + gpf_world_height2.Sample( cpx + ix, cpz + iz )
                    + gpf_world_height3.Sample( cpx + ix, cpz + iz )
                    + gpf_world_height4.Sample( cpx + ix, cpz + iz )
                    + gpf_world_height5.Sample( cpx + ix, cpz + iz )
                    + gpf_world_height6.Sample( cpx + ix, cpz + iz );

                int map_height = (int)wrd::ConvertHeight(real_sample_height, biome_weights);
                if (map_height >= consts::MAP_Y) map_height = consts::MAP_Y - 1;
                if (map_height < 0) map_height = 0;
                int iy = 0;
                for (; iy <= map_height - 5; ++iy)
                {
                    this->SetBlock( ix, iy, iz, Block( data::EBlockID::COBBLESTONE ) );
                }
                for (; iy <= map_height; ++iy )
                {
                    this->SetBlock( ix, iy, iz, Block( blk ) );
                }
                if (gpf_cover_block.Sample( cpx + ix, cpz + iz ) > cover_threshold)
                {
                    this->SetBlock( ix, std::min(consts::MAP_Y - 1, std::max(0, iy - 1)), iz, Block( cover_blk ) );
                }
#endif

            }

        std::vector<std::tuple<int,int,float,int>> sample_points;
        for (int iix = -1; iix <= 1; ++iix)
            for (int iiz = -1; iiz <= 1; ++iiz)
                for (int cnt = 0; cnt < 10; ++cnt)
                {
                    sample_points.push_back(std::make_tuple(
                        iix * consts::CHUNK_X + (int)(consts::CHUNK_X *
                            guf_object.Sample(cpx + iix * consts::CHUNK_X + cnt, cpz + iiz * consts::CHUNK_Z)),
                        iiz * consts::CHUNK_Z + (int)(consts::CHUNK_Z *
                            guf_object.Sample(cpx + iix * consts::CHUNK_X + cnt, cpz + iiz * consts::CHUNK_Z + 1)),
                        guf_object.Sample(cpx + iix * consts::CHUNK_X + cnt, cpz + iiz * consts::CHUNK_Z + 2),
                        (int)(256.0f * guf_object.Sample(cpx + iix * consts::CHUNK_X + cnt, cpz + iiz * consts::CHUNK_Z + 3))
                    ));
                }

        for (auto &o : sample_points)
        {
            auto [px, pz, type, flag] = o;

            static rand::GlobalPerlinField gpf_object_dist1(22334455U, 256, 1.0f);
            static rand::GlobalUniformField guf_object_dist11(33445566U);
            static rand::GlobalPerlinField gpf_object_dist2(44556677U, 32, 1.0f);

            int type_int;
            if (type > 0.7) type_int = 1;  // stone
            else type_int = 0;  // other

            if (type_int == 0)
            {
                if (gpf_object_dist1.Sample(cpx + px, cpz + pz) +
                    guf_object_dist11.Sample(cpx + px, cpz + pz) * 0.3f < 0.4f)
                    continue;
            }
            else
            {
                if (gpf_object_dist2.Sample(cpx + px, cpz + pz) < 0.5f)
                    continue;
            }

            float biome_weights[6];
            wrd::GetBiomeFraction(biome_weights,
                gpf_height.Sample(cpx + px, cpz + pz),
                gpf_temperature.Sample(cpx + px, cpz + pz));

            const float acc = guf_blk_type.Sample(cpx + px, cpz + pz);
            float sum = 0.0f;
            int blk_id = 0;
            for (; blk_id < 6; ++blk_id)
            {
                sum += biome_weights[blk_id];
                if (sum >= acc) break;
            }
            data::EBlockID blk;
            data::EBlockID cover_blk;
            float cover_threshold;
            switch(blk_id)
            {
            case 0:
                blk = data::EBlockID::SAND;
                cover_blk = data::EBlockID::SAND;
                cover_threshold = -1.0f;
                break;
            case 1:
                blk = data::EBlockID::RED_SAND;
                cover_blk = data::EBlockID::DIRT;
                cover_threshold = 0.4f;
                break;
            case 2:
                blk = data::EBlockID::DIRT;
                cover_blk = data::EBlockID::GRASS;
                cover_threshold = -0.4f;
                break;
            case 3:
                blk = data::EBlockID::COBBLESTONE;
                cover_blk = data::EBlockID::DIRT;
                cover_threshold = 0.4f;
                break;
            case 4:
                blk = data::EBlockID::DIRT;
                cover_blk = data::EBlockID::SNOW;
                cover_threshold = -0.4f;
                break;
            case 5: case 6:
                blk = data::EBlockID::COBBLESTONE;
                cover_blk = data::EBlockID::SNOW;
                cover_threshold = 0.4f;
                break;
            }

            float real_sample_height =
                gpf_world_height1.Sample( cpx + px, cpz + pz )
                + gpf_world_height2.Sample( cpx + px, cpz + pz )
                + gpf_world_height3.Sample( cpx + px, cpz + pz )
                + gpf_world_height4.Sample( cpx + px, cpz + pz )
                + gpf_world_height5.Sample( cpx + px, cpz + pz )
                + gpf_world_height6.Sample( cpx + px, cpz + pz );

            int map_height = (int)wrd::ConvertHeight(real_sample_height, biome_weights);
            if (map_height >= consts::MAP_Y) map_height = consts::MAP_Y - 1;
            if (map_height < 0) map_height = 0;

            // generate
#define CHK_PLACE(x, y, z, bblk) {\
if (0 <= (x) && 0 <= (y) && 0 <= (z) &&\
(x) < consts::CHUNK_X &&\
(y) < consts::CHUNK_Y &&\
(z) < consts::CHUNK_Z)\
{ this->SetBlock( x, y, z, Block( (bblk) ) ); }}

            std::linear_congruential_engine<uint32_t, 1664525, 1013904223, 0> rand_dist(flag);
            std::uniform_real_distribution<float> real_dist(-1.0f, 1.0f);

            if (type_int == 1)
            {
                // rock
                float sz_amp = 7.0f + 6.0f * real_dist(rand_dist);
                sz_amp *= sz_amp;
                for (int ppx = -5; ppx <= 5; ++ppx)
                    for (int ppz = -5; ppz <= 5; ++ppz)
                        for (int ppy = -3; ppy <= 5; ++ppy)
                        {
                            int dist_sqr = ppx * ppx + ppz * ppz + ppy * ppy;
                            if (dist_sqr < sz_amp * (1.0f + real_dist(rand_dist)))
                                CHK_PLACE(px + ppx, map_height + 1 + ppy, pz + ppz, data::EBlockID::COBBLESTONE);
                        }
            }
            else
            {
                if (blk_id == 0 || blk_id == 1)  // cactus
                {
                    int sz_height = 4 + (int)(3.0f * real_dist(rand_dist));
                    for (int ppy = 0; ppy < sz_height; ++ppy)
                        CHK_PLACE(px, map_height + 1 + ppy, pz, data::EBlockID::CACTUS);
                }
                else if (blk_id == 2 || blk_id == 4)  // small tree
                {
                    int sz_height = 6 + (int)(3.0f * real_dist(rand_dist));
                    for (int ppy = 0; ppy < sz_height; ++ppy)
                        CHK_PLACE(px, map_height + 1 + ppy, pz, data::EBlockID::TREE);
                    int sz_rad = (int)((float)sz_height * 0.6f + 1.0f * real_dist(rand_dist));

                    for (int ppx = -sz_rad; ppx <= sz_rad; ++ppx)
                        for (int ppz = -sz_rad; ppz <= sz_rad; ++ppz)
                            for (int ppy = -sz_rad; ppy <= sz_rad; ++ppy)
                            {
                                int dist_sqr = ppx * ppx + ppz * ppz + ppy * ppy;
                                if (dist_sqr < sz_rad * sz_rad * (1.0f + real_dist(rand_dist)))
                                    CHK_PLACE(px + ppx, map_height + 1 + sz_height + ppy, pz + ppz, data::EBlockID::LEAF);
                            }
                }
                else if (blk_id == 3) // tree
                {
                    int sz_height = 14 + (int)(5.0f * real_dist(rand_dist));
                    for (int ppy = 0; ppy < sz_height; ++ppy)
                        CHK_PLACE(px, map_height + 1 + ppy, pz, data::EBlockID::TREE);
                    int sz_rad = (int)((float)sz_height * 0.4f + 3.0f * real_dist(rand_dist));

                    for (int ppx = -sz_rad; ppx <= sz_rad; ++ppx)
                        for (int ppz = -sz_rad; ppz <= sz_rad; ++ppz)
                            for (int ppy = -sz_rad; ppy <= sz_rad; ++ppy)
                            {
                                int dist_sqr = ppx * ppx + ppz * ppz + ppy * ppy;
                                if (dist_sqr < sz_rad * sz_rad * (1.0f + real_dist(rand_dist)))
                                    CHK_PLACE(px + ppx, map_height + 1 + sz_height + ppy, pz + ppz, data::EBlockID::LEAF);
                            }
                }
                else if (blk_id == 5)  // snow tree
                {
                    int sz_height = 14 + (int)(5.0f * real_dist(rand_dist));
                    int sz_rad = (int)((float)sz_height * 0.4f + 3.0f * real_dist(rand_dist));

                    int max_sz_height = 0;
                    for (int ppx = -sz_rad; ppx <= sz_rad; ++ppx)
                        for (int ppz = -sz_rad; ppz <= sz_rad; ++ppz)
                            for (int ppy = -sz_rad; ppy <= sz_rad * 3; ++ppy)
                            {
                                float leaf_rad = ((4 + sz_rad - ppy) % 5 + 3) / 5.0f;
                                int dist_sqr = ppx * ppx + ppz * ppz + ppy;
                                if (dist_sqr < sz_rad * leaf_rad * sz_rad * (1.0f + real_dist(rand_dist)))
                                {
                                    CHK_PLACE(px + ppx, map_height + 1 + sz_height + ppy, pz + ppz, data::EBlockID::LEAF);
                                    max_sz_height = std::max(max_sz_height, sz_height + ppy);
                                }
                            }

                    for (int ppy = 0; ppy <= max_sz_height + 1; ++ppy)
                    {
                        CHK_PLACE(px + 0, map_height + 1 + ppy, pz + 0, data::EBlockID::TREE);
                        CHK_PLACE(px + 1, map_height + 1 + ppy, pz + 0, data::EBlockID::TREE);
                        CHK_PLACE(px + 0, map_height + 1 + ppy, pz + 1, data::EBlockID::TREE);
                        CHK_PLACE(px + 1, map_height + 1 + ppy, pz + 1, data::EBlockID::TREE);
                    }

                    CHK_PLACE(px + 0, map_height + 2 + max_sz_height, pz + 0, data::EBlockID::LEAF);
                    CHK_PLACE(px + 1, map_height + 2 + max_sz_height, pz + 0, data::EBlockID::LEAF);
                    CHK_PLACE(px + 0, map_height + 2 + max_sz_height, pz + 1, data::EBlockID::LEAF);
                    CHK_PLACE(px + 1, map_height + 2 + max_sz_height, pz + 1, data::EBlockID::LEAF);
                }
            }
#undef CHK_PLACE

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
                if ( block.id != vox::data::EBlockID::AIR/* || block.data != 0*/ )
                {
                    out.push_back( z | x << 8 );
                    out.push_back( (unsigned short)block.id );
                    //out.push_back( block.data );
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
                    if ( block.id != block_up.id/* || block.data != block_up.data*/ )
                    {
                        out.push_back( z | x << 8 );
                        out.push_back( (unsigned short)block.id );
                        //out.push_back( block.data );
                    }
                }
            out.push_back( -1 );
        }
        int sz = (int)out.size();
        fwrite( &sz, sizeof( sz ), 1, fp );
        fwrite( &out[0], sizeof( unsigned short ), out.size(), fp );
        fclose( fp );
    }


    size_t Chunk::GetStoringData(unsigned char** pp_data, size_t offset) const
    {
        std::vector<unsigned short> out;
        for (int i = 0; i < 4; ++i)
            out.push_back(0);
        *(unsigned *)&out[2] = vox::consts::GAME_VERSION;

        for (int z = 0; z < vox::consts::CHUNK_Z; ++z)
            for ( int x = 0; x < vox::consts::CHUNK_X; ++x )
            {
                const Block block = this->GetBlock( x, vox::consts::CHUNK_Y - 1, z );
                if ( block.id != vox::data::EBlockID::AIR/* || block.data != 0*/ )
                {
                    out.push_back( z | x << 8 );
                    out.push_back( (unsigned short)block.id );
                    //out.push_back( block.data );
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
                    if ( block.id != block_up.id/* || block.data != block_up.data*/ )
                    {
                        out.push_back( z | x << 8 );
                        out.push_back( (unsigned short)block.id );
                        //out.push_back( block.data );
                    }
                }
            out.push_back( -1 );
        }
        *(int *)&out[0] = (int)out.size() - 4;
        size_t sz = out.size() * sizeof(unsigned short);
        *pp_data = new unsigned char[sz];
        memcpy(*pp_data, &out[0], sz);
        return sz;
    }

    FILE* Chunk::GetFP(int x, int y, int z, const char* mode)
    {
        char file_name[256];
        sprintf_s( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d.chnk", x, y, z);
        FILE *fp;
        fopen_s( &fp, file_name, mode );
        return fp;
    }

    constexpr static bool APPLY_GLOBAL_ILLUMINATION = false;

    void Chunk::GenerateVertex( Chunk* adj_chks[9],
        unsigned char light_bfs_arr[vox::consts::CHUNK_BLOCKS_CNT * 12],
        unsigned d_lights[(vox::consts::CHUNK_X + 2) * (vox::consts::CHUNK_Y) * (vox::consts::CHUNK_Z + 2)] )
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_temp_[i].clear();
        }

        // 10 MB BFS arr!

#if APPLY_GLOBAL_ILLUMINATION
        
        constexpr static size_t LIGHT_BFS_ARR_SZ = sizeof( unsigned char ) * vox::consts::CHUNK_BLOCKS_CNT * 12;
        /*
        unsigned char* light_bfs_arr = (unsigned char*)malloc( LIGHT_BFS_ARR_SZ );
        if ( light_bfs_arr == nullptr )
        {
            return;
        }
        */
        /*
        unsigned* d_lights = (unsigned*)malloc( sizeof( unsigned ) * vox::consts::CHUNK_BLOCKS_CNT );
        if ( d_lights == nullptr )
        {
            free( light_bfs_arr );
            return;
        }
        */

            memset( d_lights, 0, sizeof( unsigned ) *
                (vox::consts::CHUNK_X + 2) * (vox::consts::CHUNK_Y) * (vox::consts::CHUNK_Z + 2) );

            int adj_chunk_max_y = 0;
            for ( int chk_ind = 0; chk_ind < 9; ++chk_ind )
            {
                adj_chunk_max_y = std::max( adj_chunk_max_y, (int)adj_chks[chk_ind]->max_block_y_ );
            }

            for ( int lit_type_ind = 0; lit_type_ind < (int)vox::data::lightinfos::EnumLightType::MAX_SIZE; ++lit_type_ind )
            {
                std::vector<vox::data::lightinfos::LightInstance> vec1, vec2;

                for ( int chk_ind = 0; chk_ind < 9; ++chk_ind )
                {
                    const Chunk* ch = adj_chks[chk_ind];
                    const int dxcv = vox::data::vector::GetX( ch->cv_ ) - vox::data::vector::GetX( cv_ ) + 1;
                    const int dzcv = vox::data::vector::GetZ( ch->cv_ ) - vox::data::vector::GetZ( cv_ ) + 1;

                    for ( const auto& lit : ch->light_infos_ )
                        if ( lit.type == (vox::data::lightinfos::EnumLightType)lit_type_ind )
                        {
                            static_assert(vox::consts::CHUNK_X == 64);
                            static_assert(vox::consts::CHUNK_Y == 256);
                            static_assert(vox::consts::CHUNK_Z == 64);

                            for ( const auto& litp : lit.instances )
                            {
                                const unsigned lit_pos_x = (litp.positions_MSB_X6_Z8_Y8_LSB >> 16U) + dxcv * vox::consts::CHUNK_X;
                                const unsigned lit_pos_y = litp.positions_MSB_X6_Z8_Y8_LSB & 0xff;
                                const unsigned lit_pos_z = (litp.positions_MSB_X6_Z8_Y8_LSB >> 8U & 0xff) + dzcv * vox::consts::CHUNK_Z;

                                if ( (int)lit_pos_x + (int)litp.power < vox::consts::CHUNK_X ) continue;
                                if ( (int)lit_pos_x - (int)litp.power >= vox::consts::CHUNK_X * 2 ) continue;
                                if ( (int)lit_pos_z + (int)litp.power < vox::consts::CHUNK_Z ) continue;
                                if ( (int)lit_pos_z - (int)litp.power >= vox::consts::CHUNK_Z * 2 ) continue;

                                const unsigned lit_pos = lit_pos_x << 16U | lit_pos_y | lit_pos_z << 8U;
                                light_bfs_arr[lit_pos] = litp.power;
                                vec1.push_back( { lit_pos, litp.power - 1 } );
                            }
                        }
                }

                if ( !vec1.empty() )
                {
                    memset( light_bfs_arr, 0, LIGHT_BFS_ARR_SZ );
                }

                int bfs_x_min = 999999999;
                int bfs_x_max = -999999999;
                int bfs_y_min = 999999999;
                int bfs_y_max = -999999999;
                int bfs_z_min = 999999999;
                int bfs_z_max = -999999999;

                while ( true )
                {
                    const size_t sz1 = vec1.size();
                    if ( sz1 == 0 ) break;
                    for ( size_t i = 0; i < sz1; ++i )
                    {
                        // must add scanning block isfull.. no....
                        const auto pos = vec1[i].positions_MSB_X6_Z8_Y8_LSB;
                        const auto lit_pow = vec1[i].power;

                        bfs_x_min = std::min( bfs_x_min, (int)pos & 0xff0000 );
                        bfs_x_max = std::max( bfs_x_max, (int)pos & 0xff0000 );
                        bfs_y_min = std::min( bfs_y_min, (int)pos & 0xff );
                        bfs_y_max = std::max( bfs_y_max, (int)pos & 0xff );
                        bfs_z_min = std::min( bfs_z_min, (int)pos & 0xff00 );
                        bfs_z_max = std::max( bfs_z_max, (int)pos & 0xff00 );

                        if ( (pos & 0xff0000) != 0x0 && light_bfs_arr[pos - 0x10000] < lit_pow )
                            light_bfs_arr[pos - 0x10000] = lit_pow, vec2.push_back( { pos - 0x10000, lit_pow - 1 } );
                        if ( (pos & 0xff0000) != 0xbf0000 && light_bfs_arr[pos + 0x10000] < lit_pow )
                            light_bfs_arr[pos + 0x10000] = lit_pow, vec2.push_back( { pos + 0x10000, lit_pow - 1 } );
                        if ( (pos & 0xff00) != 0x0 && light_bfs_arr[pos - 0x100] < lit_pow )
                            light_bfs_arr[pos - 0x100] = lit_pow, vec2.push_back( { pos - 0x100, lit_pow - 1 } );
                        if ( (pos & 0xff00) != 0xbf00 && light_bfs_arr[pos + 0x100] < lit_pow )
                            light_bfs_arr[pos + 0x100] = lit_pow, vec2.push_back( { pos + 0x100, lit_pow - 1 } );
                        if ( (pos & 0xff) != 0x0 && light_bfs_arr[pos - 0x1] < lit_pow )
                            light_bfs_arr[pos - 0x1] = lit_pow, vec2.push_back( { pos - 0x1, lit_pow - 1 } );
                        if ( (pos & 0xff) != adj_chunk_max_y && light_bfs_arr[pos + 0x1] < lit_pow )
                            light_bfs_arr[pos + 0x1] = lit_pow, vec2.push_back( { pos + 0x1, lit_pow - 1 } );
                    }
                    vec1.clear();

                    const size_t sz2 = vec2.size();
                    if ( sz2 == 0 ) break;
                    for ( size_t i = 0; i < sz2; ++i )
                    {
                        // must add scanning block isfull.. no....
                        const auto pos = vec2[i].positions_MSB_X6_Z8_Y8_LSB;
                        const auto lit_pow = vec2[i].power;

                        bfs_x_min = std::min( bfs_x_min, (int)pos & 0xff0000 );
                        bfs_x_max = std::max( bfs_x_max, (int)pos & 0xff0000 );
                        bfs_y_min = std::min( bfs_y_min, (int)pos & 0xff );
                        bfs_y_max = std::max( bfs_y_max, (int)pos & 0xff );
                        bfs_z_min = std::min( bfs_z_min, (int)pos & 0xff00 );
                        bfs_z_max = std::max( bfs_z_max, (int)pos & 0xff00 );

                        if ( (pos & 0xff0000) != 0x0 && light_bfs_arr[pos - 0x10000] < lit_pow )
                            light_bfs_arr[pos - 0x10000] = lit_pow, vec1.push_back( { pos - 0x10000, lit_pow - 1 } );
                        if ( (pos & 0xff0000) != 0xbf0000 && light_bfs_arr[pos + 0x10000] < lit_pow )
                            light_bfs_arr[pos + 0x10000] = lit_pow, vec1.push_back( { pos + 0x10000, lit_pow - 1 } );
                        if ( (pos & 0xff00) != 0x0 && light_bfs_arr[pos - 0x100] < lit_pow )
                            light_bfs_arr[pos - 0x100] = lit_pow, vec1.push_back( { pos - 0x100, lit_pow - 1 } );
                        if ( (pos & 0xff00) != 0xbf00 && light_bfs_arr[pos + 0x100] < lit_pow )
                            light_bfs_arr[pos + 0x100] = lit_pow, vec1.push_back( { pos + 0x100, lit_pow - 1 } );
                        if ( (pos & 0xff) != 0x0 && light_bfs_arr[pos - 0x1] < lit_pow )
                            light_bfs_arr[pos - 0x1] = lit_pow, vec1.push_back( { pos - 0x1, lit_pow - 1 } );
                        if ( (pos & 0xff) != adj_chunk_max_y && light_bfs_arr[pos + 0x1] < lit_pow )
                            light_bfs_arr[pos + 0x1] = lit_pow, vec1.push_back( { pos + 0x1, lit_pow - 1 } );
                    }
                    vec2.clear();
                }

                bfs_x_min = std::max( vox::consts::CHUNK_X - 1, bfs_x_min >> 16 ) - vox::consts::CHUNK_X;
                bfs_x_max = std::min( 2 * vox::consts::CHUNK_X, bfs_x_max >> 16 ) - vox::consts::CHUNK_X;
                bfs_y_min = std::max( 0, bfs_y_min & 0xff );
                bfs_y_max = std::min( vox::consts::CHUNK_Y - 1, bfs_y_max & 0xff );
                bfs_z_min = std::max( vox::consts::CHUNK_Z - 1, bfs_z_min >> 8 & 0xff ) - vox::consts::CHUNK_Z;
                bfs_z_max = std::min( 2 * vox::consts::CHUNK_Z, bfs_z_max >> 8 & 0xff ) - vox::consts::CHUNK_Z;

                const vox::data::lightinfos::R8G8B8* ptr =
                    vox::data::lightinfos::GetRGBTable( (vox::data::lightinfos::EnumLightType)lit_type_ind );

                for ( int iy = bfs_y_min; iy <= bfs_y_max; ++iy )
                    for ( int iz = bfs_z_min; iz <= bfs_z_max; ++iz )
                        for ( int ix = bfs_x_min; ix <= bfs_x_max; ++ix )
                        {
                            USING_INTEGER_PTR_TO_OTHER_INTEGER_PTR_TRICK;
                            const unsigned bfs_arr_ind = iy | (iz + vox::consts::CHUNK_Z) << 8U | (ix + vox::consts::CHUNK_X) << 16U;
                            const unsigned char lit = light_bfs_arr[bfs_arr_ind];
                            if ( lit == 0 ) continue;
                            const auto* const p_RGB = &ptr[lit];
                            const unsigned ind =
                                ix + 1U +
                                (iz + 1U) * (vox::consts::CHUNK_X + 2) +
                                iy * (vox::consts::CHUNK_X + 2U) * (vox::consts::CHUNK_Z + 2U);
                            unsigned char* const p_lit = (unsigned char*)&d_lights[ind];
                            if ( p_lit[0] < p_RGB->R ) p_lit[0] = p_RGB->R;
                            if ( p_lit[1] < p_RGB->G ) p_lit[1] = p_RGB->G;
                            if ( p_lit[2] < p_RGB->B ) p_lit[2] = p_RGB->B;
                        }

            }  // for each light types
        }  // #if  APPLY_GLOBAL_ILLUMINATION
#endif

        // mesh start
        unsigned adj_blocks_is_full = 0U;
        bool to_gen_part = false;
        for ( int iz = 0; iz < vox::consts::CHUNK_Z; ++iz )
            for ( int ix = 0; ix < vox::consts::CHUNK_X; ++ix )
            {
                to_gen_part = false;
                for ( int iy = 0; iy < vox::consts::CHUNK_Y; ++iy )
                {
                    const auto id = this->GetBlockId( ix, iy, iz );
                    if ( id != EBlockID::AIR )
                    {
#if APPLY_GLOBAL_ILLUMINATION
                            vox::data::lightinfos::R8G8B8 adj_blocks_rgb[27];
                            memset( adj_blocks_rgb, 0, sizeof( adj_blocks_rgb ) );
#endif
                        if (!to_gen_part) {
                            to_gen_part = true;
                            adj_blocks_is_full = 0U;
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
                                            adj_blocks_is_full |= is_full_dblock << adji;
                                            ++adji;
                                            continue;
                                        }
                                    
                                        if ( iiz >= 0 && iiz < vox::consts::CHUNK_Z && iix >= 0 && iix < vox::consts::CHUNK_X )
                                        {
                                            is_full_dblock = vox::data::IsFullBlock( this->GetBlockId( iix, iiy, iiz ) );
                                        }
                                        else
                                        {
                                            static_assert(vox::consts::CHUNK_Y == vox::consts::MAP_Y);
                                            const auto new_cv = vox::data::vector::Add( cv_,
                                                vox::data::vector::Set( iix >> vox::consts::CHUNK_X_LOG2, 0, iiz >> vox::consts::CHUNK_Z_LOG2, 0 )
                                            );
                                            for ( int i = 0; i < 8; ++i )
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
#if APPLY_GLOBAL_ILLUMINATION
                                        {
                                            if ( !is_full_dblock )
                                            {
                                                // this has a bug that cannot get adjacent chunk's light infos but im tired..
                                                adj_blocks_rgb[adji] = *(vox::data::lightinfos::R8G8B8*)&d_lights[
                                                    iix + 1 +
                                                        iiy * (vox::consts::CHUNK_X + 2) * (vox::consts::CHUNK_Z + 2) +
                                                        (iiz + 1) * (vox::consts::CHUNK_X + 2)];
                                            }
                                        }
#endif
                                        ++adji;
                                    }  // for y z x [-1, 1]
                        } else {
                            unsigned adji = 18U;
                            adj_blocks_is_full >>= 9;
                            const int iiy = iy + 1;
                            if ( iiy < vox::consts::CHUNK_Y )
                                for ( int dz = 1; dz >= -1; --dz )
                                {
                                    const int iiz = iz + dz;
                                    for ( int dx = -1; dx <= 1; ++dx )
                                    {
                                        const int iix = ix + dx;

                                        unsigned is_full_dblock;

                                        if ( iiz >= 0 && iiz < vox::consts::CHUNK_Z && iix >= 0 && iix < vox::consts::CHUNK_X )
                                        {
                                            is_full_dblock = vox::data::IsFullBlock( this->GetBlockId( iix, iiy, iiz ) );
                                        }
                                        else
                                        {
                                            static_assert(vox::consts::CHUNK_Y == vox::consts::MAP_Y);
                                            const auto new_cv = vox::data::vector::Add( cv_,
                                                vox::data::vector::Set( iix >> vox::consts::CHUNK_X_LOG2, 0, iiz >> vox::consts::CHUNK_Z_LOG2, 0 )
                                            );
                                            for ( int i = 0; i < 8; ++i )
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
                                        adji++;
                                    }
                                }
                        }

                        const unsigned tp = vox::data::GetTexturePos( id );
                        //const unsigned block_rgb = vox::data::GetRGB( id );
                        const unsigned dpos = ix << 24U | iy << 8U | iz << 0U;

                        if ( adj_blocks_is_full >> 13 & 0x1 )
                        {
                            static constexpr unsigned adj_ind_bits = 0b00'01100'01110'10000'01010'00100'10110U;
#define U4(x1, x2, x3, x4) (x1 | x2 << 8U | x3 << 16U | x4 << 24U)
                            static constexpr unsigned adjl_ind_bits[6] = { U4( 19, 21, 25, 23 ), U4( 7, 3, 1, 5 ), U4( 19, 11, 1, 9 ), U4( 25, 15, 7, 17 ), U4( 23, 17, 5, 11 ), U4( 21, 9, 3, 15 ) };
                            static constexpr unsigned adjm_ind_bits[6] = { U4( 20, 18, 24, 26 ), U4( 8, 6, 0, 2 ), U4( 18, 20, 2, 0 ), U4( 26, 24, 6, 8 ), U4( 20, 26, 8, 2 ), U4( 24, 18, 0, 6 ) };
                            static constexpr unsigned adjr_ind_bits[6] = { U4( 23, 19, 21, 25 ), U4( 5, 7, 3, 1 ), U4( 9, 19, 11, 1 ), U4( 17, 25, 15, 7 ), U4( 11, 23, 17, 5 ), U4( 15, 21, 9, 3 ) };
#undef U4
                            for ( unsigned side = 0; side < 6; ++side )
                            {
                                const unsigned adj_ind = (adj_ind_bits >> side * 5U) & 0b11111;
                                if ( adj_blocks_is_full >> adj_ind & 0x1 )
                                    continue;

                                unsigned vertex_ambient_level_bits = 0U;
                                unsigned vertex_RGBs[4];

                                unsigned darkest_index = 0U;
                                unsigned darkest_num = 0U;
                                unsigned darkest_level = 3U;

                                unsigned brightest_index = 0U;
                                //unsigned brightest_num = 0U;
                                unsigned brightest_level = 0U;

                                for ( unsigned vtx = 0U; vtx < 4U; ++vtx )
                                {

                                    const unsigned adjl_ind = adjl_ind_bits[side] >> (vtx << 3U) & 0xff;
                                    const unsigned adjm_ind = adjm_ind_bits[side] >> (vtx << 3U) & 0xff;
                                    const unsigned adjr_ind = adjr_ind_bits[side] >> (vtx << 3U) & 0xff;

                                    const unsigned adjl = adj_blocks_is_full >> adjl_ind & 0x1;
                                    const unsigned adjm = adj_blocks_is_full >> adjm_ind & 0x1;
                                    const unsigned adjr = adj_blocks_is_full >> adjr_ind & 0x1;

#if APPLY_GLOBAL_ILLUMINATION
                                    {
                                        unsigned vtx_r = adj_blocks_rgb[adj_ind].R;
                                        unsigned vtx_g = adj_blocks_rgb[adj_ind].G;
                                        unsigned vtx_b = adj_blocks_rgb[adj_ind].B;
                                        unsigned vtx_cnt = 1U;

                                        if ( !adjl )
                                            vtx_r += adj_blocks_rgb[adjl_ind].R,
                                            vtx_g += adj_blocks_rgb[adjl_ind].G,
                                            vtx_b += adj_blocks_rgb[adjl_ind].B,
                                            ++vtx_cnt;
                                        if ( !adjm )
                                            vtx_r += adj_blocks_rgb[adjm_ind].R,
                                            vtx_g += adj_blocks_rgb[adjm_ind].G,
                                            vtx_b += adj_blocks_rgb[adjm_ind].B,
                                            ++vtx_cnt;
                                        if ( !adjr )
                                            vtx_r += adj_blocks_rgb[adjr_ind].R,
                                            vtx_g += adj_blocks_rgb[adjr_ind].G,
                                            vtx_b += adj_blocks_rgb[adjr_ind].B,
                                            ++vtx_cnt;

                                        vtx_r /= vtx_cnt;
                                        vtx_g /= vtx_cnt;
                                        vtx_b /= vtx_cnt;

                                        vertex_RGBs[vtx] = vtx_r << 24U | vtx_g << 16U | vtx_b << 8U;
                                    }
#else
                                    {
                                        vertex_RGBs[vtx] = 0xffffff00U;
                                    }
#endif

                                    unsigned ambient_level = 0U;
                                    if ( !adjl || !adjr )
                                        ambient_level = 3 - adjl - adjm - adjr;  // branch can be removed using table

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
                                    vc.light = vertex_RGBs[ind] | ((vertex_ambient_level_bits >> (ind * 2U)) & 0b11U);
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
                            // has a bug, but there is only full block so this will be fixed later
                            // generate all side
                            constexpr unsigned vertex_select_index = 0x230120U;
                            for ( int i = 0; i < 6; ++i )
                                for ( int j = 0; j < 6; ++j )
                                {
                                    const unsigned ind = (vertex_select_index >> j * 4U) & 0xf;
                                    vox::ren::vertex::VertexChunk vc { vox::ren::vertex::VERTICES_BLOCK[i * 4 + ind] };
                                    vc.position += dpos;
                                    vc.texcoord += tp;
                                    vc.light |= 3;
                                    vertex_buffer_temp_[i].push_back( &vc );
                                }
                        }  // if not full block
                    }  // if not AIR
                    else
                    {
                        to_gen_part = false;
                    }
                }  // for y z x
            }
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