#include "Chunk.h"

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

#include "Perlin.h"
#include "Logger.h"
#include "VertexRenderer.h"


namespace vox::data
{
    Chunk::Chunk( vox::data::Vector4i cv ) :
        cv_{ cv }, d_{ }, vertex_buffer_{}, is_changed_{ false }
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
                    this->d_[vox::consts::CHUNK_Y - 1][rd1 & 0xff][(rd1 & 0xff00) >> 8] =
                        vox::data::Block((vox::data::EBlockID)rd2, rd3);
                } while ( true );
                for ( int y = vox::consts::CHUNK_Y - 2; y >= 0; --y )
                {
                    memcpy( &this->d_[y][0][0], &this->d_[y + 1][0][0], sizeof( this->d_[y] ) );
                    do
                    {
                        unsigned short rd1 = *p++;
                        if ( rd1 == (unsigned short)-1 ) break;
                        unsigned short rd2 = *p++;
                        unsigned short rd3 = *p++;
                        this->d_[y][rd1 & 0xff][(rd1 & 0xff00) >> 8] =
                            vox::data::Block((vox::data::EBlockID)rd2, rd3);
                    } while ( true );
                }
                delete[] data;
            }
            this->is_changed_ = true;
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
        if ( !this->is_changed_ ) return;
        char file_name[256];
        sprintf_s( file_name, "GameData/Map/Chunks/chunk_%d_%d_%d.chnk", this->cv_.m128i_i32[0], this->cv_.m128i_i32[1], this->cv_.m128i_i32[2]);
        FILE *fp;
        fopen_s( &fp, file_name, "wb" );
        fwrite( &vox::consts::GAME_VERSION, sizeof( vox::consts::GAME_VERSION ), 1, fp );
        
        std::vector<unsigned short> out;
        for (int z = 0; z < vox::consts::CHUNK_Z; ++z)
            for ( int x = 0; x < vox::consts::CHUNK_X; ++x )
            {
                const Block &block = this->d_[vox::consts::CHUNK_Y - 1][z][x];
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
                    const Block &block = this->d_[y][z][x];
                    if ( block.id != this->d_[y + 1][z][x].id || block.data != this->d_[y + 1][z][x].data )
                    {
                        out.push_back( z | x << 8 );
                        out.push_back( (unsigned short)block.id );
                        out.push_back( block.data );
                    }
                }
            out.push_back( -1 );
        }
        int sz = out.size();
        fwrite( &sz, sizeof( sz ), 1, fp );
        fwrite( &out[0], sizeof( unsigned short ), out.size(), fp );
        fclose( fp );
    }

    void Chunk::Touch()
    {
        this->is_changed_ = true;
    }
}