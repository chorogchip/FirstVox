#pragma once

#include "Vector.h"
#include "Entity.h"
#include "EBlockID.h"
#include "Camera.h"

namespace vox::core::gamecore {

    inline vox::core::camera::Camera camera;
    inline int hand_pos = 0;
    inline vox::data::EBlockID hand_blocks[10];

    uint32_t GetGameTicks();
    vox::data::Vector4f GetSunVec();

    void Init();
    void Update();
    void Clean();
}