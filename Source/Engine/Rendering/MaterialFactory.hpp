#pragma once
#include <memory>
#include <vector>
#include "../Core/Types.hpp"

class Material;

class MaterialFactory
{
public:
	static std::shared_ptr<Material> Create();

private:
	static uint32 MaterialCount;
	static std::vector<std::shared_ptr<Material>> _materials;
};