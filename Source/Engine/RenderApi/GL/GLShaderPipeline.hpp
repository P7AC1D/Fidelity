#pragma once
#include "../../Core/Types.hpp"

struct GLShaderPipelineDesc
{
  uint32 VertexShaderId;
  uint32 PixelShaderId;
  uint32 GeometryShaderId = 0;
  uint32 HullShaderId = 0;
  uint32 DomainShaderId = 0;
};

class GLShaderPipeline
{
  friend class GLShaderPipelineCollection;
  
public:
  ~GLShaderPipeline();
  
  uint32 GetId() const { return _id; }
  
private:
  GLShaderPipeline(const GLShaderPipelineDesc& desc);
  
  void Initialize(const GLShaderPipelineDesc& desc);
  
private:
  GLShaderPipelineDesc _desc;
  bool _initialized;
  uint32 _id;
};
