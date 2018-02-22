#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Core/System.hpp"
#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"
#include "../Maths/Vector3.hpp"
#include "../SceneManagement/Light.h"
#include "../SceneManagement/OrbitalCamera.h"
#include "../SceneManagement/Transform.h"
#include "Renderable.hpp"

class SkyBox;
class TextOverlay;

namespace Rendering
{
  class ConstantBuffer;
  class RenderTarget;
  class Shader;
  class ShaderCollection;
  class StaticMesh;
  class Texture;
  class VertexBuffer;
  enum class RenderingTechnique;

enum ClearType : uint8
{
  CT_Colour = 1 << 0,
  CT_Depth = 1 << 1,
  CT_Stencil = 1 << 2
};

enum class BlendType
{
  SrcAlpha,
  OneMinusSrcAlpha
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

class Renderer : public System<Renderer>
{
public:
  ~Renderer();

  inline void SetAmbientLight(const Colour& ambientLight) { _ambientLight = ambientLight; }
  inline void SetSkyBox(std::shared_ptr<SkyBox> skyBox) { _skyBox = skyBox; }
  inline void PushRenderable(std::shared_ptr<Renderable> renderable, std::shared_ptr<Transform> transform) { _renderables.emplace_back(renderable, transform); }
  inline void PushPointLight(const Light& pointLight) { _pointLights.push_back(pointLight); }
  inline void PushDirectionalLight(const Light& directionalLight) { _directionalLights.push_back(directionalLight); }
  inline void PushTextOverlay(const std::shared_ptr<TextOverlay>& textOverlay) { _textOverlays.push_back(textOverlay); }

  inline uint32 GetWidth() const { return _renderWidth; }
  inline uint32 GetHeight() const { return _renderHeight; }

  void SetClearColour(const Colour& colour);

  void DrawScene(OrbitalCamera& camera);
  void DrawOverlay();

  static void Draw(uint32 vertexCount, uint32 vertexOffset = 0);
  static void DrawIndexed(uint32 indexCount);

  bool Initialize();
  
  void SetRenderDimensions(uint32 width, uint32 height);

  void EnableBlend(BlendType source, BlendType destination);
  void DisableBlend();

private:
  void SetViewport(uint32 renderWidth, uint32 renderHeight);  
  Renderer();

  void UploadCameraData(OrbitalCamera& camera);
  void UploadPointLightData(const Light& pointLight);
  void UploadDirectionalLightData(const Light& directionalLight);

  void ExecuteDirectionalLightDepthPass(const Matrix4& lightSpaceTransform, uint32 shadowResolution);
  void ExecuteGeometryPass(const Vector3& viewDirection);
  void ExecuteLightingPass(const Matrix4& lightSpaceTransform, const Vector3& viewDirection);
  void DrawSkyBox();

  void ClearBuffer(uint32 clearType);
  void EnableDepthTest();
  void DisableDepthTest();
  void EnableStencilTest();
  void DisableStencilTest();

  Matrix4 BuildLightSpaceTransform(const Light& directionalLight);

private:
  std::vector<RenderableItem> _renderables;
  std::vector<Light> _pointLights;
  std::vector<Light> _directionalLights;
  std::vector<std::shared_ptr<TextOverlay>> _textOverlays;
  
  std::unique_ptr<ShaderCollection> _shaderCollection;

  std::shared_ptr<ConstantBuffer> _cameraBuffer;
  std::unique_ptr<ConstantBuffer> _lightBuffer;
  std::unique_ptr<ConstantBuffer> _ambientLightBuffer;

  std::unique_ptr<VertexBuffer> _quadVertexData;
  std::unique_ptr<VertexBuffer> _skyBoxVertexData;
  
  std::shared_ptr<RenderTarget> _gBuffer;
  std::shared_ptr<RenderTarget> _depthBuffer;

  std::shared_ptr<SkyBox> _skyBox;

  uint32 _renderWidth;
  uint32 _renderHeight;
  uint32 _shadowResolution = 4096;
  Colour _ambientLight;

  friend class System<Renderer>;
};
}
