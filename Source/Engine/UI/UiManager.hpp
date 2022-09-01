#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "../Rendering/Drawable.h"
#include "../Core/Types.hpp"
#include "../Core/Scene.h"

class GpuBuffer;
class IndexBuffer;
class PipelineState;
class RenderDevice;
class SamplerState;
class Texture;
class VertexBuffer;
struct ImDrawData;
struct ImGuiIO;
struct GLFWwindow;

class UiManager
{
public:
	static void AddTexture(uint64 id, const std::shared_ptr<Texture> &texture);

	UiManager(GLFWwindow *glfwWindow);
	~UiManager();

	bool HasMouseCapture() const;
	void Update(Scene &scene);

	void Initialize(std::shared_ptr<RenderDevice> renderDevice);

private:
	void Draw(ImDrawData *drawData);
	void SetupRenderer();
	void SetupFontAtlas();

	void DrawDrawables(const std::vector<Drawable> &drawables);

private:
	static std::unordered_map<uint64, std::shared_ptr<Texture>> TEXTURE_MAP;

	ImGuiIO *_io;
	GLFWwindow *_window;
	bool _initialized;

	uint64 _vertBuffSize;
	uint64 _idxBuffSize;

	std::shared_ptr<RenderDevice> _renderDevice;
	std::shared_ptr<PipelineState> _pipelineState;
	std::shared_ptr<Texture> _textureAtlas;
	std::shared_ptr<SamplerState> _samplerState;
	std::shared_ptr<SamplerState> _noMipSamplerState;
	std::shared_ptr<GpuBuffer> _constBuffer;
	std::shared_ptr<VertexBuffer> _vertBuffer;
	std::shared_ptr<IndexBuffer> _idxBuffer;
};