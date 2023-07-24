#include "GlobalPerlinField.h"

#include "Consts.h"
#include "Point2D.h"

namespace vox::rand
{

    GlobalPerlinField::GlobalPerlinField(uint64_t seed, int period, float amplitude) :
        seed_(seed),
        period_(period),
        amplitude_(amplitude),
        re_(),
        dist_(0.0f, vox::consts::PI_2)
    {}


    float GlobalPerlinField::Sample(int x, int z)
    {
#define GEN(x) ((6364136223846793005LL * (x) + 1442695040888963407LL))
#define SPL(x) ((float)(x) * (1.0f / (float)(uint64_t)-1) * consts::PI_2)

        int grid_x;
        if (x >= 0) grid_x = x / period_;
        else grid_x = (x - period_ + 1) / period_;

        int grid_z;
        if (z >= 0) grid_z = z / period_;
        else grid_z = (z - period_ + 1) / period_;

        const auto sample1 = GEN(seed_ ^ grid_x);
        const float thetall = SPL(GEN(sample1 ^ grid_z));
        const float thetahl = SPL(GEN(sample1 ^ (grid_z + 1)));

        const auto sample2 = GEN(seed_ ^ (grid_x + 1));
        const float thetalr = SPL(GEN(sample2 ^ grid_z));
        const float thetahr = SPL(GEN(sample2 ^ (grid_z + 1)));

#undef GEN

        const vox::data::Pointf2D hl = vox::data::Pointf2D( std::cosf( thetahl ), std::sinf( thetahl ) );
        const vox::data::Pointf2D hr = vox::data::Pointf2D( std::cosf( thetahr ), std::sinf( thetahr ) );
        const vox::data::Pointf2D ll = vox::data::Pointf2D( std::cosf( thetall ), std::sinf( thetall ) );
        const vox::data::Pointf2D lr = vox::data::Pointf2D( std::cosf( thetalr ), std::sinf( thetalr ) );

        const int rem_x = x - grid_x * period_;
        const int rem_z = z - grid_z * period_;

        const float xx = (float)rem_x / (float)period_;
        const float zz = (float)rem_z / (float)period_;
        const float xr = xx - 1.0f;
        const float zr = zz - 1.0f;

        const float hl_d = hl.x * xx + hl.z * zr;
        const float hr_d = hr.x * xr + hr.z * zr;
        const float ll_d = ll.x * xx + ll.z * zz;
        const float lr_d = lr.x * xr + lr.z * zz;

        const float xxb = (6.0f * xx * xx - 15.0f * xx + 10.0f) * xx * xx * xx;
        const float zzb = (6.0f * zz * zz - 15.0f * zz + 10.0f) * zz * zz * zz;
        const float xrb = xxb - 1.0f;
        const float zrb = zzb - 1.0f;

        const float h_ret = hr_d * xxb - hl_d * xrb;
        const float l_ret = lr_d * xxb - ll_d * xrb;

        const float v_ret = h_ret * zzb - l_ret * zrb;

        return v_ret * amplitude_;

    }

}

