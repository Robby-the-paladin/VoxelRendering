#pragma once
#include "Voxel.h"

class Node {
public:
	bool terminal = true;
	int grid_offset = -1;
	Voxel voxel;
	Node* children[8];

	Node();
	Node(Node* _children[8]);
};

