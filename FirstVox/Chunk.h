#pragma once

#include <vector>
#include <tuple>

#include "Macros.h"
#include "Consts.h"
#include "EBlockID.h"
#include "EnumSide.h"
#include "Block.h"
#include "Vector.h"
#include "ResizingArray.h"
#include "ChunkVertexBuffer.h"
#include "LightInfos.h"

namespace vox::data
{

    class Chunk
    {
    private:
        using ChunkLight = unsigned int;
        vox::data::Vector4i cv_;
        EBlockID d_id_[vox::consts::CHUNK_Y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X];
        //BlockData d_data_[vox::consts::CHUNK_Y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X];
        ResizingArray vertex_buffer_temp_[(int)vox::data::EnumSide::MAX_COUNT];
        ChunkVertexBuffer vertex_buffer_;
        std::vector<vox::data::lightinfos::LightTypesInfo> light_infos_;
        bool is_changed_;
        //unsigned char max_block_y_;

        int FORCE_INLINE GetInd( int x, int y, int z ) const
        {
            return y * vox::consts::CHUNK_Z * vox::consts::CHUNK_X + z * vox::consts::CHUNK_X + x;
        }

    public:
        Chunk( vox::data::Vector4i cv );
        void ConstructForReuse( vox::data::Vector4i cv );

        FORCE_INLINE EBlockID GetBlockId( int x, int y, int z ) const
        {
            return this->d_id_[GetInd( x, y, z )];
        }
        /*
        FORCE_INLINE BlockData GetBlockData( int x, int y, int z ) const
        {
            return this->d_data_[GetInd( x, y, z )];
        }*/
        
        FORCE_INLINE vox::data::Block GetBlock( int x, int y, int z ) const
        {
            const int ind = GetInd( x, y, z );
            return vox::data::Block( this->d_id_[ind]/*, this->d_data_[ind]*/ );
        }
        void SetBlock( int x, int y, int z, vox::data::Block block );
        /*
        FORCE_INLINE void SetBlockData( int x, int y, int z, BlockData data )
        {
            this->d_data_[GetInd( x, y, z )] = data;
        }*/
        vox::data::Vector4i GetCV() const
        {
            return this->cv_;
        }

        void Load( bool to_pass_read = false );
        void LoadFromData(void* data, size_t size);
        // adj_chks[8] must be this
        void GenerateVertex( Chunk* adj_chks[9],
            unsigned char light_bfs_arr[vox::consts::CHUNK_BLOCKS_CNT * 12],
            unsigned d_lights[(vox::consts::CHUNK_X + 2) * (vox::consts::CHUNK_Y) * (vox::consts::CHUNK_Z + 2)] );
        void MapTempVertexToBuffer();
        void Render( vox::data::EnumBitSide6 sides );
        void Clear( bool to_retrieve_buffer );
        void Touch();
        bool IsChanged() const { return is_changed_; }
        size_t GetStoringData(unsigned char** pp_data, size_t offset) const;
        static FILE* GetFP(int x, int y, int z, const char* mode);
        void SetBlocks(std::vector<std::tuple<int,int,int,data::Block>> &blocks);
    };
}