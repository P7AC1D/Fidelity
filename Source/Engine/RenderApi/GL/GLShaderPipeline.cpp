#include "GLShaderPipeline.hpp"

#include "../../Utility/Assert.hpp"
#include "GL.hpp"

GLShaderPipeline::~GLShaderPipeline()
{
  if (_id)
  {
    glCall(glDeleteProgramPipelines(1, &_id));
  }
}

GLShaderPipeline::GLShaderPipeline(const GLShaderPipelineDesc &desc) : _desc(desc), _initialized(false), _id(0)
{
  initialize(desc);
}

void GLShaderPipeline::initialize(const GLShaderPipelineDesc &desc)
{
  if (_initialized)
  {
    return;
  }

  glCall(glGenProgramPipelines(1, &_id));
  ASSERT_FALSE(_id == 0, "Unable to create GLSL shader pipeline object");

  glCall(glUseProgramStages(_id, GL_VERTEX_SHADER_BIT, desc.VertexShaderId));
  glCall(glUseProgramStages(_id, GL_TESS_CONTROL_SHADER_BIT, desc.HullShaderId));
  glCall(glUseProgramStages(_id, GL_TESS_EVALUATION_SHADER_BIT, desc.DomainShaderId));
  glCall(glUseProgramStages(_id, GL_GEOMETRY_SHADER_BIT, desc.GeometryShaderId));
  glCall(glUseProgramStages(_id, GL_FRAGMENT_SHADER_BIT, desc.fragmentShaderId));
  _initialized = true;
}
