#include "GLShaderPipelineCollection.hpp"

#include "../../Utility/Assert.hpp"
#include "../../Utility/Hash.hpp"
#include "GLShader.hpp"
#include "GLShaderPipeline.hpp"

const std::shared_ptr<GLShaderPipeline>& GLShaderPipelineCollection::GetShaderPipeline(const std::shared_ptr<Shader>& vs,
                                                                                       const std::shared_ptr<Shader>& ps,
                                                                                       const std::shared_ptr<Shader>& gs,
                                                                                       const std::shared_ptr<Shader>& hs,
                                                                                       const std::shared_ptr<Shader>& ds)
{
  auto glVS = std::static_pointer_cast<GLShader>(vs);
  auto glPS = std::static_pointer_cast<GLShader>(ps);
  auto glGS = std::static_pointer_cast<GLShader>(gs);
  auto glHS = std::static_pointer_cast<GLShader>(hs);
  auto glDS = std::static_pointer_cast<GLShader>(ds);
  
  PipelineKey key;
  key.vsId = glVS ? glVS->GetId() : 0;
  key.psId = glPS ? glPS->GetId() : 0;
  key.gsId = glGS ? glGS->GetId() : 0;
  key.hsId = glHS ? glHS->GetId() : 0;
  key.dsId = glDS ? glDS->GetId() : 0;
  
  auto iter = _shaderPipelines.find(key);
  if (iter == _shaderPipelines.end())
  {
    std::shared_ptr<GLShaderPipeline> pipeline(new GLShaderPipeline({glVS->GetId(), glPS->GetId(), glGS->GetId(), glHS->GetId(), glDS->GetId()}));
    _shaderPipelines[key] = pipeline;
    return _shaderPipelines[key];
  }
  return iter->second;
}

std::size_t GLShaderPipelineCollection::PipelineKeyHasher::operator()(const PipelineKey& key) const
{
  std::size_t seed = 0;
  Hash::Combine(seed, key.vsId);
  Hash::Combine(seed, key.psId);
  Hash::Combine(seed, key.gsId);
  Hash::Combine(seed, key.hsId);
  Hash::Combine(seed, key.dsId);
  return seed;
}

bool GLShaderPipelineCollection::PipelineKeyEquality::operator()(const PipelineKey& a, const PipelineKey& b) const
{
  return a.vsId == b.vsId && a.psId == b.psId && a.gsId == b.gsId && a.hsId == b.hsId && a.dsId == b.dsId;
}
