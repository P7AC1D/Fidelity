#pragma once
#include <string>
#include <vector>
#include "../Core/Types.hpp"

enum class SemanticType
{
  Position,
  Normal,
  Tangent,
  Bitangent,
  TexCoord
};

enum class SemanticFormat
{
  Float4,
  Float3,
  Float2,
  Float,
  Uint4,
  Uint3,
  Uint2,
  Uint,
  Int4,
  Int3,
  Int2,
  Int
};

struct VertexLayoutDesc
{
  SemanticType Type;
  SemanticFormat Format;
  uint32 Offset;
};

class VertexLayout
{
  friend class RenderDevice;
  
public:
  const std::vector<VertexLayoutDesc>& GetDesc() const { return _desc; }
  
protected:
  VertexLayout(const std::vector<VertexLayoutDesc>& desc): _desc(desc) {}

protected:
  std::vector<VertexLayoutDesc> _desc;
};
