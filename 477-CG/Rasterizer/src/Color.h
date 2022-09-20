#ifndef __COLOR_H__
#define __COLOR_H__

#include <iostream>

class Color
{
public:
    double r, g, b;

    Color();
    Color(double r, double g, double b);
    Color(const Color &other);
    // Color rgbRound();
    friend std::ostream& operator<<(std::ostream& os, const Color& c);
    // friend Color operator - (Color& c1, Color&c2);
    // friend Color operator + (Color& c1, Color&c2);
    // friend Color operator * (double d, Color& c1);
    // friend Color operator / (Color& c1, double d);
    // friend Color operator / (Color& c1, int i);
};

#endif