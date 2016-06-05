#pragma once

namespace AT3
{

typedef double Real;

/** 整数二维向量
*/
struct iVector2
{
	iVector2() : x(0), y(0) {}
	iVector2(int _x, int _y) : x(_x), y(_y) {}
	iVector2(const iVector2& other) : x(other.x), y(other.y) {}

	int x, y;
};

//pi!
#define MATH_PI				(Real)(3.14159265)
#define MATH_PI_DOUBLE		(Real)(3.14159265*2.0)
#define MATH_PI_HALF		(Real)(3.14159265*0.5)
#define MATH_PI_QUARTER		(Real)(3.14159265*0.25)

int floatCompare(Real a, Real b, Real tolerance = std::numeric_limits<Real>::epsilon());

/** 浮点数二维向量
*/
struct fVector2
{
	fVector2() : x(0), y(0) {}
	fVector2(Real _x, Real _y) : x(_x), y(_y) {}
	fVector2(const fVector2& other) : x(other.x), y(other.y) {}

	Real length(void) const;
	Real normalize(void);
	Real square(void) const { return x*x+y*y; }
	void set(Real _x, Real _y) { x=_x; y=_y; }

	void operator *= (Real a)
	{
		x *= a; y *= a;
	}
	void operator /= (Real a)
	{
		x /= a; y /= a;
	}
	void operator += (const fVector2& a)
	{
		x += a.x; y += a.y;
	}
	void operator -= (const fVector2& a)
	{
		x -= a.x; y -= a.y;
	}

	Real x, y;

	//常用静态向量
	static const fVector2 ZERO;
};

//点数组
typedef std::vector< fVector2 > PointVector;

//向量相加
inline fVector2 operator + (const fVector2& a, const fVector2& b)
{
	return fVector2(a.x + b.x, a.y + b.y);
}

//向量相减
inline fVector2 operator - (const fVector2& a, const fVector2& b)
{
	return fVector2(a.x - b.x, a.y - b.y);
}
//向量*浮点数
inline fVector2 operator * (Real s, const fVector2& a)
{
	return fVector2(s * a.x, s * a.y);
}
// 向量是否相等
inline bool operator == (const fVector2& a, const fVector2& b)
{
	return a.x==b.x && a.y == b.y;
}

// 向量是否不相等
inline bool operator != (const fVector2& a, const fVector2& b)
{
	return a.x!=b.x || a.y != b.y;
}
//点积
inline Real dotProduct(const fVector2& a, const fVector2& b)
{
	return a.x*b.x + a.y*b.y;
}

//叉积
inline Real crossProduct(const fVector2& a, const fVector2& b)
{
	return a.x * b.y - a.y * b.x;
}

//比较大小，方便某些运算，比较坐标大小，先比较x, 再比较y
inline bool operator > (const fVector2& a, const fVector2& b)
{
	return (a.x > b.x || ((a.x==b.x) && a.y > b.y));
}

}
