#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <wingdi.h>
#include <stdio.h>
#include <math.h>

#include "platform.h"
#include "viewport.h"
#include "schematic.h"

#define CLASSNAME "WindowClassName"
static HWND hwnd;
static HDC hdc;
static HPEN pen[3];
static PenThickness last_pt = PT_THIN;

// the font structures are platform specific, so they are defined in the platform.c files
struct s_glyph {
	POINT* vertex;
	int* start_index;
	int nVertices;
	int nIndeces;
};

static struct s_stickfont {
	struct s_glyph* glyph;
	POINT min;
	POINT max;
} stick_font = {0};

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
	sincosf(deg2rad(start_angle), &fsin, &fcos);
	int x3 = client_x(center_x + fcos * a);
	int y3 = client_y(center_y - fsin * b);
	sincosf(deg2rad(end_angle), &fsin, &fcos);
	int x4 = client_x(center_x + fcos * a);
	int y4 = client_y(center_y - fsin * b);

	if (pt != last_pt)
	{
		last_pt = pt;
		SelectObject(hdc, pen[pt]);
	}

	Arc(hdc, x1, y1, x2, y2, x3, y3, x4, y4);
}

void DrawString(float left, float center_y, char *text)
{
	left = center_y;
	center_y = left;
	text = text;
	int fw = stick_font.max.x - stick_font.min.x;
	int fh = stick_font.max.y - stick_font.min.y;
	int cx = client_x(0.0) + fw;
	int cy = client_y(0.0) + fh;

	for (int g= 0; g < 96; g++)
	{
		SetViewportOrgEx(hdc, cx + ((g % 16) * fw), cy + ((g / 16) * fh), NULL);
		int s = 0;
		int v = 0;
		while (v < stick_font.glyph[g].nVertices)
		{
			Polyline(hdc,
			 &stick_font.glyph[g].vertex[stick_font.glyph[g].start_index[s]],
			stick_font.glyph[g].start_index[s+1] - stick_font.glyph[g].start_index[s]);

			s++;
			v = stick_font.glyph[g].start_index[s];
		}
	}
}

void UnloadFont()
{
	if (stick_font.glyph == NULL)
		return;

	for (int i = 0; i < 96; i++)
	{
		if (stick_font.glyph[i].vertex != NULL)
			free(stick_font.glyph[i].vertex);
		if (stick_font.glyph[i].start_index != NULL)
			free(stick_font.glyph[i].start_index);
	}

	free(stick_font.glyph);
}

void LoadFont(char *filepath)
{
	char line[1024];
	FILE* fp = fopen(filepath, "rb");
	if (fp == NULL)
		return;

	if (stick_font.glyph == NULL)
		stick_font.glyph = calloc(96, sizeof(struct s_glyph));

	for (int g = 0; g < 96; g++)
	{
		memset(line, 0, 1024);
		fgets(line, 1024, fp);

		// count 'pen-up's and actual vertices
		stick_font.glyph[g].nVertices = 0;
		stick_font.glyph[g].nIndeces = 2;
		for (int bi = 10; bi < 1024; bi += 2)
		{
			if (line[bi] == '\n' || line[bi] == '\r' || line[bi] == 0)
				break;

			if (line[bi] == ' ')
				stick_font.glyph[g].nIndeces++;
			else
				stick_font.glyph[g].nVertices++;
		}

		if (stick_font.glyph[g].nVertices == 0)
			continue;

		// nVertices is now the actual number of vertices
		stick_font.glyph[g].vertex = calloc(stick_font.glyph[g].nVertices, sizeof(POINT));
		// nIndeces is one greater than the number of line groups
		// The first index is always 0. The last index is the length of the vertex array
		// This makes calculating nPoints mush easier when drawing (see DrawText)
		stick_font.glyph[g].start_index = calloc(stick_font.glyph[g].nIndeces, sizeof(int));
		stick_font.glyph[g].start_index[0] = 0;

		// now the arrays are ready for this glyph, get the data
		int si = 1;
		int vi = 0;
		for (int bi = 10; bi < 1024; bi += 2)
		{
			if (line[bi] == '\n' || line[bi] == '\r' || line[bi] == 0)
				break;

			if (line[bi] == ' ')
			{
				stick_font.glyph[g].start_index[si++] = vi;
			}
			else
			{
				stick_font.glyph[g].vertex[vi].x = line[bi] - 'R';
				stick_font.glyph[g].vertex[vi].y = line[bi+1] - 'R';

				if (stick_font.glyph[g].vertex[vi].x < stick_font.min.x)
				 stick_font.min.x = stick_font.glyph[g].vertex[vi].x;
				if (stick_font.glyph[g].vertex[vi].y < stick_font.min.y)
				 stick_font.min.y = stick_font.glyph[g].vertex[vi].y;
				if (stick_font.glyph[g].vertex[vi].x > stick_font.max.x)
				 stick_font.max.x = stick_font.glyph[g].vertex[vi].x;
				if (stick_font.glyph[g].vertex[vi].y > stick_font.max.y)
				 stick_font.max.y = stick_font.glyph[g].vertex[vi].y;

				vi++;
			}
		}

		stick_font.glyph[g].start_index[si] = vi;
	}

	fclose(fp);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_PAINT)
	{
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		SetMapMode(hdc, MM_ISOTROPIC);
		HGDIOBJ oldpen = SelectObject(hdc, GetStockObject(NULL_PEN));
		HGDIOBJ oldbrush = SelectObject(hdc, GetStockObject(WHITE_BRUSH));
		FillRectangle(0.0, 0.0, canvas_width(), canvas_height());
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		SelectObject(hdc, pen[PT_THIN]);

		render_schematic();

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
		LoadFont("C:/Users/ehandrich/Documents/MyProjects/StickFonts/timesr.jhf");
		pen[PT_THIN] = CreatePen(PS_SOLID, 1, RGB(0,0,0));
		pen[PT_NORMAL] = CreatePen(PS_SOLID, 3, RGB(0,0,0));
		pen[PT_THICK] = CreatePen(PS_SOLID, 5, RGB(0,0,0));
		float sw, sh;
		create_schematic(&sw, &sh);
		set_canvas_size(sw, sh);
	}
	else if (msg == WM_DESTROY)
	{
		UnloadFont();
		DeleteObject(pen[PT_THIN]);
		DeleteObject(pen[PT_NORMAL]);
		DeleteObject(pen[PT_THICK]);
		PostQuitMessage(0);
		return 1LL;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int main()
{
	HINSTANCE instance = GetModuleHandle(0);
	WNDCLASS wc = {0};

	wc.hInstance = instance;
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
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

	ShowWindow(hwnd, SW_NORMAL);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
