#include "stdafx.h"
#include "Arti3D_SSE_Math.h"




SSE_Float::SSE_Float(const __m128 &sf)
{
	f = sf;
}

SSE_Float::SSE_Float(float ss)
{
	_mm_set1_ps(ss);
}

SSE_Float::SSE_Float(float a, float b, float c, float d)
{
	_mm_set_ps(a, b, c, d);
}

SSE_Float::operator __m128() const
{
	return f;
}

SSE_Float& SSE_Float::operator+=(const SSE_Float &rsf)
{
	f = _mm_add_ps(f, rsf.f);
	return *this;
}

SSE_Float& SSE_Float::operator-=(const SSE_Float &rsf)
{
	f = _mm_sub_ps(f, rsf.f);
	return *this;
}

SSE_Float& SSE_Float::operator*=(const SSE_Float &rsf)
{
	f = _mm_mul_ps(f, rsf.f);
	return *this;
}

SSE_Float& SSE_Float::operator/=(const SSE_Float &rsf)
{
	f = _mm_div_ps(f, rsf.f);
	return *this;
}

SSE_Float& SSE_Float::operator+=(float val)
{
	f = _mm_add_ps(f, _mm_set1_ps(val));
	return *this;
}

SSE_Float& SSE_Float::operator-=(float val)
{
	f = _mm_sub_ps(f, _mm_set1_ps(val));
	return *this;
}

SSE_Float& SSE_Float::operator*=(float val)
{
	f = _mm_mul_ps(f, _mm_set1_ps(val));
	return *this;
}

SSE_Float& SSE_Float::operator/=(float val)
{
	f = _mm_div_ps(f, _mm_set1_ps(val));
	return *this;
}



SSE_Float SSE_Float::operator+() const
{
	return *this;
}

SSE_Float SSE_Float::operator-() const
{
	return _mm_xor_ps(f, _mm_set1_ps(0x80000000));
}

SSE_Float SSE_Float::operator+(const SSE_Float& crsf) const
{
	return SSE_Float(_mm_add_ps(f, crsf.f));
}


SSE_Float SSE_Float::operator-(const SSE_Float &crsf) const
{
	return SSE_Float(_mm_sub_ps(f, crsf.f));
}

SSE_Float SSE_Float::operator*(const SSE_Float &crsf) const
{
	return SSE_Float(_mm_mul_ps(f, crsf.f));
}

SSE_Float SSE_Float::operator/(const SSE_Float &crsf) const
{
	return SSE_Float(_mm_div_ps(f, crsf.f));
}

SSE_Float SSE_Float::operator+(float val) const
{
	return SSE_Float(_mm_add_ps(f, _mm_set_ps1(val)));
}

SSE_Float SSE_Float::operator-(float val) const
{
	return SSE_Float(_mm_sub_ps(f, _mm_set_ps1(val)));
}

SSE_Float SSE_Float::operator*(float val) const
{
	return SSE_Float(_mm_mul_ps(f, _mm_set_ps1(val)));
}

SSE_Float SSE_Float::operator/(float val) const
{
	return SSE_Float(_mm_div_ps(f, _mm_set_ps1(val)));
}


// SSE_Vec2 Implementation!


SSE_Vec2::SSE_Vec2(const SSE_Float &_x, const SSE_Float &_y) : x(_x), y(_y)
{
}

SSE_Float SSE_Vec2::Length() const
{
	return SSqrt(x*x + y*y);
}

SSE_Float SSE_Vec2::LengthSquare() const
{
	return x*x + y*y;
}

SSE_Float SSE_Vec2::Dot(const SSE_Vec2 &v) const
{
	return x * v.x + y * v.y;
}

void SSE_Vec2::Lerp(const SSE_Vec2 &v1, const SSE_Vec2 &v2, const SSE_Float &t)
{
	x = v1.x + (v2.x - v1.x) * t;
	y = v1.y + (v2.y - v1.y) * t;
}

void SSE_Vec2::Normalize()
{
	SSE_Float rs = SRSqrt(x * x + y * y);
	x *= rs;
	y *= rs;
}

SSE_Vec2& SSE_Vec2::operator+=(const SSE_Vec2 &v)
{
	x += v.x;
	y += v.y;
	return *this;
}



