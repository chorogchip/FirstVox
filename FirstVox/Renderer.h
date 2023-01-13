#pragma once

namespace vox::ren::base
{
    inline long width_;
    inline long height_;

    inline long GetScreenWidth()
    {
        return width_;
    }
    inline long GetScreenHeight()
    {
        return height_;
    }
}