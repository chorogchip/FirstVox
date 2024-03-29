﻿
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>

#include "resource.h"

#include "FirstVoxHeader.h"
#include "Macros.h"
#include "Consts.h"
#include "Timer.h"
#include "Logger.h"
#include "EventHandler.h"
#include "Renderer_for_Winmain.h"
#include "VertexRenderer_for_Winmain.h"
#include "VertexRenderer.h"

#include "GameUtils.h"
#include "GameCore.h"
#include "EventHandler.h"
#include "ChunkManager.h"
#include "NetWorkManager.h"
#include "SeedManager.h"


static ATOM                MyRegisterClass( HINSTANCE hInstance );
static BOOL                InitInstance( HINSTANCE, int );
static LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
static INT_PTR CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
static void WinInit();
static void ResizeRenderer( long new_width, long new_height );

HWND h_wnd_;
HINSTANCE h_instance_;
const WCHAR* window_title_ = L"SecondVox Pre v0.1";
const WCHAR* window_class_ = L"MyClass";
static uint64_t game_states_ = 0ULL;

static void FilterError( HRESULT hr )
{
    if ( FAILED( hr ) )
    {
        std::cout << "hr : " << hr << std::endl;
        if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
        {
            std::cout << "dx device lost : " << vox::ren::base::GetDeviceRemovedReason();
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if constexpr ( TO_USE_CRTDBG )
    {
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
    }

    MyRegisterClass(hInstance);
    if ( !InitInstance( hInstance, nCmdShow ) )
    {
        return FALSE;
    }
    WinInit();

    FilterError( vox::ren::base::Init( h_wnd_ ) );
    FilterError( vox::ren::vertex::Init( h_wnd_ ) );
    if (vox::net::NMInit()) OutputDebugStringA("error init network\n");
    vox::core::gamecore::camera.SetAspectRatio( (float)vox::ren::base::GetScreenWidth() / (float)vox::ren::base::GetScreenHeight() );
    vox::core::chunkmanager::Init();
    vox::core::gamecore::Init();

    HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_FIRSTVOX ) );
    MSG msg{};
    vox::utils::Timer timer{};
    timer.Start();

    while ( WM_QUIT != msg.message )
    {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            for ( int i = 0; i < vox::consts::MAX_FRAME_SKIP; ++i )
            {
                if ( timer.GetElapsedMicroSec() >= vox::consts::MICROSEC_PER_TICK )
                {
                    timer.AddTimeMicroSec( vox::consts::MICROSEC_PER_TICK );

                    // update
                    vox::core::gamecore::Update();
                    if ( !(game_states_ & vox::consts::GAMESTAT_MOUSEENABLED) )
                    {
                        vox::core::eventhandler::Update();
                    }
                    vox::core::chunkmanager::Update();
                }
                else goto RENDER_FRAME;
            }
            {
                // when the frame skip is too much, which means game is laggy
                timer.Start();  // reset the timer to slow game
            }
RENDER_FRAME:
            // render
            float delta_time = (float)timer.GetElapsedMicroSec().count() / 100000.0f;
            float sun_vec[4];
            vox::data::vector::Store( sun_vec, vox::core::gamecore::GetSunVec() );
            vox::data::vector::Store( sun_vec, vox::gameutils::GetSkyColorBySunAltitude( sun_vec[1] ) );
            vox::ren::base::Clear( sun_vec );
            vox::ren::vertex::StartRenderChunks( delta_time, sun_vec );
            vox::core::chunkmanager::Render();
            vox::ren::vertex::RenderUI();
            FilterError( vox::ren::base::Present() );
        }
    }
    vox::core::gamecore::Clean();
    vox::core::chunkmanager::Clean();
    vox::net::NMClear();
    vox::ren::vertex::Clean();
    vox::ren::base::Clean();

    return (int)msg.wParam;
}


static ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_FIRSTVOX ) );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_FIRSTVOX );
    wcex.lpszClassName = window_class_;
    wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

    return RegisterClassExW(&wcex);
}

static void WinInit()
{
    ShowCursor( false );
    
    RECT rect; 
    GetClientRect( h_wnd_, &rect );
    MapWindowPoints( h_wnd_, nullptr, reinterpret_cast<POINT*>(&rect), 2 );
    ClipCursor( &rect );

    game_states_ &= ~vox::consts::GAMESTAT_MOUSEENABLED;

    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = 0;
    rid.hwndTarget = NULL;
    if ( RegisterRawInputDevices( &rid, 1, sizeof( rid ) ) == FALSE )
    {
        OutputDebugStringA( "raw input registration failed\n" );
    }
}


static void ResizeRenderer( long new_width, long new_height )
{
    vox::core::gamecore::camera.SetAspectRatio( (float)new_width / (float)new_height );

    HRESULT hr{ S_OK };
    hr = vox::ren::base::ResizeScreen( h_wnd_, new_width, new_height );
    if ( hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET )
    {
        M_LOGERROR( "dx device lost after resize" );
        vox::logger::GLogger << vox::ren::base::GetDeviceRemovedReason();
        vox::logger::GLogger.LogDebugString();
    }
    vox::ren::vertex::ResizeScreen();
}

static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   h_instance_ = hInstance;

   h_wnd_ = CreateWindowW(window_class_, window_title_, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!h_wnd_)
   {
       int a = GetLastError();
       printf( "Oh shi- %d\n", a );
       return FALSE;
   }

   ShowWindow( h_wnd_, nCmdShow );
   UpdateWindow( h_wnd_ );

   return TRUE;
}

