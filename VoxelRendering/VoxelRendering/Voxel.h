#pragma once
#include "Color.h"
class Voxel {
public:
	bool empty = true;
	Color color;
	int reflection_k;
	Voxel() {};
	Voxel(Color _color, int _reflection_k, bool _empty);

	friend bool operator== (const Voxel& voxel1, const Voxel& voxel2);

	friend bool operator!= (const Voxel& voxel1, const Voxel& voxel2);
};

