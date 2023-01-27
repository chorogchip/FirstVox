#include "VertexRenderer_for_Winmain.h"
#include "VertexRenderer.h"

#include <Windows.h>

#include "Macros.h"
#include "Consts.h"
#include "DXHeaders.h"
#include "Renderer_for_VertexRenderer.h"
#include"Logger.h"
#include "Timer.h"
#include "GameCore.h"

namespace vox::ren::vertex
{
    struct Vertex1
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 Color;
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

    struct CBChunkChangesOnResize
    {
        DirectX::XMMATRIX mat_projection;
    };
    struct CBChunkChangesEveryFrame
    {
        DirectX::XMMATRIX mat_view;
    };
    struct CBChunkChangesByChunk
    {
        DirectX::XMMATRIX mat_world;
    };

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



    static ID3D11VertexShader* chunk_vertex_shader_ = nullptr;
    static ID3D11PixelShader* chunk_pixel_shader_ = nullptr;
    static ID3D11InputLayout* chunk_input_layout_ = nullptr;

    static ID3D11Buffer* chunk_vertex_buffer_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_on_resize_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_every_frame_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_by_chunk_ = nullptr;

    static ID3D11ShaderResourceView* chunk_texture_rv_ = nullptr;
    static ID3D11SamplerState* chunk_sampler_point_ = nullptr;

    DirectX::XMMATRIX mat_chunk_world_;
    DirectX::XMMATRIX mat_chunk_view_;
    DirectX::XMMATRIX mat_chunk_projection_;

#define CATCH_RES(x) do { if ( FAILED( hr = (x) ) ) { return hr; } } while (false)
#define RLS_IF(x) do { if ( x ) x->Release(); x = nullptr; } while (false)


