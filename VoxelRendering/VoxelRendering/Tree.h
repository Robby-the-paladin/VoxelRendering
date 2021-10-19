#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "Node.h"
#include "Vec3.h"
using namespace std;
class Tree {
private:
	int max_size = 0;
	Node* recursive_build(vector<vector<vector<Voxel>>>* mat, Vec3 coords0, Vec3 coords1);
	void recursive_destroy(Node* node);
	Voxel* recursive_get(Node* node, Vec3 l, Vec3 r, Vec3 coords);
	void recursive_set(Node* node, Vec3 l, Vec3 r, Vec3 coords0, Vec3 coords1, Voxel value);
	void push(Node* node);
public:
	Node root;

	Tree() {};

	void build(vector<vector<vector<Voxel>>> mat);
	void destroy();
	void set(Vec3 coords0, Vec3 coords1, Voxel value);
	Voxel* get(Vec3 coords);
};

