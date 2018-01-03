#pragma once
#include <memory>

#include "Types.h"

class Texture;

class Material
{
public:
  Material();
  ~Material();
  
  void SetDiffuseFactor(uint32 kd);
  void SetDiffuseMap(const std::shared_ptr<Texture> diffuseMap);
  
  Texture* GetDiffuseMap() const { return _diffuseMap.get(); }
  uint32 GetId() const { return _id; }
  
  void UploadUniforms() const;
  
private:
  std::shared_ptr<Texture> _diffuseMap;
  
  uint32 _diffuseFactor;
  uint32 _id;
  bool _needsUpdate;
};
