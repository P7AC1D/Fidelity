#pragma once
#include "../Core/Types.hpp"

// Explicit resource states and pipeline/memory flags (cross-API model)

// Resource (image/buffer) states used for barriers/transitions
enum class ResourceState : uint32
{
  Undefined = 0,
  General,
  Sampled,
  Storage,
  ColorAttachment,
  DepthRead,
  DepthWrite,
  TransferSrc,
  TransferDst,
  Present,
};

// Coarse pipeline stage flags (bitmask); kept simple for portability
namespace PipelineStage
{
  enum Bits : uint32
  {
    None = 0,
    TopOfPipe = 1u << 0,
    DrawIndirect = 1u << 1,
    VertexShader = 1u << 2,
    FragmentShader = 1u << 3,
    EarlyFragmentTests = 1u << 4,
    LateFragmentTests = 1u << 5,
    ColorAttachmentOutput = 1u << 6,
    Transfer = 1u << 7,
    ComputeShader = 1u << 8,
    BottomOfPipe = 1u << 9,
    AllGraphics = 1u << 10,
    AllCommands = 1u << 11,
  };
}

// Coarse memory dependency flags (bitmask) to model read/write hazards
namespace MemoryDeps
{
  enum Bits : uint32
  {
    None = 0,
    ShaderRead = 1u << 0,
    ShaderWrite = 1u << 1,
    UniformRead = 1u << 2,
    StorageRead = 1u << 3,
    StorageWrite = 1u << 4,
    ColorAttachmentRead = 1u << 5,
    ColorAttachmentWrite = 1u << 6,
    DepthStencilRead = 1u << 7,
    DepthStencilWrite = 1u << 8,
    TransferRead = 1u << 9,
    TransferWrite = 1u << 10,
    Host = 1u << 11,
  };
}
