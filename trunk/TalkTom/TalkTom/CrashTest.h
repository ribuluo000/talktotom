#include "stdafx.h"
#include <math.h>

#ifndef __CCRASHTEST__

#define __CCRASHTEST__

class CCrashTest
{
public:
	void calculateAABB(const int (*vertex)[3], const int size);

	void calculateAABB(const int XMin, const int YMin, const int ZMin,
					   const int XMax, const int YMax, const int ZMax);

	// Parametric intersection with a ray.  Returns -1 if no intresection
	// we assume that all the vertexes are in the world coordinates
	float	rayIntersect(const int XOrin, const int YOrin, const int ZOrin,
						 const int XRay , const int YRay,  const int ZRay) const;
private:
	// the following two functions are used to calculate the AABB
	void empty();
	void addPoint(int x, int y, int z);

private:
	typedef struct Vector3
	{
		int x;
		int y;
		int z;
		void normalize()
		{
			double length = sqrt(double(x*x + y*y + z*z));
			x = (int)(x / length);
			y = (int)(y / length);
			z = (int)(z / length);
		}
	}VECTOR3D, *PVECTOR3D;

	VECTOR3D minAABB;
	VECTOR3D maxAABB;
};

#endif