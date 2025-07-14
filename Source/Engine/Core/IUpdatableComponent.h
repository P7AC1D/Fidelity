#pragma once

#include "Types.hpp"

/// Interface for components that need to be updated each frame
class IUpdatableComponent
{
public:
    virtual ~IUpdatableComponent() = default;
    
    /// Called once per frame with delta time in milliseconds
    virtual void update(float32 dt) = 0;
};
