#include "GlobalUniformField.h"
#include "GlobalPerlinField.h"

#include "Consts.h"
#include "Point2D.h"

namespace vox::rand
{

    GlobalUniformField::GlobalUniformField(uint64_t seed) :
        seed_(seed),
        re_(),
        dist_(0.0f, 1.0f)
    {}


    float GlobalUniformField::Sample(int x, int z)
    {
#define GEN(x) ((6364136223846793005LL * (x) + 1442695040888963407LL))
#define SPL(x) ((float)(x) * (1.0f / (float)(uint64_t)-1))

        const auto sample1 = GEN(seed_ ^ x);
        const float sample = SPL(GEN(sample1 ^ z));

#undef GEN

        return sample;
    }

}

