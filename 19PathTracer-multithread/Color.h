#ifndef _COLOR_H
#define _COLOR_H

class Color{
public:
	Color();
	Color(const float r, const float g, const float b);
	

	~Color();

	

	Color operator*(const Color &rhs) const;
	Color operator+(const Color &rhs) const;
	Color operator*(float scalar) const;
	Color operator/(float scalar) const;

	void clamp();
	float Max();
	static Color getRandom();

public:

	float r, g, b;
};

#endif