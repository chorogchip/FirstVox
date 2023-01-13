
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"

#include "Consts.h"
#include "Timer.h"
#include "EventHandler.h"
#include "Renderer_for_Winmain.h"
#include "VertexRenderer_for_Winmain.h"
#include "VertexRenderer.h"

#include "GameCore.h"
#include "EventHandler.h"


static ATOM                MyRegisterClass( HINSTANCE hInstance );
static BOOL                InitInstance( HINSTANCE, int );
static LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
static INT_PTR CALLBACK    About( HWND, UINT, WPARAM, LPARAM );
static void WinInit();

HWND h_wnd_;
HINSTANCE h_instance_;
const WCHAR* window_title_ = L"Title";
const WCHAR* window_class_ = L"MyClass";
static uint64_t game_states_ = 0ULL;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    MyRegisterClass(hInstance);
    if ( !InitInstance( hInstance, nCmdShow ) )
    {
        return FALSE;
    }
    WinInit();

    vox::ren::base::Init( h_wnd_ );
    vox::ren::vertex::Init( h_wnd_ );
    vox::core::gamecore::Init();

    HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_FIRSTVOX ) );

    MSG msg{};
    vox::util::Timer timer{};
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
                if ( auto elapsed_t = timer.GetElapsedMicroSec(); elapsed_t >= vox::consts::MICROSEC_PER_TICK )
                {
                    elapsed_t -= vox::consts::MICROSEC_PER_TICK;  // delta time
                    timer.AddTimeMicroSec( vox::consts::MICROSEC_PER_TICK );
                    // update
                    vox::core::eventhandler::Update();
                    vox::core::gamecore::Update();
                }
                else break;
            }

            // render
            vox::ren::base::Clear();
            vox::ren::vertex::Render1();
            vox::ren::base::Present();
        }
    }
    vox::core::gamecore::Quit();

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
        case WM_SIZE:
        {
            UINT new_width = LOWORD( lParam );
            UINT new_height = HIWORD( lParam );

            RECT rect;
            GetClientRect( h_wnd_, &rect );
            MapWindowPoints( h_wnd_, nullptr, reinterpret_cast<POINT*>(&rect), 2 );
            ClipCursor( &rect );

            vox::ren::base::ResizeScreen( h_wnd_, new_width, new_height );
            vox::ren::vertex::Resize();
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
                    (char)wParam
                );
                break;
            }
            VK_ADD;
            break;
        case WM_KEYUP:
            vox::core::eventhandler::OnKeyReleased(
                (char)wParam
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
        case WM_SIZE:
        {
            UINT new_width = LOWORD( lParam );
            UINT new_height = HIWORD( lParam );

            vox::ren::base::ResizeScreen( h_wnd_, new_width, new_height );
            vox::ren::vertex::Resize();
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
