#include "Frustrum.hpp"

Frustrum::Frustrum()
{
}

Frustrum::Frustrum(const std::array<Plane, 6>& planes) : _planes(planes)
{
}

Frustrum::Frustrum(const Matrix4& projection)
{
	// Left
	{
		Plane plane;
		plane.Normal.X = projection[3][0] + projection[0][0];
		plane.Normal.Y = projection[3][1] + projection[0][1];
		plane.Normal.Z = projection[3][2] + projection[0][2];
		plane.D = projection[3][3] + projection[0][3];

		_planes[0] = plane;
	}

	// Right
	{
		Plane plane;
		plane.Normal.X = projection[3][0] - projection[0][0];
		plane.Normal.Y = projection[3][1] - projection[0][1];
		plane.Normal.Z = projection[3][2] - projection[0][2];
		plane.D = projection[3][3] - projection[0][3];

		_planes[1] = plane;
	}

	// Top
	{
		Plane plane;
		plane.Normal.X = projection[3][0] - projection[1][0];
		plane.Normal.Y = projection[3][1] - projection[1][1];
		plane.Normal.Z = projection[3][2] - projection[1][2];
		plane.D = projection[3][3] - projection[1][3];

		_planes[2] = plane;
	}

	// Bottom
	{
		Plane plane;
		plane.Normal.X = projection[3][0] + projection[1][0];
		plane.Normal.Y = projection[3][1] + projection[1][1];
		plane.Normal.Z = projection[3][2] + projection[1][2];
		plane.D = projection[3][3] + projection[1][3];

		_planes[3] = plane;
	}

	// Near
	{
		Plane plane;
		plane.Normal.X = projection[3][0] + projection[2][0];
		plane.Normal.Y = projection[3][1] + projection[2][1];
		plane.Normal.Z = projection[3][2] + projection[2][2];
		plane.D = projection[3][3] + projection[2][3];

		_planes[4] = plane;
	}

	// Far
	{
		Plane plane;
		plane.Normal.X = projection[3][0] - projection[2][0];
		plane.Normal.Y = projection[3][1] - projection[2][1];
		plane.Normal.Z = projection[3][2] - projection[2][2];
		plane.D = projection[3][3] - projection[2][3];

		_planes[5] = plane;
	}

	for (uint64 i = 0; i < 6; i++)
	{
		float32 length = Vector3::Length(_planes[i].Normal);
		_planes[i].D /= -length;
	}
}

bool Frustrum::Intersects(const Aabb& box) const
{
	Vector3 center = box.GetCenter();
	Vector3 extents = box.GetHalfSize();
	Vector3 absExtents(std::fabs(extents.X), std::fabs(extents.Y), std::fabs(extents.Z));

	for (auto& plane : _planes)
	{
		float32 dist = Vector3::Dot(center, plane.Normal) + plane.D;

		float32 effectiveRadius = absExtents.X * std::fabs(plane.Normal.X);
		effectiveRadius += absExtents.Y * std::fabs(plane.Normal.Y);
		effectiveRadius += absExtents.Z * std::fabs(plane.Normal.Z);

		if (dist < -effectiveRadius)
		{
			return false;
		}
	}
	return true;
}
