#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "types.h"
#include "platform.h"
#include "viewport.h"

#define CLASSNAME "WindowClassName"
static HWND hwnd;
static HDC hdc;
static HBRUSH brush[COLOR_COUNT];
static HPEN pen[COLOR_COUNT];

void FillRectangle(Rect rect, Color color)
{
	SelectObject(hdc, pen[NONE]);
	SelectObject(hdc, brush[color]);
	Rectangle(hdc, rect.point.x, rect.point.y, rect.point.x + rect.size.width, rect.point.y + rect.size.height);
	SelectObject(hdc, brush[NONE]);
}

void DrawRectangle(Rect rect, Color color)
{
	SelectObject(hdc, pen[color]);
	Rectangle(hdc, rect.point.x, rect.point.y, rect.point.x + rect.size.width, rect.point.y + rect.size.height);
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
		HGDIOBJ oldbrush = SelectObject(hdc, brush[NONE]);
		HGDIOBJ oldpen = SelectObject(hdc, pen[NONE]);

		FillRectangle(get_canvas_client_bounds(), WHITE);
		RectF foo;
		foo.point.x = 2.0;
		foo.point.y = 2.0;
		foo.size.width = 1.0;
		foo.size.height = 1.0;
		DrawRectangle(rectf_to_client(foo), BLACK);

		SelectObject(hdc, oldbrush);
		SelectObject(hdc, oldpen);
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

void initGDI(void)
{
	brush[NONE] = GetStockObject(NULL_BRUSH);
	brush[BLACK] = GetStockObject(BLACK_BRUSH);
	brush[WHITE] = GetStockObject(WHITE_BRUSH);
	brush[RED] = CreateSolidBrush(RGB(255,0,0));
	brush[YELLOW] = CreateSolidBrush(RGB(255,255,0));
	brush[GREEN] = CreateSolidBrush(RGB(0,255,0));
	brush[CYAN] = CreateSolidBrush(RGB(0,255,255));
	brush[BLUE] = CreateSolidBrush(RGB(0,0,255));
	brush[PURPLE] = CreateSolidBrush(RGB(255,0,255));
	pen[NONE] = GetStockObject(NULL_PEN);
	pen[BLACK] = GetStockObject(BLACK_PEN);
	pen[WHITE] = GetStockObject(WHITE_PEN);
	pen[RED] = CreatePen(PS_SOLID, 1, RGB(255,0,0));
	pen[YELLOW] = CreatePen(PS_SOLID, 1, RGB(255,255,0));
	pen[GREEN] = CreatePen(PS_SOLID, 1, RGB(0,255,0));
	pen[CYAN] = CreatePen(PS_SOLID, 1, RGB(0,255,255));
	pen[BLUE] = CreatePen(PS_SOLID, 1, RGB(0,0,255));
	pen[PURPLE] = CreatePen(PS_SOLID, 1, RGB(255,0,255));
}

void releaseGDI(void)
{
	for (int i = RED; i < COLOR_COUNT; i++)
	{
		DeleteObject(brush[i]);
		DeleteObject(pen[i]);
	}
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

	initGDI();
	set_canvas_size(5, 5);
	ShowWindow(hwnd, SW_NORMAL);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	releaseGDI();
	return 0;
}
