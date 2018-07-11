#include "GLShaderPipeline.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

GLShaderPipeline::~GLShaderPipeline()
{
  if (_id)
  {
    GLCall(glDeleteProgramPipelines(1, &_id));
  }
}

GLShaderPipeline::GLShaderPipeline(const GLShaderPipelineDesc& desc): _desc(desc), _initialized(false), _id(0)
{
  Initialize(desc);
}

void GLShaderPipeline::Initialize(const GLShaderPipelineDesc& desc)
{
  if (_initialized)
  {
    return;
  }
  
  GLCall(glGenProgramPipelines(1, &_id));
  ASSERT_FALSE(_id == 0, "Unable to create GLSL shader pipeline object");

  GLCall(glUseProgramStages(_id, GL_VERTEX_SHADER_BIT, desc.VertexShaderId));
  GLCall(glUseProgramStages(_id, GL_TESS_CONTROL_SHADER_BIT, desc.HullShaderId));
  GLCall(glUseProgramStages(_id, GL_TESS_EVALUATION_SHADER_BIT, desc.DomainShaderId));
  GLCall(glUseProgramStages(_id, GL_GEOMETRY_SHADER_BIT, desc.GeometryShaderId));
  GLCall(glUseProgramStages(_id, GL_FRAGMENT_SHADER_BIT, desc.PixelShaderId));
  _initialized = true;
}
