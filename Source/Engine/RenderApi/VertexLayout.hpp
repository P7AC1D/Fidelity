#pragma once
#include <string>
#include <vector>
#include "../Core/Types.hpp"

enum class SemanticType
{
  Position = 0,
  Normal = 1,
  TexCoord = 2,
  Tangent = 3,
  Bitangent = 4,
  Colour = 5
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
  Int,
  Byte4,
  Byte3,
  Byte2,
  Byte,
  Ubyte4,
  Ubyte3,
  Ubyte2,
  Ubyte
};

struct VertexLayoutDesc
{
  VertexLayoutDesc(SemanticType type, SemanticFormat format, bool normalized = false) : Type(type),
                                                                                        Format(format),
                                                                                        Normalised(normalized)
  {
  }

  SemanticType Type;
  SemanticFormat Format;
  bool Normalised;
};

class VertexLayout
{
  friend class RenderDevice;

public:
  const std::vector<VertexLayoutDesc> &getDesc() const { return _desc; }

protected:
  VertexLayout(const std::vector<VertexLayoutDesc> &desc) : _desc(desc) {}

protected:
  std::vector<VertexLayoutDesc> _desc;
};
