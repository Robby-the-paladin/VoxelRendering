#include "Vec3.h"

Vec3::Vec3(int _x, int _y, int _z) {
	x = _x;
	y = _y;
	z = _z;
}

bool Vec3::belongs(Vec3 l, Vec3 r) {
	return !(!(*this >= l) || !(*this < r));
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