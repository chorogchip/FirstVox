#pragma once

namespace vox::core::eventhandler {

    void OnKeyPressed( char c );
    void OnKeyReleased( char c );
    void OnChar( char c );
    void OnMouseLPressed( short x, short y );
    void OnMouseLReleased( short x, short y );
    void OnMouseMPressed( short x, short y );
    void OnMouseMReleased( short x, short y );
    void OnMouseRPressed( short x, short y );
    void OnMouseRReleased( short x, short y );
    void OnMouseMoved( short x, short y );
    void OnWheelScrolled( int delta );
    void OnRawMouseInput( int dx, int dy );

    void Update();
}