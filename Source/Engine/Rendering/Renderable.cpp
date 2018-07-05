#include "Renderable.hpp"

#include "StaticMesh.h"

void Renderable::Update()
{
}

void Renderable::SetMesh(const std::shared_ptr<StaticMesh>& mesh)
{
	_mesh = mesh;
}

std::shared_ptr<StaticMesh> Renderable::GetMesh() const
{
	return _mesh;
}