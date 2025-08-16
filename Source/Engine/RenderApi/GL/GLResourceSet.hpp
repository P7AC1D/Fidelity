#pragma once
#include "RenderApi/ResourceSet.hpp"
#include "Core/Types.hpp"
#include <vector>
#include <memory>

// Forward declarations
class RenderDevice;
class Texture;
class GpuBuffer;
class SamplerState;
class ImageView;

/**
 * @brief OpenGL 4.1 implementation of IResourceSetLayout
 *
 * Since OpenGL doesn't have descriptor set layouts, this class primarily
 * validates binding configurations and stores layout information for
 * compatibility with the interface.
 */
class GLResourceSetLayout : public IResourceSetLayout
{
public:
  GLResourceSetLayout() = default;
  ~GLResourceSetLayout() override = default;

  void addBinding(uint32 binding, ResourceType type, uint32 count = 1) override;
  bool build(const sptr<RenderDevice> &renderDevice) override;
  bool isBuilt() const override { return mIsBuilt; }

private:
  struct BindingInfo
  {
    uint32 binding;
    ResourceType type;
    uint32 count;
  };

  std::vector<BindingInfo> mBindings;
  bool mIsBuilt = false;
};

/**
 * @brief OpenGL 4.1 implementation of IResourceSet
 *
 * This implementation stores resource bindings and applies them individually
 * when bind() is called, since OpenGL doesn't support descriptor sets.
 * Resources are bound to their respective OpenGL binding points.
 */
class GLResourceSet : public IResourceSet
{
public:
  explicit GLResourceSet(const std::unique_ptr<IResourceSetLayout> &layout);
  ~GLResourceSet() override = default;

  void addTexture(uint32 binding, const sptr<Texture> &texture) override;
  void addUniformBuffer(uint32 binding, const sptr<GpuBuffer> &buffer) override;
  void addStorageBuffer(uint32 binding, const sptr<GpuBuffer> &buffer) override;
  void addSampler(uint32 binding, const sptr<SamplerState> &sampler) override;
  void addTextureArray(uint32 binding, const std::vector<sptr<Texture>> &textures) override;
  void addImageView(uint32 binding, const sptr<ImageView> &view) override;

  bool build(const sptr<RenderDevice> &renderDevice) override;
  void bind(const sptr<RenderDevice> &renderDevice, uint32 setIndex = 0) const override;
  bool isBuilt() const override { return mIsBuilt; }
  void reset() override;

private:
  struct GLResourceBinding
  {
    ResourceType type;
    uint32 binding;
    std::vector<void *> resources; // Vector to handle arrays
    uint32 arraySize;
  };

public:
  // OpenGL-specific methods
  const std::vector<GLResourceBinding> &getBindings() const { return mBindings; }

  void bindTexture(uint32 binding, const std::vector<void *> &textures) const;
  void bindUniformBuffer(uint32 binding, void *buffer) const;
  void bindSampler(uint32 binding, void *sampler) const;

  bool validateBinding(uint32 binding, ResourceType type, uint32 count) const;

  const IResourceSetLayout *mLayout;
  std::vector<GLResourceBinding> mBindings;
  bool mIsBuilt = false;
};

/**
 * @brief OpenGL 4.1 implementation of IResourceSetFactory
 */
class GLResourceSetFactory : public IResourceSetFactory
{
public:
  GLResourceSetFactory() = default;
  ~GLResourceSetFactory() override = default;

  std::unique_ptr<IResourceSetLayout> createLayout() override;
  std::unique_ptr<IResourceSet> createResourceSet(const std::unique_ptr<IResourceSetLayout> &layout) override;
};
