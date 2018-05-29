#pragma once
#include <unordered_map>
#include "../Shader.hpp"

class GLShader : public Shader
{
  friend class GLRenderDevice;
  
public:
  ~GLShader();
  
  uint32 GetId() const { return _id; }
  
  void Compile() override;
  
  void BindUniformBlock(const std::string& name, uint32 bindingPoint);
  
private:
  GLShader(const ShaderDesc& desc);
  
  uint32 GetUniformBlockIndex(const std::string& name);
  
private:
  uint32 _id;
  std::unordered_map<std::string, uint32> _uniformBlockIndices;
  std::unordered_map<uint32, uint32> _uniformBindingPoints;
};
