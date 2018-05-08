#pragma once
#include "../Shader.hpp"

class GLShader : public Shader
{
public:
  virtual ~GLShader();
  
  uint32 GetId() const { return _id; }
  
  void Compile() override;
  
protected:
  GLShader(const ShaderDesc& desc);
  
private:
  uint32 _id;
};
