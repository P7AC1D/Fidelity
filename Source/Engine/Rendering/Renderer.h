#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"
#include "../Components/PointLight.h"
#include "../Maths/Vector3.hpp"
#include "../SceneManagement/OrbitalCamera.h"
#include "../SceneManagement/Transform.h"
#include "Renderable.hpp"

namespace UI
{
class Panel;
class UiManager;
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

enum class ClearType
{
  Color,
  Depth,
  All,
};

struct RenderableItem
{
  RenderableItem(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform):
    Renderable(renderable),
    Transform(transform)
  {}

  std::shared_ptr<Renderable> Renderable;
  std::shared_ptr<Transform> Transform;
};

class Renderer
{
public:
  Renderer(int32 renderWidth, int32 renderHeight);
  virtual ~Renderer();

  void SetViewport(int32 renderWidth, int32 renderHeight);
  void SetClearColour(const Vector3& colour);

  void PushRenderable(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform);
  void PushPointLight(std::shared_ptr<Components::PointLight> pointLight);
  void DrawScene(OrbitalCamera& camera);

  void DrawUI(std::vector<std::shared_ptr<UI::Panel>> panelCollection);
  bool Initialize();

  static void SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride);

private:
  void UploadCameraData(OrbitalCamera& camera);
  void UploadLightData(std::shared_ptr<Components::PointLight>);

  void ClearBuffer(ClearType clearType);

private:
  std::vector<RenderableItem> _renderables;
  std::vector<std::shared_ptr<Components::PointLight>> _pointLights;
  
  std::unique_ptr<ShaderCollection> _shaderCollection;

  std::unique_ptr<ConstantBuffer> _cameraBuffer;
  std::unique_ptr<ConstantBuffer> _lightBuffer;
  std::unique_ptr<ConstantBuffer> _ambientLightBuffer;
  bool _projectionMatrixDirty;
  bool _viewMatrixDirty;

  std::unique_ptr<VertexBuffer> _guiQuadVertexData;
  std::unique_ptr<VertexBuffer> _skyBoxVertexData;

  int32 _renderWidth;
  int32 _renderHeight;
};
}