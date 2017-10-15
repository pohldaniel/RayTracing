#ifndef _COLOR_H
#define _COLOR_H

class Color
{


public:
	int r, g, b;

	Color();
	Color(int r, int g, int b);
	~Color();

	Color &operator+(const Color &rhs) const;
	Color operator*(float scalar) const;
};

#endif