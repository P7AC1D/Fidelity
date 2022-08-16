#pragma once
#include <memory>
#include <string>
#include <vector>
#include "../Core/Types.hpp"
#include "../Maths/Colour.hpp"
#include "../Maths/Matrix4.hpp"
#include "../Maths/Vector2I.hpp"
#include "../SceneManagement/CameraNode.hpp"
#include "../SceneManagement/LightNode.hpp"
#include "StaticMesh.h"

class Aabb;
class GpuBuffer;
class Material;
class PipelineState;
class SamplerState;
class RenderDevice;
class RenderQueue;
class RenderTarget;
class Renderable;
class Texture;
class Transform;
class VertexBuffer;

struct DirectionalLightData
{
	DirectionalLightData() : Colour(Colour::White),
													 Direction(-Vector3::Identity),
													 Intensity(1.0f)
	{
	}

	DirectionalLightData(const Colour &colour, const Vector3 &direction, float32 intensity) : Colour(colour),
																																														Direction(direction),
																																														Intensity(intensity)
	{
	}

	Colour Colour;
	Vector3 Direction;
	float32 Intensity;
};

struct AmbientLightData
{
	AmbientLightData() : Colour(Colour::White), Intensity(0.2f), SpecularExponent(10.0f), SsaoEnabled(1) {}
	AmbientLightData(const Colour &colour, float32 intensity) : Colour(colour), Intensity(intensity), SpecularExponent(10.0f), SsaoEnabled(1) {}

	Colour Colour;
	float32 Intensity;
	float32 SpecularExponent;
	int32 SsaoEnabled;
};

enum class RenderApi
{
	GL41
};

enum class DebugDisplayType
{
	Disabled,
	ShadowMap,
	Position,
	Normal,
	Albedo,
	Depth
};

// In nano-seconds
struct RenderTimings
{
	uint64 Frame = 0;
	uint64 Shadow = 0;
	uint64 GBuffer = 0;
	uint64 Lighting = 0;
	uint64 Ssao = 0;
};

struct RenderCounts
{
	uint64 TriangleCount = 0;
	uint64 MaterialCount = 0;
	uint64 DrawCount = 0;
};

struct RendererDesc
{
	uint32 RenderWidth;
	uint32 RenderHeight;
	RenderApi RenderApi = RenderApi::GL41;
	bool FullscreenEnabled = false;
	bool VsyncEnabled = false;
	Vector2I ShadowRes = Vector2I(2048, 2048);
};

class Renderer
{
public:
	static std::shared_ptr<RenderDevice> GetRenderDevice();

	Renderer(const RendererDesc &desc);
	virtual ~Renderer() = default;

	uint32 GetRenderWidth() const { return _desc.RenderWidth; }
	uint32 GetRenderHeight() const { return _desc.RenderHeight; }
	RenderTimings GetFrameRenderTimings() const { return _renderTimings; }
	RenderCounts GetFrameRenderCounts() const { return _renderCounts; }

	void SetDirectionalLight(const DirectionalLightData &directionalLightData) { _directionalLightData = directionalLightData; }
	void SetAmbientLight(const AmbientLightData &ambientLightData) { _ambientLightData = ambientLightData; }

	void EnableDebugPass(DebugDisplayType debugDisplayType) { _debugDisplayType = debugDisplayType; }

	void EnableSsao(bool enabled) { _ssaoEnabled = enabled; }
	void EnableHdr(bool enabled) { _hdrEnabled = enabled; }

	bool IsSsaoEnabled() const { return _ssaoEnabled; }
	bool IsHdrEnabled() const { return _hdrEnabled; }

	void Push(const std::shared_ptr<Renderable> &renderable, const std::shared_ptr<Transform> &transform, const Aabb &bounds);

	void DrawFrame();

	void BindCamera(std::shared_ptr<CameraNode> camera);
	sptr<CameraNode> GetBoundCamera() const { return _camera; }

	void SubmitLight(const sptr<LightNode> &lightNode);
	void SubmitRenderable(const sptr<Renderable> &renderable);

