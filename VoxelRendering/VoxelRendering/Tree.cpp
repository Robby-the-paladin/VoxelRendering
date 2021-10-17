#include "Tree.h"

Node* Tree::recursive_build(vector<vector<vector<Voxel>>>* mat, Vec3 coords0, Vec3 coords1) {
	if (coords0 == coords1) {
		if (mat->size() < coords0.x || mat->operator[](coords0.x).size() < coords0.y
			|| mat->operator[](coords0.x)[coords0.y].size()) {
			return nullptr;
		}
		Node* ans = new Node();
		ans->terminal = 1;
		ans->voxel = mat->operator[](coords0.x)[coords0.y][coords0.z];
	}
	Node* children[8];
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Node* child = children[i * 4 + j * 2 + k];
				Vec3 add = Vec3(i * coords1.x / 2, j * coords1.y / 2, k * coords1.z / 2);
				child = recursive_build(mat, coords0 + add, (coords1 / 2) + add);
			}
		}
	}
	return new Node(children);
}

void Tree::build(vector<vector<vector<Voxel>>> mat) {
	max_size = max(max(mat.size(), mat[0].size()), mat[0][0].size());
	max_size = (1 << int(log(max_size - 1) / log(2) + 1));
	root = *recursive_build(&mat, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size));
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

Voxel* Tree::recursive_get(Node* node, Vec3 l, Vec3 r, Vec3 coords) {
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
				Vec3 add = Vec3(i * r.x / 2, j * r.y / 2, k * r.z / 2);
				Voxel* result = recursive_get(child, l + add, (r / 2) + add, coords);
				if (result != nullptr) 
					return result;
			}
		}
	}
	return nullptr;
}

Voxel* Tree::get(Vec3 coords) {
	return recursive_get(&root, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), coords);
}

void Tree::push(Node* node) {
	if (node->terminal) {
		for (int i = 0; i < 8; i++) {
			node->children[i]->terminal = true;
			node->children[i]->voxel = node->voxel;
		}
	}
}

void Tree::recursive_set(Node* node, Vec3 l, Vec3 r, Vec3 coords0, Vec3 coords1, Voxel value) {
	if (node == nullptr) {
		return;
	}
	if (!coords0.belongs(l, r) && !coords1.belongs(l, r)) {
		if (l.belongs(coords0, coords1) && r.belongs(coords0, coords1)) {
			node->terminal = true;
			node->voxel = value;
		}
		return;
	}
	push(node);
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Node* child = node->children[i * 4 + j * 2 + k];
				Vec3 add = Vec3(i * r.x / 2, j * r.y / 2, k * r.z / 2);
				recursive_set(child, l + add, (r / 2) + add, coords0, coords1, value);
			}
		}
	}
}

void Tree::set(Vec3 coords0, Vec3 coords1, Voxel value) {
	recursive_set(&root, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), coords0, coords1, value);
}