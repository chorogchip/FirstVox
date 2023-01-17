#include "VertexRenderer_for_Winmain.h"
#include "VertexRenderer.h"

#include <Windows.h>

#include "Macros.h"
#include "DXHeaders.h"
#include "Renderer_for_VertexRenderer.h"

#include "Timer.h"
#include "GameCore.h"

namespace vox::ren::vertex
{
    struct Vertex1
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 Color;
    };
    struct Vertex2
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT3 Tex0;
        DirectX::XMFLOAT3 Tex1;
    };

    struct CBNeverChanges
    {
        //DirectX::XMMATRIX mat_view;
    };

    struct CBChangesOnResize
    {
        DirectX::XMMATRIX mat_projection;
    };

    struct CBChangesEveryFrame
    {
        DirectX::XMMATRIX mat_world;
        DirectX::XMMATRIX mat_view;
    };

    static ID3D11Device* d3d_device_ = nullptr;
    static ID3D11DeviceContext* immediate_context_ = nullptr;

    static ID3D11VertexShader* vertex_shader_ = nullptr;
    static ID3D11PixelShader* pixel_shader_ = nullptr;
    static ID3D11InputLayout* input_layout1_ = nullptr;

    static ID3D11Buffer* vertex_buffer1_ = nullptr;
    static ID3D11Buffer* vertex_buffer1_indices_ = nullptr;
    static ID3D11Buffer* CB_never_changes_ = nullptr;
    static ID3D11Buffer* CB_changes_on_resize_ = nullptr;
    static ID3D11Buffer* CB_changes_every_frame_ = nullptr;

    static ID3D11ShaderResourceView* texture_rv_ = nullptr;
    static ID3D11SamplerState* sampler_point_ = nullptr;

    DirectX::XMMATRIX mat_world_;
    DirectX::XMMATRIX mat_view_;
    DirectX::XMMATRIX mat_projection_;

    HRESULT Init( HWND h_wnd_ )
    {
        d3d_device_ = vox::ren::base::get_device();
        immediate_context_ = vox::ren::base::get_immediate_context();

        HRESULT hr{ S_OK };

        UINT vertex_shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( M_DEBUG )
        vertex_shader_flags |= D3DCOMPILE_DEBUG;
#endif

        const LPCSTR profile = ( d3d_device_->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "vs_4_0" : "vs_4_0";
        static constexpr D3D_SHADER_MACRO defines[] = 
        {
            //"EXAMPLE_DEFINE", "1",
            NULL, NULL
        };
        ID3DBlob* vertex_shader_blob = nullptr;
        ID3DBlob* vertex_shader_error_blob = nullptr;
        hr = D3DCompileFromFile( L"Shaders/vs1.fx", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "VS", profile, vertex_shader_flags, 0,
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
            NULL, &vertex_shader_
        );
        if ( FAILED( hr ) )
        {
            vertex_shader_blob->Release();
            return hr;
        }

        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC1[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        constexpr UINT NUM_ELEMENTS_DESC1 = ARRAYSIZE( VERTEX_DESC1 );

        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC2[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        constexpr UINT NUM_ELEMENTS_DESC2 = ARRAYSIZE( VERTEX_DESC2 );

        hr = d3d_device_->CreateInputLayout(
            VERTEX_DESC1, NUM_ELEMENTS_DESC1,
            vertex_shader_blob->GetBufferPointer(), vertex_shader_blob->GetBufferSize(),
            &input_layout1_
        );
        vertex_shader_blob->Release();
        if (FAILED(hr))
        {
            return hr;
        }
        immediate_context_->IASetInputLayout( input_layout1_ );


        ID3DBlob* pixel_shader_blob = nullptr;
        ID3DBlob* pixel_shader_error_blob = nullptr;
        hr = D3DCompileFromFile( L"Shaders/ps1.fx", defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
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
            NULL, &pixel_shader_
        );
        if ( FAILED( hr ) )
        {
            vertex_shader_blob->Release();
            return hr;
        }

        static constexpr Vertex1 vertices1[] =
        {
            { DirectX::XMFLOAT3( -1.0f, 1.0f, -1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( -1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, -1.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( -1.0f, -1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( -1.0f, -1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( -1.0f, 1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( -1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( 1.0f, -1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( -1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, -1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( -1.0f, 1.0f, -1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( -1.0f, -1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, -1.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( -1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },
        };

        D3D11_BUFFER_DESC buffer_desc1;
        ZeroMemory( &buffer_desc1, sizeof( buffer_desc1 ) );
        buffer_desc1.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc1.ByteWidth = sizeof( vertices1 );
        buffer_desc1.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc1.CPUAccessFlags = 0;
        buffer_desc1.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA init_data1;
        ZeroMemory( &init_data1, sizeof( init_data1 ) );
        init_data1.pSysMem = vertices1;

        hr = d3d_device_->CreateBuffer( &buffer_desc1, &init_data1, &vertex_buffer1_ );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        const UINT stride = sizeof( Vertex1 );
        const UINT offset = 0;
        immediate_context_->IASetVertexBuffers( 0, 1, &vertex_buffer1_, &stride, &offset );

        static constexpr WORD indices[] =
        {
            3,1,0,
            2,1,3,

            6,4,5,
            7,4,6,

            11,9,8,
            10,9,11,

            14,12,13,
            15,12,14,

            19,17,16,
            18,17,19,

            22,20,21,
            23,20,22
        };

        D3D11_BUFFER_DESC buffer_desc1_indices;
        ZeroMemory( &buffer_desc1_indices, sizeof( buffer_desc1_indices ) );
        buffer_desc1_indices.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc1_indices.ByteWidth = sizeof( indices );
        buffer_desc1_indices.BindFlags = D3D11_BIND_INDEX_BUFFER;
        buffer_desc1_indices.CPUAccessFlags = 0;
        buffer_desc1_indices.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA init_data1_indices;
        ZeroMemory( &init_data1_indices, sizeof( init_data1_indices ) );
        init_data1_indices.pSysMem = indices;

        hr = d3d_device_->CreateBuffer(
            &buffer_desc1_indices, &init_data1_indices, &vertex_buffer1_indices_
        );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        immediate_context_->IASetIndexBuffer( vertex_buffer1_indices_, DXGI_FORMAT_R16_UINT, 0 );

        immediate_context_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        D3D11_BUFFER_DESC buffer_desc_cb;
        ZeroMemory( &buffer_desc_cb, sizeof( buffer_desc_cb ) );
        buffer_desc_cb.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc_cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer_desc_cb.CPUAccessFlags = 0;
        buffer_desc_cb.StructureByteStride = 0;

        /*buffer_desc_cb.ByteWidth = sizeof( CBNeverChanges );
        hr = d3d_device_->CreateBuffer(
            &buffer_desc_cb, NULL, &CB_never_changes_
        );
        if ( FAILED( hr ) )
        {
            return hr;
        }*/

        buffer_desc_cb.ByteWidth = sizeof( CBChangesOnResize );
        hr = d3d_device_->CreateBuffer(
            &buffer_desc_cb, NULL, &CB_changes_on_resize_
        );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        buffer_desc_cb.ByteWidth = sizeof( CBChangesEveryFrame );
        hr = d3d_device_->CreateBuffer(
            &buffer_desc_cb, NULL, &CB_changes_every_frame_
        );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        

        D3D11_SAMPLER_DESC sample_desc;
        ZeroMemory( &sample_desc, sizeof( sample_desc ) );
        sample_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sample_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sample_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sample_desc.MinLOD = 0;
        sample_desc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = d3d_device_->CreateSamplerState( &sample_desc, &sampler_point_ );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        DirectX::ScratchImage image;
        hr = DirectX::LoadFromTGAFile( L"Textures/dirt.tga", DirectX::TGA_FLAGS_NONE, nullptr, image );
        DirectX::CreateShaderResourceView(
            d3d_device_, image.GetImages(), image.GetImageCount(), image.GetMetadata(),
            &texture_rv_
        );
        if ( FAILED( hr ) )
        {
            return hr;
        }

        mat_world_ = DirectX::XMMatrixIdentity();

        //CBNeverChanges cb_never_changes;

        /*DirectX::XMVECTOR eye = DirectX::XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
        DirectX::XMVECTOR at = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        DirectX::XMVECTOR up = DirectX::XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
        auto mat_cam_look = DirectX::XMMatrixLookAtLH( eye, at, up );*/
        //immediate_context_->UpdateSubresource( CB_never_changes_, 0, NULL, &cb_never_changes, 0, 0 );

        mat_projection_ = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4,
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            0.01f, 100.0f
        );

        CBChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_projection_ );
        immediate_context_->UpdateSubresource( CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );

        return hr;
    }

    void Clean()
    {

        if ( immediate_context_ ) immediate_context_->ClearState();

#define RLS_IF(x) do { if ( x ) x->Release(); x = nullptr; } while (false)
        RLS_IF( sampler_point_ );
        RLS_IF( texture_rv_ );
        RLS_IF( CB_changes_every_frame_ );
        RLS_IF( CB_changes_on_resize_ );
        RLS_IF( CB_never_changes_ );
        RLS_IF( pixel_shader_ );
        RLS_IF( vertex_shader_ );
        RLS_IF( vertex_buffer1_ );
        RLS_IF( vertex_buffer1_indices_ );
        RLS_IF( input_layout1_ );
#undef RLS_IF

    }

    void ResizeScreen()
    {
        if ( immediate_context_ == nullptr )
        {
            return;
        }

        mat_projection_ = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4,
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            0.01f, 100.0f
        );

        CBChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_projection_ );
        immediate_context_->UpdateSubresource( CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );
    }

    void Render1()
    {
        static vox::util::Timer timer{};
        float t = (float)(timer.GetElapsedMicroSec().count() / 10000 % 1000);
        t *= DirectX::XM_2PI / 1000.0f;
        t = 0.0f;

        mat_world_ = DirectX::XMMatrixRotationRollPitchYaw( t, t * 2.0f, t * 3.0f );

        auto& cam = vox::core::gamecore::camera;
        auto mat_cam_rot_t = DirectX::XMMatrixTranspose( DirectX::XMMatrixRotationRollPitchYaw( cam.rotation[0], cam.rotation[1], cam.rotation[2]));
        auto mat_cam_pos_t = DirectX::XMMatrixTranslation( -cam.position[0], -cam.position[1], -cam.position[2]);

        mat_view_ = DirectX::XMMatrixMultiply( mat_cam_pos_t, mat_cam_rot_t  );

        CBChangesEveryFrame cb_changes_every_frame;
        cb_changes_every_frame.mat_world = DirectX::XMMatrixTranspose( mat_world_ );
        cb_changes_every_frame.mat_view = DirectX::XMMatrixTranspose( mat_view_ );
        immediate_context_->UpdateSubresource( CB_changes_every_frame_, 0, NULL, &cb_changes_every_frame, 0, 0 );

        immediate_context_->VSSetShader( vertex_shader_, NULL, 0 );
        //immediate_context_->VSSetConstantBuffers( 0, 1, &CB_never_changes_ );
        immediate_context_->VSSetConstantBuffers( 1, 1, &CB_changes_on_resize_ );
        immediate_context_->VSSetConstantBuffers( 2, 1, &CB_changes_every_frame_ );
        immediate_context_->PSSetShader( pixel_shader_, NULL, 0 );
        immediate_context_->PSSetShaderResources( 0, 1, &texture_rv_ );
        immediate_context_->PSSetSamplers( 0, 1, &sampler_point_ );
        immediate_context_->DrawIndexed( 36, 0, 0 );
    }
}