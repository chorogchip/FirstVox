#pragma once

#include <DirectXMath.h>

namespace vox::ren::vertex
{
    struct VertexChunk
    {
        unsigned int position;
        unsigned int light;
        unsigned int texcoord;
    };
    inline constexpr unsigned int VERTEX_CHUNK_STRIDE = sizeof( VertexChunk );
    inline constexpr unsigned int VERTEX_CHUNK_OFFSET = 0;

    inline constexpr float TEX_BL_SZ = 256.0f / 16.0f;

    //#define GEN_VC(x, y, z, n, u, v) { x << 24U | y << 14U | z << 6U | n, v << 16U | u }
#define GEN_VC(x, y, z, n, u, v) { x << 24U | y << 8U | z << 0U, 0U, v << 8U | u }

    inline constexpr VertexChunk VERTICES_BLOCK[] =
    {
        // up
        GEN_VC( 1, 1, 1, 0, 1, 1 ),
        GEN_VC( 0, 1, 1, 0, 0, 1 ),
        GEN_VC( 0, 1, 0, 0, 0, 0 ),
        GEN_VC( 1, 1, 0, 0, 1, 0 ),

        // down
        GEN_VC( 1, 0, 0, 1, 1, 1 ),
        GEN_VC( 0, 0, 0, 1, 0, 1 ),
        GEN_VC( 0, 0, 1, 1, 0, 0 ),
        GEN_VC( 1, 0, 1, 1, 1, 0 ),

        // front
        GEN_VC( 0, 1, 1, 2, 1, 1 ),
        GEN_VC( 1, 1, 1, 2, 0, 1 ),
        GEN_VC( 1, 0, 1, 2, 0, 0 ),
        GEN_VC( 0, 0, 1, 2, 1, 0 ),

        // back
        GEN_VC( 1, 1, 0, 3, 1, 1 ),
        GEN_VC( 0, 1, 0, 3, 0, 1 ),
        GEN_VC( 0, 0, 0, 3, 0, 0 ),
        GEN_VC( 1, 0, 0, 3, 1, 0 ),

        // right
        GEN_VC( 1, 1, 1, 4, 1, 1 ),
        GEN_VC( 1, 1, 0, 4, 0, 1 ),
        GEN_VC( 1, 0, 0, 4, 0, 0 ),
        GEN_VC( 1, 0, 1, 4, 1, 0 ),

        // left
        GEN_VC( 0, 1, 0, 5, 1, 1 ),
        GEN_VC( 0, 1, 1, 5, 0, 1 ),
        GEN_VC( 0, 0, 1, 5, 0, 0 ),
        GEN_VC( 0, 0, 0, 5, 1, 0 ),
    };
    /*
    inline constexpr VertexChunk VERTICES_BLOCK[] =
    {
        // up
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f ) },

        // down
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f, -1.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f, -1.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f, -1.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f, -1.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f, -1.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f, -1.0f,  0.0f ) },

        // front
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f,  1.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f,  1.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f,  1.0f ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f,  1.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f,  1.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f,  1.0f ) },

        // back
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f, -1.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f, -1.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f, -1.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f, -1.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f, -1.0f ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  0.0f,  0.0f, -1.0f ) },

        // right
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3(  0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3(  1.0f,  0.0f,  0.0f ) },

        // left
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3( -1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3( -1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3( -1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3( -1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f, -0.5f, -0.5f ), DirectX::XMFLOAT2( 1.0f / TEX_BL_SZ, 0.0f / TEX_BL_SZ ), DirectX::XMFLOAT3( -1.0f,  0.0f,  0.0f ) },
        { DirectX::XMFLOAT3( -0.5f,  0.5f,  0.5f ), DirectX::XMFLOAT2( 0.0f / TEX_BL_SZ, 1.0f / TEX_BL_SZ ), DirectX::XMFLOAT3( -1.0f,  0.0f,  0.0f ) },
    };*/
    inline constexpr int VERTICES_BLOCK_COUNT = sizeof( VERTICES_BLOCK ) / sizeof( VertexChunk );

    void CreateVertexBuffer( void* pp_vertex_buffer, size_t size );
    void ReleaseVertexBuffer( void* p_vertex_buffer );
    void MapVertex( void* p_vertex_buffer, const VertexChunk* vertex_chunk, size_t vertex_chunk_count );
    void RenderChunk( float x, float y, float z, const void* pp_vertex_buffer, size_t vertex_size );
}