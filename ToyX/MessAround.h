#pragma once

#include "ToyMath.h"

class Plane
{
public:
	Plane() : mVec(1.0f)
	{
	}

	toy::vec3 GetNormal(void) const
	{
		return toy::vec3(mVec.v[0],mVec.v[1],mVec.v[2]);
	}

public:
	toy::vec4	mVec;
};

enum FrustumPlane {
	TOY_FRUSTUM_NEAR = 0,
	TOY_FRUSTUM_FAR,
	TOY_FRUSTUM_LEFT,
	TOY_FRUSTUM_RIGHT,
	TOY_FRUSTUM_TOP,
	TOY_FRUSTUM_BOTTOM,
	TOY_FRUSTUM_PLANE_NUM
};


class Frustum
{
public:
	Frustum();

	toy::mat4 GetViewMatrix();
	toy::mat4 GetProjectionMatrix();

public:
	Plane	mPlanes[TOY_FRUSTUM_PLANE_NUM];
	toy::vec3	mCameraPos;
	toy::vec3   mFocus;
	toy::vec3	mUp;
	float		mRatio;
	float		mFovy;
	float		mNear;
	float		mFar;
};