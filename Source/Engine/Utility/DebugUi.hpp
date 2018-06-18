#pragma once
#include <memory>

class Camera;
class GpuBuffer;
class PipelineState;
class Texture;
struct ImDrawData;
struct ImGuiIO;
struct SDL_Window;
typedef void* SDL_GLContext;
typedef union SDL_Event;

class DebugUi
{
public:
	DebugUi(SDL_Window* sdlWindow, SDL_GLContext sdlGlContext);

	void SetCamera(const std::shared_ptr<Camera>& camera);

	void ProcessEvents(SDL_Event* sdlEvent);
	void Update();

private:
	void Draw(ImDrawData* drawData);
	void SetupRenderer();
	void SetupFontAtlas();

private:
	ImGuiIO* _io;
	SDL_Window* _sdlWindow;
	bool _initialized;

	std::shared_ptr<Camera> _camera;
	std::shared_ptr<PipelineState> _pipelineState;
	std::shared_ptr<Texture> _textureAtlas;
	std::shared_ptr<GpuBuffer> _constBuffer;
};