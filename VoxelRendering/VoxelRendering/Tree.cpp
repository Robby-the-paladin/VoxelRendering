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
				Vec3 add = Vec3(i * (coords1.x - coords0.x) / 2, j * (coords1.y - coords0.y) / 2, k * (coords1.z - coords0.z) / 2);
				child = recursive_build(mat, coords0 + add, ((coords1 + coords0) / 2) + add);
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
				Vec3 add = Vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
				Voxel* result = recursive_get(child, l + add, ((r + l) / 2) + add, coords);
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
				Vec3 add = Vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
				recursive_set(child, l + add, ((r + l) / 2) + add, coords0, coords1, value);
			}
		}
	}
}

void Tree::set(Vec3 coords0, Vec3 coords1, Voxel value) {
	recursive_set(&root, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), coords0, coords1, value);
}

void Tree::shader_serializing(Shader* shader, Vec3 beg, Vec3 end) {
	// Finding subroot (minimal root containing beg & end)
	Vec3 l = Vec3(0, 0, 0), r = Vec3(max_size, max_size, max_size);
	bool cycle = true;
	Node* subroot = &root;
	while (cycle) {
		cycle = false;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					Vec3 add = Vec3(i * (r.x - l.x) / 2, j * (r.y - l.y) / 2, k * (r.z - l.z) / 2);
					Vec3 newl = l + add;
					Vec3 newr = ((r + l) / 2) + add;
					if (beg.belongs(newl, newr)) {
						subroot = subroot->children[i * 4 + j * 2 + k];
						l = newl;
						r = newr;
						cycle = true;
					}
				}
				if (cycle)
					break;
			}
			if (cycle)
				break;
		}
	}

	// Setting subroot coords
	shader->set3f("treel", l.x, l.y, l.z);
	shader->set3f("treer", r.x, r.y, r.z);
	// Init bfs queue
	queue<pair<Node*, pair<int, int>>> q;

	// Serializing
	q.push(make_pair(subroot, make_pair(-1, -1)));
	int num = 0;
	while (!q.empty()) {
		Node* cur = q.front().first;
		q.pop();
		string curShNode = "tree[" + to_string(num) + "]";
		shader->setBool(curShNode + ".terminal", cur->terminal);
		if (cur->terminal) {
			shader->set3f(curShNode + ".voxel.color", cur->voxel.color.r, cur->voxel.color.g, cur->voxel.color.b);
			shader->setFloat(curShNode + ".voxel.reflection_k", cur->voxel.reflection_k);
		}
		else {
			for (int i = 0; i < 8; i++) {
				if (cur->children[i] != nullptr) {
					q.push(make_pair(cur->children[i], make_pair(num, i)));
				}
			}
		}
		if (q.front().second.first != -1) {
			shader->setInt("tree[" + to_string(q.front().second.first) + "].children[" + to_string(q.front().second.second) + "]", num);
		}
		num++;
	}
}