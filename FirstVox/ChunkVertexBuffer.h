#pragma once

#include <vector>

#include "Vector.h"
#include "Consts.h"
#include "Block.h"
#include "EnumSide.h"

namespace vox::data
{

    struct ChunkVertexUnit
    {
        void* vertex_buffer;
        int size;

        static constexpr int VERTEX_UNIT_SIZE = 1024 * 6;
    };

    class ChunkVertexBuffer
    {
    private:
        std::vector<ChunkVertexUnit> vertex_buffer_[(int)vox::data::EnumSide::MAX_COUNT];
    public:
        ChunkVertexBuffer();
        ~ChunkVertexBuffer();

        void GenerateVertex(
            vox::data::Vector4i cv,
            vox::data::Block d[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
            vox::data::Block (*d_front)[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
            vox::data::Block (*d_back)[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
            vox::data::Block( *d_right )[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X],
            vox::data::Block( *d_left )[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X]
        );
        void Render(vox::data::Vector4i cv, vox::data::EnumBitSide6 sides ) const;
    };

}

