#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Rendering {
  class Shader;

  class ShaderCollection
  {
  public:
    ShaderCollection(std::string shaderDirectory);
    ~ShaderCollection();

    template <typename T>
    std::shared_ptr<T> GetShader();

  private:
    std::string _shaderDirectory;
    std::unordered_map<std::string, std::shared_ptr<Shader>> _shaderCollectionDeprecated;
    std::vector<std::shared_ptr<Shader>> _shaderCollection;
  };

  template <typename T>
  std::shared_ptr<T> ShaderCollection::GetShader()
  {
    static_assert(std::is_base_of<Shader, T>::value, "Must be a sub-class of Shader");
    auto iter = std::find_if(_shaderCollection.begin(), _shaderCollection.end(), [&](std::shared_ptr<Shader> shader)
    {
      return std::dynamic_pointer_cast<T>(shader) != nullptr;
    });

    if (iter == _shaderCollection.end())
    {
      try
      {
        auto shader = std::make_shared<T>(_shaderDirectory);
        _shaderCollection.push_back(shader);
        return shader;
      }
      catch (std::exception exception)
      {
        std::cerr << "Failed to load shader: " << exception.what();
        return nullptr;
      }
    }
    return std::dynamic_pointer_cast<T>(*iter);
  }
}