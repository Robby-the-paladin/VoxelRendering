#include "Vec3.h"

Vec3::Vec3(int _x, int _y, int _z) {
	x = _x;
	y = _y;
	z = _z;
}

bool Vec3::belongs(Vec3 l, Vec3 r) {
	Vec3 new_l = Vec3(std::min(l.x, r.x), std::min(l.y, r.y), std::min(l.z, r.z));
	Vec3 new_r = Vec3(std::max(l.x, r.x), std::max(l.y, r.y), std::max(l.z, r.z));
	l = new_l;
	r = new_r;
	return (*this >= l) && (*this < r);
}

bool intersected(const Vec3& l1, const Vec3& r1, const Vec3& l2, const Vec3& r2) {
	// l1 - (X1, Y1, Z1) - координаты нижнего левого угла первого параллелепипеда
	// r1 - (X2, Y2, Z2) - координаты верхнего правого угла первого параллелепипеда
	// l2 - (X3, Y3, Z3) - координаты нижнего левого угла второго параллелепипеда
	// r2 - (X4, Y4, Z4) - координаты верхнего правого угла второго параллелепипеда
	int xmin = std::max(l1.x, l2.x);
	int xmax = std::min(r1.x, r2.x);
	int ymin = std::max(l1.y, l2.y);
	int ymax = std::min(r1.y, r2.y);
	int zmin = std::max(l1.z, l2.z);
	int zmax = std::min(r1.z, r2.z);
	int xsize = xmax - xmin;
	int ysize = ymax - ymin;
	int zsize = zmax - zmin;
	return !(xsize <= 0 || ysize <= 0 || zsize <= 0);
}

Vec3 operator+(const Vec3& vec1, const Vec3& vec2) {
	return Vec3(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}

Vec3 operator-(const Vec3& vec) {
	return Vec3(-vec.x, -vec.y, -vec.z);
}

Vec3 operator-(const Vec3& vec1, const Vec3& vec2) {
	return vec1 + (-vec2);
}

Vec3 operator*(const Vec3& vec1, const int k) {
	return Vec3(vec1.x * k, vec1.y * k, vec1.z * k);
}

Vec3 operator/(const Vec3& vec1, const int k) {
	return Vec3(vec1.x / k, vec1.y / k, vec1.z / k);
}

bool operator>=(const Vec3& vec1, const Vec3& vec2) {
	return (vec1.x >= vec2.x && vec1.y >= vec2.y && vec1.z >= vec2.z);
}

bool operator>(const Vec3& vec1, const Vec3& vec2) {
	return (vec1.x > vec2.x && vec1.y > vec2.y && vec1.z > vec2.z);
}

bool operator<=(const Vec3& vec1, const Vec3& vec2) {
	return (vec1.x <= vec2.x && vec1.y <= vec2.y && vec1.z <= vec2.z);
}

bool operator<(const Vec3& vec1, const Vec3& vec2) {
	return (vec1.x < vec2.x && vec1.y < vec2.y && vec1.z < vec2.z);
}

bool operator== (const Vec3& vec1, const Vec3& vec2) {
	return (vec1.x == vec2.x && vec1.y == vec2.y && vec1.z == vec2.z);
}

bool operator!= (const Vec3& vec1, const Vec3& vec2) {
	return !(vec1 == vec2);
}