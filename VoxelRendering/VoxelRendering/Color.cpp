#include "Color.h"

Color::Color(int _r, int _g, int _b, int _a) {
	r = _r;
	g = _g;
	b = _b;
	a = _a;
}

bool operator== (const Color& color1, const Color& color2) {
	return (color1.r == color2.r && color1.g == color2.g && color1.b == color2.b && color1.a == color2.a);
}

bool operator!= (const Color& color1, const Color& color2) {
	return !(color1 == color2);
}