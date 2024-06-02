#pragma once
#include <vector>
#include <glm.hpp>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Voxel.h"

using namespace std;

class VOXLoader {
public:
	static void load_vox_file(string name, vector<vector<vector<Voxel>>>& mat);
	static void load_vox_files(vector<string> names, const vector<glm::vec3>& offsets, vector<vector<vector<Voxel>>>& mat);
};

