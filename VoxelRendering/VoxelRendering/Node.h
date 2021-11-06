#pragma once
#include "Voxel.h"

class Node {
public:
	bool terminal = true;
	Voxel voxel;
	Node* children[8];

	Node() {};
	Node(Node* _children[8]);
};

