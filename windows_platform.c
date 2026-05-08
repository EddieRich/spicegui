#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "viewport.h"

#define CLASSNAME "WindowClassName"
static HWND hwnd;
static HDC hdc;

void FillRectangle(Rect rect, unsigned int color)
{
	RECT r;
	r.left = rect.point.x;
	r.top = rect.point.y;
	r.right = rect.point.x + rect.size.width;
	r.bottom = rect.point.y + rect.size.height;
	HBRUSH brush = CreateSolidBrush(color);
	FillRect(hdc, &r, brush);
	DeleteObject(brush);
}

void DrawRectangle(Rect rect, unsigned int color)
{
	HPEN pen = CreatePen(PS_SOLID, 1, color);
	HGDIOBJ oldpen = SelectObject(hdc, pen);
	Rectangle(hdc, rect.point.x, rect.point.y, rect.point.x + rect.size.width, rect.point.y + rect.size.height);
	SelectObject(hdc, oldpen);
	DeleteObject(pen);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
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
	  hdc = BeginPaint(hwnd, &ps);
		FillRectangle(get_canvas_client_bounds(), 0xFFFFFF);
		RectF foo;
		foo.point.x = 2.0;
		foo.point.y = 2.0;
		foo.size.width = 1.0;
		foo.size.height = 1.0;
		DrawRectangle(rectf_to_client(foo), 0xff0000);
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

	wc.hInstance = instance;
	wc.hbrBackground = GetStockObject(DKGRAY_BRUSH);
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

	hdc = GetWindowDC(hwnd);
	set_canvas_size(5, 5);
	ShowWindow(hwnd, SW_NORMAL);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
