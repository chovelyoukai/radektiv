#include <stdio.h>

#include "inputs.h"
#include "window.h"

bool initWindowSystem(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Could not init GLFW!\n");
		return false;
	}

	return true;
}

void shutdownWindowSystem(void)
{
	glfwTerminate();
}

bool makeWindow(Window *win, int width, int height, char *name)
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	win->window = glfwCreateWindow(width, height, name, NULL, NULL);
	if (!win->window)
	{
		fprintf(stderr, "Could not open window!\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(win->window);
	captureMouse(win);
	if (glfwRawMouseMotionSupported())
	{
		printf("Turning on raw mouse input.\n");
		glfwSetInputMode(win->window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	if (glfwGetWindowAttrib(win->window, GLFW_FOCUSED))
	{
		int w, h;
		glfwGetWindowSize(win->window, &w, &h);
		glfwSetCursorPos(win->window, w / 2.0f, h / 2.0f);
	}
}

bool shouldWindowClose(Window *win)
{
	return glfwWindowShouldClose(win->window);
}

void updateWindow(Window *win)
{
	glfwPollEvents();
	glfwSwapBuffers(win->window);
}

void destroyWindow(Window *win)
{
	glfwDestroyWindow(win->window);
	glfwTerminate();
	win->window = NULL;
}

float getAspectRatio(Window *win)
{
	int w, h;
	glfwGetWindowSize(win->window, &w, &h);
	return (float)w / (float)h;
}

float getTime(void)
{
	return glfwGetTime();
}

bool keyPressed(Window *win, int key)
{
	return (glfwGetKey(win->window, key) == GLFW_PRESS);
}

unsigned int getInputs(Window *win)
{
	unsigned int inputs = 0;

	if (keyPressed(win, GLFW_KEY_W))
		inputs |= IN_FORWARD;

	if (keyPressed(win, GLFW_KEY_S))
		inputs |= IN_BACKWARD;

	if (keyPressed(win, GLFW_KEY_A))
		inputs |= IN_LEFT;

	if (keyPressed(win, GLFW_KEY_D))
		inputs |= IN_RIGHT;

	return inputs;
}

bool getFocused(Window *win)
{
	return (glfwGetWindowAttrib(win->window, GLFW_FOCUSED) != 0);
}

void resetMousePos(Window *win)
{
	int w, h;
	glfwGetWindowSize(win->window, &w, &h);
	glfwSetCursorPos(win->window, w / 2.0f, h / 2.0f);
}

void captureMouse(Window *win)
{
	glfwSetInputMode(win->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void releaseMouse(Window *win)
{
	glfwSetInputMode(win->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void getMousePos(Window *win, float *x, float *y)
{
	if (glfwGetWindowAttrib(win->window, GLFW_FOCUSED) == 0)
	{
		 *x = *y = 0.0f;
		 return;
	}

	double xd, yd;
	glfwGetCursorPos(win->window, &xd, &yd);
	resetMousePos(win);

	int w, h;
	glfwGetWindowSize(win->window, &w, &h);
	*x = xd - (w / 2.0f);
	*y = yd - (h / 2.0f);
}
