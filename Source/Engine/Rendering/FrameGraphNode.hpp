#pragma once
#include <memory>

class FrameGraphNodeInputs;
class Texture;

struct FrameGraphNodeInputs
{
	
};

class FrameGraphNode
{
public:
	virtual ~FrameGraphNode() {}
	virtual void Execute(const FrameGraphNodeInputs& inputs) = 0;
};

class GBufferNode : public FrameGraphNode
{
public:
	void Execute(const FrameGraphNodeInputs& inputs) override;

private:
	std::shared_ptr<Texture> _positions;
	std::shared_ptr<Texture> _normals;
	std::shared_ptr<Texture> _albedoSpec;
};