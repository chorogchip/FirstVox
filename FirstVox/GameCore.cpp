#include "GameCore.h"

namespace vox::core::gamecore
{

    void Init()
    {
        camera.position = vox::data::Vector3{ 0.0f, 3.0f, -6.0f };
        camera.speed = vox::data::Vector3{ 0.0f, 0.0f, 0.0f };
        camera.rotation = vox::data::Vector3{ 0.0f, 0.0f, 0.0f };
    }

    void Update()
    {
        camera.position += camera.speed;
        camera.speed.SetZero();
    }

    void Quit()
    {

    }
}