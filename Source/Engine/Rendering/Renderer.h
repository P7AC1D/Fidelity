#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Vector3.hpp"

namespace Components
{
class PointLight;
}

namespace UI
{
class Panel;
class UiManager;
}

namespace SceneManagement
{
class OrbitalCamera;
class Scene;
class WorldObject;
}

namespace Rendering
{
class ConstantBuffer;
class CubeMap;
class Shader;
class ShaderCollection;
class StaticMesh;
class VertexBuffer;
enum class RenderingTechnique;

typedef std::vector<std::shared_ptr<SceneManagement::WorldObject>> ObjectPtrArray;

enum class ClearType
{
  Color,
  Depth,
  All,
};

class Renderer
{
public:
  Renderer(int32 renderWidth, int32 renderHeight);
  virtual ~Renderer();

  void SetViewport(int32 renderWidth, int32 renderHeight);
  void SetClearColour(const Vector3& colour);

  void PreRender();
  void DrawScene(std::shared_ptr<SceneManagement::Scene> scene);
  void DrawUI(std::vector<std::shared_ptr<UI::Panel>> panelCollection);
  bool Initialize();

  static void SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride);

private:
  void DrawSkyBox(std::shared_ptr<SceneManagement::Scene> scene);
  void DrawTexturedObjects(ObjectPtrArray objects, const Vector3& ambientColour);
  void UploadCameraData(std::shared_ptr<SceneManagement::OrbitalCamera> camera);
  void UploadLightData(std::shared_ptr<SceneManagement::WorldObject> lightObject);

  void ClearBuffer(ClearType clearType);
  
  std::unique_ptr<ShaderCollection> _shaderCollection;

  std::unique_ptr<ConstantBuffer> _cameraBuffer;
  std::unique_ptr<ConstantBuffer> _lightBuffer;
  std::shared_ptr<SceneManagement::OrbitalCamera> _activeCamera;
  bool _projectionMatrixDirty;
  bool _viewMatrixDirty;

  std::unique_ptr<VertexBuffer> _guiQuadVertexData;
  std::unique_ptr<VertexBuffer> _skyBoxVertexData;

  int32 _renderWidth;
  int32 _renderHeight;
};
}