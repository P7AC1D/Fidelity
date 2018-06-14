#include "ShaderParams.hpp"

#include <algorithm>
#include "../Utility/Hash.hpp"

void ShaderParams::AddParam(const ShaderParam& param)
{
  _params.insert(param);
}

void ShaderParams::AddParam(const std::string& name, ShaderParamType type, uint32 slot)
{
  ShaderParam param(name, type, slot);
  AddParam(param);
}

void ShaderParams::RemoveParam(const ShaderParam &param)
{
  auto iter = _params.find(param);
  if (iter != _params.end())
  {
    _params.erase(iter);
  }
}

void ShaderParams::RemoveParam(const std::string& name, ShaderParamType type, uint32 slot)
{
  ShaderParam paramToFind(name, type, slot);
  RemoveParam(paramToFind);
}

const std::string& ShaderParams::GetParamName(ShaderParamType type, uint32 slot) const
{
  auto iter = std::find_if(_params.begin(), _params.end(), [&](ShaderParam param)
                  {
                    return param.Slot == slot && param.Type == type;
                  });
  if (iter != _params.end())
  {
    return iter->Name;
  }
  throw std::runtime_error("Could not find a valid param with slot" + std::to_string(slot));
}

std::size_t ShaderParams::Hasher::operator()(const ShaderParam& key) const
{
  std::size_t seed = 0;
  Hash::Combine(seed, key.Name);
  Hash::Combine(seed, static_cast<uint32>(key.Type));
  Hash::Combine(seed, key.Slot);
  return seed;
}

bool ShaderParams::Equality::operator()(const ShaderParam& a, const ShaderParam& b) const
{
  return a.Name == b.Name && a.Type == b.Type && a.Slot == b.Slot;
}
