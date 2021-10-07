#pragma once
#include "Color.h"
class Voxel {
public:
	Color color;
	int reflection_k;
	Voxel() {};
	Voxel(Color _color, int _reflection_k);
};

