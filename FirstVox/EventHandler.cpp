#include "EventHandler.h"

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include "Consts.h"
#include "Utils.h"
#include "GameCore.h"

namespace vox::core::eventhandler {

    void OnKeyPressed( char c ) {
        switch ( c )
        {
        case VK_ESCAPE:
            break;
        }
    }
    void OnKeyReleased( char c ) {

    }
    void OnChar( char c ) {

    }
    void OnMouseLPressed( short x, short y ) {

    }
    void OnMouseLReleased( short x, short y ) {

    }
    void OnMouseMPressed( short x, short y ) {

    }
    void OnMouseMReleased( short x, short y ) {

    }
    void OnMouseRPressed( short x, short y ) {

    }
    void OnMouseRReleased( short x, short y ) {

    }
    void OnMouseMoved( short x, short y ) {

    }
    void OnWheelScrolled( int delta ) {

    }

    void OnRawMouseInput( int dx, int dy )
    {

        vox::core::gamecore::camera.rotation[1] = std::remainder(
            vox::core::gamecore::camera.rotation[1] + (float)(dx) * 0.003f, vox::consts::PI_2
        );
        vox::core::gamecore::camera.rotation[0] = std::clamp(
            vox::core::gamecore::camera.rotation[0] + (float)(dy) * 0.01f, -vox::consts::PI_DIV2 * 0.6f, vox::consts::PI_DIV2 * 0.6f
        );
    }

    void Update()
    {
        unsigned int pressed_w = (unsigned short)GetKeyState( 'W' ) & 0x8000;
        unsigned int pressed_s = (unsigned short)GetKeyState( 'S' ) & 0x8000;
        unsigned int pressed_a = (unsigned short)GetKeyState( 'A' ) & 0x8000;
        unsigned int pressed_d = (unsigned short)GetKeyState( 'D' ) & 0x8000;
        unsigned int pressed_sh = (unsigned short)GetKeyState( VK_LSHIFT ) & 0x8000;
        unsigned int pressed_sp = (unsigned short)GetKeyState( VK_SPACE ) & 0x8000;
        unsigned int pressed_keys = pressed_w >> 15 | pressed_s >> 14 | pressed_a >> 11 | pressed_d >> 10
            | pressed_sh >> 7 | pressed_sp >> 6;

        if ( pressed_keys )
        {
            static unsigned int prev_pressed_keys = 0;
            unsigned int pressed_both = pressed_keys & (pressed_keys >> 1);
            pressed_both |= pressed_both << 1;
            pressed_keys -= pressed_both & prev_pressed_keys;
            prev_pressed_keys = pressed_both ^ pressed_keys;

            constexpr float SPD = vox::consts::CAM_SPEED / (float)vox::consts::TPS;

            if ( int pressed_keys_y = pressed_keys >> 8 )
            {
                pressed_keys_y = ((pressed_keys_y ^ 1) - 1) >> 1;
                float spd_y = (float)pressed_keys_y * SPD;
                vox::core::gamecore::camera.speed[1] = spd_y;
            }
            if ( pressed_keys & 63 )
            {
                pressed_keys = (pressed_keys | pressed_keys >> 2) & 15;
                // 0 2 6 0 4 3 5 4 0 1 7 0 0 2 6 0 starting from lsb, 4 bit each
                int theta_cnt = (int)(441360529944217120LL >> (uint64_t)(pressed_keys * 4U)) & 15;
                float yaw = vox::core::gamecore::camera.rotation[1];
                float theta = -yaw + (float)theta_cnt * vox::consts::PI_DIV4;
                vox::core::gamecore::camera.speed[0] = std::cosf( theta ) * SPD;
                vox::core::gamecore::camera.speed[2] = std::sinf( theta ) * SPD;
            }
        }
    }
}