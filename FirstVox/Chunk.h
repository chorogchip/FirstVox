#pragma once

#include "Macros.h"
#include "Consts.h"
#include "EBlockID.h"
#include "EnumSide.h"
#include "Block.h"
#include "Vector.h"
#include "ResizingArray.h"
#include "ChunkVertexBuffer.h"

namespace vox::data
{
    class Chunk
    {
    private:
        using ChunkLight = unsigned int;
        vox::data::Vector4i cv_;
        EBlockID d_id_[vox::consts::CHUNK_Y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X];
        BlockData d_data_[vox::consts::CHUNK_Y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X];
        ChunkLight d_light_[vox::consts::CHUNK_Y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X];
        ResizingArray vertex_buffer_temp_[(int)vox::data::EnumSide::MAX_COUNT];
        ChunkVertexBuffer vertex_buffer_;
        bool is_changed_;

        static_assert(sizeof( vox::data::Block ) == 4);
        
        FORCE_INLINE void SetBlockLight( int x, int y, int z, ChunkLight light )
        {
            const int ind = y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x;
            this->d_light_[ind] = light;
        }
        FORCE_INLINE void OrBlockLight( int x, int y, int z, ChunkLight light )
        {
            const int ind = y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x;
            this->d_light_[ind] |= light;
        }

    public:
        Chunk( vox::data::Vector4i cv );
        void ConstructForReuse( vox::data::Vector4i cv );

        FORCE_INLINE EBlockID GetBlockId( int x, int y, int z ) const
        {
            return this->d_id_[y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x];
        }
        FORCE_INLINE BlockData GetBlockData( int x, int y, int z ) const
        {
            return this->d_data_[y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x];
        }
        FORCE_INLINE BlockData GetBlockLight( int x, int y, int z ) const
        {
            return this->d_light_[y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x];
        }
        FORCE_INLINE vox::data::Block GetBlock( int x, int y, int z ) const
        {
            const int ind = y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x;
            return vox::data::Block( this->d_id_[ind], this->d_data_[ind] );
        }
        FORCE_INLINE void SetBlock( int x, int y, int z, vox::data::Block block )
        {
            const int ind = y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x;
            this->d_id_[ind] = block.id;
            this->d_data_[ind] = block.data;
        }
        FORCE_INLINE void SetBlockData( int x, int y, int z, BlockData data )
        {
            const int ind = y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x;
            this->d_data_[ind] = data;
        }
        vox::data::Vector4i GetCV() const
        {
            return this->cv_;
        }

        void Load();
        void GenerateVertex( Chunk* adj_chks[8] );
        void MapTempVertexToBuffer();
        void Render( vox::data::EnumBitSide6 sides );
        void Clear( bool to_retrieve_buffer );
        void Touch();
    };
}