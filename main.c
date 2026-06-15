#include "raylib.h"

int main(void)
{
	const int screenWidth = 800;
	const int screenHeight = 450;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "Spice GUI");
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();

		ClearBackground(WHITE);

		DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

		EndDrawing();
	}

	CloseWindow(); // Close window and OpenGL context
	return 0;
}
