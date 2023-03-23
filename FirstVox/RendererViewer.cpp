#include "RendererViewer.h"

#include "VertexRenderer.h"

namespace vox::ren::renviewer
{
    const float* GetLastCalculatedCameraViewMatrix()
    {
        return vox::ren::vertex::GetLastCalculatedCameraViewMatrix();
    }
}