#pragma once

#include <vector>

#include "Vector.h"
#include "Consts.h"
#include "Block.h"
#include "EnumSide.h"
#include "ResizingArray.h"

namespace vox::data
{
    struct ChunkVertexUnit
    {
        void* vertex_buffer;
        int size;

        static constexpr inline int VERTEX_UNIT_SIZE = 1024 * 6;
    };

    class ChunkVertexBuffer
    {
    private:
        std::vector<ChunkVertexUnit> vertex_buffer_[(int)vox::data::EnumSide::MAX_COUNT];
    public:
        ChunkVertexBuffer();
        ~ChunkVertexBuffer();
        void RetrieveBuffer();

        void MapData( ResizingArray( &cpu_vertex_buffer )[(int)vox::data::EnumSide::MAX_COUNT] );
        void Render(vox::data::Vector4i cv, vox::data::EnumBitSide6 sides ) const;
    };

}