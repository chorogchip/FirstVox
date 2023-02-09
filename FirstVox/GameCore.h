#pragma once

#include "Vector.h"
#include "Entity.h"
#include "EBlockID.h"

namespace vox::core::gamecore {

    inline vox::data::Entity camera;
    inline vox::data::EBlockID hand_block = vox::data::EBlockID::GRASS;

    uint32_t GetGameTicks();
    vox::data::Vector4f GetSunVec();

    void Init();
    void Update();
    void Clean();
}