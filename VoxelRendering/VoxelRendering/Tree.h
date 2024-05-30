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
#include <glm.hpp>

using namespace std;

struct Sh_node {
	GLint children[8];
	GLint terminal_empty_texture_using[4];
	GLfloat color_refl[4];
};

class Tree {
private:
	Node* recursive_build(vector<vector<vector<Voxel>>>* mat, Vec3 coords0, Vec3 coords1, vector<glm::vec4>& grid_buffer, int grid_depth);
	void recursive_destroy(Node* node);
	Voxel* recursive_get(Node* node, Vec3 l, Vec3 r, Vec3 coords, vector<glm::vec4>& grid_buffer);
	void recursive_set(Node* node, Vec3 l, Vec3 r, Vec3 coords0, Vec3 coords1, Voxel value, vector<glm::vec4>& grid_buffer);
	void push(Node* node);
	void grid_build(vector<vector<vector<Voxel>>>* mat, Vec3 beg, Vec3 end, vector<glm::vec4>& grid_buffer);

	GLuint ssbo = 0;
public:


	Node root;
	int max_size = 0;

	Tree(){};

	void load_vox_file(string name, vector<glm::vec4>& grid_buffer, int grid_depth = -1);
	void build(vector<vector<vector<Voxel>>> mat, vector<glm::vec4>& grid_buffer, int grid_depth = -1);
	void destroy();
	void set(Vec3 coords0, Vec3 coords1, Voxel value, vector<glm::vec4>& grid_buffer);
	Voxel* get(Vec3 coords, vector<glm::vec4>& grid_buffer);
	
	void shader_serializing(Shader* shader);
};

