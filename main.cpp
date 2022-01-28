#include "bs.h"
#include <uxtheme.h>
#include <vssym32.h>
#include <dwmapi.h>
#include <algorithm>
#include <time.h>

#define TITLE_BAR_BIG_SIZE 32
#define TITLE_BAR_SMALL_SIZE 28
#define FULLSCREEN_COMP 8
#define BG_COLOR 0x171717

HANDLE g_exit_lock;
const auto g_exit_lock_name = L"EdiTorExitLock";
HWND g_main_window;
bool g_fillscreen = false;
bool g_active;
int g_title_bar_size;

struct ScreenBuffer {
	BITMAPINFO info;
	void* mem;

	const LONG width() const
	{
		return info.bmiHeader.biWidth;
	}

	const LONG height() const
	{
		return -info.bmiHeader.biHeight;
	}
};

static ScreenBuffer g_window_buffer;

void debug_log(const char* const msg)
{
	SYSTEMTIME time;
	GetSystemTime(&time);
	printf("%i:%i ", time.wSecond, time.wMilliseconds);
	printf(msg);
}

BOOL WINAPI console_handler_routine(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
			{
				SendMessageW(g_main_window, WM_CLOSE, 0, 0); // Send Message to close main thread

				if (ctrl_type == CTRL_CLOSE_EVENT)
				{
					// Windows sets a timer to close before sending CTRL_CLOSE_EVENT, that i think is irrevertible!
					WaitForSingleObject(g_exit_lock, INFINITE); // Wait for main thread to reach end as after this, ExitProcess() will be called
					CloseHandle(g_exit_lock);
					Sleep(10); // Just to make sure it has time to return ????
				}
				return 1;
			}
	}
	return 0;
}

bool open_console()
{
	auto ret = AttachConsole(ATTACH_PARENT_PROCESS);
	if (!ret) ret = AllocConsole();
	if (!ret) return false;

	// Try to disable exit from debug log console
	SetConsoleCtrlHandler(console_handler_routine, 1);

	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	freopen("CONIN$", "r", stdin);

	debug_log("Console Started!\n");

	return true;
}

void remove_fillscreen(const HWND hwnd)
{
	g_fillscreen = 0;
	ShowWindow(hwnd, SW_RESTORE);
	SetWindowLongPtrW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
}

void set_fillscreen(const HWND hwnd)
{
	if (IsZoomed(hwnd)) ShowWindow(hwnd, SW_RESTORE);
	g_fillscreen = 1;
	SetWindowLongPtrW(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_THICKFRAME));
	ShowWindow(hwnd, SW_MAXIMIZE);
}

void resize_screen_buffer(ScreenBuffer& buffer, const LONG width, const LONG height)
{
	buffer.info.bmiHeader.biSize = sizeof(buffer.info.bmiHeader);
	buffer.info.bmiHeader.biWidth = width;
	buffer.info.bmiHeader.biHeight = -height;
	buffer.info.bmiHeader.biPlanes = 1;
	buffer.info.bmiHeader.biBitCount = 32;
	buffer.info.bmiHeader.biCompression = BI_RGB;
	buffer.info.bmiHeader.biSizeImage = width * height * 4;
}

void draw_rectangle(const ScreenBuffer& buffer, const int x, const int y, const int width, const int height, const u32 color)
{
	const LONG right = SafeTrunc<LONG>(x + width);
	const LONG bottom = SafeTrunc<LONG>(y + height);

	for (u64 y_pos = y; y_pos < std::min(bottom, buffer.height()); y_pos++)
	{
		for (u64 x_pos = x; x_pos < std::min(right, buffer.width()); x_pos++)
		{
			const auto offset = y_pos * buffer.width() + x_pos;
			u32* pixel = (u32*)buffer.mem + offset;
			*pixel = (u32)((0xff << 24) | color);
			// aa rr gg bb
		}
	}
}

void draw_rectangle(const ScreenBuffer& buffer, const RECT& rect, u32 color)
{
	for (u64 y_pos = rect.top; y_pos < std::min(rect.bottom, buffer.height()); y_pos++)
	{
		for (u64 x_pos = rect.left; x_pos < std::min(rect.right, buffer.width()); x_pos++)
		{
			const auto offset = y_pos * buffer.width() + x_pos;
			u32* pixel = (u32*)buffer.mem + offset;
			*pixel = (u32)((0xff << 24) | color);
			// aa rr gg bb
		}
	}
}

