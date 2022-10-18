#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"

class GameObject;

class CullingTest : public Application
{
public:
  CullingTest(const ApplicationDesc &desc);

  void onStart() override;
  void onUpdate(uint32 dtMs) override;
};
