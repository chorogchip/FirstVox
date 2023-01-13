#pragma once
#include "Renderer_for_Winmain.h"
#include "Renderer.h"

#include "DXHeaders.h"

namespace vox::ren::base
{
    ID3D11Device* get_device();
    ID3D11DeviceContext* get_immediate_context();
}