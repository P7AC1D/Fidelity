#pragma once
#include <unordered_set>
#include <string>
#include "../Core/Types.hpp"

enum class ShaderParamType
{
  ConstBuffer,
  Texture,
  Sampler,
};

struct ShaderParam
{
  ShaderParam(const std::string &name, ShaderParamType type, uint32 slot) : Name(name),
                                                                            Type(type),
                                                                            Slot(slot)
  {
  }

  std::string Name;
  ShaderParamType Type;
  uint32 Slot;
};

class ShaderParams
{
public:
  void addParam(const ShaderParam &param);
  void addParam(const std::string &name, ShaderParamType type, uint32 slot);

  void removeParam(const ShaderParam &param);
  void removeParam(const std::string &name, ShaderParamType type, uint32 slot);

  const std::string &getParamName(ShaderParamType type, uint32 slot) const;

private:
  class Hasher
  {
  public:
    std::size_t operator()(const ShaderParam &key) const;
  };

  class Equality
  {
  public:
    bool operator()(const ShaderParam &a, const ShaderParam &b) const;
  };

private:
  static std::string NoParameter;
  std::unordered_set<ShaderParam, Hasher, Equality> _params;
};
