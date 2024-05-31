//#define wall
#include "Tree.h"

void Tree::grid_build(vector<vector<vector<Voxel>>>* mat, Vec3 beg, Vec3 end, vector<glm::vec4>& grid_buffer) {
	for (int i = beg.x; i < end.x; i++) {
		for (int j = beg.y; j < end.y; j++) {
			for (int k = beg.z; k < end.z; k++) {
				if (mat->size() <= i || mat->operator[](i).size() <= j || mat->operator[](i)[j].size() <= k)
					grid_buffer.push_back(glm::vec4(0, 0, 0, 0));
				else {
					auto mat_cell = mat->operator[](k)[j][i];
					grid_buffer.push_back(glm::vec4(mat_cell.color.r / 255.0, mat_cell.color.g / 255.0, mat_cell.color.b / 255.0, !mat_cell.empty));
				}
			}
		}
	}
}

Node* Tree::recursive_build(vector<vector<vector<Voxel>>>* mat, Vec3 coords0, Vec3 coords1, vector<glm::vec4>& grid_buffer, int grid_depth) {
	if (grid_depth == 0) {
		Node* ans = new Node();
		ans->terminal = 1;
		ans->grid_offset = grid_buffer.size();
		grid_build(mat, coords0, coords1, grid_buffer);
		return ans;
	}
	if (coords0 + Vec3(1, 1, 1) == coords1) {
		if (mat->size() < coords1.x || mat->operator[](coords0.x).size() < coords1.y
			|| mat->operator[](coords0.x)[coords0.y].size() < coords1.z) {
			Node* ans = new Node();
			ans->terminal = 1;
			ans->voxel.empty = true;
			return ans;
		}
		Node* ans = new Node();
		ans->terminal = 1;
		ans->voxel = mat->operator[](coords0.x)[coords0.y][coords0.z];
		return ans;
	}
	Node* children[8];
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				Vec3 add = Vec3(i * (coords1.x - coords0.x) / 2, j * (coords1.y - coords0.y) / 2, k * (coords1.z - coords0.z) / 2);
				children[i * 4 + j * 2 + k] = recursive_build(mat, coords0 + add, ((coords1 + coords0) / 2) + add, grid_buffer, grid_depth - 1);
			}
		}
	}
	return new Node(children);
}

void Tree::build(vector<vector<vector<Voxel>>> mat, vector<glm::vec4>& grid_buffer, int grid_depth) {
	max_size = max(max(mat.size(), mat[0].size()), mat[0][0].size());
	max_size = (1 << int(log(max_size - 1) / log(2) + 1));
	cout << "\n Max size = " << max_size << " real size " << mat.size() << " " << mat[0].size() << " " << mat[0][0].size() << "\n";
	root = *recursive_build(&mat, Vec3(0, 0, 0), Vec3(max_size, max_size, max_size), grid_buffer, grid_depth);
}

