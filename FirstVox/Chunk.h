#pragma once

#include "Macros.h"
#include "Consts.h"
#include "EBlockID.h"
#include "EnumSide.h"
#include "Block.h"
#include "Vector.h"
#include "ChunkVertexBuffer.h"

namespace vox::data
{
    enum class EnumChunkStates
    {
        VERTEX_NEEDED = 0,
        SET = 1,
    };

    class Chunk
    {
    private:
        EnumChunkStates state_;
        vox::data::Vector4i cv_;
        Block d_[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X];
        ChunkVertexBuffer vertex_buffer_;

    public:
        Chunk( vox::data::Vector4i cv );

        FORCE_INLINE Block& At( int x, int y, int z )
        {
            return this->d_[y][z][x];
        }
        FORCE_INLINE const Block& At( int x, int y, int z ) const
        {
            return this->d_[y][z][x];
        }
        FORCE_INLINE bool IsRenderable() const
        {
            return this->state_ == EnumChunkStates::SET;
        }
        FORCE_INLINE vox::data::Vector4i GetChunkPos() const
        {
            return this->cv_;
        }

        void GenerateVertex( Chunk* front, Chunk* back, Chunk* right, Chunk* left );
        void Render( vox::data::EnumBitSide6 sides );
    };
}