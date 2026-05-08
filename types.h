#ifndef __TYPES__
#define __TYPES__

typedef struct point_s
{
	int x;
	int y;
} Point;

typedef struct pointf_s
{
	float x;
	float y;
} PointF;

typedef struct size_s
{
	int width;
	int height;
} Size;

typedef struct sizef_s
{
	int width;
	int height;
} SizeF;

typedef struct rect_s
{
	Point point;
	Size size;
} Rect;

typedef struct rectf_s
{
	PointF point;
	SizeF size;
} RectF;

#endif // __TYPES__
