#pragma once

#include "ComponentTypeId.h"

/// IComponent is the modern base interface for all engine components.
/// It defines optional lifecycle methods and a type-safe identifier.
class IComponent
{
public:
    virtual ~IComponent() = default;

    /// Called once after creation; set up any required state here.
    virtual void initialize() {}

    /// Called when the component is activated (e.g. added to GameObject).
    virtual void activate() {}

    /// Called when the component is deactivated or removed.
    virtual void deactivate() {}

    /// Returns the unique ComponentTypeId for this concrete component.
    virtual ComponentTypeId getTypeId() const = 0;

    /// Optional inspector hook for editor UI.
    virtual void drawInspector() {}
};
