//#define wall
#include "Tree.h"

Color Tree::grid_build(vector<vector<vector<Voxel>>>& mat, Vec3 beg, Vec3 end, vector<glm::vec4>& grid_buffer) {
	vector<Color> colors;
	for (int i = beg.x; i < end.x; i++) {
		for (int j = beg.y; j < end.y; j++) {
			for (int k = beg.z; k < end.z; k++) {
				if (mat.size() <= k || mat[k].size() <= j || mat[k][j].size() <= i)
					grid_buffer.push_back(glm::vec4(0, 0, 0, 0));
				else {
					auto mat_cell = mat[k][j][i];
					grid_buffer.push_back(glm::vec4(mat_cell.color.r / 255.0, mat_cell.color.g / 255.0, mat_cell.color.b / 255.0, !mat_cell.empty));
					colors.push_back(mat_cell.color);
				}
			}
		}
	}

	Color color = Color(0, 0, 0, 1);
	for (int i = 0; i < colors.size(); i++) {
		color.r += colors[i].r;
		color.g += colors[i].g;
		color.b += colors[i].b;
	}
	if (colors.size()) {
		color.r /= colors.size();
		color.g /= colors.size();
		color.b /= colors.size();
	}
	return color;
}

Node* Tree::recursive_build(vector<vector<vector<Voxel>>>& mat, Vec3 coords0, Vec3 coords1, vector<glm::vec4>& grid_buffer, int grid_depth) {
	// Запись терминальной вершины
	if (coords0 + Vec3(1, 1, 1) == coords1) {
		if (mat.size() < coords1.x || mat[coords0.x].size() < coords1.y
			|| mat[coords0.x][coords0.y].size() < coords1.z) {
			Node* ans = new Node();
			ans->terminal = 1;
			ans->voxel.empty = true;
			return ans;
		}
		Node* ans = new Node();
		ans->terminal = 1;
		ans->voxel = mat[coords0.x][coords0.y][coords0.z];
		return ans;
	}
	// Генерация полной воксельной сетки
	if (grid_depth == 0) {
		Node* ans = new Node();
		ans->terminal = 1;
		ans->grid_offset = grid_buffer.size();
		ans->voxel.color = grid_build(mat, coords0, coords1, grid_buffer);
		ans->voxel.empty = false;
		return ans;
	}
	Node* children[8];
	vector<Color> colors;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Vec3 add = Vec3(k * (coords1.x - coords0.x) / 2, j * (coords1.y - coords0.y) / 2, i * (coords1.z - coords0.z) / 2);
				children[i * 4 + j * 2 + k] = recursive_build(mat, coords0 + add, ((coords1 + coords0) / 2) + add, grid_buffer, grid_depth - 1);
				if (!children[i * 4 + j * 2 + k]->terminal || !children[i * 4 + j * 2 + k]->voxel.empty)
					colors.push_back(children[i * 4 + j * 2 + k]->voxel.color);
			}
		}
	}
	auto res = new Node(children);
	// Поиск среднего цвета вершины
	res->voxel.color = Color(0, 0, 0, 1);
	for (int i = 0; i < colors.size(); i++) {
		res->voxel.color.r += colors[i].r;
		res->voxel.color.g += colors[i].g;
		res->voxel.color.b += colors[i].b;
	}
	if (colors.size()) {
		res->voxel.color.r /= colors.size();
		res->voxel.color.g /= colors.size();
		res->voxel.color.b /= colors.size();
	}
	return res;
}

void Tree::build(vector<vector<vector<Voxel>>>& mat, vector<glm::vec4>& grid_buffer, int grid_depth) {
	max_size = max(max(mat.size(), mat[0].size()), mat[0][0].size());
	max_size = (1 << int(log(max_size - 1) / log(2) + 1));
	//cout << "\n Max size = " << max_size << " real size " << mat.size() << " " << mat[0].size() << " " << mat[0][0].size() << "\n";
	root = *recursive_build(mat, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), grid_buffer, grid_depth);
}

void Tree::recursive_destroy(Node* node) {
	for (int i = 0; i < 8; i++) {
		if (node->children[i] != nullptr) {
			recursive_destroy(node->children[i]);
		}
	}
	delete node;
}

void Tree::destroy() {
	recursive_destroy(&root);
}

Voxel* Tree::recursive_get(Node* node, Vec3 l, Vec3 r, Vec3 coords, vector<glm::vec4>& grid_buffer) {
	if (!coords.belongs(l, r) || node == nullptr) {
		return nullptr;
	}
	if (node->terminal) {
		return &(node->voxel);
	}
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Node* child = node->children[i * 4 + j * 2 + k];
				Vec3 add = Vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
				Voxel* result = recursive_get(child, l + add, ((r + l) / 2) + add, coords, grid_buffer);
				if (result != nullptr) 
					return result;
			}
		}
	}
	return nullptr;
}

Voxel* Tree::get(Vec3 coords, vector<glm::vec4>& grid_buffer) {
	return recursive_get(&root, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), coords, grid_buffer);
}

void Tree::push(Node* node) {
	if (node->terminal) {
		for (int i = 0; i < 8; i++) {
			if (node->children[i] != nullptr) {
				node->children[i]->terminal = true;
				node->children[i]->voxel = node->voxel;
			}
		}
	}
}

void Tree::recursive_set(Node* node, Vec3 l, Vec3 r, Vec3 coords0, Vec3 coords1, Voxel value, vector<glm::vec4>& grid_buffer) {
	if (node == nullptr) {
		return;
	}
	if (l.belongs(coords0, coords1) && r.belongs(coords0, coords1 + Vec3(1, 1, 1))) {
		node->terminal = true;
		node->voxel = value;
		return;
	}
	if (!intersected(l, r, coords0, coords1)) {
		return;
	}
	push(node);
	node->terminal = false;
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Node* child = node->children[i * 4 + j * 2 + k];
				Vec3 add = Vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
				recursive_set(child, l + add, ((r + l) / 2) + add, coords0, coords1, value, grid_buffer);
			}
		}
	}
}

void Tree::set(Vec3 coords0, Vec3 coords1, Voxel value, vector<glm::vec4>& grid_buffer) {
	recursive_set(&root, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), coords0, coords1, value, grid_buffer);
}