enum class NCButton { Close, Maximize, Minimize };

RECT get_nc_button_rect(const HWND hwnd, const RECT& rect, const LONG title_bar_size, const NCButton button)
{
	const int margin = IsZoomed(hwnd) ? 5 : 8;
	const int button_width = 30;
	const int margin_bottom = 5;
	const int space = 3;

	switch (button)
	{
		case NCButton::Close:
			return {rect.right - margin - button_width, rect.top + margin, rect.right - margin, rect.top + title_bar_size - margin_bottom};

		case NCButton::Maximize:
			return {rect.right - margin - (2*button_width) - space, rect.top + margin, rect.right - margin - button_width - space, rect.top + title_bar_size - margin_bottom};

		case NCButton::Minimize:
			return {rect.right - margin - (3*button_width) - (2*space), rect.top + margin, rect.right - margin - (2*button_width) - (2*space), rect.top + title_bar_size - margin_bottom};
	}
	return {};
}

bool intersect_rect_point(const int x, const int y, const RECT& rect)
{
    return (x >= rect.left && x < rect.right) &&
        (y >= rect.top && y < rect.bottom);
}

struct Label {
	RECT rect;
	const char* text;
};

Label labels[128] = {{.rect = {400, 300, 500, 600}, .text = "ola"}};

RECT button_rects[128] = {};
int button_rects_count = 1;

