#include "RenderQueue.h"
#include "Drawable.h"
#include "Camera.h"
#include "Material.h"
#include <algorithm>

RenderQueue::RenderQueue(QueueType type) : _type(type) {
}

void RenderQueue::clear() {
    _drawables.clear();
}

void RenderQueue::add(std::shared_ptr<Drawable> drawable) {
    _drawables.push_back(drawable);
}

void RenderQueue::sort(const Camera& camera) {
    if (_drawables.empty()) {
        return;
    }
    
    if (_type == QueueType::Opaque) {
        std::sort(_drawables.begin(), _drawables.end(), 
                 [this, &camera](const auto& a, const auto& b) {
                     return compareOpaque(a, b, camera);
                 });
    } else {
        std::sort(_drawables.begin(), _drawables.end(), 
                 [this, &camera](const auto& a, const auto& b) {
                     return compareTransparent(a, b, camera);
                 });
    }
}

bool RenderQueue::compareOpaque(const std::shared_ptr<Drawable>& a, 
                               const std::shared_ptr<Drawable>& b, 
                               const Camera& camera) const {
    auto materialA = a->getMaterial();
    auto materialB = b->getMaterial();
    
    // 1. Sort by shader/material variant first (minimize state changes)
    uint32 shaderIdA = materialA->getShaderID();
    uint32 shaderIdB = materialB->getShaderID();
    if (shaderIdA != shaderIdB) {
        return shaderIdA < shaderIdB;
    }
    
    // 2. Sort by texture hash second (minimize texture binding)
    uint64 textureHashA = materialA->getTextureHash();
    uint64 textureHashB = materialB->getTextureHash();
    if (textureHashA != textureHashB) {
        return textureHashA < textureHashB;
    }
    
    // 3. Sort by distance last (front-to-back for Z-rejection)
    float32 distanceA = camera.distanceFrom(a->getPosition());
    float32 distanceB = camera.distanceFrom(b->getPosition());
    return distanceA < distanceB;
}

bool RenderQueue::compareTransparent(const std::shared_ptr<Drawable>& a, 
                                    const std::shared_ptr<Drawable>& b, 
                                    const Camera& camera) const {
    auto materialA = a->getMaterial();
    auto materialB = b->getMaterial();
    
    // 1. Sort by shader/material variant first (minimize state changes)
    uint32 shaderIdA = materialA->getShaderID();
    uint32 shaderIdB = materialB->getShaderID();
    if (shaderIdA != shaderIdB) {
        return shaderIdA < shaderIdB;
    }
    
    // 2. Sort by texture hash second (minimize texture binding)
    uint64 textureHashA = materialA->getTextureHash();
    uint64 textureHashB = materialB->getTextureHash();
    if (textureHashA != textureHashB) {
        return textureHashA < textureHashB;
    }
    
    // 3. Sort by distance last (back-to-front for proper alpha blending)
    float32 distanceA = camera.distanceFrom(a->getPosition());
    float32 distanceB = camera.distanceFrom(b->getPosition());
    return distanceA > distanceB; // Note: reversed for back-to-front
}
