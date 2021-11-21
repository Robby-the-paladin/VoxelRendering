#pragma once
#include <vector>
#include <queue>
#include "Node.h"
#include "Vec3.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;
class Tree {
private:
	Node* recursive_build(vector<vector<vector<Voxel>>>* mat, Vec3 coords0, Vec3 coords1);
	void recursive_destroy(Node* node);
	Voxel* recursive_get(Node* node, Vec3 l, Vec3 r, Vec3 coords);
	void recursive_set(Node* node, Vec3 l, Vec3 r, Vec3 coords0, Vec3 coords1, Voxel value);
	void push(Node* node);

	struct Sh_node {
		GLint children[8];
		GLint terminal_empty_texture_using[4];
		GLfloat color_refl[4];
	};
	vector<Sh_node> buffer;
	GLuint ssbo = 0;

	void update_buffer(Shader* shader);
public:
	Node root;
	int max_size = 0;

	Tree() {};

	void load_vox_file(string name, Shader* shader);
	void build(vector<vector<vector<Voxel>>> mat, Shader* shader);
	void destroy();
	void set(Vec3 coords0, Vec3 coords1, Voxel value, Shader* shader);
	Voxel* get(Vec3 coords);
	
	void shader_serializing(Shader* shader);
};

