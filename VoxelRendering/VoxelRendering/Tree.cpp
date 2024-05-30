#include "Tree.h"

void Tree::grid_build(vector<vector<vector<Voxel>>>* mat, Vec3 beg, Vec3 end, vector<glm::vec4>& grid_buffer) {
	for (int i = beg.x; i < end.x; i++) {
		for (int j = beg.y; j < end.y; j++) {
			for (int k = beg.z; k < end.z; k++) {
				if (mat->size() <= i || mat->operator[](i).size() <= j || mat->operator[](i)[j].size() <= k)
					grid_buffer.push_back(glm::vec4(0, 0, 0, 0));
				else {
					auto mat_cell = mat->operator[](i)[j][k];
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

void Tree::load_vox_file(string name, vector<glm::vec4>& grid_buffer, int grid_depth) {
	char ChunkID[4], Format[4];
	int ChunkX, ChunkY, ChunkZ, Version, NumVoxels, ColorIndex;
	int MainChunkContentSize, MainChunkChildrenSize;
	int ChunkContentSize, ChunkChildrenSize;
	unsigned int default_palette[256] = {
		0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff,
		0xff6699ff, 0xff3399ff, 0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff, 0xffff00ff,
		0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
		0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc,
		0xff6633cc, 0xff3333cc, 0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99, 0xffffcc99,
		0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
		0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66,
		0xff66ff66, 0xff33ff66, 0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966, 0xffff6666,
		0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
		0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933,
		0xff669933, 0xff339933, 0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333, 0xffff0033,
		0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
		0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600, 0xffff3300, 0xffcc3300, 0xff993300,
		0xff663300, 0xff333300, 0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055, 0xff000044,
		0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
		0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555, 0xff444444, 0xff222222, 0xff111111
	};

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
	if (Version != 150) {
		cout << "warninig: version is " << Version << endl;
	}
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
	if (MainChunkChildrenSize != 0) {
		fread(ChunkID, 1, 4, fhandle);
		if (string(ChunkID).substr(0, 4) != "RGBA") {
			cout << "error: RGBA chunk lost";
		} else {
			fread(&ChunkContentSize, 4, 1, fhandle);
			fread(&ChunkChildrenSize, 4, 1, fhandle);
			for (int i = 0; i <= 254; i++) {
				unsigned int color;
				fread(&color, 4, 1, fhandle);
				default_palette[i + 1] = color;
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
					unsigned int color = default_palette[mat[i][j][k].reflection_k];
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