#include <GLFW/glfw3.h>
#include <stdio.h>

#include "window.h"

static GLFWwindow *window;

void initWindowSystem(void)
{
	window = NULL;
}

bool createWindow(int width, int height, char *name)
{
	if (window)
	{
		fprintf(stderr, "Window already exists!\n");
		return false;
	}

	if (!glfwInit())
	{
		fprintf(stderr, "Could not init GLFW!\n");
		return false;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(width, height, name, NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Could not open window!\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	captureMouse();
	if (glfwRawMouseMotionSupported())
	{
		printf("Turning on raw mouse input.\n");
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
	{
		int w, h;
		glfwGetWindowSize(window, &w, &h);
		glfwSetCursorPos(window, w / 2.0f, h / 2.0f);
	}
}

bool shouldWindowClose(void)
{
	return glfwWindowShouldClose(window);
}

void updateWindow(void)
{
	glfwPollEvents();
	glfwSwapBuffers(window);
}

void destroyWindow(void)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	window = NULL;
}

float getAspectRatio(void)
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	return (float)w / (float)h;
}

float getTime(void)
{
	return glfwGetTime();
}

bool getFocused(void)
{
	return (glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0);
}

void resetMousePos(void)
{
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	glfwSetCursorPos(window, w / 2.0f, h / 2.0f);
}

void captureMouse(void)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void releaseMouse(void)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void getMousePos(float *x, float *y)
{
	if (glfwGetWindowAttrib(window, GLFW_FOCUSED) == 0)
	{
		 *x = *y = 0.0f;
		 return;
	}

	double xd, yd;
	glfwGetCursorPos(window, &xd, &yd);
	resetMousePos();

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	*x = xd - (w / 2.0f);
	*y = yd - (h / 2.0f);
}
