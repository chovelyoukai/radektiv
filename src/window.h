#ifndef WINDOW_H
#define WINDOW_H

#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct
{
	GLFWwindow *window;
} Window;

bool initWindowSystem(void);
bool createWindow(Window *win, int width, int height, char *name);
bool shouldWindowClose(Window *win);
void updateWindow(Window *win);
void destroyWindow(Window *win);
float getAspectRatio(Window *win);
float getTime(void);
bool keyPressed(Window *win, int key);
unsigned int getInputs(Window *win);
bool getFocused(Window *win);
void resetMousePos(Window *win);
void captureMouse(Window *win);
void releaseMouse(Window *win);
void getMousePos(Window *win, float *x, float *y);

#endif
