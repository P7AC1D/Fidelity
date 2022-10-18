#include "ShaderParams.hpp"

#include <algorithm>
#include "../Utility/Hash.hpp"

std::string ShaderParams::NoParameter = std::string();

void ShaderParams::addParam(const ShaderParam &param)
{
  _params.insert(param);
}

void ShaderParams::addParam(const std::string &name, ShaderParamType type, uint32 slot)
{
  ShaderParam param(name, type, slot);
  addParam(param);
}

void ShaderParams::removeParam(const ShaderParam &param)
{
  auto iter = _params.find(param);
  if (iter != _params.end())
  {
    _params.erase(iter);
  }
}

void ShaderParams::removeParam(const std::string &name, ShaderParamType type, uint32 slot)
{
  ShaderParam paramToFind(name, type, slot);
  removeParam(paramToFind);
}

const std::string &ShaderParams::getParamName(ShaderParamType type, uint32 slot) const
{
  auto iter = std::find_if(_params.begin(), _params.end(), [&](ShaderParam param)
                           { return param.Slot == slot && param.Type == type; });
  if (iter != _params.end())
  {
    return iter->Name;
  }
  // TODO: No parameter specified for slot - Maybe log this info somewhere?
  return NoParameter;
}

std::size_t ShaderParams::Hasher::operator()(const ShaderParam &key) const
{
  std::size_t seed = 0;
  Hash::combine(seed, key.Name);
  Hash::combine(seed, static_cast<uint32>(key.Type));
  Hash::combine(seed, key.Slot);
  return seed;
}

bool ShaderParams::Equality::operator()(const ShaderParam &a, const ShaderParam &b) const
{
  return a.Name == b.Name && a.Type == b.Type && a.Slot == b.Slot;
}
