#pragma once

#include "EBlockID.h"

namespace vox::data
{

    class Block
    {
    public:
        EBlockID id;
        unsigned short data;
    };

}