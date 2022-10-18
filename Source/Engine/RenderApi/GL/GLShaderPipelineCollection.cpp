#include "GLShaderPipelineCollection.hpp"

#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "GLShader.hpp"
#include "GLShaderPipeline.hpp"

const std::shared_ptr<GLShaderPipeline> &GLShaderPipelineCollection::getShaderPipeline(const std::shared_ptr<Shader> &vs,
                                                                                       const std::shared_ptr<Shader> &fs,
                                                                                       const std::shared_ptr<Shader> &gs,
                                                                                       const std::shared_ptr<Shader> &hs,
                                                                                       const std::shared_ptr<Shader> &ds)
{
  auto glVS = std::static_pointer_cast<GLShader>(vs);
  auto glFS = std::static_pointer_cast<GLShader>(fs);
  auto glGS = std::static_pointer_cast<GLShader>(gs);
  auto glHS = std::static_pointer_cast<GLShader>(hs);
  auto glDS = std::static_pointer_cast<GLShader>(ds);

  PipelineKey key;
  key.vsId = glVS ? glVS->GetId() : 0;
  key.fsId = glFS ? glFS->GetId() : 0;
  key.gsId = glGS ? glGS->GetId() : 0;
  key.hsId = glHS ? glHS->GetId() : 0;
  key.dsId = glDS ? glDS->GetId() : 0;

  auto iter = _shaderPipelines.find(key);
  if (iter == _shaderPipelines.end())
  {
    GLShaderPipelineDesc shaderPipelineDesc;
    shaderPipelineDesc.VertexShaderId = key.vsId;
    shaderPipelineDesc.fragmentShaderId = key.fsId;
    shaderPipelineDesc.GeometryShaderId = key.gsId;
    shaderPipelineDesc.HullShaderId = key.hsId;
    shaderPipelineDesc.DomainShaderId = key.dsId;
    std::shared_ptr<GLShaderPipeline> pipeline(new GLShaderPipeline(shaderPipelineDesc));

    _shaderPipelines[key] = pipeline;
    return _shaderPipelines[key];
  }
  return iter->second;
}

std::size_t GLShaderPipelineCollection::PipelineKeyHasher::operator()(const PipelineKey &key) const
{
  std::size_t seed = 0;
  Hash::combine(seed, key.vsId);
  Hash::combine(seed, key.fsId);
  Hash::combine(seed, key.gsId);
  Hash::combine(seed, key.hsId);
  Hash::combine(seed, key.dsId);
  return seed;
}

bool GLShaderPipelineCollection::PipelineKeyEquality::operator()(const PipelineKey &a, const PipelineKey &b) const
{
  return a.vsId == b.vsId && a.fsId == b.fsId && a.gsId == b.gsId && a.hsId == b.hsId && a.dsId == b.dsId;
}
