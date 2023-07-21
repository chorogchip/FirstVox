#pragma once

#include "Renderer_for_Winmain.h"
#include "Renderer.h"

#include "DXHeaders.h"

namespace vox::ren::base
{
    ID3D11DeviceContext* get_immediate_context();

    HRESULT CreateShaderAndInputLayout(
        LPCWSTR vs_name, ID3D11VertexShader** pp_vs, ID3D11InputLayout** pp_il,
        const D3D11_INPUT_ELEMENT_DESC* elem_desc, UINT elem_desc_sz,
        LPCWSTR ps_name, ID3D11PixelShader** pp_ps
    );
    HRESULT CreateDefaultBuffer(
        ID3D11Buffer** pp_buffer, const void* buffer, UINT buffer_size, D3D11_BIND_FLAG flag
    );
    HRESULT CreateComplexBuffer(
        ID3D11Buffer** pp_buffer, UINT buffer_size,
        D3D11_USAGE usage, D3D11_BIND_FLAG flag, UINT cpu_access_flags
    );
    HRESULT CreateSampler(
        ID3D11SamplerState** pp_ss
    );
    HRESULT CreateTextureFromImage(
        const wchar_t* image_name, ID3D11ShaderResourceView** pp_srv
    );

    void SetDepthTest(bool value);
}