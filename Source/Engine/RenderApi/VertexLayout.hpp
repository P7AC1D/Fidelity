#pragma once
#include <string>
#include <vector>
#include "../Core/Types.hpp"

enum class SemanticFormat
{
  Unknown,
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

enum class InputClassification
{
  PerVertexData,
  PerInstanceData
};

struct VertexLayoutDesc
{
  std::string SemanticName;
  uint32 SemanticIndex;
  SemanticFormat Format;
  uint32 InputSlot;
  uint32 AlignedByteOffset;
  InputClassification InputSlotClass;
  uint32 InstanceDataStepRate;
};

class VertexLayout
{
public:
  const std::vector<VertexLayoutDesc>& GetDesc() const { return _desc; }

private:
  std::vector<VertexLayoutDesc> _desc;
};