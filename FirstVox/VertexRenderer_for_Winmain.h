#pragma once

#include <Windows.h>

namespace vox::ren::vertex
{
    HRESULT Init( HWND h_wnd_);
    void Clean();
    void StartRenderChunks( float delta_time, float* sky_color );
    void ResizeScreen();
}