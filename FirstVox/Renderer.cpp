
#include "Renderer_for_Winmain.h"
#include "Renderer.h"

#include <Windows.h>

#include "FirstVoxHeader.h"
#include "Macros.h"
#include "Consts.h"
#include "DXHeaders.h"

namespace vox::ren::base
{
    static D3D_DRIVER_TYPE driver_type_{};
    static D3D_FEATURE_LEVEL feature_level_{};
    
    static ID3D11Device* d3d_device_ = nullptr;
    static ID3D11DeviceContext* immediate_context_ = nullptr;
    static IDXGISwapChain* swap_chain_ = nullptr;
    static ID3D11RenderTargetView* render_target_view_ = nullptr;
    
    static ID3D11Texture2D* depth_stencil_ = nullptr;
    static ID3D11DepthStencilView* depth_stencil_view_ = nullptr;

    ID3D11DeviceContext* get_immediate_context()
    {
        return immediate_context_;
    }

    HRESULT Init( HWND hwnd )
    {
        HRESULT hr{ S_OK };

        RECT rc;
        GetClientRect( hwnd, &rc );
        width_ = rc.right - rc.left;
        height_ = rc.bottom - rc.top;

        constexpr UINT createDeviceFlags = vox::consts::DEBUG_ENABLED ? D3D11_CREATE_DEVICE_DEBUG : 0;
        static constexpr D3D_DRIVER_TYPE driver_types_[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        constexpr UINT numDriverTypes{ ARRAYSIZE( driver_types_ ) };
        static constexpr D3D_FEATURE_LEVEL feature_levels_[] =
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        constexpr UINT numFeatureLevels{ ARRAYSIZE( feature_levels_ ) };

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory( &sd, sizeof( sd ) );
        sd.BufferDesc.Width = width_;
        sd.BufferDesc.Height = height_;
        sd.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;
        sd.OutputWindow = hwnd;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = 0;
        
        for ( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex )
        {
            driver_type_ = driver_types_[driverTypeIndex];
            hr = D3D11CreateDeviceAndSwapChain(
                NULL, driver_type_, NULL,
                createDeviceFlags, feature_levels_, numFeatureLevels,
                D3D11_SDK_VERSION, &sd,
                &swap_chain_, &d3d_device_, &feature_level_, &immediate_context_
            );
            if ( SUCCEEDED( hr ) )
            {
                break;
            }
        }
        if ( FAILED( hr ) )
        {
            return hr;
        }

        ID3D11Texture2D* pBackBuffer = NULL;
        hr = swap_chain_->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        hr = d3d_device_->CreateRenderTargetView( pBackBuffer, NULL, &render_target_view_ );
        pBackBuffer->Release();
        if ( FAILED( hr ) )
        {
            return hr;
        }

        D3D11_TEXTURE2D_DESC descDepth;
        ZeroMemory( &descDepth, sizeof( descDepth ) );
        descDepth.Width = width_;
        descDepth.Height = height_;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = d3d_device_->CreateTexture2D( &descDepth, NULL, &depth_stencil_ );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof( descDSV ) );
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = d3d_device_->CreateDepthStencilView( depth_stencil_, &descDSV, &depth_stencil_view_ );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        immediate_context_->OMSetRenderTargets( 1, &render_target_view_, depth_stencil_view_ );

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width_;
        vp.Height = (FLOAT)height_;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        immediate_context_->RSSetViewports( 1, &vp );

        return hr;
    }

