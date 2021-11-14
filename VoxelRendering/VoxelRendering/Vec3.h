#pragma once
#include <algorithm>
#include <vector>
class Vec3 {
public:
	int x = 0;
	int y = 0;
	int z = 0;

	Vec3() {};
	Vec3(int _x, int _y, int _z);

	bool belongs(Vec3 l, Vec3 r);

	std::vector<Vec3> cube_vertices(Vec3 end);

	friend bool intersected(const Vec3& l1, const Vec3& r1, const Vec3& l2, const Vec3& r2);
		
	friend Vec3 operator+(const Vec3& vec1, const Vec3& vec2);

	friend Vec3 operator-(const Vec3& vec);

	friend Vec3 operator-(const Vec3& vec1, const Vec3& vec2);

	friend Vec3 operator*(const Vec3& vec1, const int k);

	friend Vec3 operator/(const Vec3& vec1, const int k);

	friend bool operator>=(const Vec3& vec1, const Vec3& vec2);

	friend bool operator>(const Vec3& vec1, const Vec3& vec2);

	friend bool operator<=(const Vec3& vec1, const Vec3& vec2);

	friend bool operator<(const Vec3& vec1, const Vec3& vec2);

	friend bool operator== (const Vec3& vec1, const Vec3& vec2);

	friend bool operator!= (const Vec3& vec1, const Vec3& vec2);
};

