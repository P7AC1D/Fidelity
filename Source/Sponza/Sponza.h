#pragma once
#include <memory>

#include "../Engine/Core/Fidelity.h"

class GameObject;

class Sponza : public Application
{
public:
  Sponza(const ApplicationDesc &desc);

  void onStart() override;
  void onUpdate(uint32 dtMs) override;
};
