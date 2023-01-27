#pragma once

#include "Entity.h"

namespace vox::core::gamecore {

    inline vox::data::Entity camera;

    void Init();
    void Update();
    void Clean();
}