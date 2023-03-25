#include "ChunkVertexBuffer.h"

#include <vector>

#include "FirstVoxHeader.h"
#include "Macros.h"
#include "Consts.h"
#include "Logger.h"
#include "EnumSide.h"
#include "QueueFor1WorkerThread.h"
#include "VertexRenderer.h"

namespace vox::data
{
    static QueueFor1WorkerThread<void*, 64> gpu_buf_reuse_buf;

    ChunkVertexBuffer::ChunkVertexBuffer() :
        vertex_buffer_{
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}},
            {ChunkVertexUnit{nullptr, 0}}, }
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vox::ren::vertex::CreateVertexBuffer(
                &(this->vertex_buffer_[i].front().vertex_buffer),
                sizeof( vox::ren::vertex::VertexChunk ) * vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
            );
        }
    }

    ChunkVertexBuffer::~ChunkVertexBuffer()
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            const int unit_count = (int)this->vertex_buffer_[i].size();
            for ( int j = 0; j < unit_count; ++j )
            {
                vox::ren::vertex::ReleaseVertexBuffer( this->vertex_buffer_[i][j].vertex_buffer );
            }
        }
    }

    void ChunkVertexBuffer::RetrieveBuffer()
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            vertex_buffer_[i].front().size = 0;
            while ( vertex_buffer_[i].size() > 1 )
            {
                void* const bv = vertex_buffer_[i].back().vertex_buffer;
                if ( !gpu_buf_reuse_buf.IsFull() )
                {
                    gpu_buf_reuse_buf.Push( bv );
                }
                else
                {
                    vox::ren::vertex::ReleaseVertexBuffer( bv );
                }
                vertex_buffer_[i].pop_back();
            }
        }
    }

    void ChunkVertexBuffer::MapData( ResizingArray( &cpu_vertex_buffer )[(int)vox::data::EnumSide::MAX_COUNT] )
    {
        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
        {
            const int vertices_count = (int)cpu_vertex_buffer[i].size();
            const int unit_count = (int)(
                (unsigned)(vertices_count + vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE - 1)
                / (unsigned)vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
                );
            const int cur_unit_count = (int)this->vertex_buffer_[i].size();

            for ( int j = cur_unit_count; j < unit_count; ++j )
            {
                this->vertex_buffer_[i].push_back( vox::data::ChunkVertexUnit { nullptr, 0 } );

                if ( !gpu_buf_reuse_buf.IsEmpty() )
                    vertex_buffer_[i].back().vertex_buffer = gpu_buf_reuse_buf.Pop();
                else
                    vox::ren::vertex::CreateVertexBuffer(
                        &(this->vertex_buffer_[i].back().vertex_buffer),
                        sizeof( vox::ren::vertex::VertexChunk ) * vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
                    );
            }

            for ( int j = 0; j < unit_count - 1; ++j )
            {
                vox::ren::vertex::MapVertex(
                    this->vertex_buffer_[i][j].vertex_buffer,
                    (const vox::ren::vertex::VertexChunk*)cpu_vertex_buffer[i][j * vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE],
                    vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE
                );
                this->vertex_buffer_[i][j].size = vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE;
            }
            const int other_vertices_cnt = (unit_count - 1) * vox::data::ChunkVertexUnit::VERTEX_UNIT_SIZE;
            const int remain_vertices_cnt = vertices_count - other_vertices_cnt;
            if ( unit_count > 0 )
            {
                vox::ren::vertex::MapVertex(
                    this->vertex_buffer_[i][unit_count - 1].vertex_buffer,
                    (const vox::ren::vertex::VertexChunk*)cpu_vertex_buffer[i][other_vertices_cnt],
                    remain_vertices_cnt
                );
                this->vertex_buffer_[i][unit_count - 1].size = remain_vertices_cnt;
            }

            const int new_unit_count = (int)this->vertex_buffer_[i].size();
            for ( int j = unit_count; j < new_unit_count; ++j )
            {
                this->vertex_buffer_[i][j].size = 0;
            }
        }
    }

    void ChunkVertexBuffer::Render( vox::data::Vector4i cv, vox::data::EnumBitSide6 sides ) const
    {
        vox::data::Vector4f cvf = vox::data::vector::ConvertToVector4f( cv );
        vox::data::Vector4f multiplier = vox::data::vector::Set(
            (float)vox::consts::CHUNK_X,
            (float)vox::consts::CHUNK_Y,
            (float)vox::consts::CHUNK_Z,
            0.0f
        );
        alignas(16) float cvfs[4];
        vox::data::vector::Store( cvfs, vox::data::vector::Mul( cvf, multiplier ) );

        for ( int i = 0; i < (int)vox::data::EnumSide::MAX_COUNT; ++i )
            if ( (int)sides & (1 << i) )
            {
                const int unit_count = (int)this->vertex_buffer_[i].size();
                for ( int j = 0; j < unit_count && this->vertex_buffer_[i][j].size; ++j )
                {
                    vox::ren::vertex::RenderChunk(
                        cvfs[0], cvfs[1], cvfs[2],
                        &this->vertex_buffer_[i][j].vertex_buffer,
                        this->vertex_buffer_[i][j].size
                    );
                }
            }
    }
}