#include "ChunkManager.h"

#include <vector>

#include "GameDatas.h"

namespace vox::core::chunkmanager
{

    enum class EnumChunkStates
    {
        NOT_USED = 0,
        LOAD_NEEDED = 1,
        LOADING = 2,
        VERTEX_NEEDED = 3,
        SET = 4,
    };

    class ChunkHolder
    {
    public:
        EnumChunkStates state;
        vox::data::Chunk* chunk;
    };

    static int render_chunk_dist_ = 0;
    static int render_chunk_width_ = 0;
    static ChunkHolder* game_chunks_arr_= nullptr;

    void Init()
    {
        render_chunk_dist_ = 5;
        render_chunk_width_ = render_chunk_dist_ * 2 + 1;
        game_chunks_arr_ = new ChunkHolder[render_chunk_width_ * render_chunk_width_]{};
    }

    int GetRenderChunkDist()
    {
        return render_chunk_dist_;
    }

    void SetRenderChunkDist( int render_chunk_dist )
    {
        const int new_game_chunk_width = render_chunk_dist * 2 + 1;
        ChunkHolder* const new_game_chunks_arr =
            new ChunkHolder[new_game_chunk_width * new_game_chunk_width]{};

        //TODO
        

        delete[] game_chunks_arr_;
        game_chunks_arr_ = new_game_chunks_arr;
        render_chunk_dist_ = render_chunk_dist;
        render_chunk_width_ = new_game_chunk_width;
    }

    void Clean()
    {
        delete[] game_chunks_arr_;
        game_chunks_arr_ = nullptr;
        render_chunk_dist_ = 0;
        render_chunk_width_ = 0;
    }

}