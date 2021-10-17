#pragma once
class Color {
public:
	int r, g, b, a;
	Color() {};
	Color(int _r, int _g, int _b, int _a);

	friend bool operator== (const Color& color1, const Color& color2);

	friend bool operator!= (const Color& color1, const Color& color2);
};

