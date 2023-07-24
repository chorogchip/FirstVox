#pragma once

#include "Macros.h"

namespace vox::wrd
{
    enum class EnumBiomes
    {
        DESSERT,
        MESA,
        PLAIN,
        MOUNTAIN,
        SNOWLAND,
        SNOW_MOUNTAIN,
        MAX_COUNT,
    };

    FORCE_INLINE void GetBiomeFraction(float* dest, float height, float temperature)
    {
        constexpr float BORDER_HIGH = 0.15f;
        constexpr float BORDER_LOW = 0.05f;

        float height_high;
        if (height > BORDER_HIGH)
            height_high = 1.0f;
        else if (height > BORDER_LOW)
            height_high = (height - BORDER_LOW) / (BORDER_HIGH - BORDER_LOW);
        else
            height_high = 0.0f;
        const float height_low = 1.0f - height_high;

        constexpr float BORDER_HOT = 0.23f;
        constexpr float BORDER_WARM = 0.2f;
        constexpr float BORDER_COOL = -0.2f;
        constexpr float BORDER_COLD = -0.23f;

        float temp_hot, temp_cold;
        if (temperature > BORDER_HOT)
        {
            temp_hot = 1.0f;
            temp_cold = 0.0f;
        }
        else if (temperature > BORDER_WARM)
        {
            temp_hot = (temperature - BORDER_WARM) / (BORDER_HOT - BORDER_WARM);
            temp_cold = 0.0f;
        }
        else if (temperature > BORDER_COOL)
        {
            temp_hot = 0.0f;
            temp_cold = 0.0f;
        }
        else if (temperature > BORDER_COLD)
        {
            temp_hot = 0.0f;
            temp_cold = (BORDER_COOL - temperature) / (BORDER_COOL - BORDER_COLD);
        }
        else
        {
            temp_hot = 0.0f;
            temp_cold = 1.0f;
        }
        float temp_mid = 1.0f - temp_hot - temp_cold;

        dest[0] = temp_hot * height_low;
        dest[1] = temp_hot * height_high;
        dest[2] = temp_mid * height_low;
        dest[3] = temp_mid * height_high;
        dest[4] = temp_cold * height_low;
        dest[5] = temp_cold * height_high;
    }

    static constexpr float biome_heights[6][9] = {
        {32.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f, 40.0f},
        {32.0f, 33.0f, 34.0f, 35.0f, 40.0f, 55.0f, 90.0f, 95.0f, 98.0f},
        {32.0f, 34.0f, 36.0f, 38.0f, 40.0f, 42.0f, 44.0f, 46.0f, 48.0f},
        {25.0f, 35.0f, 45.0f, 55.0f, 65.0f, 75.0f, 85.0f, 90.0f, 93.0f},
        {32.0f, 34.0f, 36.0f, 38.0f, 40.0f, 42.0f, 44.0f, 46.0f, 48.0f},
        {25.0f, 35.0f, 45.0f, 55.0f, 65.0f, 75.0f, 85.0f, 90.0f, 93.0f},  
    };
    
    FORCE_INLINE float ConvertHeight(float height, float* biome_weight)
    {
        float res = 0.0f;
        for (int i = 0; i < 6; ++i)
        {   
            const float intv = 2.0f / 8.0f;
            const float proj_val = (height + 1.0f) * (1.0f / intv);
            int index = (int)proj_val;
            if (index < 0) index = 0;
            if (index > 8) index = 8;
            const float rem = proj_val - (float)index;

            const float sample1 = biome_heights[i][index];
            const float sample2 = biome_heights[i][index + 1];
            const float sample = sample1 * (1.0f - rem) + sample2 * rem;

            res += sample * biome_weight[i];
        }
        return res;
    
    }

}