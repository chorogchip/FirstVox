#pragma once

#include "Macros.h"
#include "Vector.h"
#include "Block.h"
#include "Entity.h"

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
    int GetRenderChunkDist();
    void SetRenderChunkDist( int render_chunk_dist );

    void Init();
    void Clean();
    void Update();
    void Render();

    //vox::data::Block* VEC_CALL GetModifyableBlockByBlockPos( vox::data::Vector4i block_pos );
    // return Block of id MAX_COUNT if failed
    vox::data::Block VEC_CALL GetBlock( vox::data::Vector4i block_pos );
    void VEC_CALL SetBlock( vox::data::Vector4i block_pos, vox::data::Block block );


    void RegisterDynamicChunkLoader( vox::data::Entity* p_chunk_loader, int load_distance );
    // this method cannot be called in init of some object, but in update or clean is ok
    void CleanDynamicChunkLoader( vox::data::Entity* p_chunk_loader );

    void VEC_CALL RegisterStaticChunkLoader( vox::data::Vector4i chunk_loader_pos, int load_distance );
    // this method cannot be called in init of some object, but in update or clean is ok
    void VEC_CALL CleanStaticChunkLoader( vox::data::Vector4i chunk_loader_pos );
}