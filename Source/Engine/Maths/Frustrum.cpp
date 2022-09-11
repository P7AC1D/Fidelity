#include "Frustrum.hpp"

#include "Math.hpp"

Frustrum::Frustrum()
{
}

Frustrum::Frustrum(const std::array<Plane, 6> &planes) : _planes(planes)
{
}

Frustrum::Frustrum(const Matrix4 &projection)
{
	// Taken from http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf

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
		_planes[i].Normal.X /= length;
		_planes[i].Normal.Y /= length;
		_planes[i].Normal.Z /= length;
		_planes[i].D /= length;
	}
}

bool Frustrum::Intersects(const Aabb &box) const
{
	for (auto &plane : _planes)
	{
		Vector3 aabbCenter = box.GetCenter();
		Vector3 aabbExtents = box.GetHalfSize();

		Vector3 absPlaneNormal = Math::Abs(plane.Normal);
		float32 d = Vector3::Dot(aabbCenter, plane.Normal);
		float32 r = Vector3::Dot(aabbExtents, absPlaneNormal);
		if (d + r < -plane.D)
		{
			return false;
		}
	}
	return true;
}