	float32 GetOrthographicSize() const;
	void SetOrthographicSize(float32 shadowOrthographicSize);

private:
	void InitShadowDepthPass();
	void InitDepthRenderTarget();
	void InitDepthBuffer();
	void InitGeometryPass();
	void InitFrameBuffer();
	void InitMaterialBuffer();
	void InitLightingPass();
	void InitSsaoPass();
	void InitSsaoBlurPass();
	void InitFullscreenQuad();
	void InitPointLightMesh();
	void InitDepthDebugPass();
	void InitGBufferDebugPass();

	void GenerateSsaoKernel();

	void StartFrame();
	void EndFrame();

	void ShadowDepthPass();
	void GeometryPass();
	void LightingPass();
	void SsaoPass();
	void SsaoBlurPass();

	void DepthDebugPass(const std::shared_ptr<Texture> &depthTexture);
	void GBufferDebugPass(uint32 i);

	void SetMaterialData(const std::shared_ptr<Material> &material);

private:
	static std::shared_ptr<RenderDevice> _renderDevice;
	static const uint32 MaxKernelSize = 64;

	struct ShadowBufferData
	{
		Matrix4 Projection;
		Matrix4 View;
		Vector2 TexelDims;
	};

	struct SsaoDetailsData
	{
		Vector4 Samples[MaxKernelSize];
		int32 Enabled = 1;
		int32 KernelSize = 64;
		int32 QuadWidth;
		int32 QuadHeight;
		float32 Radius = 0.5f;
		float32 Bias = 0.025f;
		float32 _paddingA;
		float32 _paddingB;
		float32 _paddingC;
	} _ssaoDetailsData;

	struct HdrData
	{
		int Enabled = 1;
		float32 Exposure = 1.0f;
	} _hdrData;

	struct FrameBufferData
	{
		Matrix4 Proj;
		Matrix4 View;
		Matrix4 ViewInvs;
		DirectionalLightData DirectionalLight;
		Vector4 ViewPosition;
		AmbientLightData AmbientLight;
		SsaoDetailsData SsaoDetails;
		HdrData Hdr;
	};

public:
	[[nodiscard]] float32 ShadowOrthographicSize() const
	{
		return _shadowOrthographicSize;
	}

private:
	RendererDesc _desc;
	DebugDisplayType _debugDisplayType;
	DirectionalLightData _directionalLightData;
	AmbientLightData _ambientLightData;

	std::shared_ptr<CameraNode> _camera;

	std::shared_ptr<GpuBuffer> _frameBuffer;
	std::shared_ptr<GpuBuffer> _materialBuffer;
	std::shared_ptr<GpuBuffer> _depthBuffer;

	std::shared_ptr<PipelineState> _shadowDepthPso;
	std::shared_ptr<PipelineState> _geomPassPso;
	std::shared_ptr<PipelineState> _lightPassPso;
	std::shared_ptr<PipelineState> _ssaoPassPso;
	std::shared_ptr<PipelineState> _ssaoBlurPassPso;
	std::shared_ptr<PipelineState> _gBufferDebugPso;
	std::shared_ptr<PipelineState> _depthDebugPso;

	std::shared_ptr<SamplerState> _basicSamplerState;
	std::shared_ptr<SamplerState> _noMipSamplerState;
	std::shared_ptr<SamplerState> _ssaoSamplerState;

	std::shared_ptr<RenderTarget> _depthRenderTarget;
	std::shared_ptr<RenderTarget> _gBuffer;
	std::shared_ptr<RenderTarget> _ssaoRT;
	std::shared_ptr<RenderTarget> _ssaoBlurRT;

	std::shared_ptr<VertexBuffer> _fsQuadBuffer;
	std::shared_ptr<Material> _activeMaterial;
	std::shared_ptr<RenderQueue> _opaqueQueue;
	std::shared_ptr<Texture> _ssaoNoiseTexture;

	std::shared_ptr<StaticMesh> _pointLightMesh;

	std::vector<Vector3> _ssaoKernel;
	std::vector<Vector3> _ssaoNoise;

	RenderTimings _renderTimings;
	RenderCounts _renderCounts;

	bool _ssaoEnabled;
	bool _hdrEnabled;

	uint32 _shadowResolution;
	float32 _shadowOrthographicSize;

	std::vector<sptr<Renderable>> _renderables;
	std::vector<sptr<LightNode>> _pointLights;

	// temp
	std::shared_ptr<GpuBuffer> _lightObjectBuffer;
};
