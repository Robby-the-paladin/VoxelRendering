#pragma once
#include "Color.h"
class Voxel {
public:
	Color color;
	int reflection_k;
	Voxel() {};
	Voxel(Color color_, int reflection_k_);
};

