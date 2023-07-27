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
#include "ChunkManager.h"
#include "EBlockID.h"

namespace vox::ren::vertex
{
    /*
    struct CBChunkChangesOnResize
    {
        DirectX::XMMATRIX mat_projection;
    };*/
    /*
    struct CBChunkChangesEveryFrame
    {
        DirectX::XMMATRIX mat_view;
        DirectX::XMVECTOR vec_sun;
        DirectX::XMVECTOR pos_cam;
    };*/

    struct CBChunkChangesEveryFrameVS
    {
        DirectX::XMMATRIX mat_view_proj;
    };
    struct CBChunkChangesEveryFramePS
    {
        DirectX::XMVECTOR vec_sky_color;
    };
    
    struct CBChunkChangesByChunk
    {
        DirectX::XMVECTOR chunk_pos;
        DirectX::XMVECTOR fog_values;
    };

    static ID3D11DeviceContext* immediate_context_ = nullptr;

    static ID3D11VertexShader* chunk_vertex_shader_ = nullptr;
    static ID3D11PixelShader* chunk_pixel_shader_ = nullptr;
    static ID3D11InputLayout* chunk_input_layout_ = nullptr;

    static ID3D11VertexShader* ui_vs_ = nullptr;
    static ID3D11PixelShader* ui_ps_ = nullptr;
    static ID3D11InputLayout* ui_input_layout_ = nullptr;

    static ID3D11Buffer* chunk_vertex_buffer_ = nullptr;
    //static ID3D11Buffer* chunk_CB_changes_on_resize_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_every_frame_vs_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_every_frame_ps_ = nullptr;
    static ID3D11Buffer* chunk_CB_changes_by_chunk_ = nullptr;

    static ID3D11Buffer* ui_buffer_ = nullptr;

    static ID3D11ShaderResourceView* chunk_texture_rv_ = nullptr;
    static ID3D11SamplerState* chunk_sampler_point_ = nullptr;

    DirectX::XMMATRIX mat_chunk_world_;
    DirectX::XMMATRIX mat_chunk_view_;
    DirectX::XMMATRIX mat_chunk_projection_;