SSE_Vec2& SSE_Vec2::operator-=(const SSE_Vec2 &v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

SSE_Vec2& SSE_Vec2::operator*=(const SSE_Vec2 &v)
{
	x *= v.x;
	y *= v.y;
	return *this;
}

SSE_Vec2& SSE_Vec2::operator/=(const SSE_Vec2 &v)
{
	x /= v.x;
	y /= v.y;
	return *this;
}

SSE_Vec2& SSE_Vec2::operator+=(const SSE_Float &f)
{
	x += f;
	y += f;
	return *this;
}

SSE_Vec2& SSE_Vec2::operator-=(const SSE_Float &f)
{
	x -= f;
	y -= f;
	return *this;
}

SSE_Vec2& SSE_Vec2::operator*=(const SSE_Float &f)
{
	x *= f;
	y *= f;
	return *this;
}

SSE_Vec2& SSE_Vec2::operator/=(const SSE_Float &f)
{
	x /= f;
	y /= f;
	return *this;
}

SSE_Vec2 SSE_Vec2::operator+() const
{
	return *this;
}

SSE_Vec2 SSE_Vec2::operator-() const
{
	return SSE_Vec2(-x, -y);
}

SSE_Vec2 SSE_Vec2::operator+(const SSE_Vec2 &v) const
{
	return SSE_Vec2(x + v.x, y + v.y);
}

SSE_Vec2 SSE_Vec2::operator-(const SSE_Vec2 &v) const
{
	return SSE_Vec2(x - v.x, y - v.y);
}

SSE_Vec2 SSE_Vec2::operator*(const SSE_Vec2 &v) const
{
	return SSE_Vec2(x * v.x, y * v.y);
}

SSE_Vec2 SSE_Vec2::operator/(const SSE_Vec2 &v) const
{
	return SSE_Vec2(x / v.x, y / v.y);
}


SSE_Vec2 SSE_Vec2::operator+(const SSE_Float &v) const
{
	return SSE_Vec2(x + v, y + v);
}

SSE_Vec2 SSE_Vec2::operator-(const SSE_Float &v) const
{
	return SSE_Vec2(x - v, y - v);
}

SSE_Vec2 SSE_Vec2::operator*(const SSE_Float &v) const
{
	return SSE_Vec2(x * v, y * v);
}

SSE_Vec2 SSE_Vec2::operator/(const SSE_Float &v) const
{
	return SSE_Vec2(x / v, y / v);
}

SSE_Vec2 operator*(const SSE_Float &f, const SSE_Vec2 &v)
{
	return SSE_Vec2(f + v.x, f + v.y);
}




//! SSE_Vec3 Implementation!

SSE_Vec3::SSE_Vec3(const SSE_Float &_x, const SSE_Float &_y, const SSE_Float &_z) : x(_x), y(_y), z(_z)
{
}

SSE_Float SSE_Vec3::Length() const
{
	return SSqrt(x * x + y * y + z * z);
}

SSE_Float SSE_Vec3::LengthSquare() const
{
	return x * x + y * y + z * z;
}

SSE_Float SSE_Vec3::Dot(const SSE_Vec3 &v) const
{
	return x * v.x + y * v.y + z * v.z;
}

void SSE_Vec3::Lerp(const SSE_Vec3 &v1, const SSE_Vec3 &v2, const SSE_Float &t)
{
	x = v1.x + (v2.x - v1.x) * t;
	y = v1.y + (v2.y - v1.y) * t;
	z = v1.z + (v2.z - v1.z) * t;
}

void SSE_Vec3::Normalize()
{
	SSE_Float f = SRSqrt(x * x + y * y + z * z);
	x *= f;
	y *= f;
	z *= f;
}

SSE_Vec3& SSE_Vec3::operator+=(const SSE_Vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

SSE_Vec3& SSE_Vec3::operator-=(const SSE_Vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

SSE_Vec3& SSE_Vec3::operator*=(const SSE_Vec3& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

SSE_Vec3& SSE_Vec3::operator/=(const SSE_Vec3& v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}


SSE_Vec3& SSE_Vec3::operator+=(const SSE_Float& f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}

SSE_Vec3& SSE_Vec3::operator-=(const SSE_Float& f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}

SSE_Vec3& SSE_Vec3::operator*=(const SSE_Float& f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

SSE_Vec3& SSE_Vec3::operator/=(const SSE_Float& f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

SSE_Vec3 SSE_Vec3::operator+() const
{
	return *this;
}

SSE_Vec3 SSE_Vec3::operator-() const
{
	return SSE_Vec3(-x, -y, -z);
}


SSE_Vec3 SSE_Vec3::operator+(const SSE_Vec3 &v) const
{
	return SSE_Vec3(x + v.x, y + v.y, z + v.z);
}

SSE_Vec3 SSE_Vec3::operator-(const SSE_Vec3 &v) const
{
	return SSE_Vec3(x - v.x, y - v.y, z - v.z);
}

SSE_Vec3 SSE_Vec3::operator*(const SSE_Vec3 &v) const
{
	return SSE_Vec3(x * v.x, y * v.y, z * v.z);
}

SSE_Vec3 SSE_Vec3::operator/(const SSE_Vec3 &v) const
{
	return SSE_Vec3(x / v.x, y / v.y, z / v.z);
}


SSE_Vec3 SSE_Vec3::operator+(const SSE_Float &f) const
{
	return SSE_Vec3(x + f, y + f, z + f);
}

SSE_Vec3 SSE_Vec3::operator-(const SSE_Float &f) const
{
	return SSE_Vec3(x - f, y - f, z - f);
}

SSE_Vec3 SSE_Vec3::operator*(const SSE_Float &f) const
{
	return SSE_Vec3(x * f, y * f, z * f);
}

SSE_Vec3 SSE_Vec3::operator/(const SSE_Float &f) const
{
	return SSE_Vec3(x / f, y / f, z / f);
}

SSE_Vec3 operator*(const SSE_Float &f, const SSE_Vec3 &v)
{
	return SSE_Vec3(f + v.x, f + v.y, f + v.z);
}

//!SSE_Vec4 Implementation!

SSE_Vec4::SSE_Vec4(const SSE_Float &_x, const SSE_Float &_y, const SSE_Float &_z, const SSE_Float &_w) : x(_x), y(_y), z(_z), w(_w)
{

}

SSE_Float SSE_Vec4::Length() const
{
	return SSqrt(x*x + y*y + z*z + w*w);
}

SSE_Float SSE_Vec4::LengthSquare() const
{
	return x*x + y*y + z*z + w*w;
}

SSE_Float SSE_Vec4::Dot(const SSE_Vec4 &v) const
{
	return x*v.x + y*v.y + z*v.z + w*v.w;
}

void SSE_Vec4::Lerp(const SSE_Vec4 &v1, const SSE_Vec4 &v2, const SSE_Float &t)
{
	x = v1.x + (v2.x - v1.x) * t;
	y = v1.y + (v2.y - v1.y) * t;
	z = v1.z + (v2.z - v1.z) * t;
	w = v1.w + (v2.w - v1.w) * t;
}

void SSE_Vec4::Normalize()
{
	SSE_Float f = SRSqrt(x*x + y*y + z*z + w*w);
	x *= f;
	y *= f;
	z *= f;
	w *= f;
}

SSE_Vec4& SSE_Vec4::operator+=(const SSE_Vec4 &v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

SSE_Vec4& SSE_Vec4::operator-=(const SSE_Vec4 &v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}


SSE_Vec4& SSE_Vec4::operator*=(const SSE_Vec4 &v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
	return *this;
}


SSE_Vec4& SSE_Vec4::operator/=(const SSE_Vec4 &v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
	return *this;
}


SSE_Vec4& SSE_Vec4::operator+=(const SSE_Float &f)
{
	x += f;
	y += f;
	z += f;
	w += f;
	return *this;
}

SSE_Vec4& SSE_Vec4::operator-=(const SSE_Float &f)
{
	x -= f;
	y -= f;
	z -= f;
	w -= f;
	return *this;
}

SSE_Vec4& SSE_Vec4::operator*=(const SSE_Float &f)
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

SSE_Vec4& SSE_Vec4::operator/=(const SSE_Float &f)
{
	x /= f;
	y /= f;
	z /= f;
	w /= f;
	return *this;
}

SSE_Vec4 SSE_Vec4::operator+() const
{
	return *this;
}

SSE_Vec4 SSE_Vec4::operator-() const
{
	return SSE_Vec4(-x, -y, -z, -w);
}

SSE_Vec4 SSE_Vec4::operator+(const SSE_Vec4 &v)
{
	return SSE_Vec4(x + v.x, y + v.y, z + v.z, w + v.w);
}

SSE_Vec4 SSE_Vec4::operator-(const SSE_Vec4 &v)
{
	return SSE_Vec4(x - v.x, y - v.y, z - v.z, w - v.w);
}

SSE_Vec4 SSE_Vec4::operator*(const SSE_Vec4 &v)
{
	return SSE_Vec4(x * v.x, y * v.y, z * v.z, w * v.w);
}

SSE_Vec4 SSE_Vec4::operator/(const SSE_Vec4 &v)
{
	return SSE_Vec4(x / v.x, y / v.y, z / v.z, w / v.w);
}

SSE_Vec4 SSE_Vec4::operator+(const SSE_Float &f)
{
	return SSE_Vec4(x + f, y + f, z + f, w + f);
}

SSE_Vec4 SSE_Vec4::operator-(const SSE_Float &f)
{
	return SSE_Vec4(x - f, y - f, z - f, w - f);
}

SSE_Vec4 SSE_Vec4::operator*(const SSE_Float &f)
{
	return SSE_Vec4(x * f, y * f, z * f, w * f);
}

SSE_Vec4 SSE_Vec4::operator/(const SSE_Float &f)
{
	return SSE_Vec4(x / f, y / f, z / f, w / f);
}


SSE_Vec4 operator*(const SSE_Float &f, const SSE_Vec4 &v)
{
	return SSE_Vec4(v.x*f, v.y*f, v.z*f, v.w*f);
}
