#pragma once

#include <Windows.h>

namespace vox::ren::vertex
{

    HRESULT InitForTut( HWND h_wnd_);
    void CleanForTut();
    void RenderForTut();
    void ResizeScreen();

    HRESULT InitForChunk( HWND h_wnd_);
    void CleanForChunk();
    void RenderForChunk();
    void ResizeScreenForChunk();
}