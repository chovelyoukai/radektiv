#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>

void initWindowSystem(void);
bool createWindow(int width, int height, char *name);
bool shouldWindowClose(void);
void updateWindow(void);
void destroyWindow(void);
float getAspectRatio(void);
float getTime(void);
unsigned int getInputs(void);
bool getFocused(void);
void resetMousePos(void);
void captureMouse(void);
void releaseMouse(void);
void getMousePos(float *x, float *y);

#endif
