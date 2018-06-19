#pragma once
#include <memory>
#include "../Core/Types.hpp"

class Camera;
class GpuBuffer;
class IndexBuffer;
class PipelineState;
class SamplerState;
class SceneManager;
class Texture;
class VertexBuffer;
struct ImDrawData;
struct ImGuiIO;
struct SDL_Window;
typedef void* SDL_GLContext;
typedef union SDL_Event;

class DebugUi
{
public:
	DebugUi(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext);
	~DebugUi();

  void SetSceneManager(const std::shared_ptr<SceneManager>& sceneManager);

	void ProcessEvents(SDL_Event* sdlEvent);
	void Update();

private:
	void Draw(ImDrawData* drawData);
	void SetupRenderer();
	void SetupFontAtlas();

private:
	ImGuiIO* _io;
	SDL_Window* _sdlWindow;
	SDL_GLContext _sdlGlContext;
	bool _initialized;

	uint64 _vertBuffSize;
	uint64 _idxBuffSize;

	std::shared_ptr<PipelineState> _pipelineState;
	std::shared_ptr<Texture> _textureAtlas;
	std::shared_ptr<SamplerState> _samplerState;
	std::shared_ptr<SceneManager> _sceneManager;
	std::shared_ptr<GpuBuffer> _constBuffer;
	std::shared_ptr<VertexBuffer> _vertBuffer;
	std::shared_ptr<IndexBuffer> _idxBuffer;
};