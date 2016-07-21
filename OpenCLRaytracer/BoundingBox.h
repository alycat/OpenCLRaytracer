#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "CL\cl.h"
#include "Global.h"

struct BoundingBox{
	cl_float3 position, dimensions;

	bool operator==(const BoundingBox& other)
	{
		if (position != other.position)
			return false;
		if (dimensions != other.dimensions)
			return false;
		return true;
	}

	bool operator!=(const BoundingBox& other)
	{
		if (position != other.position)
			return true;
		if (dimensions != other.dimensions)
			return true;
		return false;
	}
};

#endif
