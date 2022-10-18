#pragma once
#include "../../Core/Types.hpp"

struct GLShaderPipelineDesc
{
  uint32 VertexShaderId;
  uint32 fragmentShaderId;
  uint32 GeometryShaderId = 0;
  uint32 HullShaderId = 0;
  uint32 DomainShaderId = 0;
};

class GLShaderPipeline
{
  friend class GLShaderPipelineCollection;

public:
  ~GLShaderPipeline();

  uint32 getId() const { return _id; }

private:
  GLShaderPipeline(const GLShaderPipelineDesc &desc);

  void initialize(const GLShaderPipelineDesc &desc);

private:
  GLShaderPipelineDesc _desc;
  bool _initialized;
  uint32 _id;
};
