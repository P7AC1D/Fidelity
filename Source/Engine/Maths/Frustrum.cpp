#include "Frustrum.hpp"
#include "../Core/TransformComponent.h"
#include "../Rendering/CameraComponent.h"
#include "Vector3.hpp"
#include <cmath>

Frustrum::Frustrum()
{
    // Create a default frustum that contains everything
    // This ensures backward compatibility for uninitialized frustums
    // The Plane constructor expects (normal, point_on_plane)
    _left = Plane(Vector3(1.0f, 0.0f, 0.0f), Vector3(-1000.0f, 0.0f, 0.0f));
    _right = Plane(Vector3(-1.0f, 0.0f, 0.0f), Vector3(1000.0f, 0.0f, 0.0f));
    _top = Plane(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 1000.0f, 0.0f));
    _bottom = Plane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, -1000.0f, 0.0f));
    _near = Plane(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -0.01f));
    _far = Plane(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1000.0f));
}

Frustrum::Frustrum(const CameraComponent &camera)
{
    // Extract frustum planes from camera's view-projection matrix
    Matrix4 viewMatrix = camera.getView();
    Matrix4 projMatrix = camera.getProj();
    
    // Check if the view matrix is identity (camera not properly set up)
    if (viewMatrix == Matrix4::Identity)
    {
        // Create a default permissive frustum that includes most objects
        // This prevents culling when the camera transform isn't properly set up
        _left = Plane(Vector3(1.0f, 0.0f, 0.0f), Vector3(-1000.0f, 0.0f, 0.0f));
        _right = Plane(Vector3(-1.0f, 0.0f, 0.0f), Vector3(1000.0f, 0.0f, 0.0f));
        _top = Plane(Vector3(0.0f, -1.0f, 0.0f), Vector3(0.0f, 1000.0f, 0.0f));
        _bottom = Plane(Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, -1000.0f, 0.0f));
        _near = Plane(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, -camera.getNear()));
        _far = Plane(Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, camera.getFar()));
        return;
    }
    
    Matrix4 viewProjMatrix = projMatrix * viewMatrix;
    extractPlanesFromMatrix(viewProjMatrix);
}

Frustrum::Frustrum(const Plane& left, const Plane& right, const Plane& top, 
                   const Plane& bottom, const Plane& nearPlane, const Plane& farPlane)
    : _left(left), _right(right), _top(top), _bottom(bottom), _near(nearPlane), _far(farPlane)
{
}

bool Frustrum::contains(const Aabb &box, const TransformComponent &transform) const
{
    // Get transform matrix from component
    Matrix4 transformMatrix = transform.getWorldMatrix();
    
    // Check if transform is axis-aligned for optimization
    if (isTransformAxisAligned(transform))
    {
        return containsAxisAligned(box, transform);
    }
    else
    {
        return containsOriented(box, transform);
    }
}

bool Frustrum::contains(const Aabb &box, const Matrix4 &transform) const
{
    // For Matrix4, we can't easily determine if it's axis-aligned without decomposing it
    // So we'll use the oriented bounding box approach directly
    
    // Get AABB min/max and compute the 8 corners
    Vector3 min = box.getMin();
    Vector3 max = box.getMax();
    
    Vector3 corners[8] = {
        Vector3(min.X, min.Y, min.Z), // 0: min corner
        Vector3(max.X, min.Y, min.Z), // 1: max X
        Vector3(min.X, max.Y, min.Z), // 2: max Y  
        Vector3(max.X, max.Y, min.Z), // 3: max X,Y
        Vector3(min.X, min.Y, max.Z), // 4: max Z
        Vector3(max.X, min.Y, max.Z), // 5: max X,Z
        Vector3(min.X, max.Y, max.Z), // 6: max Y,Z
        Vector3(max.X, max.Y, max.Z)  // 7: max corner
    };
    
    // Transform all corners
    for (int i = 0; i < 8; ++i)
    {
        Vector4 corner4D(corners[i].X, corners[i].Y, corners[i].Z, 1.0f);
        Vector4 transformedCorner = transform * corner4D;
        corners[i] = Vector3(transformedCorner.X, transformedCorner.Y, transformedCorner.Z);
    }
    
    // Test the transformed AABB against each frustum plane
    for (int i = 0; i < 8; ++i)
    {
        const Vector3& corner = corners[i];
        
        // If any corner is inside all planes, the AABB intersects the frustum
        bool insideAll = true;
        
        if (_left.getSignedDistance(corner) < 0) insideAll = false;
        if (_right.getSignedDistance(corner) < 0) insideAll = false;
        if (_top.getSignedDistance(corner) < 0) insideAll = false;
        if (_bottom.getSignedDistance(corner) < 0) insideAll = false;
        if (_near.getSignedDistance(corner) < 0) insideAll = false;
        if (_far.getSignedDistance(corner) < 0) insideAll = false;
        
        if (insideAll)
        {
            return true; // At least one corner is inside
        }
    }
    
    return false; // No corners are inside the frustum
}

