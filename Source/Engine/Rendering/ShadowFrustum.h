#pragma once

#include "../Core/Maths.h"
#include "../Maths/Frustrum.hpp"
#include <vector>
#include <memory>
#include "CameraComponent.h"
#include "LightComponent.h"
#include "DrawableComponent.h"

/**
 * @brief Manages frustum culling specifically for shadow rendering
 * 
 * This class handles the creation of shadow frustums for cascade shadow maps
 * and provides efficient culling methods for shadow casters.
 */
class ShadowFrustum
{
public:
    ShadowFrustum();
    ~ShadowFrustum() = default;

    /**
     * @brief Build shadow frustums from light transform matrices
     * @param lightTransforms Array of light-space transform matrices for each cascade
     * @param cascadeCount Number of cascade levels
     */
    void buildFromLightTransforms(const std::vector<Matrix4>& lightTransforms, uint32 cascadeCount);

    /**
     * @brief Build extended camera frustum for broad-phase culling
     * @param camera Main camera
     * @param shadowDistance Maximum shadow distance multiplier
     */
    void buildExtendedCameraFrustum(const CameraComponent& camera, float32 shadowDistance = 1.5f);

    /**
     * @brief Cull objects against a specific cascade frustum
     * @param cascadeIndex Index of the cascade (0-3)
     * @param objects List of objects to cull
     * @return Vector of objects that intersect with the cascade frustum
     */
    std::vector<std::shared_ptr<DrawableComponent>> cullForCascade(uint32 cascadeIndex,
                                                          const std::vector<std::shared_ptr<DrawableComponent>>& objects) const;

    /**
     * @brief Broad phase culling against extended camera frustum
     * @param objects List of objects to cull
     * @return Vector of objects that could potentially cast shadows
     */
    std::vector<std::shared_ptr<DrawableComponent>> broadPhaseCull(const std::vector<std::shared_ptr<DrawableComponent>>& objects) const;

    /**
     * @brief Filter objects based on shadow casting relevance
     * @param objects List of objects to filter
     * @return Vector of objects that should cast shadows
     */
    std::vector<std::shared_ptr<DrawableComponent>> shadowRelevanceFilter(const std::vector<std::shared_ptr<DrawableComponent>>& objects) const;

    /**
     * @brief Get the frustum for a specific cascade
     * @param cascadeIndex Index of the cascade
     * @return Reference to the cascade frustum
     */
    const Frustrum& getCascadeFrustum(uint32 cascadeIndex) const;

    /**
     * @brief Get the extended camera frustum
     * @return Reference to the extended camera frustum
     */
    const Frustrum& getExtendedCameraFrustum() const;

private:
    static constexpr uint32 MAX_CASCADE_LAYERS = 4;
    
    Frustrum _cascadeFrustums[MAX_CASCADE_LAYERS];
    Frustrum _extendedCameraFrustum;
    uint32 _cascadeCount;
    
    /**
     * @brief Extract frustum planes from a transform matrix
     * @param transform Light-space transform matrix
     * @return Frustum extracted from the matrix
     */
    Frustrum extractFrustumFromMatrix(const Matrix4& transform) const;
    
    /**
     * @brief Check if an object should cast shadows based on material properties
     * @param drawable Object to check
     * @return True if the object should cast shadows
     */
    bool shouldCastShadows(const std::shared_ptr<DrawableComponent>& drawable) const;
    
    /**
     * @brief Check if an object is large enough to cast meaningful shadows
     * @param drawable Object to check
     * @param camera Main camera for screen-space size calculation
     * @return True if the object is large enough
     */
    bool isLargeEnoughForShadows(const std::shared_ptr<DrawableComponent>& drawable, const CameraComponent& camera) const;
};
