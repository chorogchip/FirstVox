#pragma once
#include "Consts.h"
#include "EBlockID.h"

namespace vox::data
{

    class Block
    {
    public:
        EBlockID id;
        unsigned short data;
    };

    class Chunk
    {
    public:
        Block d[vox::consts::CHUNK_Y][vox::consts::CHUNK_Z][vox::consts::CHUNK_X];
    };

}