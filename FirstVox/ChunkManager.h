#pragma once

#include "Macros.h"
#include "Vector.h"
#include "Chunk.h"

namespace vox::core::chunkmanager
{
    void Init();
    void Clean();
    int GetRenderChunkDist();
    void SetRenderChunkDist( int render_chunk_dist );
    void Render();

    vox::data::Block* VEC_CALL GetBlockByPos( vox::data::Vector4i block_pos );
    void VEC_CALL RebuildMeshByBlockPos( vox::data::Vector4i block_pos );
}