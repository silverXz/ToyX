#pragma once

#include <cmath>
#include <smmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

#define SSE_ALIGN __declspec(align(16))

class SSE_Float;
class SSE_Vec2;
class SSE_Vec3;
class SSE_Vec4;

using SSE_Color3 = SSE_Vec3;

SSE_ALIGN class SSE_Float
{
public:
	SSE_Float() {}
	SSE_Float(const __m128 &sf);
	SSE_Float(float ss);
	SSE_Float(float a, float b, float c, float d);

	SSE_Float& operator += (const SSE_Float&);
	SSE_Float& operator -= (const SSE_Float&);
	SSE_Float& operator *= (const SSE_Float&);
	SSE_Float& operator /= (const SSE_Float&);
	SSE_Float& operator += (float);
	SSE_Float& operator -= (float);
	SSE_Float& operator *= (float);
	SSE_Float& operator /= (float);

	SSE_Float operator +() const;
	SSE_Float operator -() const;



	SSE_Float operator + (const SSE_Float&) const;
	SSE_Float operator - (const SSE_Float&) const;
	SSE_Float operator * (const SSE_Float&) const;
	SSE_Float operator / (const SSE_Float&) const;
	SSE_Float operator + (float) const;
	SSE_Float operator - (float) const;
	SSE_Float operator * (float) const;
	SSE_Float operator / (float) const;

public:
	__m128 f;
};

SSE_ALIGN class SSE_Vec2
{
public:
	SSE_Vec2() {};
	SSE_Vec2(const SSE_Float &_x, const SSE_Float &_y);

	SSE_Float Length() const;
	SSE_Float LengthSquare() const;

	SSE_Float Dot(const SSE_Vec2 &v) const;
	void Lerp(const SSE_Vec2 &v1, const SSE_Vec2 &v2, const SSE_Float &t);
	void Normalize();

	SSE_Vec2& operator += (const SSE_Vec2 &v);
	SSE_Vec2& operator -= (const SSE_Vec2 &v);
	SSE_Vec2& operator *= (const SSE_Vec2 &v);
	SSE_Vec2& operator /= (const SSE_Vec2 &v);
	SSE_Vec2& operator += (const SSE_Float &f);
	SSE_Vec2& operator -= (const SSE_Float &f);
	SSE_Vec2& operator *= (const SSE_Float &f);
	SSE_Vec2& operator /= (const SSE_Float &f);

	SSE_Vec2 operator + () const;
	SSE_Vec2 operator - () const;

	SSE_Vec2 operator + (const SSE_Vec2 &v) const;
	SSE_Vec2 operator - (const SSE_Vec2 &v) const;
	SSE_Vec2 operator * (const SSE_Vec2 &v) const;
	SSE_Vec2 operator / (const SSE_Vec2 &v) const;
	SSE_Vec2 operator + (const SSE_Float &v) const;
	SSE_Vec2 operator - (const SSE_Float &v) const;
	SSE_Vec2 operator * (const SSE_Float &v) const;
	SSE_Vec2 operator / (const SSE_Float &v) const;

	friend SSE_Vec2 operator * (const SSE_Float &f, const SSE_Vec2 &v);

public:
	SSE_Float	x, y;
};

SSE_Vec2 operator *(const SSE_Float &f, const SSE_Vec2 &v);


SSE_ALIGN class SSE_Vec3
{
public:
	SSE_Vec3() {}
	SSE_Vec3(const SSE_Float &_x, const SSE_Float &_y, const SSE_Float &_z);

	SSE_Float Length() const;
	SSE_Float LengthSquare() const;
	SSE_Float Dot(const SSE_Vec3 &v) const;
	void Lerp(const SSE_Vec3 &v1, const SSE_Vec3 &v2, const SSE_Float &t);
	void Normalize();

	SSE_Vec3& operator += (const SSE_Vec3& v);
	SSE_Vec3& operator -= (const SSE_Vec3& v);
	SSE_Vec3& operator *= (const SSE_Vec3& v);
	SSE_Vec3& operator /= (const SSE_Vec3& v);
	SSE_Vec3& operator += (const SSE_Float& f);
	SSE_Vec3& operator -= (const SSE_Float& f);
	SSE_Vec3& operator *= (const SSE_Float& f);
	SSE_Vec3& operator /= (const SSE_Float& f);

	SSE_Vec3 operator +() const;
	SSE_Vec3 operator -() const;

	SSE_Vec3 operator + (const SSE_Vec3 &v) const;
	SSE_Vec3 operator - (const SSE_Vec3 &v) const;
	SSE_Vec3 operator * (const SSE_Vec3 &v) const;
	SSE_Vec3 operator / (const SSE_Vec3 &v) const;
	SSE_Vec3 operator + (const SSE_Float &f) const;
	SSE_Vec3 operator - (const SSE_Float &f) const;
	SSE_Vec3 operator * (const SSE_Float &f) const;
	SSE_Vec3 operator / (const SSE_Float &f) const;

	friend SSE_Vec3 operator * (const SSE_Float &f, const SSE_Vec3 &v);

public:
	union {
		struct {
			SSE_Float x, y, z;
		};
		struct {
			SSE_Float r, g, b;
		};
	};
};

SSE_Vec3 operator * (const SSE_Float &f, const SSE_Vec3 &v);


SSE_ALIGN class SSE_Vec4
{
public:
	SSE_Vec4() {}
	SSE_Vec4(const SSE_Float &_x, const SSE_Float &_y, const SSE_Float &_z, const SSE_Float &_w);

	SSE_Float Length() const;
	SSE_Float LengthSquare() const;
	SSE_Float Dot(const SSE_Vec4 &v) const;
	void Lerp(const SSE_Vec4 &v1, const SSE_Vec4 &v2, const SSE_Float &t);
	void Normalize();

	SSE_Vec4& operator+=(const SSE_Vec4 &v);
	SSE_Vec4& operator-=(const SSE_Vec4 &v);
	SSE_Vec4& operator*=(const SSE_Vec4 &v);
	SSE_Vec4& operator/=(const SSE_Vec4 &v);
	SSE_Vec4& operator+=(const SSE_Float &f);
	SSE_Vec4& operator-=(const SSE_Float &f);
	SSE_Vec4& operator*=(const SSE_Float &f);
	SSE_Vec4& operator/=(const SSE_Float &f);

	SSE_Vec4 operator + () const;
	SSE_Vec4 operator - () const;

	SSE_Vec4 operator + (const SSE_Vec4 &v);
	SSE_Vec4 operator - (const SSE_Vec4 &v);
	SSE_Vec4 operator * (const SSE_Vec4 &v);
	SSE_Vec4 operator / (const SSE_Vec4 &v);
	SSE_Vec4 operator + (const SSE_Float &f);
	SSE_Vec4 operator - (const SSE_Float &f);
	SSE_Vec4 operator * (const SSE_Float &f);
	SSE_Vec4 operator / (const SSE_Float &f);

	friend SSE_Vec4 operator * (const SSE_Float &f, const SSE_Vec4 &v);


public:
	union 
	{
		struct {
			SSE_Float x, y, z, w;
		};
		struct {
			SSE_Float r, g, b, a;
		};
	};
};

SSE_Vec4 operator * (const SSE_Float &f, const SSE_Vec4 &v);


inline SSE_Float SSqrt(const SSE_Float &f)
{
	return SSE_Float(_mm_sqrt_ps(f.f));
}

inline SSE_Float SRSqrt(const SSE_Float &f)
{
	return SSE_Float(_mm_rsqrt_ps(f.f));
}




