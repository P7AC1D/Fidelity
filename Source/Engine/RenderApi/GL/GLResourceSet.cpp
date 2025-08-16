#include "GLResourceSet.hpp"
#include "RenderApi/RenderDevice.hpp"
#include "RenderApi/Texture.hpp"
#include "RenderApi/ImageView.hpp"
#include "RenderApi/GpuBuffer.hpp"
#include "RenderApi/SamplerState.hpp"
#include "GLCommon.hpp"
#include <algorithm>
#include <cassert>

// GLResourceSetLayout Implementation
void GLResourceSetLayout::addBinding(uint32 binding, ResourceType type, uint32 count)
{
  assert(!mIsBuilt && "Cannot add bindings after layout is built");

  // Check for duplicate bindings of the same type at the same slot
  // Different resource types can share the same binding slot (e.g., texture and sampler at binding 0)
  auto it = std::find_if(mBindings.begin(), mBindings.end(),
                         [binding, type](const BindingInfo &info)
                         { return info.binding == binding && info.type == type; });

  assert(it == mBindings.end() && "Binding slot already used for this resource type");

  mBindings.push_back({binding, type, count});
}

bool GLResourceSetLayout::build(const sptr<RenderDevice> &renderDevice)
{
  if (mIsBuilt)
    return false;

  // OpenGL doesn't need to build layouts, just validate them
  mIsBuilt = true;
  return true;
}

// GLResourceSet Implementation
GLResourceSet::GLResourceSet(const std::unique_ptr<IResourceSetLayout> &layout)
    : mLayout(layout.get())
{
  assert(layout && layout->isBuilt() && "Layout must be built before creating resource set");
}

void GLResourceSet::addTexture(uint32 binding, const sptr<Texture> &texture)
{
  assert(!mIsBuilt && "Cannot add resources after resource set is built");
  assert(validateBinding(binding, ResourceType::TEXTURE_2D, 1) && "Invalid binding for texture");

  mBindings.push_back({ResourceType::TEXTURE_2D,
                       binding,
                       {texture.get()},
                       1});
}

void GLResourceSet::addImageView(uint32 binding, const sptr<ImageView> &view)
{
  assert(!mIsBuilt && "Cannot add resources after resource set is built");
  // Treat as a texture binding in GL; aspect/range not supported at bind time.
  assert(validateBinding(binding, ResourceType::TEXTURE_2D, 1) && "Invalid binding for image view");

  // Store the ImageView pointer to keep API semantics; bind step unwraps handle
  mBindings.push_back({ResourceType::TEXTURE_2D,
                       binding,
                       {const_cast<ImageView *>(view.get())},
                       1});
}

void GLResourceSet::addUniformBuffer(uint32 binding, const sptr<GpuBuffer> &buffer)
{
  assert(!mIsBuilt && "Cannot add resources after resource set is built");
  assert(validateBinding(binding, ResourceType::UNIFORM_BUFFER, 1) && "Invalid binding for uniform buffer");

  mBindings.push_back({ResourceType::UNIFORM_BUFFER,
                       binding,
                       {buffer.get()},
                       1});
}

void GLResourceSet::addStorageBuffer(uint32 binding, const sptr<GpuBuffer> &buffer)
{
  throw std::runtime_error("Storage buffers are not supported in OpenGL 4.1");
}

void GLResourceSet::addSampler(uint32 binding, const sptr<SamplerState> &sampler)
{
  assert(!mIsBuilt && "Cannot add resources after resource set is built");
  assert(validateBinding(binding, ResourceType::SAMPLER, 1) && "Invalid binding for sampler");

  mBindings.push_back({ResourceType::SAMPLER,
                       binding,
                       {sampler.get()},
                       1});
}

void GLResourceSet::addTextureArray(uint32 binding, const std::vector<sptr<Texture>> &textures)
{
  assert(!mIsBuilt && "Cannot add resources after resource set is built");
  assert(!textures.empty() && "Texture array cannot be empty");
  assert(validateBinding(binding, ResourceType::TEXTURE_2D, static_cast<uint32>(textures.size())) && "Invalid binding for texture array");

  std::vector<void *> texturePointers;
  texturePointers.reserve(textures.size());
  for (const auto &texture : textures)
  {
    texturePointers.push_back(texture.get());
  }

  mBindings.push_back({ResourceType::TEXTURE_2D,
                       binding,
                       std::move(texturePointers),
                       static_cast<uint32>(textures.size())});
}

