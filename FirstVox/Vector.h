#pragma once

#include <immintrin.h>
#include <ostream>

#include "Macros.h"

namespace vox::data {

    using Vector4f = __m128;
    using Vector4i = __m128i;
    
    std::ostream& operator<<( std::ostream& ostr, Vector4f v );
    std::ostream& operator<<( std::ostream& ostr, Vector4i v );

    namespace vector
    {
        FORCE_INLINE Vector4i VEC_CALL Load( const Vector4i* src )
        {
            return _mm_load_si128( src );
        }
        FORCE_INLINE Vector4i VEC_CALL Loadu( const Vector4i* src )
        {
            return _mm_loadu_si128( src );
        }
        FORCE_INLINE Vector4i VEC_CALL Loadu( const int* src )
        {
            return _mm_loadu_epi32( src );
        }
        FORCE_INLINE Vector4i VEC_CALL SetZero4i()
        {
            return _mm_setzero_si128();
        }
        FORCE_INLINE Vector4i VEC_CALL SetAll14i()
        {
            auto zr = _mm_setzero_si128();
            return _mm_cmpeq_epi32( zr, zr );
        }
        FORCE_INLINE Vector4i VEC_CALL Set( int x, int y, int z, int w )
        {
            return _mm_set_epi32( w, z, y, x );
        }
        FORCE_INLINE Vector4i VEC_CALL SetBroadcast( unsigned i )
        {
            return _mm_set1_epi32( (int)i );
        }
        FORCE_INLINE Vector4i VEC_CALL SetBroadcast( int i )
        {
            return _mm_set1_epi32( i );
        }
        FORCE_INLINE void VEC_CALL Store( Vector4i* dest, Vector4i v )
        {
            return _mm_store_si128( dest, v );
        }
        FORCE_INLINE void VEC_CALL Storeu( Vector4i* dest, Vector4i v )
        {
            return _mm_storeu_si128( dest, v );
        }
        FORCE_INLINE void VEC_CALL Storeu( int* dest, Vector4i v )
        {
            return _mm_storeu_epi32( dest, v );
        }
        FORCE_INLINE int VEC_CALL GetX( Vector4i v )
        {
            return _mm_extract_epi32( v, 0 );
        }
        FORCE_INLINE int VEC_CALL GetY( Vector4i v )
        {
            return _mm_extract_epi32( v, 1 );
        }
        FORCE_INLINE int VEC_CALL GetZ( Vector4i v )
        {
            return _mm_extract_epi32( v, 2 );
        }
        FORCE_INLINE int VEC_CALL GetW( Vector4i v )
        {
            return _mm_extract_epi32( v, 3 );
        }