LRESULT CALLBACK main_window_proc(
		HWND   hwnd,
		UINT   msg,
		WPARAM wparam,
		LPARAM lparam
		)
{
	switch (msg)
	{
		case WM_CREATE:
			{
				const MARGINS margins = {1,1,1,1};

				if (!SUCCEEDED(DwmExtendFrameIntoClientArea(hwnd, &margins)))
					fprintf(stderr, "Failed to DwmExtendFrameIntoClientArea\n");
			}
			return 0;

		case WM_SIZE:
			{
				// WS_POPUP windows can restore,
				// if that occurs remove fillscreen mode imediately!
				if (g_fillscreen && wparam == SIZE_RESTORED)
					remove_fillscreen(hwnd);

				auto width = LOWORD(lparam);
				auto height = HIWORD(lparam);
				if (IsZoomed(hwnd))
				{
					g_title_bar_size = TITLE_BAR_SMALL_SIZE;
					if (!g_fillscreen)
					{
						width -= FULLSCREEN_COMP * 2;
						height -= FULLSCREEN_COMP * 2;
					}
				}
				else g_title_bar_size = TITLE_BAR_BIG_SIZE;
				resize_screen_buffer(g_window_buffer, width, height);
			}
			return 0;

		case WM_NCHITTEST:
			{
				auto x = GET_X_LPARAM(lparam);
				auto y = GET_Y_LPARAM(lparam);

				RECT client_rect;
				GetClientRect(hwnd, &client_rect);
				ClientToScreen(hwnd, (POINT*)&client_rect.left);
				ClientToScreen(hwnd, (POINT*)&client_rect.right);

				if (IsZoomed(hwnd) && !g_fillscreen)
				{
					client_rect.top += FULLSCREEN_COMP;
					client_rect.bottom -= FULLSCREEN_COMP;
					client_rect.left += FULLSCREEN_COMP;
					client_rect.right -= FULLSCREEN_COMP;
				}

				if (intersect_rect_point(x, y, get_nc_button_rect(hwnd, client_rect, g_title_bar_size, NCButton::Close)))
					return HTCLOSE;
				else if (intersect_rect_point(x, y, get_nc_button_rect(hwnd, client_rect, g_title_bar_size, NCButton::Maximize)))
					return HTMAXBUTTON;
				else if (intersect_rect_point(x, y, get_nc_button_rect(hwnd, client_rect, g_title_bar_size, NCButton::Minimize)))
					return HTMINBUTTON;
				else
				{
					if (!IsZoomed(hwnd))
					{
						const auto border = 10;
						const auto corner_border = 15;
						const auto resize = (x < client_rect.left + corner_border) * 0b0001 +
							(y < client_rect.top + corner_border) * 0b0010 +
							(x > client_rect.right - corner_border) * 0b0100 +
							(y > client_rect.bottom - corner_border) * 0b1000;

						if ((x < client_rect.left + border) ||
								(y < client_rect.top + border) ||
								(x > client_rect.right - border) ||
								(y > client_rect.bottom - border))
						{
							switch (resize)
							{
								case 0b0011: return HTTOPLEFT;
								case 0b0110: return HTTOPRIGHT;
								case 0b1001: return HTBOTTOMLEFT;
								case 0b1100: return HTBOTTOMRIGHT;
								case 0b0001: return HTLEFT;
								case 0b0010: return HTTOP;
								case 0b0100: return HTRIGHT;
								case 0b1000: return HTBOTTOM;
							}
						}
					}

					if (y < client_rect.top + g_title_bar_size) return HTCAPTION;
				}
			}
			return HTCLIENT;

		case WM_NCLBUTTONDOWN:
			{
				// if close maximize or minimize buttons are hitted and nclbuttondown gets called, but doesnt return 0, some weird win 95 buttons appear
				// only caption and resizing need to not responde because windows treats of dragging behaviour
				if (wparam == HTCAPTION || wparam >= HTLEFT && wparam <= HTBOTTOMRIGHT) break;
			}
			return 0;

		case WM_NCRBUTTONUP:
			{
				if (wparam == HTCAPTION)
				{
					SendMessageW(hwnd, WM_SYSCOMMAND, SC_MOUSEMENU, lparam);
					return 0;
				}
			}
			break;

		case WM_SYSCOMMAND:
			{
				auto actual_wparam = wparam & 0xFFF0;
				switch (actual_wparam)
				{
					case SC_KEYMENU:
						if (lparam != ' ') break;

						RECT client_rect;
						GetClientRect(hwnd, &client_rect);
						ClientToScreen(hwnd, (POINT*)&client_rect.left);

						if (IsZoomed(hwnd) && !g_fillscreen)
						{
							client_rect.top += FULLSCREEN_COMP;
							client_rect.left += FULLSCREEN_COMP;
						}

						// fall through

					case SC_MOUSEMENU:
						const auto menu = GetSystemMenu(hwnd, 0);
						MENUITEMINFOW menu_item_info = {.cbSize = sizeof(menu_item_info), .fMask = MIIM_STATE, .fState = MFS_DEFAULT};
						SetMenuItemInfoW(menu, SC_CLOSE, 0, &menu_item_info);

						auto x = GET_X_LPARAM(lparam);
						auto y = GET_Y_LPARAM(lparam);
						if (actual_wparam == SC_KEYMENU)
						{
							x = client_rect.left;
							y = client_rect.top + g_title_bar_size;
						}

						const auto menu_item = TrackPopupMenuEx(menu, TPM_RETURNCMD, x, y, hwnd, 0);
						SendMessageW(hwnd, WM_SYSCOMMAND, menu_item, 0);
						return 0;
				}
			}
			break;

		case WM_NCLBUTTONUP:
			{
				switch (wparam)
				{
					case HTCLOSE:
						DestroyWindow(hwnd);
						return 0;
					case HTMAXBUTTON:
						ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
						return 0;
					case HTMINBUTTON:
						ShowWindow(hwnd, SW_MINIMIZE);
						return 0;
				}
			}
			break;

		case WM_NCCALCSIZE:
			return 0;

		case WM_NCACTIVATE:
			g_active = wparam != 0;
			if (g_window_buffer.mem) RedrawWindow(hwnd, 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
			return 0;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc = BeginPaint(hwnd, &ps);

				const auto window_width = g_window_buffer.width();
				const auto window_height = g_window_buffer.height();

				const auto src_dc = CreateCompatibleDC(dc);

				void* mem;
				const auto bitmap = CreateDIBSection(src_dc, &g_window_buffer.info, DIB_RGB_COLORS, &mem, 0, 0);
				SelectObject(src_dc, bitmap);
				g_window_buffer.mem = mem;

				// draw titlebar
				const u32 title_bar_color = g_active ? BG_COLOR : 0x2e2e2e;
				draw_rectangle(g_window_buffer, 0, 0, window_width, g_title_bar_size, title_bar_color);

				// draw title bar buttons
				const RECT window_rect = {0, 0, window_width, window_height};
				const auto close_button_rect = get_nc_button_rect(hwnd, window_rect, g_title_bar_size, NCButton::Close);
				draw_rectangle(g_window_buffer, close_button_rect, 0x3d395b);
				const auto max_button_rect = get_nc_button_rect(hwnd, window_rect, g_title_bar_size, NCButton::Maximize);
				draw_rectangle(g_window_buffer, max_button_rect, 0x3d395b);
				const auto min_button_rect = get_nc_button_rect(hwnd, window_rect, g_title_bar_size, NCButton::Minimize);
				draw_rectangle(g_window_buffer, min_button_rect, 0x3d395b);

				// draw client
				draw_rectangle(g_window_buffer, 0, g_title_bar_size, window_width, window_height - g_title_bar_size, BG_COLOR);
				RECT button_rect = {0, g_title_bar_size, 200, 140};
				draw_rectangle(g_window_buffer, button_rect, 0xFFFFFF);
#if 0
				for (int i = 0; i < button_rects_count; i++)
				{
					auto& button_rect = button_rects[i];
				}
#endif

				auto margin = 0;
				if (IsZoomed(hwnd) && !g_fillscreen)
					margin = FULLSCREEN_COMP;

				BitBlt(dc, margin, margin, window_width, window_height, src_dc, 0, 0, SRCCOPY);

				LOGFONTW log_font;
				GetThemeSysFont(0, TMT_CAPTIONFONT, &log_font);
				RECT text_rect = {margin + 10, margin + 1, min_button_rect.left - 10, margin + g_title_bar_size};
				auto font = CreateFontIndirectW(&log_font);
				SelectObject(dc, font);
				SetBkMode(dc, TRANSPARENT);
				const u32 title_bar_text_color = g_active ? 0xFFFFFF : 0x787878;
				SetTextColor(dc, title_bar_text_color);
				wchar_t title[128];
				GetWindowTextW(hwnd, title, ARR_COUNT(title));
				DrawTextW(dc, title, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP | DT_WORD_ELLIPSIS);

				DeleteObject(bitmap);
				DeleteDC(src_dc);

				EndPaint(hwnd, &ps);
			}
			return 0;

		case WM_LBUTTONUP:
			{
				auto x = GET_X_LPARAM(lparam);
				auto y = GET_Y_LPARAM(lparam);

				if (IsZoomed(hwnd) && !g_fillscreen)
				{
					x -= FULLSCREEN_COMP;
					y -= FULLSCREEN_COMP;
				}

				for (int i = 0; i < button_rects_count; i++)
				{
					if (intersect_rect_point(x, y, button_rects[i]))
						debug_log("button clicked\n");
				}
			}
			return 0;

		case WM_KEYDOWN:
			{
				auto repeating = lparam & (1 << 30);
				switch (wparam)
				{
					case VK_ESCAPE:
						{
							if (!repeating && g_fillscreen)
								remove_fillscreen(hwnd);
						}
						return 0;

					case VK_F11:
						{
							if (!repeating)
							{
								if (!g_fillscreen)
									set_fillscreen(hwnd);
								else
									remove_fillscreen(hwnd);
							}
						}
						return 0;
				}
			}
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

typedef int SetPreferredAppMode(int);

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
	// Create Semaphore to coordenate exit
	g_exit_lock = CreateSemaphoreW(0, 1, 1, g_exit_lock_name);
	if (!g_exit_lock)
	{
		fprintf(stderr, "Failed to CreateSemaphoreW\n");
		return 1;
	}

#ifdef M_INTERNAL
	if (!open_console())
	{
		fprintf(stderr, "Failed to open console\n");
		return 1;
	}
#endif

	auto uxtheme_dll = LoadLibraryW(L"uxtheme.dll");
	if (uxtheme_dll)
	{
		auto ord135 = GetProcAddress(uxtheme_dll, MAKEINTRESOURCE(135));
		((SetPreferredAppMode*)ord135)(1);
	}

	const auto main_window_class_name = L"EdiTorWindowClass";

	WNDCLASSW window_class = {};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = main_window_proc;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursorA(0, IDC_ARROW);
	window_class.lpszClassName = main_window_class_name;
	RegisterClassW(&window_class);

	const auto window_styles = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	g_main_window = CreateWindowExW(0, main_window_class_name, L"EdiTK", window_styles, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

	MSG msg;
	while(GetMessageW(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	debug_log("Program end\n");

	// Can unblock already reached the end
	WaitForSingleObject(g_exit_lock, 0);

	return 0;
}

