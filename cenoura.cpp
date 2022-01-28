#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <unknwn.h>
#include <gdiplus.h>
#pragma comment( lib, "dwmapi" )
#pragma comment( lib, "gdiplus" )
#pragma comment( lib, "user32" )
namespace gdip = Gdiplus;

INT_PTR CALLBACK MyDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    // Initialize GDI+
    gdip::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdipToken = 0;
    gdip::GdiplusStartup( &gdipToken, &gdiplusStartupInput, nullptr );

    struct MyDialog : DLGTEMPLATE {
        WORD dummy[3] = { 0 };  // unused menu, class and title
    }
    dlg;
    dlg.style = WS_POPUP|WS_CAPTION|DS_CENTER;
    dlg.dwExtendedStyle = 0;
    dlg.cdit = 0;  // no controls in template
    dlg.x = 0;
    dlg.y = 0;
    dlg.cx = 300;  // width in dialog units
    dlg.cy = 200;  // height in dialog units

    DialogBoxIndirectW( hInstance, &dlg, nullptr, MyDialogProc );

    gdip::GdiplusShutdown( gdipToken );

    return 0;
}

INT_PTR CALLBACK MyDialogProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
        case WM_INITDIALOG:
        {
            SetWindowTextW( hDlg, L"Borderless Window with Shadow" );

            // This plays together with WM_NCALCSIZE.
            MARGINS m{ 0, 0, 0, 0 };
            DwmExtendFrameIntoClientArea( hDlg, &m );

            // Force the system to recalculate NC area (making it send WM_NCCALCSIZE).
            SetWindowPos( hDlg, nullptr, 0, 0, 0, 0,
                SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);
            return TRUE;
        }
        case WM_NCCALCSIZE:
        {
			auto Arroz = (RECT*)lParam;
			Arroz->top += 10;
			return 0;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps{ 0 };
            HDC hdc = BeginPaint( hDlg, &ps );

            // Draw with GDI+ to make sure the alpha channel is opaque.
            gdip::Graphics gfx{ hdc };
            gdip::SolidBrush brush{ gdip::Color{ 200, 255, 255 } };
            gfx.FillRectangle( &brush, (int)ps.rcPaint.left, (int)ps.rcPaint.top,
                (int)ps.rcPaint.right - ps.rcPaint.left, (int)ps.rcPaint.bottom - ps.rcPaint.top );

            EndPaint( hDlg, &ps );
            return TRUE;
        }
        case WM_NCHITTEST:
        {
            // Returning HTCAPTION allows the user to move the window around by clicking
            // anywhere.
            // Depending on the mouse coordinates passed in LPARAM, you may
            // return other values to enable resizing.
			RECT WindowRect;
			GetWindowRect(hDlg, &WindowRect);
			auto CursorX = GET_X_LPARAM(lParam);
			auto CursorY = GET_Y_LPARAM(lParam);
			if (CursorX >= WindowRect.left && CursorX <= WindowRect.right &&
					CursorY >= WindowRect.top - 10 && CursorY <= WindowRect.top + 20)
				return HTTOP;
			return HTCAPTION;
        }
        case WM_COMMAND:
        {
            WORD id = LOWORD(wParam);
            if( id == IDOK || id == IDCANCEL )
            {
                EndDialog( hDlg, id );
                return TRUE;
            }
            return FALSE;
        }
    }
    return FALSE; // return FALSE to let DefDialogProc handle the message
}