        FORCE_INLINE Vector4i VEC_CALL Add( Vector4i a, Vector4i b )
        {
            return _mm_add_epi32( a, b );
        }
        // return Vector4i{ b[3]+b[2], b[1]+b[0], a[3]+a[2], a[1]+a[0] };
        FORCE_INLINE Vector4i VEC_CALL AddHorizontal( Vector4i a, Vector4i b )
        {
            return _mm_hadd_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL Sub( Vector4i a, Vector4i b )
        {
            return _mm_sub_epi32( a, b );
        }
        // return Vector4i{ -b[3]+b[2], -b[1]+b[0], -a[3]+a[2], -a[1]+a[0] };
        FORCE_INLINE Vector4i VEC_CALL SubHorizontal( Vector4i a, Vector4i b )
        {
            return _mm_hsub_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL Mul( Vector4i a, Vector4i b )
        {
            return _mm_mullo_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL DivSignedTruncate( Vector4i a, Vector4i b )
        {
            return _mm_div_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL DivUnsignedTruncate( Vector4i a, Vector4i b )
        {
            return _mm_div_epu32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL DivRemainderSignedTruncate( Vector4i a, Vector4i b, Vector4i* remainder_out )
        {
            return _mm_divrem_epi32( remainder_out, a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL DivRemainderUnsignedTruncate( Vector4i a, Vector4i b, Vector4i* remainder_out )
        {
            return _mm_divrem_epu32( remainder_out, a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL RemainderSigned( Vector4i a, Vector4i b )
        {
            return _mm_rem_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL RemainderUnsigned( Vector4i a, Vector4i b )
        {
            return _mm_rem_epu32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL Max( Vector4i a, Vector4i b )
        {
            return _mm_max_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL Min( Vector4i a, Vector4i b )
        {
            return _mm_min_epi32( a, b );
        }


        FORCE_INLINE Vector4i VEC_CALL Minus( Vector4i v )
        {
            return _mm_sign_epi32( v, _mm_set1_epi32(-1) );
        }
        FORCE_INLINE Vector4i VEC_CALL Abs( Vector4i v )
        {
            return _mm_abs_epi32( v );
        }


        FORCE_INLINE Vector4i VEC_CALL ShiftLeftZeroExtend( Vector4i a, const int b )
        {
            return _mm_slli_epi32( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL ShiftLeftZeroExtend( Vector4i v, Vector4i sh )
        {
            return _mm_sllv_epi32( v, sh );
        }

        FORCE_INLINE Vector4i VEC_CALL ShiftRightZeroExtend( Vector4i v, const int sh )
        {
            return _mm_srli_epi32( v, sh );
        }
        FORCE_INLINE Vector4i VEC_CALL ShiftRightSignExtend( Vector4i v, const int sh )
        {
            return _mm_srai_epi32( v, sh );
        }
        FORCE_INLINE Vector4i VEC_CALL ShiftRightZeroExtend( Vector4i v, Vector4i sh )
        {
            return _mm_srlv_epi32( v, sh );
        }
        FORCE_INLINE Vector4i VEC_CALL ShiftRightSignExtend( Vector4i v, Vector4i sh )
        {
            return _mm_srav_epi32( v, sh );
        }


        FORCE_INLINE Vector4i VEC_CALL And( Vector4i a, Vector4i b )
        {
            return _mm_and_si128( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL AndNot( Vector4i a_not, Vector4i b )
        {
            return _mm_andnot_si128( a_not, b );
        }
        FORCE_INLINE Vector4i VEC_CALL Or( Vector4i a, Vector4i b )
        {
            return _mm_or_si128( a, b );
        }
        FORCE_INLINE Vector4i VEC_CALL Xor( Vector4i a, Vector4i b )
        {
            return _mm_xor_si128( a, b );
        }

        FORCE_INLINE int VEC_CALL IsAll1( Vector4i v )
        {
            return _mm_test_all_ones( v );
        }
        FORCE_INLINE int VEC_CALL IsAll0( Vector4i v, Vector4i mask )
        {
            return _mm_test_all_zeros( v, mask );
        }
        FORCE_INLINE int VEC_CALL GetSignBitsEPI8( Vector4i v )
        {
            return _mm_movemask_epi8( v );
        }


        FORCE_INLINE Vector4f VEC_CALL CastToVector4f( Vector4i v )
        {
            return _mm_castsi128_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL ConvertToVector4f( Vector4i v )
        {
            return _mm_cvtepi32_ps( v );
        }


        FORCE_INLINE Vector4i VEC_CALL Set1sIfEqual( Vector4i a, Vector4i b )
        {
            return _mm_cmpeq_epi32( a, b );
        }
        FORCE_INLINE int VEC_CALL Equal( Vector4i a, Vector4i b )
        {
            auto eq = _mm_cmpeq_epi32( a, b );
            return _mm_testc_si128( eq, _mm_cmpeq_epi32( a, a ) );
        }






        FORCE_INLINE Vector4f VEC_CALL Load( const float* f )
        {
            return _mm_load_ps( f );
        }
        FORCE_INLINE Vector4f VEC_CALL Loadu( const float* f )
        {
            return _mm_loadu_ps( f );
        }
        FORCE_INLINE Vector4f VEC_CALL LoadBroadcast( const float* f )
        {
            return _mm_load_ps1( f );
        }
        FORCE_INLINE Vector4f VEC_CALL SetZero4f()
        {
            return _mm_setzero_ps();
        }
        FORCE_INLINE Vector4f VEC_CALL SetAll14f()
        {
            auto zr = _mm_setzero_ps();
            return _mm_cmpeq_ps( zr, zr );
        }
        FORCE_INLINE Vector4f VEC_CALL Set(float x, float y, float z, float w)
        {
            return _mm_set_ps( w, z, y, x );
        }
        FORCE_INLINE Vector4f VEC_CALL SetBroadcast(float f)
        {
            return _mm_set_ps1( f );
        }
        FORCE_INLINE void VEC_CALL Store( float* dest, Vector4f v )
        {
            return _mm_store_ps( dest, v );
        }
        FORCE_INLINE void VEC_CALL Storeu( float* dest, Vector4f v )
        {
            return _mm_storeu_ps( dest, v );
        }
        FORCE_INLINE float VEC_CALL GetX(Vector4f v)
        {
            return _mm_cvtss_f32( v );
        }


        FORCE_INLINE Vector4f VEC_CALL Add( Vector4f a, Vector4f b )
        {
            return _mm_add_ps( a, b );
        }
        // return Vector4i{ b[3]+b[2], b[1]+b[0], a[3]+a[2], a[1]+a[0] };
        FORCE_INLINE Vector4f VEC_CALL AddHorizontal( Vector4f a, Vector4f b )
        {
            return _mm_hadd_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Sub( Vector4f a, Vector4f b )
        {
            return _mm_sub_ps( a, b );
        }
        // return Vector4i{ -b[3]+b[2], -b[1]+b[0], -a[3]+a[2], -a[1]+a[0] };
        FORCE_INLINE Vector4f VEC_CALL SubHorizontal( Vector4f a, Vector4f b )
        {
            return _mm_hsub_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Mul( Vector4f a, Vector4f b )
        {
            return _mm_mul_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Dot3( Vector4f a, Vector4f b )
        {
            return _mm_dp_ps( a, b, 0b01110001 );
        }
        FORCE_INLINE Vector4f VEC_CALL Dot4( Vector4f a, Vector4f b )
        {
            return _mm_dp_ps( a, b, 0b11110001 );
        }
        FORCE_INLINE Vector4f VEC_CALL Dot3BroadCast( Vector4f a, Vector4f b )
        {
            return _mm_dp_ps( a, b, 0b01111111 );
        }
        FORCE_INLINE Vector4f VEC_CALL Dot4BroadCast( Vector4f a, Vector4f b )
        {
            return _mm_dp_ps( a, b, 0b11111111 );
        }
        FORCE_INLINE Vector4f VEC_CALL Div( Vector4f a, Vector4f b )
        {
            return _mm_div_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Sqrt( Vector4f v )
        {
            return _mm_sqrt_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL ReciprocalSqrt( Vector4f v )
        {
            return _mm_invsqrt_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL ReciprocalSqrtApproximate( Vector4f v )
        {
            return _mm_rsqrt_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL Pythagoras( Vector4f a, Vector4f b )
        {
            return _mm_hypot_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL ReciprocalApproximate( Vector4f v )
        {
            return _mm_rcp_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL Ceil(Vector4f v)
        {
            return _mm_ceil_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL Floor(Vector4f v)
        {
            return _mm_floor_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL Round(Vector4f v)
        {
            return _mm_round_ps( v, _MM_FROUND_TO_NEAREST_INT );
        }
        FORCE_INLINE Vector4f VEC_CALL RoundToZero(Vector4f v)
        {
            return _mm_round_ps( v, _MM_FROUND_TO_ZERO );
        }
        FORCE_INLINE Vector4f VEC_CALL Max(Vector4f a, Vector4f b)
        {
            return _mm_max_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Min(Vector4f a, Vector4f b)
        {
            return _mm_min_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Sin(Vector4f v)
        {
            return _mm_sin_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL Cos(Vector4f v)
        {
            return _mm_cos_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL SinCos(Vector4f v, Vector4f* cos_dest)
        {
            return _mm_sincos_ps( cos_dest, v );
        }
        FORCE_INLINE Vector4f VEC_CALL Tan(Vector4f v)
        {
            return _mm_tan_ps( v );
        }
        FORCE_INLINE Vector4f VEC_CALL Minus( Vector4f v )
        {
            return _mm_sub_ps( _mm_setzero_ps(), v );
        }
        FORCE_INLINE Vector4f VEC_CALL Abs( Vector4f v )
        {
            return _mm_and_ps( v,
                _mm_castsi128_ps( _mm_set1_epi32( 0x7fffffff ) )
            );
        }

        FORCE_INLINE Vector4f VEC_CALL And( Vector4f a, Vector4f b )
        {
            return _mm_and_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL AndNot( Vector4f a_not, Vector4f b )
        {
            return _mm_andnot_ps( a_not, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Or( Vector4f a, Vector4f b )
        {
            return _mm_or_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Xor( Vector4f a, Vector4f b )
        {
            return _mm_xor_ps( a, b );
        }


        FORCE_INLINE Vector4i VEC_CALL CastToVector4i( Vector4f v )
        {
            return _mm_castps_si128( v );
        }
        FORCE_INLINE Vector4i VEC_CALL ConvertToVector4i( Vector4f v )
        {
            return _mm_cvtps_epi32( v );
        }


        FORCE_INLINE Vector4f VEC_CALL Set1sIfEqual( Vector4f a, Vector4f b )
        {
            return _mm_cmpeq_ps( a, b );
        }
        FORCE_INLINE Vector4f VEC_CALL Set1sIfNotEqual( Vector4f a, Vector4f b )
        {
            return _mm_cmpneq_ps( a, b );
        }
        FORCE_INLINE int VEC_CALL GetSignBits( Vector4f v )
        {
            return _mm_movemask_ps( v );
        }
        FORCE_INLINE int VEC_CALL Equal( Vector4f a, Vector4f b )
        {
            auto eq = _mm_cmpeq_ps( a, b );
            return _mm_testc_ps( eq, _mm_cmpeq_ps( a, a ) );
        }
        
    }

    static_assert(sizeof( int ) * 4 == sizeof( __m128i ));
    static_assert(sizeof( float ) * 4 == sizeof( __m128 ));
}