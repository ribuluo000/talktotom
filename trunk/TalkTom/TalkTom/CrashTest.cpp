#include "stdafx.h"
#include "CrashTest.h"


void CCrashTest::empty()
{
	const int kBigNumber = 0x137f;
	minAABB.x = minAABB.y = minAABB.z = kBigNumber;
	maxAABB.x = maxAABB.y = maxAABB.z = -kBigNumber;
}



void CCrashTest::addPoint( int x, int y, int z )
{
	if (x < minAABB.x) minAABB.x = x;
	if (x > maxAABB.x) maxAABB.x = x;
	if (y < minAABB.x) minAABB.y = y;
	if (y > maxAABB.x) maxAABB.y = y;
	if (z < minAABB.x) minAABB.z = z;
	if (z > maxAABB.x) maxAABB.z = z;
}


void CCrashTest::calculateAABB( const int (*vertex)[3], const int size )
{
	this->empty();
	for (int i = 0; i < size; i++)
	{
		addPoint(vertex[i][0], vertex[i][1], vertex[i][2]);
	}
}


void CCrashTest::calculateAABB( const int XMin, const int YMin, const int ZMin, 
							    const int XMax, const int YMax, const int ZMax )
{
	minAABB.x = XMin;
	minAABB.y = YMin;
	minAABB.z = ZMin;

	maxAABB.x = XMax;
	maxAABB.y = YMax;
	maxAABB.z = ZMax;
}

float CCrashTest::rayIntersect(const int XOrin, const int YOrin, const int ZOrin,
						  	   const int XRay , const int YRay,  const int ZRay) const
{
	// We'll return -1 if no intersection

	const float kNoIntersection = -1;

	// Check for point inside box, trivial reject, and determine parametric
	// distance to each front face

	bool inside = true;

	float xt, xn;
	if (XOrin < minAABB.x) {
		xt = (float)(minAABB.x - XOrin);
		if (xt > XRay) return kNoIntersection;
		xt /= XRay;
		inside = false;
		xn = -1.0f;
	} else if (XOrin > maxAABB.x) {
		xt = (float)(maxAABB.x - XOrin);
		if (xt < XRay) return kNoIntersection;
		xt /= XRay;
		inside = false;
		xn = 1.0f;
	} else {
		xt = -1.0f;
	}

	float yt, yn;
	if (YOrin < minAABB.y) {
		yt = (float)(minAABB.y - YOrin);
		if (yt > YRay) return kNoIntersection;
		yt /= YRay;
		inside = false;
		yn = -1.0f;
	} else if (YOrin > maxAABB.y) {
		yt = (float)(maxAABB.y - YOrin);
		if (yt < YRay) return kNoIntersection;
		yt /= YRay;
		inside = false;
		yn = 1.0f;
	} else {
		yt = -1.0f;
	}

	float zt, zn;
	if (ZOrin < minAABB.z) {
		zt = (float)(minAABB.z - ZOrin);
		if (zt > ZRay) return kNoIntersection;
		zt /= ZRay;
		inside = false;
		zn = -1.0f;
	} else if (ZOrin > maxAABB.z) {
		zt = (float)(maxAABB.z - ZOrin);
		if (zt < ZRay) return kNoIntersection;
		zt /= ZRay;
		inside = false;
		zn = 1.0f;
	} else {
		zt = -1.0f;
	}

	// Inside box?

	if (inside) {
		return 0.0f;
	}

	// Select farthest plane - this is
	// the plane of intersection.

	int which = 0;
	float t = xt;
	if (yt > t) {
		which = 1;
		t = yt;
	}
	if (zt > t) {
		which = 2;
		t = zt;
	}

	switch (which) {

		case 0: // intersect with yz plane
			{
				float y = YOrin + YRay*t;
				if (y < minAABB.y || y > maxAABB.y) return kNoIntersection;
				float z = ZOrin + ZRay*t;
				if (z < minAABB.z || z > maxAABB.z) return kNoIntersection;

			} break;

		case 1: // intersect with xz plane
			{
				float x = XOrin + XRay*t;
				if (x < minAABB.x || x > maxAABB.x) return kNoIntersection;
				float z = ZOrin + ZRay*t;
				if (z < minAABB.z || z > maxAABB.z) return kNoIntersection;

			} break;

		case 2: // intersect with xy plane
			{
				float x = XOrin + XRay*t;
				if (x < minAABB.x || x > maxAABB.x) return kNoIntersection;
				float y = YOrin + YRay*t;
				if (y < minAABB.y || y > maxAABB.y) return kNoIntersection;
			} break;
	}

	// Return parametric point of intersection

	return t;
}