void Tree::load_vox_files(vector<string> names, vector<glm::vec4>& grid_buffer, int grid_depth, const vector<glm::vec3>& offsets) {
	vector<vector<vector<Voxel>>> mat;
	int ind = 0;
	
	for (auto name : names) {
		char ChunkID[4], Format[4];
		int ChunkX, ChunkY, ChunkZ, Version, NumVoxels, ColorIndex;
		int MainChunkContentSize, MainChunkChildrenSize;
		int ChunkContentSize, ChunkChildrenSize;

		// Read the wave file
		FILE* fhandle;
		auto error = fopen_s(&fhandle, name.c_str(), "rb");
		if (error) {
			cout << "fopen error: " << error << endl;
		}
		fread(Format, 1, 4, fhandle);
		if (string(Format).substr(0, 4) != "VOX ") {
			cout << string(Format).substr(0, 4) << "!= VOX error: Isn't a VOX format file" << endl;
			return;
		}
		fread(&Version, 4, 1, fhandle);
#ifdef wall
		if (Version != 150) {
			cout << "warninig: version is " << Version << endl;
		}
#endif // wall
		fread(ChunkID, 1, 4, fhandle);
		if (string(ChunkID).substr(0, 4) != "MAIN") {
			cout << "error: MAIN chunk lost" << endl;
			return;
		}
		fread(&MainChunkContentSize, 4, 1, fhandle);
		fread(&MainChunkChildrenSize, 4, 1, fhandle);
		fread(ChunkID, 1, 4, fhandle);
		if (string(ChunkID).substr(0, 4) == "PACK") {
			cout << "error: PACK chunk found" << endl;
			return;
		}
		if (string(ChunkID).substr(0, 4) != "SIZE") {
			cout << "error: SIZE chunk lost" << endl;
			return;
		}
		fread(&ChunkContentSize, 4, 1, fhandle);
		fread(&ChunkChildrenSize, 4, 1, fhandle);
		MainChunkChildrenSize -= 12;
		MainChunkChildrenSize -= ChunkContentSize;
		MainChunkChildrenSize -= ChunkChildrenSize;
		fread(&ChunkX, 4, 1, fhandle);
		fread(&ChunkY, 4, 1, fhandle);
		fread(&ChunkZ, 4, 1, fhandle);
		int _size = max(ChunkX + int(offsets[ind].r), max(ChunkY + int(offsets[ind].g), ChunkZ + int(offsets[ind].b)));
		std::cout << "Mat size " << _size << " " << name << "\n";
		mat.resize(max(_size, int(mat.size())));
		for (int i = 0; i < mat.size(); i++) {
			mat[i].resize(max(_size, int(mat.size())));
			for (int j = 0; j < mat[i].size(); j++) {
				mat[i][j].resize(max(_size, int(mat.size())));
				for (int k = 0; k < mat[i][j].size(); k++) {
					if (mat[i][j][k].empty) {
						mat[i][j][k].color = Color(0, 0, 0, 255);
						mat[i][j][k].reflection_k = -1;
					}
					else {
						//cout << "not empty " << i << " " << j << " " << k << "\n";
						mat[i][j][k].reflection_k = -2;
					}
				}
			}
		}
		fread(ChunkID, 1, 4, fhandle);
		if (string(ChunkID).substr(0, 4) != "XYZI") {
			cout << "error: XYZI chunk lost" << endl;
			return;
		}
		fread(&ChunkContentSize, 4, 1, fhandle);
		fread(&ChunkChildrenSize, 4, 1, fhandle);
		MainChunkChildrenSize -= 12;
		MainChunkChildrenSize -= ChunkContentSize;
		MainChunkChildrenSize -= ChunkChildrenSize;
		fread(&NumVoxels, 4, 1, fhandle);
		cout << "ehh " << int(offsets[ind].r) << "\n";
		for (int i = 0; i < NumVoxels; i++) {
			char pos;
			fread(&pos, 1, 1, fhandle);
			ChunkX = int(pos) + int(offsets[ind].r);
			fread(&pos, 1, 1, fhandle);
			ChunkY = int(pos) + int(offsets[ind].g);
			fread(&pos, 1, 1, fhandle);
			ChunkZ = int(pos) + int(offsets[ind].b);
			fread(&pos, 1, 1, fhandle);

			if (mat[ChunkX][ChunkY][ChunkZ].reflection_k == -1)
				mat[ChunkX][ChunkY][ChunkZ].reflection_k = pos;
		}
		unsigned int palette[256];
		for (int i = 0; i < 256; i++)
			palette[i] = default_palette[i];
		if (MainChunkChildrenSize != 0) {
			fread(ChunkID, 1, 4, fhandle);

			if (string(ChunkID).substr(0, 4) != "RGBA") {
#ifdef wall
				cout << "warning: RGBA chunk lost";
#endif // wall
			}
			else {
				fread(&ChunkContentSize, 4, 1, fhandle);
				fread(&ChunkChildrenSize, 4, 1, fhandle);
				for (int i = 0; i <= 255; i++) {
					unsigned int color;
					fread(&color, 4, 1, fhandle);
					palette[i] = color;
				}
			}
		}
		for (int i = 0; i < mat.size(); i++) {
			for (int j = 0; j < mat[i].size(); j++) {
				for (int k = 0; k < mat[i][j].size(); k++) {
					if (mat[i][j][k].reflection_k != -1 && mat[i][j][k].reflection_k != -2) {
						//cout << "setting " << i << " " << j << " " << k << "\n";
						unsigned int color = palette[mat[i][j][k].reflection_k];
						unsigned int a = color >> 24;
						unsigned int b = (color ^ (a << 24)) >> 16;
						unsigned int g = (color ^ (a << 24) ^ (b << 16)) >> 8;
						unsigned int r = (color ^ (a << 24) ^ (b << 16) ^ (g << 8));

						mat[i][j][k].color = Color(r, g, b, a);
						mat[i][j][k].empty = false;
					}
					mat[i][j][k].reflection_k = 0;
				}
			}
		}
		ind++;
	}

	build(mat, grid_buffer, grid_depth);
}