bool Frustrum::isTransformAxisAligned(const TransformComponent &transform) const
{
    // A transform is axis-aligned if it only has translation and uniform scale
    Vector3 scale = transform.getScale();
    Quaternion rotation = transform.getRotation();
    
    // Check if rotation is identity (or very close to it)
    Quaternion identity = Quaternion::Identity;
    float32 dotProduct = abs(rotation.W * identity.W + rotation.X * identity.X + 
                           rotation.Y * identity.Y + rotation.Z * identity.Z);
    
    return dotProduct > 0.999f; // Very close to identity
}

bool Frustrum::containsAxisAligned(const Aabb &aabb, const TransformComponent &transform) const
{
    // For axis-aligned transforms, we can transform the AABB directly
    Vector3 position = transform.getPosition();
    Vector3 scale = transform.getScale();
    
    // Transform AABB
    Vector3 min = aabb.getMin() * scale + position;
    Vector3 max = aabb.getMax() * scale + position;
    
    // Ensure min/max are correct after scaling
    if (scale.X < 0.0f) { float32 temp = min.X; min.X = max.X; max.X = temp; }
    if (scale.Y < 0.0f) { float32 temp = min.Y; min.Y = max.Y; max.Y = temp; }
    if (scale.Z < 0.0f) { float32 temp = min.Z; min.Z = max.Z; max.Z = temp; }
    
    // Test against each frustum plane
    return testAABBAgainstPlane(min, max, _left) &&
           testAABBAgainstPlane(min, max, _right) &&
           testAABBAgainstPlane(min, max, _top) &&
           testAABBAgainstPlane(min, max, _bottom) &&
           testAABBAgainstPlane(min, max, _near) &&
           testAABBAgainstPlane(min, max, _far);
}

bool Frustrum::containsOriented(const Aabb &aabb, const TransformComponent &transform) const
{
    // For oriented transforms, we need to test all 8 corners of the AABB
    Matrix4 transformMatrix = transform.getWorldMatrix();
    
    Vector3 min = aabb.getMin();
    Vector3 max = aabb.getMax();
    
    // Generate all 8 corners of the AABB
    Vector3 corners[8] = {
        {min.X, min.Y, min.Z},
        {max.X, min.Y, min.Z},
        {min.X, max.Y, min.Z},
        {max.X, max.Y, min.Z},
        {min.X, min.Y, max.Z},
        {max.X, min.Y, max.Z},
        {min.X, max.Y, max.Z},
        {max.X, max.Y, max.Z}
    };
    
    // Transform all corners
    Vector3 transformedCorners[8];
    for (int i = 0; i < 8; i++)
    {
        transformedCorners[i] = transformMatrix * corners[i];
    }
    
    // Test against each frustum plane
    Plane planes[6] = {_left, _right, _top, _bottom, _near, _far};
    
    for (int p = 0; p < 6; p++)
    {
        bool allOutside = true;
        for (int c = 0; c < 8; c++)
        {
            if (planes[p].getSignedDistance(transformedCorners[c]) >= 0.0f)
            {
                allOutside = false;
                break;
            }
        }
        
        // If all corners are outside this plane, the AABB is outside the frustum
        if (allOutside)
        {
            return false;
        }
    }
    
    return true;
}

