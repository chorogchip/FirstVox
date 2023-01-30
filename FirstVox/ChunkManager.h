#pragma once

#include "Macros.h"
#include "Vector.h"
#include "Chunk.h"

/*
    ûũ �Ŵ����� ûũ�� �����ϸ鼭 ûũ�� ������ ����� �߻�ȭ ������ �����Ѵ�.
    �� ��ǥ�� �ش��ϴ� ���� �ּҸ� ������ �� �� �ִ�.
    ���� �ʿ��ϴٸ� ûũ���� �޸𸮿��� �ҷ����ų� �����ϰų� �޽��� �����϶�� ����� ������.
    �� ���۵��� ûũ �Ŵ����� ûũ �ε� ����� ������ ��ƼƼ(�÷��̾�, ûũ �δ�) ���� ��������ν� �����Ѵ�.
    �����̴� ��ƼƼ�� ��� ��ƼƼ�� �ּҸ� ����ϸ� ûũ �Ŵ����� �� �����Ӹ��� ��ƼƼ�� �ּҸ� ���� ��ƼƼ�� ��ǥ�� �����´�.
    ���� �����̴� ��ƼƼ�� �ּҴ� ������ �ʾƾ� �Ѵ�.
    ûũ �Ŵ����� ����� ûũ�ε� ����� ������ ��ƼƼ�� ȸ���ϴ� å���� �ܺο� �ִ�.
    ���� ûũ �Ŵ����� �ʿ��� ûũ�鿡 ���� ����� �����ϴ� ���ҵ� ����Ѵ�.
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