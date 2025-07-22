#pragma once
#include "Types.hpp"

/// ComponentTypeId is a unique identifier for each component type.
using ComponentTypeId = uint32;

/// Generates a new unique ComponentTypeId at runtime.
inline ComponentTypeId getUniqueComponentTypeId()
{
    static ComponentTypeId lastId = 0;
    return lastId++;
}

/// Returns the unique ComponentTypeId for type T.
template <typename T>
inline ComponentTypeId getComponentTypeId() noexcept
{
    static ComponentTypeId typeId = getUniqueComponentTypeId();
    return typeId;
}
