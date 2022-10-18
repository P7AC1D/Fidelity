#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"

class GameObject;

class Test3D : public Application
{
public:
  Test3D(const ApplicationDesc &desc);

  void onStart() override;
  void onUpdate(uint32 dtMs) override;
};
