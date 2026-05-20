// 数学工具

#pragma once
#include <cmath>
#include <algorithm>

constexpr float PI_F = 3.14159265358979323846f;

inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float clampf(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

struct Vec2 {
    float x = 0, y = 0;
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}
    Vec2 operator+(const Vec2& v) const { return {x+v.x, y+v.y}; }
    Vec2 operator-(const Vec2& v) const { return {x-v.x, y-v.y}; }
    Vec2 operator*(float s) const { return {x*s, y*s}; }
    Vec2& operator+=(const Vec2& v) { x+=v.x; y+=v.y; return *this; }
    float length() const { return std::sqrt(x*x+y*y); }
};

struct Vec4 {
    float x=0, y=0, z=0, w=0;
    Vec4() = default;
    Vec4(float x,float y,float z,float w):x(x),y(y),z(z),w(w){}
};

// 列主序 4×4 矩阵
struct Mat4 {
    float m[16] = {};
    Mat4() { m[0]=m[5]=m[10]=m[15]=1.f; }

    static Mat4 ortho(float l, float r, float b, float t,
                      float n=-1.f, float f=1.f)
    {
        Mat4 o;
        o.m[0]=2.f/(r-l);  o.m[5]=2.f/(t-b);  o.m[10]=-2.f/(f-n);
        o.m[12]=-(r+l)/(r-l); o.m[13]=-(t+b)/(t-b); o.m[14]=-(f+n)/(f-n);
        o.m[15]=1.f;
        return o;
    }
    const float* ptr() const { return m; }
};
