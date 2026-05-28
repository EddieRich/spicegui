#ifndef __VIEWPORT__
#define __VIEWPORT__

int set_viewport_window(int width, int height);
int set_canvas_size(float width, float height);
float canvas_width(void);
float canvas_height(void);
int client_x(float x);
int client_y(float y);
int client_size(float sz);
int point_in_window(int x, int y);
void zoom_in_at(int x, int y);

#endif // __VIEWPORT__
