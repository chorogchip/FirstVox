#include "VertexRenderer_for_Winmain.h"
#include "VertexRenderer.h"

#include <Windows.h>

#include "FirstVoxHeader.h"
#include "Marker.h"
#include "Macros.h"
#include "Consts.h"
#include "ConstsTime.h"
#include "DXHeaders.h"
#include "Renderer_for_VertexRenderer.h"
#include"Logger.h"
#include "Timer.h"
#include "GameCore.h"

namespace vox::ren::vertex
{

    struct CBChunkChangesOnResize
    {
        DirectX::XMMATRIX mat_projection;
    };
    struct CBChunkChangesEveryFrame
    {
        DirectX::XMMATRIX mat_view;
        DirectX::XMVECTOR vec_sun;
        DirectX::XMVECTOR pos_cam;
    };
    struct CBChunkChangesByChunk
    {
        DirectX::XMMATRIX mat_world;
    };

    static ID3D11DeviceContext* immediate_context_ = nullptr;

    static ID3D11VertexShader* chunk_vertex_shader_ = nullptr;
    static ID3D11PixelShader* chunk_pixel_shader_ = nullptr;
    static ID3D11InputLayout* chunk_input_layout_ = nullptr;

    static ID3D11Buffer* chunk_vertex_buffer_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_on_resize_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_every_frame_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_by_chunk_ = nullptr;
    static ID3D11Buffer* block1_buffer_ = nullptr;

    static ID3D11ShaderResourceView* chunk_texture_rv_ = nullptr;
    static ID3D11SamplerState* chunk_sampler_point_ = nullptr;

    DirectX::XMMATRIX mat_chunk_world_;
    DirectX::XMMATRIX mat_chunk_view_;
    DirectX::XMMATRIX mat_chunk_projection_;

#define CATCH_RES(x) do { if ( FAILED( hr = (x) ) ) { return hr; } } while (false)
#define RLS_IF(x) do { if ( x ) x->Release(); x = nullptr; } while (false)

    HRESULT Init( HWND h_wnd_ )
    {
        immediate_context_ = vox::ren::base::get_immediate_context();

        HRESULT hr{ S_OK };

        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC1[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "LIGHT", 0, DXGI_FORMAT_R32_UINT, 0, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32_UINT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        /*
        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC1[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };*/
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

        CATCH_RES( vox::ren::base::CreateComplexBuffer(
            &block1_buffer_, sizeof( VERTICES_BLOCK ), D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0
        ) );


        CATCH_RES( vox::ren::base::CreateSampler(
            &chunk_sampler_point_
        ) );

        CATCH_RES( vox::ren::base::CreateTextureFromImage(
            L"Textures/block_textures.tga", &chunk_texture_rv_
        ) );
        
        mat_chunk_projection_ = DirectX::XMMatrixPerspectiveFovLH( vox::core::gamecore::camera.GetFovY(),
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            vox::core::gamecore::camera.GetNearZ(), vox::core::gamecore::camera.GetFarZ()
        );
        CBChunkChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_chunk_projection_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );

        VertexChunk block1_vertices[VERTICES_BLOCK_COUNT];
        memcpy( block1_vertices, VERTICES_BLOCK, sizeof( block1_vertices ) );
        for ( int i = 0; i < VERTICES_BLOCK_COUNT; ++i )
        {/* TODO
            constexpr float SUN_SIZE_MULTIPLIER = 0.05f;
            block1_vertices[i].Pos.x *= vox::consts::SUN_ALTITUDE * SUN_SIZE_MULTIPLIER;
            block1_vertices[i].Pos.y *= vox::consts::SUN_ALTITUDE * SUN_SIZE_MULTIPLIER;
            block1_vertices[i].Pos.z *= vox::consts::SUN_ALTITUDE * SUN_SIZE_MULTIPLIER;
            block1_vertices[i].Tex.x = 0.0f * (vox::consts::BLOCK_WID_PIX / vox::consts::TEX_BLOCK_WID_PIX);
            block1_vertices[i].Tex.y = 2.0f * (vox::consts::BLOCK_HEI_PIX / vox::consts::TEX_BLOCK_HEI_PIX);*/
        }
        immediate_context_->UpdateSubresource(block1_buffer_, 0, NULL, block1_vertices, 0, 0);
        return hr;
    }

    void Clean()
    {
        if ( immediate_context_ ) immediate_context_->ClearState();

        RLS_IF( block1_buffer_ );
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

    void ResizeScreen()
    {
        if ( immediate_context_ == nullptr )
        {
            return;
        }

        mat_chunk_projection_ = DirectX::XMMatrixPerspectiveFovLH( vox::core::gamecore::camera.GetFovY(),
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            vox::core::gamecore::camera.GetNearZ(), vox::core::gamecore::camera.GetFarZ()
        );

        CBChunkChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_chunk_projection_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );
    }