    HRESULT ResizeScreen( HWND hwnd, long new_width, long new_height )
    {
        if ( swap_chain_ == nullptr )
        {
            return S_OK;
        }

        width_ = new_width;
        height_ = new_height;

        immediate_context_->OMSetRenderTargets( 0, nullptr, nullptr );
        render_target_view_->Release();
        depth_stencil_->Release();
        depth_stencil_view_->Release();

        HRESULT hr{ S_OK };

        hr = swap_chain_->ResizeBuffers( 1, width_, height_, DXGI_FORMAT_R16G16B16A16_FLOAT, 0 );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        ID3D11Texture2D* pBackBuffer = NULL;
        hr = swap_chain_->GetBuffer( 0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        hr = d3d_device_->CreateRenderTargetView( pBackBuffer, NULL, &render_target_view_ );
        pBackBuffer->Release();
        if ( FAILED( hr ) )
        {
            return hr;
        }

        D3D11_TEXTURE2D_DESC descDepth;
        ZeroMemory( &descDepth, sizeof( descDepth ) );
        descDepth.Width = width_;
        descDepth.Height = height_;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        hr = d3d_device_->CreateTexture2D( &descDepth, NULL, &depth_stencil_ );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
        ZeroMemory( &descDSV, sizeof( descDSV ) );
        descDSV.Format = descDepth.Format;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        hr = d3d_device_->CreateDepthStencilView( depth_stencil_, &descDSV, &depth_stencil_view_ );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        immediate_context_->OMSetRenderTargets( 1, &render_target_view_, depth_stencil_view_ );

        D3D11_VIEWPORT vp;
        vp.Width = (FLOAT)width_;
        vp.Height = (FLOAT)height_;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        immediate_context_->RSSetViewports( 1, &vp );

        return hr;

    }

    void Clean()
    {

#define RLS_IF(x) do { if ( x ) x->Release(); x = nullptr; } while (false)
        RLS_IF( depth_stencil_ );
        RLS_IF( depth_stencil_view_ );
        RLS_IF( render_target_view_ );
        RLS_IF( swap_chain_ );
        RLS_IF( immediate_context_ );
        RLS_IF( d3d_device_ );
#undef RLS_IF

    }

    HRESULT Present()
    {
        return swap_chain_->Present( 0, 0 );
    }


    void Clear( const float* clear_color )
    {
        immediate_context_->ClearRenderTargetView( render_target_view_,
            clear_color );
        immediate_context_->ClearDepthStencilView( depth_stencil_view_,
            D3D11_CLEAR_DEPTH, 1.0f, 0 );
    }

    HRESULT CreateShaderAndInputLayout(
        LPCWSTR vs_name, ID3D11VertexShader** pp_vs, ID3D11InputLayout** pp_il,
        const D3D11_INPUT_ELEMENT_DESC* elem_desc, UINT elem_desc_sz,
        LPCWSTR ps_name, ID3D11PixelShader** pp_ps)
    {
        HRESULT hr{ S_OK };

        UINT vertex_shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( M_DEBUG )
        vertex_shader_flags |= D3DCOMPILE_DEBUG;
#endif

        static constexpr D3D_SHADER_MACRO defines[] = { NULL, NULL };
        ID3DBlob* vertex_shader_blob = nullptr;
        ID3DBlob* vertex_shader_error_blob = nullptr;
        hr = D3DCompileFromFile( vs_name, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VS", "vs_4_0", vertex_shader_flags, 0,
            &vertex_shader_blob, &vertex_shader_error_blob
        );
        if ( FAILED( hr ) )
        {
            if ( vertex_shader_error_blob )
            {
                OutputDebugStringA( (char*)vertex_shader_error_blob->GetBufferPointer() );
                vertex_shader_error_blob->Release();
            }
            if ( vertex_shader_blob )
            {
                vertex_shader_blob->Release();
            }
            MessageBox( NULL,
                L"The FX file cannot be compiled. Please run this executable "
                L"from the directory that contains the FX file.",
                L"Error", MB_OK );
            return hr;
        }
        hr = d3d_device_->CreateVertexShader(
            vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(),
            NULL, pp_vs
        );
        if ( FAILED( hr ) )
        {
            vertex_shader_blob->Release();
            return hr;
        }

        hr = d3d_device_->CreateInputLayout(
            elem_desc, elem_desc_sz,
            vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(),
            pp_il
        );
        vertex_shader_blob->Release();
        if (FAILED(hr))
        {
            return hr;
        }
        //immediate_context_->IASetInputLayout( input_layout1_ );


        ID3DBlob* pixel_shader_blob = nullptr;
        ID3DBlob* pixel_shader_error_blob = nullptr;
        hr = D3DCompileFromFile( ps_name, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "PS", "ps_4_0", vertex_shader_flags, 0,
            &pixel_shader_blob, &pixel_shader_error_blob
        );
        if ( FAILED( hr ) )
        {
            if ( pixel_shader_error_blob )
            {
                OutputDebugStringA( (char*)pixel_shader_error_blob->GetBufferPointer() );
                pixel_shader_error_blob->Release();
            }
            if ( pixel_shader_blob )
            {
                pixel_shader_blob->Release();
            }
            MessageBox( NULL,
                L"The FX file cannot be compiled. Please run this executable "
                L"from the directory that contains the FX file.",
                L"Error", MB_OK );
            return hr;
        }
        hr = d3d_device_->CreatePixelShader(
            pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(),
            NULL, pp_ps
        );
        if ( FAILED( hr ) )
        {
            vertex_shader_blob->Release();
            return hr;
        }

        return hr;
    }

    HRESULT CreateDefaultBuffer(
        ID3D11Buffer** pp_buffer, const void* buffer, UINT buffer_size, D3D11_BIND_FLAG flag
    )
    {
        HRESULT hr{ S_OK };

        D3D11_BUFFER_DESC buffer_desc;
        ZeroMemory( &buffer_desc, sizeof( buffer_desc ) );
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.ByteWidth = buffer_size;
        buffer_desc.BindFlags = flag;
        buffer_desc.CPUAccessFlags = 0;
        buffer_desc.StructureByteStride = 0;

        if ( buffer == nullptr )
        {
            hr = d3d_device_->CreateBuffer( &buffer_desc, nullptr, pp_buffer );
        }
        else
        {
            D3D11_SUBRESOURCE_DATA init_data1;
            ZeroMemory( &init_data1, sizeof( init_data1 ) );
            init_data1.pSysMem = buffer;

            hr = d3d_device_->CreateBuffer( &buffer_desc, &init_data1, pp_buffer );
        }
        if ( FAILED( hr ) )
        {
            return hr;
        }

        return hr;
    }


    HRESULT CreateComplexBuffer(
        ID3D11Buffer** pp_buffer, UINT buffer_size,
        D3D11_USAGE usage, D3D11_BIND_FLAG flag, UINT cpu_access_flags
    )
    {
        HRESULT hr{ S_OK };

        D3D11_BUFFER_DESC buffer_desc;
        ZeroMemory( &buffer_desc, sizeof( buffer_desc ) );
        buffer_desc.Usage = usage;
        buffer_desc.ByteWidth = buffer_size;
        buffer_desc.BindFlags = flag;
        buffer_desc.CPUAccessFlags = cpu_access_flags;
        buffer_desc.StructureByteStride = 0;

        hr = d3d_device_->CreateBuffer( &buffer_desc, nullptr, pp_buffer );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        return hr;
    }

    HRESULT CreateSampler(ID3D11SamplerState** pp_ss)
    {
        HRESULT hr{ S_OK };

        D3D11_SAMPLER_DESC sample_desc;
        ZeroMemory( &sample_desc, sizeof( sample_desc ) );
        sample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sample_desc.MinLOD = 0;
        sample_desc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = d3d_device_->CreateSamplerState( &sample_desc, pp_ss );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        return hr;
    }

    HRESULT CreateTextureFromImage(const wchar_t* image_name, ID3D11ShaderResourceView** pp_srv)
    {
        HRESULT hr{ S_OK };

        DirectX::ScratchImage image;
        hr = DirectX::LoadFromTGAFile( image_name, DirectX::TGA_FLAGS_DEFAULT_SRGB, nullptr, image );
        DirectX::CreateShaderResourceView(
            d3d_device_, image.GetImages(), image.GetImageCount(), image.GetMetadata(),
            pp_srv
        );

        if ( FAILED( hr ) )
        {
            return hr;
        }

        return hr;
    }

    HRESULT GetDeviceRemovedReason()
    {
        return d3d_device_->GetDeviceRemovedReason();
    }

}

