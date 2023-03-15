#pragma once

#include "EBlockID.h"

namespace vox::data
{

    typedef unsigned short BlockData;

    class Block
    {
    public:
        EBlockID id;
        BlockData data;
    };

}