#include "ComponentBase.h"
#include "GameObject.h"

void ComponentBase::initialize()
{
    _initialized = true;
    onInitialize();
}

void ComponentBase::activate()
{
    _active = true;
    onActivate();
}

void ComponentBase::deactivate()
{
    _active = false;
    onDeactivate();
}

void ComponentBase::update(float32 dt)
{
    // Default implementation calls virtual hook
    // Components can override either this method or the onUpdate() hook
    onUpdate(dt);
}

// Template implementations - must be in header for template instantiation
// These are moved to header file for proper template compilation
