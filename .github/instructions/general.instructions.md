---
applyTo: "**/*.cpp **/*.h **/*.hpp **/*.c **/*.cc **/*.cxx **/*.inl"
---

## Project Context

This project is a C++ application focused on high-performance computing. It utilizes modern C++ standards (C++20 and above) and emphasizes clean, maintainable code. The project is structured to facilitate modular development and testing.

🔧 Core Practices
Modular Design: Split into clear modules (e.g., Renderer, Scene, Assets, Platform).
Smart Pointers & RAII: Use std::unique_ptr/shared_ptr to manage memory safely.
Consistent Naming: Use PascalCase for classes, camelCase for variables, and namespaces for grouping.
🧱 Design Principles
SOLID: Follow object-oriented best practices for clean, extensible code.
Component-Based Architecture: For scene objects (e.g., Transform, Mesh, Camera).
Data-Oriented Design: Optimize performance-critical paths using SoA and cache-friendly layouts.
🧪 Testing & Debugging
Unit Tests: Use Google Test or Catch2 for math and utility modules.
Debug Tools: Integrate RenderDoc, ImGui, and logging for runtime inspection.
📦 Build & Style
Cross-Platform: Use CMake and abstract platform-specific code.
Code Style: Enforce with clang-format and clang-tidy.
Documentation: Use Doxygen and maintain a dev wiki.

## 🧩 Rendering Engine Abstractions for Metal, Vulkan, and DX12

### 1. **Device / Adapter**

- **Purpose**: Interface to the GPU.
- **APIs**: `VkPhysicalDevice`, `MTLDevice`, `ID3D12Device`
- **Notes**: Metal is tightly integrated with Apple hardware; Vulkan and DX12 allow more control.

### 2. **Command Queue**

- **Purpose**: Submit work to the GPU.
- **APIs**: `VkQueue`, `MTLCommandQueue`, `ID3D12CommandQueue`
- **Notes**: Vulkan supports multiple queues with different capabilities.

### 3. **Command Buffer**

- **Purpose**: Record GPU commands.
- **APIs**: `VkCommandBuffer`, `MTLCommandBuffer`, `ID3D12CommandList`
- **Notes**: DX12 uses command lists and allocators; Metal is simpler but less flexible.

### 4. **Swapchain / Presentation**

- **Purpose**: Present rendered image to the screen.
- **APIs**: `VkSwapchainKHR`, `CAMetalLayer`, `IDXGISwapChain3`
- **Notes**: Metal uses layers; Vulkan and DX12 use explicit swapchains.

### 5. **Pipeline State**

- **Purpose**: Encapsulate shaders and fixed-function state.
- **APIs**: `VkPipeline`, `MTLRenderPipelineState`, `ID3D12PipelineState`
- **Notes**: Vulkan separates pipeline layout; Metal is more monolithic.

### 6. **Shader Modules**

- **Purpose**: GPU programs.
- **APIs**: `VkShaderModule`, `MTLLibrary`, `D3DCompile` / `DXC`
- **Notes**: Vulkan uses SPIR-V; Metal uses MSL; DX12 uses HLSL.

### 7. **Descriptor Sets / Heaps**

- **Purpose**: Bind resources to shaders.
- **APIs**: `VkDescriptorSet`, `MTLArgumentEncoder`, `ID3D12DescriptorHeap`
- **Notes**: Vulkan is explicit; Metal is more automatic; DX12 uses descriptor tables.

### 8. **Resource Management**

- **Purpose**: Buffers, textures, memory allocation.
- **APIs**: `VkBuffer`, `MTLBuffer`, `ID3D12Resource`
- **Notes**: Vulkan requires manual memory management; Metal and DX12 abstract more.

### 9. **Synchronization**

- **Purpose**: Ensure correct execution order.
- **APIs**: `VkFence`, `MTLEvent`, `ID3D12Fence`
- **Notes**: Vulkan is very explicit; Metal is implicit; DX12 is in between.

### 10. **Render Pass / Encoder**

- **Purpose**: Define rendering operations.
- **APIs**: `VkRenderPass`, `MTLRenderCommandEncoder`, `ID3D12GraphicsCommandList`
- **Notes**: Vulkan uses render passes; Metal uses encoders; DX12 uses command lists.

### 11. **Compute Support**

- **Purpose**: GPU compute operations.
- **APIs**: `VkComputePipeline`, `MTLComputePipelineState`, `ID3D12ComputePipelineState`
- **Notes**: All support compute shaders but differ in setup and dispatch.

### 12. **Debugging / Validation**

- **Purpose**: Development tools and diagnostics.
- **APIs**: Vulkan layers, Metal tools, DX12 debug layer
- **Notes**: Vulkan has extensive validation layers; Metal uses Xcode tools; DX12 has PIX.
