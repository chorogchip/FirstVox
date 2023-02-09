#include "Chunk.h"

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

#include "Perlin.h"
#include "Logger.h"
#include "VertexRenderer.h"


namespace vox::data
{
    Chunk::Chunk( vox::data::Vector4i cv ):
        cv_{ cv }, d_{ }, vertex_buffer_{}
    {}

    void Chunk::Load()
    {
        char file_name[256];
        sprintf( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d", this->cv_.m128i_i32[0], this->cv_.m128i_i32[1], this->cv_.m128i_i32[2]);
        FILE* fp = fopen( file_name, "rb" );
        if ( fp != nullptr )
        {
            fread( this->d_, sizeof( this->d_ ), 1, fp );
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
                    Block& block = this->At( ix, iy, iz );
                    block.id = vox::data::EBlockID::COBBLESTONE;
                    block.data = 0;
                }
                for ( ; iy < rand_height_dirt; ++iy )
                {
                    Block& block = this->At( ix, iy, iz );
                    block.id = vox::data::EBlockID::DIRT;
                    block.data = 0;
                }

            }
    }

    void Chunk::GenerateVertex( Chunk* front, Chunk* back, Chunk* right, Chunk* left )
    {
        this->vertex_buffer_.GenerateVertex(
            this->cv_, this->d_,
            &front->d_, &back->d_, &right->d_, &left->d_
        );
    }

    void Chunk::Render( vox::data::EnumBitSide6 sides )
    {
        this->vertex_buffer_.Render( this->cv_, sides );
    }

    void Chunk::Clear()
    {
        char file_name[256];
        sprintf( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d", this->cv_.m128i_i32[0], this->cv_.m128i_i32[1], this->cv_.m128i_i32[2]);
        FILE* fp = fopen( file_name, "wb" );
        fwrite( this->d_, sizeof( this->d_ ), 1, fp );
        fclose( fp );
    }
}