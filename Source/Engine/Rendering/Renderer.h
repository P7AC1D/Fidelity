#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"
#include "../Maths/Vector3.hpp"
#include "../SceneManagement/Light.h"
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
  class FrameBuffer;
  class Shader;
  class ShaderCollection;
  class StaticMesh;
  class Texture;
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

  inline void SetAmbientLight(const Colour& ambientLight) { _ambientLight = ambientLight; }
  inline void PushRenderable(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform) { _renderables.emplace_back(renderable, transform); }
  inline void PushPointLight(const Light& pointLight) { _pointLights.push_back(pointLight); }
  inline void PushDirectionalLight(const Light& directionalLight) { _directionalLights.push_back(directionalLight); }

  void SetClearColour(const Colour& colour);

  void DrawScene(OrbitalCamera& camera);

  void DrawUI(std::vector<std::shared_ptr<UI::Panel>> panelCollection);
  bool Initialize();

  static void SetVertexAttribPointers(StaticMesh* staticMesh, int32 stride);

private:
  void SetViewport(int32 renderWidth, int32 renderHeight);
  
  void UploadCameraData(OrbitalCamera& camera);
  void UploadPointLightData(const Light& pointLight);
  void UploadDirectionalLightData(const Light& directionalLight);

  void ExecuteGeometryPass();
  void ExecuteLightingPass(const Vector3& viewDirection);

  void DirLightColourPass(OrbitalCamera& camera, const Matrix4& lightSpaceTransform, std::shared_ptr<Texture> shadowMap, const Vector2& shadowTexelSize);
  void DirLightDepthPass(const Matrix4& lightSpaceTransform, uint32 shadowRes);
  void PointLightRender(OrbitalCamera& camera);

  void ClearBuffer(ClearType clearType);

private:
  std::vector<RenderableItem> _renderables;
  std::vector<Light> _pointLights;
  std::vector<Light> _directionalLights;
  
  std::unique_ptr<ShaderCollection> _shaderCollection;

  std::unique_ptr<ConstantBuffer> _cameraBuffer;
  std::unique_ptr<ConstantBuffer> _lightBuffer;
  std::unique_ptr<ConstantBuffer> _ambientLightBuffer;

  std::unique_ptr<VertexBuffer> _quadVertexData;
  std::unique_ptr<VertexBuffer> _skyBoxVertexData;
  
  std::unique_ptr<FrameBuffer> _gBuffer;
  std::unique_ptr<FrameBuffer> _depthBuffer;

  int32 _renderWidth;
  int32 _renderHeight;
  Colour _ambientLight;
};
}
