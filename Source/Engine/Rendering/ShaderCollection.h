#pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace Rendering
{
class Shader;

class ShaderCollection
{
public:
  ShaderCollection(std::string shaderDirectory);
  ~ShaderCollection();

  std::shared_ptr<Shader> GetShader(const std::string& shaderName);
  bool HasShader(const std::string& shaderName) const;

private:
  std::shared_ptr<Shader> LoadShader(const std::string& shaderName);

  std::string _shaderDirectory;
  std::unordered_map<std::string, std::shared_ptr<Shader>> _shaderCollection;
};
}