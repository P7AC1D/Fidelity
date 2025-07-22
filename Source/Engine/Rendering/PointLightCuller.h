#pragma once

#include "../Core/Maths.h"
#include "../Maths/AABB.hpp"
#include "../Maths/Frustrum.hpp"
#include "LightComponent.h"
#include "DrawableComponent.h"
#include <array>
#include <vector>
#include <memory>

/**
 * @brief Specialized culling system for point light shadow rendering
 * 
 * This class provides efficient culling of scene objects for point light shadow maps,
 * implementing both sphere-based culling (objects outside light radius) and 
 * per-face frustum culling for each cubemap face.
 */
class PointLightCuller
{
public:
    /**
     * @brief Results of point light culling operations
     */
    struct CullingResult
    {
        std::vector<std::shared_ptr<DrawableComponent>> sphereCulled;       // Objects within light radius
        std::array<std::vector<std::shared_ptr<DrawableComponent>>, 6> faceCulled; // Objects per cubemap face
        uint32 originalCount;                                               // Original object count
        uint32 sphereCulledCount;                                          // Objects after sphere culling
        std::array<uint32, 6> faceCulledCounts;                           // Objects per face after face culling
        
        // Performance metrics
        float sphereCullingRatio() const { return originalCount > 0 ? (float)sphereCulledCount / originalCount : 0.0f; }
        float averageFaceCullingRatio() const;
    };

    /**
     * @brief Configuration settings for point light culling
     */
    struct CullingSettings
    {
        bool enableSphereCulling = true;        // Enable sphere-AABB intersection culling
        bool enableFaceCulling = true;          // Enable per-face frustum culling
        bool enableDistanceLOD = true;          // Enable distance-based LOD culling
        float maxShadowDistance = 200.0f;       // Maximum distance for shadow casting
        float minObjectSize = 0.1f;             // Minimum object size to cast shadows
        float faceCullingExpansion = 1.1f;      // Expansion factor for face frustums (reduces edge artifacts)
    };

public:
    PointLightCuller();
    ~PointLightCuller() = default;

    /**
     * @brief Perform complete culling for a point light
     * @param pointLight The point light to cull for
     * @param objects All scene objects to consider
     * @return Complete culling results with per-face object lists
     */
    CullingResult cullObjectsForPointLight(
        const std::shared_ptr<LightComponent>& pointLight,
        const std::vector<std::shared_ptr<DrawableComponent>>& objects) const;

    /**
     * @brief Cull objects against light sphere (radius-based culling)
     * @param lightPos Position of the point light
     * @param lightRadius Radius/range of the point light
     * @param objects Objects to cull
     * @return Objects within the light's influence radius
     */
    std::vector<std::shared_ptr<DrawableComponent>> sphereCull(
        const Vector3& lightPos, 
        float lightRadius,
        const std::vector<std::shared_ptr<DrawableComponent>>& objects) const;

    /**
     * @brief Cull objects for a specific cubemap face
     * @param lightPos Position of the point light
     * @param lightRadius Radius/range of the point light
     * @param faceIndex Cubemap face index (0-5: +X, -X, +Y, -Y, +Z, -Z)
     * @param objects Objects to cull (should be pre-culled by sphere)
     * @return Objects visible from the specified face
     */
    std::vector<std::shared_ptr<DrawableComponent>> faceCull(
        const Vector3& lightPos,
        float lightRadius,
        uint32 faceIndex,
        const std::vector<std::shared_ptr<DrawableComponent>>& objects) const;

    /**
     * @brief Get/Set culling configuration
     */
    void setCullingSettings(const CullingSettings& settings) { _settings = settings; }
    const CullingSettings& getCullingSettings() const { return _settings; }

private:
    /**
     * @brief Test sphere-AABB intersection
     * @param sphereCenter Center of the sphere (light position)
     * @param sphereRadius Radius of the sphere (light radius)
     * @param aabb Axis-aligned bounding box of the object
     * @return True if sphere intersects or contains the AABB
     */
    bool sphereIntersectsAABB(const Vector3& sphereCenter, float sphereRadius, const Aabb& aabb) const;

    /**
     * @brief Calculate distance from light to object (closest point on AABB)
     * @param lightPos Position of the light
     * @param aabb Object's bounding box
     * @return Distance from light to closest point on object
     */
    float getObjectDistanceFromLight(const Vector3& lightPos, const Aabb& aabb) const;
    
    /**
     * @brief Create frustums for all 6 cubemap faces
     * @param lightPos Position of the point light
     * @param lightRadius Radius of the point light (far plane)
     * @return Array of 6 frustums, one for each cubemap face
     */
    std::array<Frustrum, 6> createCubemapFrustums(const Vector3& lightPos, float lightRadius) const;

    /**
     * @brief Create frustum for a specific cubemap face
     * @param lightPos Position of the point light
     * @param lightRadius Radius of the point light (far plane)
     * @param faceIndex Face index (0-5)
     * @return Frustum for the specified face
     */
    Frustrum createFaceFrustum(const Vector3& lightPos, float lightRadius, uint32 faceIndex) const;
    
    /**
     * @brief Check if object should cast shadows for this light
     * @param drawable Object to check
     * @param lightPos Position of the light
     * @param lightRadius Radius of the light
     * @return True if object should cast shadows
     */
    bool shouldCastShadowForLight(
        const std::shared_ptr<DrawableComponent>& drawable,
        const Vector3& lightPos,
        float lightRadius) const;

private:
    CullingSettings _settings;
    
    // Cubemap face directions (same as used in renderer)
    static const std::array<Vector3, 6> FACE_DIRECTIONS;
    static const std::array<Vector3, 6> FACE_UPS;
    
    // Near plane for point light shadow frustums
    static constexpr float POINT_LIGHT_NEAR_PLANE = 0.1f;
    static constexpr float POINT_LIGHT_FOV_DEGREES = 90.0f;
    
    /**
     * @brief Extract frustum planes from a view-projection matrix
     * @param viewProjection Combined view-projection matrix
     * @return Frustum object with extracted planes
     */
    Frustrum extractFrustumFromMatrix(const Matrix4& viewProjection) const;
};