    HRESULT InitForTut( HWND h_wnd_ )
    {
        immediate_context_ = vox::ren::base::get_immediate_context();

        HRESULT hr{ S_OK };

        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC1[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        constexpr UINT NUM_ELEMENTS_DESC1 = ARRAYSIZE( VERTEX_DESC1 );

        CATCH_RES( vox::ren::base::CreateShaderAndInputLayout(
            L"Shaders/vs1.fx", &vertex_shader_, &input_layout1_,
            VERTEX_DESC1, NUM_ELEMENTS_DESC1,
            L"Shaders/ps1.fx", &pixel_shader_
        ) );
        immediate_context_->IASetInputLayout( input_layout1_ );

        static constexpr Vertex1 vertices1[] =
        {
            { DirectX::XMFLOAT3( 0.0f, 1.0f, 0.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 0.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 1.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 0.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( 0.0f, 0.0f, 0.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 0.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 0.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 1.0f, 0.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },

            { DirectX::XMFLOAT3( 0.0f, 0.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 0.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 0.0f ) },
            { DirectX::XMFLOAT3( 1.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 1.0f, 1.0f ) },
            { DirectX::XMFLOAT3( 0.0f, 1.0f, 1.0f ), DirectX::XMFLOAT2( 0.0f, 1.0f ) },
        };

        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &vertex_buffer1_, vertices1, sizeof( vertices1 ), D3D11_BIND_VERTEX_BUFFER
        ) );

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

        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &vertex_buffer1_indices_, indices, sizeof( indices ), D3D11_BIND_INDEX_BUFFER
        ) );
        immediate_context_->IASetIndexBuffer( vertex_buffer1_indices_, DXGI_FORMAT_R16_UINT, 0 );
        immediate_context_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &CB_changes_on_resize_, nullptr, sizeof( CBChangesOnResize ), D3D11_BIND_CONSTANT_BUFFER
        ) );
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &CB_changes_every_frame_, nullptr, sizeof( CBChangesEveryFrame ), D3D11_BIND_CONSTANT_BUFFER
        ) );


        CATCH_RES( vox::ren::base::CreateSampler(
            &sampler_point_
        ) );

        CATCH_RES( vox::ren::base::CreateTextureFromImage(
            L"Textures/dirt.tga", &texture_rv_
        ) );

        mat_projection_ = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4,
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            0.01f, 100.0f
        );
        CBChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_projection_ );
        immediate_context_->UpdateSubresource( CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );

        return hr;
    }

    void CleanForTut()
    {
        if ( immediate_context_ ) immediate_context_->ClearState();

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

    void RenderForTut()
    {
        static vox::util::Timer timer{};
        float t = (float)(timer.GetElapsedMicroSec().count() / 10000 % 1000);
        t *= DirectX::XM_2PI / 1000.0f;
        t = 0.0f;

        mat_world_ = DirectX::XMMatrixRotationRollPitchYaw( t, t * 2.0f, t * 3.0f );

        auto& cam = vox::core::gamecore::camera;
        auto mat_cam_rot_t = DirectX::XMMatrixTranspose( DirectX::XMMatrixRotationRollPitchYaw( cam.rotation[0], cam.rotation[1], cam.rotation[2] ) );
        auto mat_cam_pos_t = DirectX::XMMatrixIdentity();
        mat_cam_pos_t.r[3] = (- cam.position).m128;

        mat_view_ = DirectX::XMMatrixMultiply( mat_cam_pos_t, mat_cam_rot_t );

        CBChangesEveryFrame cb_changes_every_frame;
        cb_changes_every_frame.mat_world = DirectX::XMMatrixTranspose( mat_world_ );
        cb_changes_every_frame.mat_view = DirectX::XMMatrixTranspose( mat_view_ );
        immediate_context_->UpdateSubresource( CB_changes_every_frame_, 0, NULL, &cb_changes_every_frame, 0, 0 );

        immediate_context_->VSSetShader( vertex_shader_, NULL, 0 );
        immediate_context_->VSSetConstantBuffers( 1, 1, &CB_changes_on_resize_ );
        immediate_context_->VSSetConstantBuffers( 2, 1, &CB_changes_every_frame_ );
        immediate_context_->PSSetShader( pixel_shader_, NULL, 0 );
        immediate_context_->PSSetShaderResources( 0, 1, &texture_rv_ );
        immediate_context_->PSSetSamplers( 0, 1, &sampler_point_ );
        immediate_context_->DrawIndexed( 36, 0, 0 );
    }

    HRESULT InitForChunk( HWND h_wnd_ )
    {
        immediate_context_ = vox::ren::base::get_immediate_context();

        HRESULT hr{ S_OK };

        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC1[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        constexpr UINT NUM_ELEMENTS_DESC1 = ARRAYSIZE( VERTEX_DESC1 );

        CATCH_RES( vox::ren::base::CreateShaderAndInputLayout(
            L"Shaders/vs_chunk.fx", &chunk_vertex_shader_, &chunk_input_layout_,
            VERTEX_DESC1, NUM_ELEMENTS_DESC1,
            L"Shaders/ps_chunk.fx", &chunk_pixel_shader_
        ) );
        immediate_context_->IASetInputLayout( chunk_input_layout_ );
        immediate_context_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_on_resize_, nullptr, sizeof( CBChunkChangesOnResize ), D3D11_BIND_CONSTANT_BUFFER
        ) );
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_every_frame_, nullptr, sizeof( CBChunkChangesEveryFrame ), D3D11_BIND_CONSTANT_BUFFER
        ) );
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_by_chunk_, nullptr, sizeof( CBChunkChangesByChunk ), D3D11_BIND_CONSTANT_BUFFER
        ) );


        CATCH_RES( vox::ren::base::CreateSampler(
            &chunk_sampler_point_
        ) );

        CATCH_RES( vox::ren::base::CreateTextureFromImage(
            L"Textures/block_textures.tga", &chunk_texture_rv_
        ) );

        mat_chunk_projection_ = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4,
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            0.01f, 1000.0f
        );
        CBChunkChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_chunk_projection_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );

        return hr;
    }

    void CleanForChunk()
    {
        if ( immediate_context_ ) immediate_context_->ClearState();

        RLS_IF( chunk_sampler_point_ );
        RLS_IF( chunk_texture_rv_ );
        RLS_IF( chunk_CB_changes_by_chunk_ );
        RLS_IF( chunk_CB_changes_every_frame_ );
        RLS_IF( chunk_CB_changes_on_resize_ );
        RLS_IF( chunk_pixel_shader_ );
        RLS_IF( chunk_vertex_shader_ );
        RLS_IF( chunk_vertex_buffer_ );
        RLS_IF( chunk_input_layout_ );
    }

    void ResizeScreenForChunk()
    {
        if ( immediate_context_ == nullptr )
        {
            return;
        }

        mat_chunk_projection_ = DirectX::XMMatrixPerspectiveFovLH( DirectX::XM_PIDIV4,
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            0.01f, 1000.0f
        );

        CBChunkChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_chunk_projection_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );
    }

    void RenderForChunk()
    {
        auto& cam = vox::core::gamecore::camera;
        alignas(16) float vec_rot[4];
        vox::data::vector::Store( vec_rot, cam.rotation );
        auto mat_cam_rot_t = DirectX::XMMatrixTranspose( DirectX::XMMatrixRotationRollPitchYaw( -vec_rot[0], vec_rot[1], -vec_rot[2] ) );
        auto mat_cam_pos_t = DirectX::XMMatrixIdentity();
        mat_cam_pos_t.r[3] = vox::data::vector::Sub( vox::data::vector::Set( 0.0f, 0.0f, 0.0f, 1.0f ), cam.position ).m128;

        mat_chunk_view_ = DirectX::XMMatrixMultiply( mat_cam_pos_t, mat_cam_rot_t );

        CBChunkChangesEveryFrame cb_changes_every_frame;
        cb_changes_every_frame.mat_view = DirectX::XMMatrixTranspose( mat_chunk_view_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_every_frame_, 0, NULL, &cb_changes_every_frame, 0, 0 );

        immediate_context_->VSSetShader( chunk_vertex_shader_, NULL, 0 );
        immediate_context_->VSSetConstantBuffers( 1, 1, &chunk_CB_changes_on_resize_ );
        immediate_context_->VSSetConstantBuffers( 2, 1, &chunk_CB_changes_every_frame_ );
        immediate_context_->PSSetShader( chunk_pixel_shader_, NULL, 0 );
        immediate_context_->PSSetShaderResources( 0, 1, &chunk_texture_rv_ );
        immediate_context_->PSSetSamplers( 0, 1, &chunk_sampler_point_ );
    }

    void RenderChunk( float x, float y, float z, const void* pp_vertex_buffer, size_t vertex_size )
    {
        ID3D11Buffer* const* pp_vb = (ID3D11Buffer* const*)pp_vertex_buffer;
        const UINT stride = sizeof( VertexChunk );
        const UINT offset = 0;
        immediate_context_->IASetVertexBuffers( 0, 1, pp_vb, &stride, &offset );

        mat_chunk_world_ = DirectX::XMMatrixTranslation( x, y, z );

        CBChunkChangesByChunk cb_changes_by_chunk;
        cb_changes_by_chunk.mat_world = DirectX::XMMatrixTranspose( mat_chunk_world_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_by_chunk_, 0, NULL, &cb_changes_by_chunk, 0, 0 );

        immediate_context_->VSSetConstantBuffers( 3, 1, &chunk_CB_changes_by_chunk_ );

        immediate_context_->Draw( (UINT)vertex_size, 0 );
    }


    void MapVertex( void* p_vertex_buffer, const VertexChunk* vertex_chunk, size_t size )
    {
        ID3D11Buffer* p_vb = (ID3D11Buffer*)p_vertex_buffer;
        D3D11_MAPPED_SUBRESOURCE resource;
        ZeroMemory( &resource, sizeof( resource ) );

        immediate_context_->Map( p_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
        memcpy( resource.pData, vertex_chunk, sizeof(VertexChunk) * size );
        immediate_context_->Unmap( p_vb, 0 );
    }

    void CreateVertexBuffer(void* pp_vertex_buffer, size_t size)
    {
        HRESULT hr{ S_OK };
        ID3D11Buffer** pp_vb = (ID3D11Buffer**)pp_vertex_buffer;
        hr = vox::ren::base::CreateComplexBuffer(
            pp_vb, (UINT)(sizeof( VERTICES_BLOCK ) * size),
            D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE
        );
        if ( FAILED( hr ) )
        {
            M_LOGERROR( "vertex buffer creation failed" );
            vox::logger::GLogger << hr;
            vox::logger::GLogger.LogDebugString();
        }
    }

    void ReleaseVertexBuffer( void* p_vertex_buffer )
    {
        ID3D11Buffer* p_vb = (ID3D11Buffer*)p_vertex_buffer;
        p_vb->Release();
    }

#undef RLS_IF
#undef CATCH_RES

}