#pragma once

#include "Macros.h"
#include "Vector.h"
#include "Chunk.h"

/*
    청크 매니저는 청크를 관리하면서 청크의 개념을 숨기는 추상화 계층을 제공한다.
    블럭 좌표에 해당하는 블럭의 주소를 제공해 줄 수 있다.
    또한 필요하다면 청크들을 메모리에서 불러오거나 생성하거나 메쉬를 생성하라는 명령을 내린다.
    이 동작들은 청크 매니저에 청크 로딩 기능을 가지는 엔티티(플레이어, 청크 로더) 들을 등록함으로써 동작한다.
    움직이는 엔티티의 경우 엔티티의 주소를 등록하면 청크 매니저가 매 프레임마다 엔티티의 주소를 통해 엔티티의 좌표를 가져온다.
    따라서 움직이는 엔티티의 주소는 변하지 않아야 한다.
    청크 매니저에 등록한 청크로딩 기능을 가지는 엔티티를 회수하는 책임은 외부에 있다.
    또한 청크 매니저는 필요한 청크들에 렌더 명령을 전달하는 역할도 담당한다.
*/

namespace vox::core::chunkmanager
{
    void Init();
    void Clean();
    int GetRenderChunkDist();
    void SetRenderChunkDist( int render_chunk_dist );
    void Render();

    // assumes input block pos is valid, which means y is in correct range and block's chunk is already loaded
    vox::data::Block* VEC_CALL GetBlockByBlockPos( vox::data::Vector4i block_pos );
    // assumes input block pos is valid, which means y is in correct range and block's chunk is already loaded
    void VEC_CALL SetToRebuildMeshByBlockPos( vox::data::Vector4i block_pos );

    void VEC_CALL RegisterDynamicChunkLoader( vox::data::Vector4f* p_chunk_loader_pos );
    void VEC_CALL CleanDynamicChunkLoader( vox::data::Vector4f* p_chunk_loader_pos );
    void VEC_CALL RegisterStaticChunkLoader( vox::data::Vector4i chunk_loader_pos );
    void VEC_CALL CleanStaticChunkLoader( vox::data::Vector4i chunk_loader_pos );
}