bool GLResourceSet::build(const sptr<RenderDevice> &renderDevice)
{
  if (mIsBuilt)
    return false;

  // OpenGL doesn't need to build resource sets, just validate them
  // All resources should be valid at this point
  mIsBuilt = true;
  return true;
}

void GLResourceSet::bind(const sptr<RenderDevice> &renderDevice, uint32 setIndex) const
{
  assert(mIsBuilt && "Resource set must be built before binding");

  for (const auto &binding : mBindings)
  {
    switch (binding.type)
    {
    case ResourceType::TEXTURE_2D:
    case ResourceType::TEXTURE_CUBE:
      bindTexture(binding.binding, binding.resources);
      break;

    case ResourceType::UNIFORM_BUFFER:
      bindUniformBuffer(binding.binding, binding.resources[0]);
      break;

    case ResourceType::STORAGE_BUFFER:
      throw std::runtime_error("Storage buffers are not supported in OpenGL 4.1");
      break;

    case ResourceType::SAMPLER:
      bindSampler(binding.binding, binding.resources[0]);
      break;
    }
  }
}

void GLResourceSet::reset()
{
  mBindings.clear();
  mIsBuilt = false;
}

void GLResourceSet::bindTexture(uint32 binding, const std::vector<void *> &textures) const
{
  for (size_t i = 0; i < textures.size(); ++i)
  {
    // Accept either Texture* or ImageView* and unwrap to underlying texture
    const void *res = textures[i];
    const Texture *texture = nullptr;

    if (res)
    {
      // First cast through the polymorphic base to enable safe RTTI use
      const ResourceHandle *rh = static_cast<const ResourceHandle *>(res);
      if (rh && rh->isValid())
      {
        // Try direct Texture*
        if (!texture)
          texture = dynamic_cast<const Texture *>(rh);

        // Fallback: try ImageView* and unwrap underlying image
        if (!texture)
        {
          const ImageView *iv = dynamic_cast<const ImageView *>(rh);
          if (iv)
            texture = iv->getDesc().image;
        }
      }
    }

    if (texture && texture->isValid())
    {
      glCall(glActiveTexture(GL_TEXTURE0 + binding + static_cast<uint32>(i)));
      glCall(glBindTexture(getTextureTargetFromType(texture->getTextureType()), static_cast<GLuint>(reinterpret_cast<uintptr_t>(texture->getNativeHandle()))));
    }
  }
}

void GLResourceSet::bindUniformBuffer(uint32 binding, void *buffer) const
{
  const GpuBuffer *uniformBuffer = static_cast<const GpuBuffer *>(buffer);
  if (uniformBuffer)
  {
    // TODO: Support dynamic offsets by switching to glBindBufferRange with an offset passed from bindDescriptorSets
    glCall(glBindBufferBase(GL_UNIFORM_BUFFER, binding, static_cast<GLuint>(reinterpret_cast<uintptr_t>(uniformBuffer->getNativeHandle()))));
  }
}

void GLResourceSet::bindSampler(uint32 binding, void *sampler) const
{
  const SamplerState *samplerState = static_cast<const SamplerState *>(sampler);
  if (samplerState)
  {
    glCall(glBindSampler(binding, static_cast<GLuint>(reinterpret_cast<uintptr_t>(samplerState->getNativeHandle()))));
  }
}

bool GLResourceSet::validateBinding(uint32 binding, ResourceType type, uint32 count) const
{
  // This would validate against the layout if needed
  // For now, we'll assume all bindings are valid
  return true;
}

// GLResourceSetFactory Implementation
std::unique_ptr<IResourceSetLayout> GLResourceSetFactory::createLayout()
{
  return std::make_unique<GLResourceSetLayout>();
}

std::unique_ptr<IResourceSet> GLResourceSetFactory::createResourceSet(const std::unique_ptr<IResourceSetLayout> &layout)
{
  return std::make_unique<GLResourceSet>(layout);
}
