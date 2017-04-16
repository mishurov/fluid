#ifndef __FLUID_H__
#define __FLUID_H__

#include "engine/compute.h"

void FluidInit(int width, int height);
void FluidUpdate(float elapsed_time);
void FluidTouch(float x, float y);

#endif // __FLUID_H__