void Tree::load_vox_file(string name, vector<glm::vec4>& grid_buffer, int grid_depth) {
	char ChunkID[4], Format[4];
	int ChunkX, ChunkY, ChunkZ, Version, NumVoxels, ColorIndex;
	int MainChunkContentSize, MainChunkChildrenSize;
	int ChunkContentSize, ChunkChildrenSize;

	// Read the wave file
	FILE* fhandle;
	auto error = fopen_s(&fhandle ,name.c_str(), "rb");
	if (error) {
		cout << "fopen error: " << error << endl;
	}
	fread(Format, 1, 4, fhandle);
	if (string(Format).substr(0, 4) != "VOX ") {
		cout << string(Format).substr(0, 4) << "!= VOX error: Isn't a VOX format file" << endl;
		return;
	}
	fread(&Version, 4, 1, fhandle);
#ifdef wall
	if (Version != 150) {
		cout << "warninig: version is " << Version << endl;
	}
#endif // wall
	fread(ChunkID, 1, 4, fhandle);
	if (string(ChunkID).substr(0, 4) != "MAIN") {
		cout << "error: MAIN chunk lost" << endl;
		return;
	}
	fread(&MainChunkContentSize, 4, 1, fhandle);
	fread(&MainChunkChildrenSize, 4, 1, fhandle);
	fread(ChunkID, 1, 4, fhandle);
	if (string(ChunkID).substr(0, 4) == "PACK") {
		cout << "error: PACK chunk found" << endl;
		return;
	}
	if (string(ChunkID).substr(0, 4) != "SIZE") {
		cout << "error: SIZE chunk lost" << endl;
		return;
	}
	fread(&ChunkContentSize, 4, 1, fhandle);
	fread(&ChunkChildrenSize, 4, 1, fhandle);
	MainChunkChildrenSize -= 12;
	MainChunkChildrenSize -= ChunkContentSize;
	MainChunkChildrenSize -= ChunkChildrenSize;
	fread(&ChunkX, 4, 1, fhandle);
	fread(&ChunkY, 4, 1, fhandle);
	fread(&ChunkZ, 4, 1, fhandle);
	vector<vector<vector<Voxel>>> mat;
	int _size = max(ChunkX, max(ChunkY, ChunkZ));
	mat.resize(_size);
	for (int i = 0; i < _size; i++) {
		mat[i].resize(_size);
		for (int j = 0; j < _size; j++) {
			mat[i][j].resize(_size);
			for (int k = 0; k < _size; k++) {
				mat[i][j][k].color = Color(0, 0, 0, 255);
				mat[i][j][k].empty = true;
				mat[i][j][k].reflection_k = -1;
			}
		}
	}
	fread(ChunkID, 1, 4, fhandle);
	if (string(ChunkID).substr(0, 4) != "XYZI") {
		cout << "error: XYZI chunk lost" << endl;
		return;
	}
	fread(&ChunkContentSize, 4, 1, fhandle);
	fread(&ChunkChildrenSize, 4, 1, fhandle);
	MainChunkChildrenSize -= 12;
	MainChunkChildrenSize -= ChunkContentSize;
	MainChunkChildrenSize -= ChunkChildrenSize;
	fread(&NumVoxels, 4, 1, fhandle);
	for (int i = 0; i < NumVoxels; i++) {
		char pos;
		fread(&pos, 1, 1, fhandle);
		ChunkX = pos;
		fread(&pos, 1, 1, fhandle);
		ChunkY = pos;
		fread(&pos, 1, 1, fhandle);
		ChunkZ = pos;
		fread(&pos, 1, 1, fhandle);
		mat[ChunkX][ChunkY][ChunkZ].reflection_k = pos;
	}
	unsigned int palette[256];
	for (int i = 0; i < 256; i++)
		palette[i] = default_palette[i];
	if (MainChunkChildrenSize != 0) {
		fread(ChunkID, 1, 4, fhandle);
		if (string(ChunkID).substr(0, 4) != "RGBA") {
#ifdef wall
			cout << "warning: RGBA chunk lost";
#endif // wall
		} else {
			fread(&ChunkContentSize, 4, 1, fhandle);
			fread(&ChunkChildrenSize, 4, 1, fhandle);
			for (int i = 0; i <= 255; i++) {
				unsigned int color;
				fread(&color, 4, 1, fhandle);
				palette[i] = color;
			}
		}
	}
	for (int i = 0; i < _size; i++) {
		for (int j = 0; j < _size; j++) {
			for (int k = 0; k < _size; k++) {
				if (mat[i][j][k].reflection_k == -1) {
					mat[i][j][k].color = Color(0, 0, 0, 255);
					mat[i][j][k].empty = true;
				} else {
					unsigned int color = palette[mat[i][j][k].reflection_k];
					unsigned int a = color >> 24;
					unsigned int b = (color ^ (a << 24)) >> 16;
					unsigned int g = (color ^ (a << 24) ^ (b << 16)) >> 8;
					unsigned int r = (color ^ (a << 24) ^ (b << 16) ^ (g << 8));
					
					mat[i][j][k].color = Color(r, g, b, a);
					mat[i][j][k].empty = false;
				}
				mat[i][j][k].reflection_k = 0;
			}
		}
	}

	build(mat, grid_buffer, grid_depth);
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

void Tree::shader_serializing(Shader* shader) {
	//Vec3 l = Vec3(0, 0, 0), r = Vec3(max_size, max_size, max_size);
	//// Setting subroot coords
	//shader->set3f("treel", l.x, l.y, l.z);
	//shader->set3f("treer", r.x, r.y, r.z);

	//cout << "treel " << l.x << " " << l.y << " " << l.z << "\n";
	//cout << "treer " << r.x << " " << r.y << " " << r.z << "\n";
}