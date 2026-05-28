#ifndef __PLATFORM__
#define __PLATFORM__

typedef enum ePenThickness
{
	PT_THIN,
	PT_NORMAL,
	PT_THICK
} PenThickness;

void DrawRectangle(float left, float top, float right, float bottom, PenThickness pt);
void DrawCircle(float center_x, float center_y, float radius, PenThickness pt);
void DrawLine(float x1, float y1, float x2, float y2, PenThickness pt);
void DrawArc(float center_x, float center_y, float a, float b, float start_angle, float end_angle, PenThickness pt);

#define _PI_ 3.1415926535897932384626433832795
#define deg2rad(d) ((float)(d * _PI_ / 180.0))

#endif // __PLATFORM__