bool Frustrum::testAABBAgainstPlane(const Vector3& min, const Vector3& max, const Plane& plane) const
{
    // Find the positive vertex (vertex most in the direction of the plane normal)
    Vector3 normal = plane.getNormal();
    Vector3 positiveVertex = min;
    
    if (normal.X >= 0.0f) positiveVertex.X = max.X;
    if (normal.Y >= 0.0f) positiveVertex.Y = max.Y;
    if (normal.Z >= 0.0f) positiveVertex.Z = max.Z;
    
    // If the positive vertex is behind the plane, the AABB is completely outside
    return plane.getSignedDistance(positiveVertex) >= 0.0f;
}

void Frustrum::extractPlanesFromMatrix(const Matrix4& viewProjMatrix)
{
    // Extract frustum planes from view-projection matrix using Gribb-Hartmann method
    // Matrix4 uses Vector4[4] array accessible via operator[]
    const Vector4& row0 = viewProjMatrix[0];
    const Vector4& row1 = viewProjMatrix[1];
    const Vector4& row2 = viewProjMatrix[2];
    const Vector4& row3 = viewProjMatrix[3];
    
    // Create planes directly with proper normal and distance
    // The plane equation is: ax + by + cz + d = 0
    // Where (a,b,c) is the normal and d is the distance
    
    // Left plane: row3 + row0
    Vector4 leftPlane = Vector4(row3.X + row0.X, row3.Y + row0.Y, row3.Z + row0.Z, row3.W + row0.W);
    _left = createPlaneFromVector4(leftPlane);
    
    // Right plane: row3 - row0
    Vector4 rightPlane = Vector4(row3.X - row0.X, row3.Y - row0.Y, row3.Z - row0.Z, row3.W - row0.W);
    _right = createPlaneFromVector4(rightPlane);
    
    // Top plane: row3 - row1
    Vector4 topPlane = Vector4(row3.X - row1.X, row3.Y - row1.Y, row3.Z - row1.Z, row3.W - row1.W);
    _top = createPlaneFromVector4(topPlane);
    
    // Bottom plane: row3 + row1
    Vector4 bottomPlane = Vector4(row3.X + row1.X, row3.Y + row1.Y, row3.Z + row1.Z, row3.W + row1.W);
    _bottom = createPlaneFromVector4(bottomPlane);
    
    // Near plane: row3 + row2
    Vector4 nearPlane = Vector4(row3.X + row2.X, row3.Y + row2.Y, row3.Z + row2.Z, row3.W + row2.W);
    _near = createPlaneFromVector4(nearPlane);
    
    // Far plane: row3 - row2
    Vector4 farPlane = Vector4(row3.X - row2.X, row3.Y - row2.Y, row3.Z - row2.Z, row3.W - row2.W);
    _far = createPlaneFromVector4(farPlane);
}

Plane Frustrum::createPlaneFromVector4(const Vector4& planeVector) const
{
    // The Vector4 represents the plane equation: ax + by + cz + d = 0
    // We need to normalize it first
    Vector3 normal(planeVector.X, planeVector.Y, planeVector.Z);
    float32 distance = planeVector.W;
    
    // Normalize the plane
    float32 length = Vector3::Length(normal);
    if (length > 0.0001f) // Avoid division by zero
    {
        normal /= length;
        distance /= length;
    }
    
    // Create a plane directly using the normal and distance
    // The Plane constructor expects normal and a point on the plane
    // For plane equation ax + by + cz + d = 0, a point on the plane is normal * (-d)
    Vector3 pointOnPlane = normal * (-distance);
    
    return Plane(normal, pointOnPlane);
}
