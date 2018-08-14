#pragma once
#include <memory>
#include <unordered_map>
#include "../../Core/Types.hpp"
#include "../Shader.hpp"

class GLShaderPipeline;

class GLShaderPipelineCollection
{
public:
  const std::shared_ptr<GLShaderPipeline>& GetShaderPipeline(const std::shared_ptr<Shader>& vs,
                                                             const std::shared_ptr<Shader>& ps,
                                                             const std::shared_ptr<Shader>& gs,
                                                             const std::shared_ptr<Shader>& hs,
                                                             const std::shared_ptr<Shader>& ds);
  
private:
  struct PipelineKey
  {
    uint32 vsId;
    uint32 psId;
    uint32 gsId;
    uint32 hsId;
    uint32 dsId;
  };
  
  class PipelineKeyHasher
  {
  public:
    std::size_t operator()(const PipelineKey& key) const;
  };
  
  class PipelineKeyEquality
  {
  public:
    bool operator()(const PipelineKey& a, const PipelineKey& b) const;
  };
  
  std::unordered_map<PipelineKey, std::shared_ptr<GLShaderPipeline>, PipelineKeyHasher, PipelineKeyEquality> _shaderPipelines;
};
