#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "viewport.h"

#define CLASSNAME "WindowClassName"
static HWND hwnd;
static HBRUSH background = 0;
static HBRUSH canvas = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		if (background)
			DeleteObject(background);

		if (canvas)
			DeleteObject(canvas);

		PostQuitMessage(0);
		return 1;
	}

	if (msg == WM_SIZE)
	{
		if (set_viewport_window(LOWORD(lparam), HIWORD(lparam)))
		{
			InvalidateRect(hwnd, NULL, TRUE);
			return 1;
		}
	}

	if (msg == WM_PAINT)
	{
		PAINTSTRUCT ps;
	  HDC hdc = BeginPaint(hwnd, &ps);
		Rect r = get_canvas_client_bounds();
		RECT rect;
		rect.left = r.point.x;
		rect.top = r.point.y;
		rect.right = r.point.x + r.size.width;
		rect.bottom = r.point.y + r.size.height;
		FillRect(hdc, &rect, canvas);
    EndPaint(hwnd, &ps);
		return 1;
	}

	// case WM_LBUTTONDOWN:
	// case WM_RBUTTONDOWN:
	// case WM_MBUTTONDOWN:
	// 	handleMousePress(0, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_KEYSTATE_WPARAM(wparam));
	// 	break;

	// case WM_LBUTTONUP:
	// case WM_RBUTTONUP:
	// case WM_MBUTTONUP:
	// 	handleMouseRelease(0, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_KEYSTATE_WPARAM(wparam));
	// 	break;

	// case WM_MOUSEMOVE:
	// 	handleMouseMove(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_KEYSTATE_WPARAM(wparam));
	// 	break;

	// case WM_MOUSEWHEEL:
	// 	handleMouseWheel(0, GET_WHEEL_DELTA_WPARAM(wparam), GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_KEYSTATE_WPARAM(wparam));
	// 	break;

	// case WM_MOUSEHWHEEL:
	// 	handleMouseWheel(GET_WHEEL_DELTA_WPARAM(wparam), 0, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_KEYSTATE_WPARAM(wparam));
	// 	break;

	// case WM_KEYDOWN:
	// 	handleKeyPress(wparam, lparam);
	// 	break;

	// case WM_KEYUP:
	// 	handleKeyRelease(wparam, lparam);
	// 	break;

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main()
{
	HINSTANCE instance = GetModuleHandle(0);
	WNDCLASS wc = {0};

	background = CreateSolidBrush(RGB(48, 48, 48));
	wc.hInstance = instance;
	wc.hbrBackground = background;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = CLASSNAME;
	wc.lpfnWndProc = WndProc;

	if (!RegisterClassA(&wc))
	{
		return -1;
	}

	hwnd = CreateWindowEx(0, CLASSNAME, "Spice GUI", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 800, 600, NULL, NULL, instance, NULL);
	if (hwnd == NULL)
	{
		return -1;
	}

	canvas = CreateSolidBrush(RGB(255,255,255));
	set_canvas_size(6.0, 6.0);
	ShowWindow(hwnd, SW_NORMAL);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
