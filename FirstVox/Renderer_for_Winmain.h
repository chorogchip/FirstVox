#pragma once
#include <Windows.h>
#include "Renderer.h"

namespace vox::ren::base
{
    HRESULT Init( HWND window );
    HRESULT ResizeScreen( HWND hwnd, long width, long height );
    void Clean();

    void Clear();
    void Present();
}