#pragma once
#include <memory>
#include <vector>
#include "../Core/Types.hpp"

class Actor;
class Camera;
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
struct SDL_Window;
typedef void* SDL_GLContext;
typedef union SDL_Event;

class DebugUi
{
public:
	DebugUi(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext);
	~DebugUi();
	
	void SetRenderer(const std::shared_ptr<Renderer>& renderer);

	bool ProcessEvents(SDL_Event* sdlEvent);
	void Update();

private:
	void Draw(ImDrawData* drawData);
	void SetupRenderer();
	void SetupFontAtlas();
  
  void AddChildNodes(const std::vector<std::shared_ptr<SceneNode>>& childNodes);
  void AddChildActors(const std::vector<std::shared_ptr<Actor>>& actors);

private:
	ImGuiIO* _io;
	SDL_Window* _sdlWindow;
	SDL_GLContext _sdlGlContext;
	bool _initialized;

	uint64 _vertBuffSize;
	uint64 _idxBuffSize;

	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<PipelineState> _pipelineState;
	std::shared_ptr<Texture> _textureAtlas;
	std::shared_ptr<SamplerState> _samplerState;
	std::shared_ptr<GpuBuffer> _constBuffer;
	std::shared_ptr<VertexBuffer> _vertBuffer;
	std::shared_ptr<IndexBuffer> _idxBuffer;
};
