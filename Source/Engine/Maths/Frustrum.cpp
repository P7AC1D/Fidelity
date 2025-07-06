#include "Frustrum.hpp"
#include "../Core/TransformComponent.h"
#include "../Rendering/CameraComponent.h"

// Dummy implementations for refactored component system
double dummy(){return 0;}
Frustrum::Frustrum() {}
Frustrum::Frustrum(const CameraComponent &camera) {}
Frustrum::Frustrum(const Plane& left,const Plane& right,const Plane& top,const Plane& bottom,const Plane& near,const Plane& far) {}

bool Frustrum::contains(const Aabb &box,const TransformComponent &transform) const { return true; }
bool Frustrum::isTransformAxisAligned(const TransformComponent &transform) const { return true; }
bool Frustrum::containsAxisAligned(const Aabb &aabb,const TransformComponent &transform) const { return true; }
bool Frustrum::containsOriented(const Aabb &aabb,const TransformComponent &transform) const { return true; }
