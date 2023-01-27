#include "Perlin.h"

#include <cmath>
#include <random>

#include "Consts.h"

namespace vox::rand::perlin
{

    PerlinGeneratorUnit::PerlinGeneratorUnit(
        int x, int z, int level, const float* multipliers, unsigned seed
    ) :
        level_{ level },
        multipliers_{ multipliers },
        point_vec_thetas_{ nullptr }
    {
        vox::data::Pointf2D** pvt = new vox::data::Pointf2D*[level];
        for ( int i = 0; i < level; ++i )
        {
            const int div = 1 << i;
            const int width = div + 1;
            pvt[i] = new vox::data::Pointf2D[width * width];
            const int seed_base_x = x * div;
            const int seed_base_z = z * div;

            std::mt19937 gen{};
            std::uniform_real_distribution<float> dis_float( 0.0f, vox::consts::PI_2 );
            std::uniform_int_distribution<unsigned> dis_int( 0, (unsigned)-1 );

            for (int zz = 0; zz < width; ++zz )
                for ( int xx = 0; xx < width; ++xx )
                {
                    gen.seed( (seed_base_x + xx) );
                    gen.seed( dis_int( gen ) ^ (seed_base_z + zz) ^ seed );
                    float rand_num = dis_float( gen );
                    const float vx = std::cosf( rand_num );
                    const float vz = std::sinf( rand_num );
                    
                    pvt[i][zz * width + xx].x = vx;
                    pvt[i][zz * width + xx].z = vz;
                }
        }
        this->point_vec_thetas_ = pvt;
    }

    PerlinGeneratorUnit::~PerlinGeneratorUnit()
    {
        for ( int i = 0; i < this->level_; ++i )
        {
            delete[] this->point_vec_thetas_[i];
        }
        delete[] this->point_vec_thetas_;
    }

    float PerlinGeneratorUnit::Sample( float x, float z ) const
    {
        float ret = 0.0f;
        for ( int i = 0; i < this->level_; ++i )
        {
            const int div = 1 << i;
            const int width = div + 1;
            const float divf = (float)div;
            const float divfr = 1.0f / divf;

            const int x_ind = (int)(x * divf);
            const int z_ind = (int)(z * divf);

            const vox::data::Pointf2D& hl = this->point_vec_thetas_[i][(z_ind + 1) * width + x_ind];
            const vox::data::Pointf2D& hr = this->point_vec_thetas_[i][(z_ind + 1) * width + x_ind + 1];
            const vox::data::Pointf2D& ll = this->point_vec_thetas_[i][z_ind * width + x_ind];
            const vox::data::Pointf2D& lr = this->point_vec_thetas_[i][z_ind * width + x_ind + 1];

            const float xx = x * divf - (float)x_ind;
            const float zz = z * divf - (float)z_ind;
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

            ret += v_ret * multipliers_[i];
        }
        return ret;
    }
}