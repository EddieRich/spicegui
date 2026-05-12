#include "viewport.h"

static t_Viewport viewport = {0};

static void calculate_layout(void)
{
	float hscale = viewport.zoom * viewport.client_width / viewport.canvas_width;
	float vscale = viewport.zoom * viewport.client_height / viewport.canvas_height;
	viewport.scale = (hscale > vscale) ? vscale : hscale;
	viewport.canvas_client_x = (viewport.client_width - (int)(viewport.canvas_width * viewport.scale)) >> 1;
	viewport.canvas_client_y = (viewport.client_height - (int)(viewport.canvas_height * viewport.scale)) >> 1;
}

int set_viewport_window(int width, int height)
{
	viewport.client_width = width;
	viewport.client_height = height;
	if (viewport.canvas_width > 0.0 && viewport.canvas_height > 0.0)
	{
		calculate_layout();
		return 1;
	}

	return 0;
}

int set_canvas_size(float width, float height)
{
	viewport.canvas_width = width;
	viewport.canvas_height = height;
	if (viewport.client_width && viewport.client_height && viewport.zoom < 0.999)
	{
		viewport.zoom = 1.0;
		calculate_layout();
		return 1;
	}

	return 0;
}

float canvas_width()
{
	return viewport.canvas_width;
}

float canvas_height()
{
	return viewport.canvas_height;
}

int client_x(float x)
{
	return viewport.canvas_client_x + (x * viewport.scale);
}

int client_y(float y)
{
	return viewport.canvas_client_y + (y * viewport.scale);
}

int client_size(float sz)
{
	return sz * viewport.scale;
}
