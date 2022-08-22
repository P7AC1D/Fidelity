#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include "../Core/Types.hpp"

class Actor;
class GpuBuffer;
class IndexBuffer;
class PipelineState;
class Renderer;
class SamplerState;
class SceneNode;
class Texture;
class VertexBuffer;
struct ImDrawData;
struct ImGuiIO;
struct GLFWwindow;

class UiManager
{
public:
	static void AddTexture(uint64 id, const std::shared_ptr<Texture>& texture);

	UiManager(GLFWwindow* glfwWindow);
	~UiManager();
	
	void SetRenderer(const std::shared_ptr<Renderer>& renderer);
	void SetSceneGraph(const std::shared_ptr<SceneNode>& sceneGraph);

	bool HasMouseCapture() const;
	void Update();

private:
	void Draw(ImDrawData* drawData);
	void SetupRenderer();
	void SetupFontAtlas();
  
  void AddChildNodes(const std::vector<std::shared_ptr<SceneNode>>& childNodes);
  void AddChildActors(const std::vector<std::shared_ptr<Actor>>& actors);
	void DrawSceneNode(const sptr<SceneNode>& node);

private:
	static std::unordered_map<uint64, std::shared_ptr<Texture>> TEXTURE_MAP;

	ImGuiIO* _io;
	GLFWwindow* _window;
	bool _initialized;

	uint64 _vertBuffSize;
	uint64 _idxBuffSize;

	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<SceneNode> _sceneGraph;
	std::shared_ptr<PipelineState> _pipelineState;
	std::shared_ptr<Texture> _textureAtlas;
	std::shared_ptr<SamplerState> _samplerState;
  std::shared_ptr<SamplerState> _noMipSamplerState;
	std::shared_ptr<GpuBuffer> _constBuffer;
	std::shared_ptr<VertexBuffer> _vertBuffer;
	std::shared_ptr<IndexBuffer> _idxBuffer;
};