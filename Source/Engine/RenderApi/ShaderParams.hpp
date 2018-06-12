#pragma once
#include <unordered_set>
#include <string>
#include "../Core/Types.hpp"

enum class ShaderParamType
{
  ConstBuffer,
  Texture,
  Sampler
};

struct ShaderParam
{
  ShaderParam(const std::string& name, ShaderParamType type, uint32 slot):
    Name(name),
    Type(type),
    Slot(slot)
  {}
  
  std::string Name;
  ShaderParamType Type;
  uint32 Slot;
};

class ShaderParams
{
public:
  void AddParam(const ShaderParam& param);
  void AddParam(const std::string& name, ShaderParamType type, uint32 slot);
  
  void RemoveParam(const ShaderParam& param);
  void RemoveParam(const std::string& name, ShaderParamType type, uint32 slot);
  
  const std::string& GetParamName(uint32 slot) const;
  
private:
  class Hasher
  {
  public:
    std::size_t operator()(const ShaderParam& key) const;
  };
  
  class Equality
  {
  public:
    bool operator()(const ShaderParam& a, const ShaderParam& b) const;
  };
  
private:
  std::unordered_set<ShaderParam, Hasher, Equality> _params;
};
