#include "MaterialFactory.hpp"

#include <algorithm>
#include "Material.hpp"

uint32 MaterialFactory::MaterialCount = 0;
std::vector<std::shared_ptr<Material>> MaterialFactory::_materials;

std::shared_ptr<Material> MaterialFactory::Create()
{
	std::shared_ptr<Material> material(new Material);
	material->_id = MaterialCount++;
	_materials.push_back(material);
	return material;
}