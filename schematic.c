#include "platform.h"
#include "schematic.h"

void create_schematic(float* pwidth, float*pheight)
{
	*pwidth = 11.0;
	*pheight = 8.5;

	// create whatever is needed for a schematic
}

void render_schematic()
{
	DrawRectangle(2.0, 2.0, 3.0, 3.0, PT_THIN);
	DrawCircle(2.5, 2.5, 0.25, PT_NORMAL);
	DrawLine(1.0, 1.0, 9.5, 1.0, PT_THICK);
	DrawArc(7.0, 3.0, 2.0, 1.0, 45, 135, PT_NORMAL);
	DrawLine(6.8, 6.9, 7.2, 6.9, PT_THIN);
	DrawArc(7.0, 8.0, 1.0, 2.0, 70, 110, PT_THIN);
}