static LRESULT CALLBACK WndProc(HWND h_wnd_, UINT message, WPARAM wParam, LPARAM lParam)
{

    if ( 0 == (game_states_ & vox::consts::GAMESTAT_MOUSEENABLED) )
    {
        switch ( message )
        {
        case WM_COMMAND:
        {
            int wmId = LOWORD( wParam );
            switch ( wmId )
            {
            case IDM_ABOUT:
                DialogBox( h_instance_, MAKEINTRESOURCE( IDD_ABOUTBOX ), h_wnd_, About );
                break;
            case IDM_EXIT:
                DestroyWindow( h_wnd_ );
                break;
            default:
                return DefWindowProc( h_wnd_, message, wParam, lParam );
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( h_wnd_, &ps );
            EndPaint( h_wnd_, &ps );
        }
        break;
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
        case WM_EXITSIZEMOVE:
        {

            RECT rect;
            GetClientRect( h_wnd_, &rect );
            const UINT new_width = rect.right = rect.left;
            const UINT new_height = rect.bottom - rect.top;
            MapWindowPoints( h_wnd_, nullptr, reinterpret_cast<POINT*>(&rect), 2 );
            ClipCursor( &rect );

            ResizeRenderer( new_width, new_height );
            break;
        }
        case WM_KEYDOWN:
            switch ( wParam )
            {
            case VK_ESCAPE:
                ShowCursor( true );
                ClipCursor( nullptr );
                game_states_ |= vox::consts::GAMESTAT_MOUSEENABLED;
                break;
            default:
                vox::core::eventhandler::OnKeyPressed(
                    (unsigned short)wParam
                );
                break;
            }
        case WM_KEYUP:
            vox::core::eventhandler::OnKeyReleased(
                (unsigned short)wParam
            );
            break;
        case WM_CHAR:
            vox::core::eventhandler::OnChar(
                (char)wParam
            );
            break;
        case WM_LBUTTONDOWN:
            vox::core::eventhandler::OnMouseLPressed(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        case WM_LBUTTONUP:
            vox::core::eventhandler::OnMouseLReleased(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        case WM_MBUTTONDOWN:
            vox::core::eventhandler::OnMouseMPressed(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        case WM_MBUTTONUP:
            vox::core::eventhandler::OnMouseMReleased(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        case WM_RBUTTONDOWN:
            vox::core::eventhandler::OnMouseRPressed(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        case WM_RBUTTONUP:
            vox::core::eventhandler::OnMouseRReleased(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        //case WM_XBUTTONDOWN:
        //case WM_XBUTTONUP:  
        case WM_MOUSEMOVE:
            vox::core::eventhandler::OnMouseMoved(
                GET_X_LPARAM( lParam ),
                GET_Y_LPARAM( lParam )
            );
            break;
        case WM_MOUSEWHEEL:
            vox::core::eventhandler::OnWheelScrolled(
                GET_WHEEL_DELTA_WPARAM( wParam )
            );
            break;
        case WM_INPUT:
        {
            UINT data_size = 0;
            static BYTE raw_input_buf[sizeof( RAWINPUT )];

            auto res = GetRawInputData( reinterpret_cast<HRAWINPUT>(lParam),
                RID_INPUT, nullptr, &data_size, sizeof( RAWINPUTHEADER ) );
            if ( res == -1 ) break;
            if ( 0 < data_size && data_size <= sizeof( RAWINPUT ) )
            {
                res = GetRawInputData( reinterpret_cast<HRAWINPUT>(lParam),
                    RID_INPUT, raw_input_buf, &data_size, sizeof( RAWINPUTHEADER ) );
                if ( res != data_size ) OutputDebugStringA( "raw input sz is weird 1\n" );

                RAWINPUT* raw_input = (RAWINPUT*)raw_input_buf;
                if ( raw_input->header.dwType == RIM_TYPEMOUSE )
                {
                    vox::core::eventhandler::OnRawMouseInput(
                        raw_input->data.mouse.lLastX, raw_input->data.mouse.lLastY
                    );
                }
            }
            else OutputDebugStringA( "raw input sz is weird 2\n" );

            break;
        }
        default:
            return DefWindowProc( h_wnd_, message, wParam, lParam );
        }
    }
    else  // if ( game_states_ & vox::consts::GAMESTAT_MOUSEENABLED )
    {
        switch ( message )
        {
        case WM_COMMAND:
        {
            int wmId = LOWORD( wParam );
            switch ( wmId )
            {
            case IDM_ABOUT:
                DialogBox( h_instance_, MAKEINTRESOURCE( IDD_ABOUTBOX ), h_wnd_, About );
                break;
            case IDM_EXIT:
                DestroyWindow( h_wnd_ );
                break;
            default:
                return DefWindowProc( h_wnd_, message, wParam, lParam );
            }
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( h_wnd_, &ps );
            EndPaint( h_wnd_, &ps );
        }
        break;
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
        case WM_EXITSIZEMOVE:
        {
            RECT rect;
            GetClientRect( h_wnd_, &rect );

            ResizeRenderer( rect.right - rect.left, rect.bottom - rect.top );
            break;
        }
        case WM_KEYDOWN:
            switch ( wParam )
            {
            case VK_ESCAPE:
            {
                ShowCursor( false );

                RECT rect;
                GetClientRect( h_wnd_, &rect );
                MapWindowPoints( h_wnd_, nullptr, reinterpret_cast<POINT*>(&rect), 2 );
                ClipCursor( &rect );

                game_states_ &= ~vox::consts::GAMESTAT_MOUSEENABLED;
                break;
            }
            }
        default:
            return DefWindowProc( h_wnd_, message, wParam, lParam );
        }
    }
    return 0;
}

static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