    struct VERTEX_UI
    {
        float posX;
        float posY;
        unsigned uv;
    };
    static constexpr UINT VERTEX_UI_STRIDE = sizeof(VERTEX_UI);
    static constexpr UINT VERTEX_UI_OFFSET = 0;
    static constexpr UINT VERTEX_UI_COUNT = 12 * 6;
    static VERTEX_UI ui_vertices_[VERTEX_UI_COUNT] = {};
    static constexpr signed char TRI2_U[6] = { 0, 0, 1, 0, 1, 1 };
    static constexpr signed char TRI2_V[6] = { 1, 0, 0, 1, 0, 1 };

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
        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC_UI[] =
        {
            { "POSITIONX", 0, DXGI_FORMAT_R32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "POSITIONY", 0, DXGI_FORMAT_R32_FLOAT, 0, 4, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "UV", 0, DXGI_FORMAT_R32_UINT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        /*
        static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_DESC1[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEX",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };*/
        constexpr UINT NUM_ELEMENTS_DESC1 = ARRAYSIZE( VERTEX_DESC1 );
        constexpr UINT NUM_ELEMENTS_DESC_UI = ARRAYSIZE( VERTEX_DESC_UI );

        CATCH_RES( vox::ren::base::CreateShaderAndInputLayout(
            L"GameData/Shaders/vs_chunk.fx", &chunk_vertex_shader_, &chunk_input_layout_,
            VERTEX_DESC1, NUM_ELEMENTS_DESC1,
            L"GameData/Shaders/ps_chunk.fx", &chunk_pixel_shader_
        ) );
        CATCH_RES( vox::ren::base::CreateShaderAndInputLayout(
            L"GameData/Shaders/vs_2d.fx", &ui_vs_, &ui_input_layout_,
            VERTEX_DESC_UI, NUM_ELEMENTS_DESC_UI,
            L"GameData/Shaders/ps_2d.fx", &ui_ps_
        ) );
        immediate_context_->IASetInputLayout( chunk_input_layout_ );
        immediate_context_->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        /*
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_on_resize_, nullptr, sizeof( CBChunkChangesOnResize ), D3D11_BIND_CONSTANT_BUFFER
        ) );*/
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_every_frame_vs_, nullptr, sizeof( CBChunkChangesEveryFrameVS ), D3D11_BIND_CONSTANT_BUFFER
        ) );
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_every_frame_ps_, nullptr, sizeof( CBChunkChangesEveryFramePS ), D3D11_BIND_CONSTANT_BUFFER
        ) );
        CATCH_RES( vox::ren::base::CreateDefaultBuffer(
            &chunk_CB_changes_by_chunk_, nullptr, sizeof( CBChunkChangesByChunk ), D3D11_BIND_CONSTANT_BUFFER
        ) );
        
        CATCH_RES( vox::ren::base::CreateComplexBuffer(
            &ui_buffer_, sizeof( ui_vertices_ ), D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE
        ) );


        CATCH_RES( vox::ren::base::CreateSampler(
            &chunk_sampler_point_
        ) );

        CATCH_RES( vox::ren::base::CreateTextureFromImage(
            L"GameData/Textures/block_texture_atlas.tga", &chunk_texture_rv_
        ) );
        
        mat_chunk_projection_ = DirectX::XMMatrixPerspectiveFovLH( vox::core::gamecore::camera.GetFovY(),
            (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight(),
            vox::core::gamecore::camera.GetNearZ(), vox::core::gamecore::camera.GetFarZ()
        );
        /*
        CBChunkChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_chunk_projection_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );
        */
        /*
        VertexChunk block1_vertices[VERTICES_BLOCK_COUNT];
        memcpy( block1_vertices, VERTICES_BLOCK, sizeof( block1_vertices ) );
        for ( int i = 0; i < VERTICES_BLOCK_COUNT; ++i )
        {/* TODO
            constexpr float SUN_SIZE_MULTIPLIER = 0.05f;
            block1_vertices[i].Pos.x *= vox::consts::SUN_ALTITUDE * SUN_SIZE_MULTIPLIER;
            block1_vertices[i].Pos.y *= vox::consts::SUN_ALTITUDE * SUN_SIZE_MULTIPLIER;
            block1_vertices[i].Pos.z *= vox::consts::SUN_ALTITUDE * SUN_SIZE_MULTIPLIER;
            block1_vertices[i].Tex.x = 0.0f * (vox::consts::BLOCK_WID_PIX / vox::consts::TEX_BLOCK_WID_PIX);
            block1_vertices[i].Tex.y = 2.0f * (vox::consts::BLOCK_HEI_PIX / vox::consts::TEX_BLOCK_HEI_PIX);
        }
        immediate_context_->UpdateSubresource(block1_buffer_, 0, NULL, block1_vertices, 0, 0);*/
        return hr;
    }

    void Clean()
    {
        if ( immediate_context_ ) immediate_context_->ClearState();

        RLS_IF( ui_buffer_ );
        RLS_IF( chunk_sampler_point_ );
        RLS_IF( chunk_texture_rv_ );
        RLS_IF( chunk_CB_changes_by_chunk_ );
        RLS_IF( chunk_CB_changes_every_frame_vs_ );
        RLS_IF( chunk_CB_changes_every_frame_ps_ );
        //RLS_IF( chunk_CB_changes_on_resize_ );
        RLS_IF( chunk_pixel_shader_ );
        RLS_IF( chunk_vertex_shader_ );
        RLS_IF( chunk_vertex_buffer_ );
        RLS_IF( chunk_input_layout_ );
        RLS_IF( ui_vs_ );
        RLS_IF( ui_ps_ );
        RLS_IF( ui_input_layout_ );
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
        /*
        CBChunkChangesOnResize cb_changes_on_resize;
        cb_changes_on_resize.mat_projection = DirectX::XMMatrixTranspose( mat_chunk_projection_ );
        immediate_context_->UpdateSubresource( chunk_CB_changes_on_resize_, 0, NULL, &cb_changes_on_resize, 0, 0 );*/
    }

    static vox::data::Vector4f CamPos;
    static float fogEnd, fogStart, fog_a, fog_b;

    void StartRenderChunks( float delta_time, float* sky_color )
    {
        vox::ren::base::SetDepthTest(true);

        const auto cam_pos_delta = vox::data::vector::Add(
            vox::core::gamecore::camera.entity.GetPositionVec(), vox::data::vector::Mul(
                vox::core::gamecore::camera.entity.GetSpeedVec(), vox::data::vector::SetBroadcast(
                    delta_time ) ) );
        CamPos = cam_pos_delta;
        alignas(16) float vec_rot[4];
        vox::data::vector::Store( vec_rot, vox::core::gamecore::camera.entity.GetEulerRotationVec() );
        auto mat_cam_rot_t = DirectX::XMMatrixTranspose( DirectX::XMMatrixRotationRollPitchYaw( -vec_rot[0], -vec_rot[1], -vec_rot[2] ) );
        auto mat_cam_pos_t = DirectX::XMMatrixIdentity();
        mat_cam_pos_t.r[3] = vox::data::vector::Sub( vox::data::vector::Set( 0.0f, 0.0f, 0.0f, 1.0f ), cam_pos_delta );

        mat_chunk_view_ = DirectX::XMMatrixMultiply( mat_cam_pos_t, mat_cam_rot_t );
        /*
        immediate_context_->IASetVertexBuffers( 0, 1, &block1_buffer_, &VERTEX_CHUNK_STRIDE, &VERTEX_CHUNK_OFFSET );
        const auto sun_vec = vox::core::gamecore::GetSunVec();
        */
        CBChunkChangesEveryFrameVS cb_changes_every_frame_vs;
        cb_changes_every_frame_vs.mat_view_proj = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixMultiply( mat_chunk_view_, mat_chunk_projection_ ) );
        immediate_context_->UpdateSubresource( chunk_CB_changes_every_frame_vs_, 0, NULL, &cb_changes_every_frame_vs, 0, 0 );
        
        CBChunkChangesEveryFramePS cb_changes_everv_frame_ps;
        memcpy(cb_changes_everv_frame_ps.vec_sky_color.m128_f32, sky_color, sizeof(float) * 4);
        immediate_context_->UpdateSubresource( chunk_CB_changes_every_frame_ps_, 0, NULL, &cb_changes_everv_frame_ps, 0, 0 );


        immediate_context_->IASetInputLayout( chunk_input_layout_ );
        immediate_context_->VSSetShader( chunk_vertex_shader_, NULL, 0 );
        //immediate_context_->VSSetConstantBuffers( 1, 1, &chunk_CB_changes_on_resize_ );
        immediate_context_->VSSetConstantBuffers( 0, 1, &chunk_CB_changes_every_frame_vs_ );
        immediate_context_->PSSetShader( chunk_pixel_shader_, NULL, 0 );
        immediate_context_->PSSetShaderResources( 0, 1, &chunk_texture_rv_ );
        immediate_context_->PSSetSamplers( 0, 1, &chunk_sampler_point_ );
        immediate_context_->PSSetConstantBuffers( 2, 1, &chunk_CB_changes_every_frame_ps_ );


        fogEnd = (float)(vox::core::chunkmanager::GetRenderChunkDist() * vox::consts::CHUNK_X - vox::consts::CHUNK_X);
        fogStart = (fogEnd - (float)vox::consts::CHUNK_X) * 0.9f;
        fogEnd *= fogEnd;
        fogStart *= fogStart;
        fog_a = 1.0f / (fogEnd - fogStart);
        fog_b = -fog_a * fogStart;

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

        //mat_chunk_world_ = DirectX::XMMatrixTranslation( x, y, z );

        const float dx0 = x - CamPos.m128_f32[0];
        const float dx1 = x + vox::consts::CHUNK_X - CamPos.m128_f32[0];
        const float dz0 = z - CamPos.m128_f32[2];
        const float dz1 = z + vox::consts::CHUNK_Z - CamPos.m128_f32[2];

        const float dfog00 = dx0 * dx0 + dz0 * dz0;
        const float dfog01 = dx1 * dx1 + dz0 * dz0;
        const float dfog10 = dx0 * dx0 + dz1 * dz1;
        const float dfog11 = dx1 * dx1 + dz1 * dz1;

        const auto fogs_vec =  vox::data::vector::Set(dfog00, dfog01, dfog10, dfog11);

        CBChunkChangesByChunk cb_changes_by_chunk;
        cb_changes_by_chunk.chunk_pos = vox::data::vector::Set(x, y, z, 0.0f);
        static_assert(vox::consts::CHUNK_X == 32);  // change 32 in fog calculation in VS
        cb_changes_by_chunk.fog_values =
            vox::data::vector::Mul( vox::data::vector::SetBroadcast((1.0f / vox::consts::CHUNK_X / vox::consts::CHUNK_X)), 
                vox::data::vector::Min( vox::data::vector::SetBroadcast(1.0f),
                    vox::data::vector::Max( vox::data::vector::SetZero4f(),
                        vox::data::vector::Add( vox::data::vector::SetBroadcast(fog_b), 
                            vox::data::vector::Mul( vox::data::vector::SetBroadcast(fog_a), fogs_vec )))));
        immediate_context_->UpdateSubresource( chunk_CB_changes_by_chunk_, 0, NULL, &cb_changes_by_chunk, 0, 0 );

        immediate_context_->VSSetConstantBuffers( 1, 1, &chunk_CB_changes_by_chunk_ );

        immediate_context_->Draw( (UINT)vertex_size, 0 );
    }

    void RenderUI()
    {
        const auto width = vox::ren::base::GetScreenWidth();
        const auto height = vox::ren::base::GetScreenHeight();
        const float pix_x = 1.0f / (float)width;
        const float pix_y = 1.0f / (float)height;
        // cursor
        for (int j = 0; j < 6; ++j)
        {
            ui_vertices_[j].posX = (float)(TRI2_U[j] * 64) * pix_x - 32.0f * pix_x;
            ui_vertices_[j].posY = (float)(-TRI2_V[j] * 64) * pix_y + 32.0f * pix_y;
            ui_vertices_[j].uv = (1 - TRI2_V[j] + 15) << 16 | TRI2_U[j];
        }
        // hand pos
        for (int j = 0; j < 6; ++j)
        {
            const int hand = vox::core::gamecore::hand_pos;
            ui_vertices_[6 + j].posX = (float)(TRI2_U[j] * 64.0f) * pix_x
                + hand * 80.0f * pix_x
                - (5 * 80.0f - 8.0f) * pix_x;
            ui_vertices_[6 + j].posY = (float)(-TRI2_V[j] * 64.0f) * pix_y
                - 0.75f + 64.0f * pix_y;
            ui_vertices_[6 + j].uv = (TRI2_V[j] + 15) << 16 | TRI2_U[j] + 1;
        }
        // hand item
        for (int i = 2; i < 12; ++i)
            for (int j = 0; j < 6; ++j)
            {
                const auto block_id = vox::core::gamecore::hand_blocks[i - 2];
                const unsigned item_uv = (unsigned)vox::data::GetTexturePos(block_id);
                const unsigned item_u = item_uv & 0xffU;
                const unsigned item_v = item_uv >> 8U;
                ui_vertices_[i * 6 + j].posX = (float)(TRI2_U[j] * 64.0f) * pix_x
                    + (i - 2) * 80.0f * pix_x
                    - (5 * 80.0f - 8.0f) * pix_x;
                ui_vertices_[i * 6 + j].posY = (float)(-TRI2_V[j] * 64.0f) * pix_y
                    - 0.75f;
                ui_vertices_[i * 6 + j].uv = (TRI2_V[j] + item_v) << 16 | TRI2_U[j] + item_u;
            }

        {
            D3D11_MAPPED_SUBRESOURCE resource;
            ZeroMemory( &resource, sizeof( resource ) );
            immediate_context_->Map( ui_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource );
            memcpy( resource.pData, ui_vertices_, sizeof(ui_vertices_) );
            immediate_context_->Unmap( ui_buffer_, 0 );
        }

        vox::ren::base::SetDepthTest(false);
        immediate_context_->IASetInputLayout( ui_input_layout_ );
        immediate_context_->VSSetShader( ui_vs_, NULL, 0 );
        immediate_context_->PSSetShader( ui_ps_, NULL, 0 );
        immediate_context_->PSSetShaderResources( 0, 1, &chunk_texture_rv_ );
        immediate_context_->PSSetSamplers( 0, 1, &chunk_sampler_point_ );
        immediate_context_->IASetVertexBuffers( 0, 1, &ui_buffer_, &VERTEX_UI_STRIDE, &VERTEX_UI_OFFSET );
        immediate_context_->Draw( VERTEX_UI_COUNT, 0 );
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