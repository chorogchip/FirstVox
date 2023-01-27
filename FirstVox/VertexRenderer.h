#pragma once

#include <DirectXMath.h>

namespace vox::ren::vertex
{
    struct VertexChunk
    {
        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT2 Tex;
    };

    inline constexpr float TEX_BL_SZ = 256.0f / 16.0f;

    inline constexpr VertexChunk VERTICES_BLOCK[] =
    {
        // up
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ) },

        // down
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },

        // front
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },

        // back
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },

        // right
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },

        // left
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ) },
    };

    void CreateVertexBuffer( void* pp_vertex_buffer, size_t size );
    void ReleaseVertexBuffer( void* p_vertex_buffer );
    void MapVertex( void* p_vertex_buffer, const VertexChunk* vertex_chunk, size_t size );
    void RenderChunk( float x, float y, float z, const void* pp_vertex_buffer, size_t vertex_size );
}