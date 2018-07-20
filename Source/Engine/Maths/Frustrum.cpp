#include "Frustrum.hpp"

Frustrum::Frustrum()
{
}

Frustrum::Frustrum(const std::array<Plane, 6>& planes) : _planes(planes)
{
}

Frustrum::Frustrum(const Matrix4& viewProjection)
{
	// Taken from http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf

	// Left
	{
		Plane plane;
		plane.Normal.X = viewProjection[3][0] + viewProjection[0][0];
		plane.Normal.Y = viewProjection[3][1] + viewProjection[0][1];
		plane.Normal.Z = viewProjection[3][2] + viewProjection[0][2];
		plane.D        = viewProjection[3][3] + viewProjection[0][3];

		_planes[0] = plane;
	}

	// Right
	{
		Plane plane;
		plane.Normal.X = viewProjection[3][0] - viewProjection[0][0];
		plane.Normal.Y = viewProjection[3][1] - viewProjection[0][1];
		plane.Normal.Z = viewProjection[3][2] - viewProjection[0][2];
		plane.D        = viewProjection[3][3] - viewProjection[0][3];

		_planes[1] = plane;
	}

	// Top
	{
		Plane plane;
		plane.Normal.X = viewProjection[3][0] - viewProjection[1][0];
		plane.Normal.Y = viewProjection[3][1] - viewProjection[1][1];
		plane.Normal.Z = viewProjection[3][2] - viewProjection[1][2];
		plane.D        = viewProjection[3][3] - viewProjection[1][3];

		_planes[2] = plane;
	}

	// Bottom
	{
		Plane plane;
		plane.Normal.X = viewProjection[3][0] + viewProjection[1][0];
		plane.Normal.Y = viewProjection[3][1] + viewProjection[1][1];
		plane.Normal.Z = viewProjection[3][2] + viewProjection[1][2];
		plane.D        = viewProjection[3][3] + viewProjection[1][3];

		_planes[3] = plane;
	}

	// Near
	{
		Plane plane;
		plane.Normal.X = viewProjection[3][0] + viewProjection[2][0];
		plane.Normal.Y = viewProjection[3][1] + viewProjection[2][1];
		plane.Normal.Z = viewProjection[3][2] + viewProjection[2][2];
		plane.D        = viewProjection[3][3] + viewProjection[2][3];

		_planes[4] = plane;
	}

	// Far
	{
		Plane plane;
		plane.Normal.X = viewProjection[3][0] - viewProjection[2][0];
		plane.Normal.Y = viewProjection[3][1] - viewProjection[2][1];
		plane.Normal.Z = viewProjection[3][2] - viewProjection[2][2];
		plane.D        = viewProjection[3][3] - viewProjection[2][3];

		_planes[5] = plane;
	}

	for (uint64 i = 0; i < 6; i++)
	{
		float32 length = Vector3::Length(_planes[i].Normal);
		_planes[i].Normal.X /= length;
		_planes[i].Normal.Y /= length;
		_planes[i].Normal.Z /= length;
		_planes[i].D        /= length;
	}
}

bool Frustrum::Intersects(const Aabb& box) const
{
	Vector3 center = box.GetCenter();
	Vector3 extents = box.GetHalfSize();

	for (auto& plane : _planes)
	{
		float32 signedDist = Vector3::Dot(center, plane.Normal) + plane.D;
		if (signedDist < 0)
		{
			return false;
		}
	}
	return true;
}
