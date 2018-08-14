#pragma once
#include <string>

class Component
{
public:
	virtual ~Component() {}

	virtual void Update() = 0;
};