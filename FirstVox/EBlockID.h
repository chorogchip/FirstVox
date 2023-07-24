#pragma once
#include "Point2D.h"

namespace vox::data {

#define BLOCK_ATTRIB_TUPLE \
/*          name, is_full_block, texture_pos_xy,y*/\
BL_ATR(AIR,             false,  0, 0)\
BL_ATR(GRASS,            true,  1, 0)\
BL_ATR(DIRT,             true,  2, 0)\
BL_ATR(SNOW,             true,  3, 0)\
BL_ATR(SAND,             true,  4, 0)\
BL_ATR(RED_SAND,         true,  5, 0)\
BL_ATR(COBBLESTONE,      true,  6, 0)\
BL_ATR(LEAF,             true,  7, 0)\
BL_ATR(TREE,             true,  8, 0)\
BL_ATR(CACTUS,           true,  9, 0)\
BL_ATR(PLANK,            true, 10, 0)\
BL_ATR(STONE_BRICK_1,    true, 11, 0)\
BL_ATR(STONE_BRICK_2,    true, 12, 0)\
BL_ATR(STONE_BRICK_3,    true, 13, 0)\
BL_ATR(BRICK,            true, 14, 0)\
BL_ATR(GOLD,             true, 15, 0)\
BL_ATR(COLOR_RED,        true, 16, 1)\
BL_ATR(COLOR_REDORANGE,  true, 17, 1)\
BL_ATR(COLOR_ORANGE,     true, 18, 1)\
BL_ATR(COLOR_ORANGEYELLOW, true, 19, 1)\
BL_ATR(COLOR_GOLD,       true, 20, 1)\
BL_ATR(COLOR_YELLOW,     true, 21, 1)\
BL_ATR(COLOR_YELLOWGREEN,true, 22, 1)\
BL_ATR(COLOR_BRIGHTGREEN,true, 23, 1)\
BL_ATR(COLOR_BLUEGREEN,  true, 24, 1)\
BL_ATR(COLOR_SKYBLUE,    true, 25, 1)\
BL_ATR(COLOR_BRIGHTBLUE, true, 26, 1)\
BL_ATR(COLOR_MEDIUMBLUE, true, 27, 1)\
BL_ATR(COLOR_BLUE,       true, 28, 1)\
BL_ATR(COLOR_PURPLE,     true, 29, 1)\
BL_ATR(COLOR_VIOLET,     true, 30, 1)\
BL_ATR(COLOR_MAGENTA,    true, 31, 1)\
BL_ATR(COLOR_PINK, true, 32, 2)\
BL_ATR(COLOR_BRIGHTORANGE, true, 33, 2)\
BL_ATR(COLOR_BRIGHTBROWN,true, 34, 2)\
BL_ATR(COLOR_BROWN,      true, 35, 2)\
BL_ATR(COLOR_OLIVE,      true, 36, 2)\
BL_ATR(COLOR_GREEN,      true, 37, 2)\
BL_ATR(COLOR_DARKCYAN,   true, 38, 2)\
BL_ATR(COLOR_DARKSKYBLUE,true, 39, 2)\
BL_ATR(COLOR_DARKBLUE,   true, 40, 2)\
BL_ATR(COLOR_DARKPURPLE, true, 41, 2)\
BL_ATR(COLOR_DARKMAGENTA,true, 42, 2)\
BL_ATR(COLOR_WHITE,      true, 43, 2)\
BL_ATR(COLOR_BRIGHTGRAY, true, 44, 2)\
BL_ATR(COLOR_GRAY,       true, 45, 2)\
BL_ATR(COLOR_DARKGRAY,   true, 46, 2)\
BL_ATR(COLOR_BLACK,      true, 47, 2)\
BL_ATR(GLASS,           false, 48, 3)\
BL_ATR(MAX_COUNT,  false, 0, 0)

    enum class EBlockID : unsigned short
    {
#define BL_ATR(name, ...) name, 
        BLOCK_ATTRIB_TUPLE
#undef BL_ATR
    };

    bool IsFullBlock( EBlockID block_id );
    const unsigned GetTexturePos( EBlockID block_id );
    //unsigned int GetRGB( EBlockID block_id );

    // pervious EnumBlocks
    /*
    enum class EnumBlocks
    {
        AIR = 0,
        STONE = 1,
        COBBLESTONE = 2,
        GRASS = 3,
        DIRT = 4,
        SAND = 5,
        LOG = 6,
        LEAVES = 7,
        PLANK = 8,
        PLANK_HALF = 9,
        GLASS = 10,
        GLASS_PANE = 11,
        GLASS_STAINED = 12,
        GLASS_PANE_STAINED = 13,
        FERN = 14,
        GLOWSTONE = 15,
        WATER = 16,
        LAVA = 17,
    };
    */

}