    void StartRenderChunks( float delta_time )
    {
        const auto cam_pos_delta = vox::data::vector::Add(
            vox::core::gamecore::camera.entity.GetPositionVec(), vox::data::vector::Mul(
                vox::core::gamecore::camera.entity.GetSpeedVec(), vox::data::vector::SetBroadcast(
                    delta_time ) ) );
        alignas(16) float vec_rot[4];
        vox::data::vector::Store( vec_rot, vox::core::gamecore::camera.entity.GetEulerRotationVec() );
        auto mat_cam_rot_t = DirectX::XMMatrixTranspose( DirectX::XMMatrixRotationRollPitchYaw( -vec_rot[0], -vec_rot[1], -vec_rot[2] ) );
        auto mat_cam_pos_t = DirectX::XMMatrixIdentity();
        mat_cam_pos_t.r[3] = vox::data::vector::Sub( vox::data::vector::Set( 0.0f, 0.0f, 0.0f, 1.0f ), cam_pos_delta );

        mat_chunk_view_ = DirectX::XMMatrixMultiply( mat_cam_pos_t, mat_cam_rot_t );

        immediate_context_->IASetVertexBuffers( 0, 1, &block1_buffer_, &VERTEX_CHUNK_STRIDE, &VERTEX_CHUNK_OFFSET );
        const auto sun_vec = vox::core::gamecore::GetSunVec();

        CBChunkChangesEveryFrame cb_changes_every_frame;
        cb_changes_every_frame.mat_view = DirectX::XMMatrixTranspose( mat_chunk_view_ );
        cb_changes_every_frame.vec_sun = sun_vec;
        cb_changes_every_frame.pos_cam = cam_pos_delta;
        immediate_context_->UpdateSubresource( chunk_CB_changes_every_frame_, 0, NULL, &cb_changes_every_frame, 0, 0 );

        immediate_context_->VSSetShader( chunk_vertex_shader_, NULL, 0 );
        immediate_context_->VSSetConstantBuffers( 1, 1, &chunk_CB_changes_on_resize_ );
        immediate_context_->VSSetConstantBuffers( 2, 1, &chunk_CB_changes_every_frame_ );
        immediate_context_->PSSetShader( chunk_pixel_shader_, NULL, 0 );
        immediate_context_->PSSetShaderResources( 0, 1, &chunk_texture_rv_ );
        immediate_context_->PSSetSamplers( 0, 1, &chunk_sampler_point_ );
        immediate_context_->PSSetConstantBuffers( 2, 1, &chunk_CB_changes_every_frame_ );

        /*
        // draw sun
        alignas(16) float sun_pos_arr[4];
        const auto sun_pos = vox::data::vector::Add( cam_pos_delta,
            vox::data::vector::Mul( sun_vec, vox::data::vector::SetBroadcast( vox::consts::SUN_ALTITUDE ) ) );
        vox::data::vector::Store( sun_pos_arr, sun_pos );
        mat_chunk_world_ = DirectX::XMMatrixTranslation( sun_pos_arr[0], sun_pos_arr[1], sun_pos_arr[2] );
        CBChunkChangesByChunk cb_changes_by_chunk;
        cb_changes_by_chunk.mat_world = DirectX::XMMatrixTranspose( mat_chunk_world_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_by_chunk_, 0, NULL, &cb_changes_by_chunk, 0, 0 );
        immediate_context_->VSSetConstantBuffers( 3, 1, &chunk_CB_changes_by_chunk_ );
        immediate_context_->Draw( VERTICES_BLOCK_COUNT, 0 );*/
    }

    void RenderChunk( float x, float y, float z, const void* pp_vertex_buffer, size_t vertex_size )
    {
        immediate_context_->IASetVertexBuffers( 0, 1, (ID3D11Buffer* const*)pp_vertex_buffer, &VERTEX_CHUNK_STRIDE, &VERTEX_CHUNK_OFFSET );

        mat_chunk_world_ = DirectX::XMMatrixTranslation( x, y, z );

        CBChunkChangesByChunk cb_changes_by_chunk;
        cb_changes_by_chunk.mat_world = DirectX::XMMatrixTranspose( mat_chunk_world_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_by_chunk_, 0, NULL, &cb_changes_by_chunk, 0, 0 );

        immediate_context_->VSSetConstantBuffers( 3, 1, &chunk_CB_changes_by_chunk_ );

        immediate_context_->Draw( (UINT)vertex_size, 0 );
    }


    void MapVertex( void* p_vertex_buffer, const VertexChunk* vertex_chunk, size_t vertex_chunk_count )
    {
        ID3D11Buffer* p_vb = (ID3D11Buffer*)p_vertex_buffer;
        D3D11_MAPPED_SUBRESOURCE resource;
        ZeroMemory( &resource, sizeof( resource ) );

        HRESULT hr{ S_OK };
        immediate_context_->Map( p_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
        memcpy( resource.pData, vertex_chunk, sizeof(VertexChunk) * vertex_chunk_count );
        immediate_context_->Unmap( p_vb, 0 );
    }

    void CreateVertexBuffer(void* pp_vertex_buffer, size_t size)
    {
        HRESULT hr{ S_OK };
        ID3D11Buffer** pp_vb = (ID3D11Buffer**)pp_vertex_buffer;
        hr = vox::ren::base::CreateComplexBuffer(
            pp_vb, (UINT)size,
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