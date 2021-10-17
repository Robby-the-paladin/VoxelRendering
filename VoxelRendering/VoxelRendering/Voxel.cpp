#include "Voxel.h"

Voxel::Voxel(Color _color, int _reflection_k) {
	color = _color;
	reflection_k = _reflection_k;
}

bool operator== (const Voxel& voxel1, const Voxel& voxel2) {
	return ((voxel1.color == voxel2.color) && (voxel1.reflection_k == voxel2.reflection_k));
}

bool operator!= (const Voxel& voxel1, const Voxel& voxel2) {
	return !(voxel1 == voxel2);
}