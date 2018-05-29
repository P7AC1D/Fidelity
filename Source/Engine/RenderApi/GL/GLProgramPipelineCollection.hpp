#pragma once
#include <memory>
#include <unordered_map>
#include "../../Core/Types.hpp"

class GLShader;

struct GLProgramPipeline
{
  uint32 Id = 0;
};

class GLShaderPipelineCollection
{
public:
  ~GLShaderPipelineCollection();
  
  const std::shared_ptr<GLProgramPipeline>& GetShaderPipeline(const std::shared_ptr<GLShader>& vs,
                                                               const std::shared_ptr<GLShader>& ps,
                                                               const std::shared_ptr<GLShader>& gs,
                                                               const std::shared_ptr<GLShader>& hs,
                                                               const std::shared_ptr<GLShader>& ds);
  
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
  
  std::unordered_map<PipelineKey, std::shared_ptr<GLProgramPipeline>, PipelineKeyHasher, PipelineKeyEquality> _programPipelines;
};
