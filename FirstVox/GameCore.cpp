#include "GameCore.h"

namespace vox::core::gamecore
{

    void Init()
    {
        camera.position = vox::data::Vector4f{ 0.0f, 40.0f, -6.0f, 0.0f };
        camera.speed = vox::data::Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f };
        camera.rotation = vox::data::Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    void Update()
    {
        camera.position = camera.position + camera.speed;
        camera.speed.SetZero();
    }

    void Clean()
    {

    }
}