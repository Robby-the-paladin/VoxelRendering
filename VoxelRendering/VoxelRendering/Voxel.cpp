#include "Voxel.h"

Voxel::Voxel(Color _color, int _reflection_k, bool _empty) {
	empty = _empty;
	color = _color;
	reflection_k = _reflection_k;
	
}

bool operator== (const Voxel& voxel1, const Voxel& voxel2) {
	return (voxel1.empty == voxel2.empty && (voxel1.empty ||
		((voxel1.color == voxel2.color) && (voxel1.reflection_k == voxel2.reflection_k))));
}

bool operator!= (const Voxel& voxel1, const Voxel& voxel2) {
	return !(voxel1 == voxel2);
}