#pragma once
#include <vector>
#include "Voxel.h"
#include <string>
#include <fstream>
#include <iterator>
#include <iostream>
using namespace std;
class Serializer {
public:
	static vector<vector<vector<Voxel>>> deserizlize_from_xraw(string path);
};

