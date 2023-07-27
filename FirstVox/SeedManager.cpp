#include "SeedManager.h"

#include <windows.h>

#include <iostream>

static uint64_t seed_ = 0U;
static bool is_seed_init_ = false;

namespace vox::core
{

    static void SMInit()
    {
        FILE* fp = nullptr;
        fopen_s(&fp, "GameData/gameinfo.txt", "r");
        if (fp == nullptr)
        {
            OutputDebugStringA("open gameinfo file failed\n");
            return;
        }

        fscanf_s(fp, "%llu", &seed_);
        fclose(fp);

        is_seed_init_ = true;
    }

    uint64_t SMGetSeed()
    {
        if (!is_seed_init_)
        {
            SMInit();
        }
        return seed_;
    }

}