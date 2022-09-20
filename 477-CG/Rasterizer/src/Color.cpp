#include "Color.h"
#include <iostream>
#include <iomanip>
#include <math.h>

using namespace std;

Color::Color() {}

Color::Color(double r, double g, double b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::Color(const Color &other)
{
    this->r = other.r;
    this->g = other.g;
    this->b = other.b;
}

// Color Color::rgbRound()
// {
//     this->r =  round(this->r);
//     this->g =  round(this->g);
//     this->b =  round(this->b);

//     return Color(r,g,b);
// }

ostream& operator<<(ostream& os, const Color& c)
{
    os << fixed << setprecision(0) << "rgb(" << c.r << ", " << c.g << ", " << c.b << ")";
    return os;
}

// Color operator - (Color& c1, Color&c2)
// {
//     double r = c1.r - c2.r;
//     if (r < 0) r = 0;
//     double g = c1.g - c2.g;
//     if (g < 0) g = 0;
//     double b = c1.b - c2.b;
//     if (b < 0) b = 0;

//     return Color(r,g,b);
// }

// Color operator + (Color& c1, Color&c2)
// {
//     double r = c1.r + c2.r;
//     if (r > 255) r = 255;
//     double g = c1.g + c2.g;
//     if (g > 255) g = 255;
//     double b = c1.b + c2.b;
//     if (b > 255) b = 255;

//     return Color(r,g,b);
// }

// Color operator * (double d, Color& c1)
// {
//     double r = c1.r * d;
//     if (r > 255) r = 255;
//     double g = c1.g * d;
//     if (g > 255) r = 255;
//     double b = c1.b * d;
//     if (b > 255) b = 255;

//     return Color(r,g,b);
// }

// Color operator / (const Color& c1, double d)
// {
//     double r = c1.r / d;
//     double g = c1.g / d;
//     double b = c1.b / d;

//     return Color(r,g,b);
// }

// Color operator / (const Color& c1, int i)
// {   
//     double d = (double) i;
//     double r = c1.r / d;
//     double g = c1.g / d;
//     double b = c1.b / d;

//     return Color(r,g,b);
// }