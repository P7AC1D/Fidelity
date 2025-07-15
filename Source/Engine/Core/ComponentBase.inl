#pragma once

#include "ComponentBase.h"
#include "GameObject.h"

// Template implementations for ComponentBase helper methods
// Include this file in .cpp files that need to use these templates

template<typename T>
T* ComponentBase::getComponent() const
{
    return _gameObject ? _gameObject->tryGetComponent<T>() : nullptr;
}

template<typename T>
std::shared_ptr<T> ComponentBase::getComponentShared() const
{
    return _gameObject ? _gameObject->getComponentShared<T>() : nullptr;
}
