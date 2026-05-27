#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <math.h>

#include "platform.h"
#include "viewport.h"

#define CLASSNAME "WindowClassName"
static HWND hwnd;
static HDC hdc;
static HPEN pen[3];
static PenThickness last_pt = PT_THIN;

void FillRectangle(float left, float top, float right, float bottom)
{
	Rectangle(hdc, client_x(left), client_y(top), client_x(right), client_y(bottom));
}

void DrawRectangle(float left, float top, float right, float bottom, PenThickness pt)
{
	if (pt != last_pt)
	{
		last_pt = pt;
		SelectObject(hdc, pen[pt]);
	}

	Rectangle(hdc, client_x(left), client_y(top), client_x(right), client_y(bottom));
}

// void FillCircle(float center_x, float center_y, float radius, Color color)
// {
// 	SelectObject(hdc, pen[NONE]);
// 	SelectObject(hdc, brush[color]);
// 	Ellipse(hdc, client_x(center_x - radius), client_y(center_y - radius), client_x(center_x + radius), client_y(center_y + radius));
// 	SelectObject(hdc, brush[NONE]);
// }

void DrawCircle(float center_x, float center_y, float radius, PenThickness pt)
{
	if (pt != last_pt)
	{
		last_pt = pt;
		SelectObject(hdc, pen[pt]);
	}

	Ellipse(hdc, client_x(center_x - radius), client_y(center_y - radius), client_x(center_x + radius), client_y(center_y + radius));
}

void DrawLine(float x1, float y1, float x2, float y2, PenThickness pt)
{
	if (pt != last_pt)
	{
		last_pt = pt;
		SelectObject(hdc, pen[pt]);
	}

	MoveToEx(hdc, client_x(x1), client_y(y1), NULL);
	LineTo(hdc, client_x(x2), client_y(y2));
}

void DrawArc(float center_x, float center_y, float a, float b, float start_angle, float end_angle, PenThickness pt)
{
	int x1 = client_x(center_x - a / 2.0f);
	int y1 = client_y(center_y - b / 2.0f);
	int x2 = client_x(center_x + a / 2.0f);
	int y2 = client_y(center_y + b / 2.0f);
	float fsin, fcos;
	sincosf(start_angle, &fsin, &fcos);
	int x3 = client_x(center_x + fcos * a);
	int y3 = client_y(center_y - fsin * b);
	sincosf(end_angle, &fsin, &fcos);
	int x4 = client_x(center_x + fcos * a);
	int y4 = client_y(center_y - fsin * b);

	if (pt != last_pt)
	{
		last_pt = pt;
		SelectObject(hdc, pen[pt]);
	}

	Arc(hdc, x1, y1, x2, y2, x3, y3, x4, y4);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_PAINT)
	{
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		HGDIOBJ oldpen = SelectObject(hdc, GetStockObject(NULL_PEN));
		HGDIOBJ oldbrush = SelectObject(hdc, GetStockObject(WHITE_BRUSH));
		FillRectangle(0.0, 0.0, canvas_width(), canvas_height());
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		SelectObject(hdc, pen[PT_THIN]);

		DrawRectangle(2.0, 2.0, 3.0, 3.0, PT_THIN);
		DrawCircle(2.5, 2.5, 0.25, PT_NORMAL);
		DrawLine(1.0, 1.0, 9.5, 1.0, PT_THICK);
		DrawArc(7.0, 3.0, 2.0, 1.0, deg2rad(45), deg2rad(135), PT_NORMAL);

		DrawLine(6.8, 6.9, 7.2, 6.9, PT_THIN);
		DrawArc(7.0, 8.0, 1.0, 2.0, deg2rad(70), deg2rad(110), PT_THIN);

		SelectObject(hdc, oldbrush);
		SelectObject(hdc, oldpen);
		EndPaint(hwnd, &ps);
		return 1LL;
	}
	else if (msg == WM_KEYUP)
	{
		if (wparam == VK_ADD)
		{
			POINT cursor;
			GetCursorPos(&cursor);
			ScreenToClient(hwnd, &cursor);
			if (point_in_window(cursor.x, cursor.y))
			{
				zoom_in_at(cursor.x, cursor.y);
				InvalidateRect(hwnd, NULL, TRUE);
				return 1LL;
			}
		}
	}
	else if (msg == WM_SIZE)
	{
		if (set_viewport_window(LOWORD(lparam), HIWORD(lparam)))
		{
			InvalidateRect(hwnd, NULL, TRUE);
			return 1LL;
		}
	}
	else if (msg == WM_CREATE)
	{
		pen[PT_THIN] = CreatePen(PS_SOLID, 1, RGB(0,0,0));
		pen[PT_NORMAL] = CreatePen(PS_SOLID, 3, RGB(0,0,0));
		pen[PT_THICK] = CreatePen(PS_SOLID, 5, RGB(0,0,0));
	}
	else if (msg == WM_DESTROY)
	{
		DeleteObject(pen[PT_THIN]);
		DeleteObject(pen[PT_NORMAL]);
		DeleteObject(pen[PT_THICK]);
		PostQuitMessage(0);
		return 1LL;
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

	set_canvas_size(10.5, 8.0);
	ShowWindow(hwnd, SW_NORMAL);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
