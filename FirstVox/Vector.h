#pragma once
#include <memory>

namespace vox::data {

    class Vector3
    {
    public:
        float d[3];

        void SetZero()
        {
            memset( d, 0, sizeof( d ) );
        }
        float& operator[](int i)
        {
            return d[i];
        }
        const float& operator[](int i) const
        {
            return d[i];
        }
        void operator+=( const Vector3& v )
        {
            d[0] += v.d[0];
            d[1] += v.d[1];
            d[2] += v.d[2];
        }
        void operator-=( const Vector3& v )
        {
            d[0] -= v.d[0];
            d[1] -= v.d[1];
            d[2] -= v.d[2];
        }
    };

    class alignas(16) Vector4
    {
    public:
        float d[4];

        void SetZero()
        {
            memset( d, 0, sizeof( d ) );
        }
        float& operator[](int i)
        {
            return d[i];
        }
        const float& operator[](int i) const
        {
            return d[i];
        }
        void operator+=( const Vector4& v )
        {
            d[0] += v.d[0];
            d[1] += v.d[1];
            d[2] += v.d[2];
            d[3] += v.d[3];
        }
        void operator-=( const Vector4& v )
        {
            d[0] -= v.d[0];
            d[1] -= v.d[1];
            d[2] -= v.d[2];
            d[3] -= v.d[3];
        }
    };

}