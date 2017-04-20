#ifndef __FLUID_H__
#define __FLUID_H__

#include "engine/compute.h"

void FluidInit(int width, int height);
void FluidUpdate(float elapsed_time);
void FluidTouch(bool is_down, float x, float y);
void FluidRotate(int angle);
void FluidSetPrefs(
	std::string fg_color,
	std::string bg_color,
	int iterations,
	int cursor_size
);

#endif // __FLUID_H__
