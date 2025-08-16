// Phase 5 API surface tests (backend-agnostic)
// These tests avoid creating any real GPU/GL context and validate
// cross-API contracts: flag ops, view validity semantics, and resource set API.

#include "catch.hpp"

#include "RenderApi/GpuBuffer.hpp"
#include "RenderApi/Texture.hpp"
#include "RenderApi/ImageView.hpp"
#include "RenderApi/BufferView.hpp"
#include "RenderApi/ResourceSet.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

using std::shared_ptr;
using std::unique_ptr;

// Minimal mock implementations to exercise interfaces without a backend

class TestBuffer final : public GpuBuffer
{
public:
  explicit TestBuffer(const GpuBufferDesc &desc)
      : GpuBuffer(desc)
  {
    _initialized = true; // pretend created for API testing only
  }

  // ResourceHandle
  void *getNativeHandle() const override { return nullptr; }
  bool isValid() const override { return _initialized; }

  // GpuBuffer required ops (no-ops for API testing)
  void writeData(uint64, uint64, const void *, AccessType) override {}
  void readData(uint64, uint64, void *) override {}
  void copyData(GpuBuffer *, uint64, uint64, uint64) override {}
};

class TestTexture final : public Texture
{
public:
  explicit TestTexture(const TextureDesc &desc, bool gamma)
      : Texture(desc, gamma) { _isInitialized = true; }

  // ResourceHandle
  void *getNativeHandle() const override { return nullptr; }
  bool isValid() const override { return _isInitialized; }

  // Texture required ops (no-ops for API testing)
  void writeData(uint32, uint32, const std::shared_ptr<ImageData> &) override {}
  void writeData(uint32, uint32, uint32, uint32, uint32, uint32, uint32, uint32, void *) override {}
  void generateMips() override {}
};

class TestResourceSetLayout final : public IResourceSetLayout
{
public:
  void addBinding(uint32 binding, ResourceType type, uint32 count = 1) override
  {
    (void)binding;
    (void)type;
    (void)count;
    _built = false;
    _bindingsPushed = true;
  }
  bool build(const std::shared_ptr<RenderDevice> &) override
  {
    _built = true;
    return true;
  }
  bool isBuilt() const override { return _built; }

private:
  bool _built = false;
  bool _bindingsPushed = false;
};

class TestResourceSet final : public IResourceSet
{
public:
  TestResourceSet() = default;

  void addTexture(uint32 binding, const shared_ptr<Texture> &texture) override
  {
    _bindings.push_back({ResourceType::TEXTURE_2D, binding, (void *)texture.get(), 1});
  }
  void addImageView(uint32 binding, const shared_ptr<ImageView> &view) override
  {
    _bindings.push_back({ResourceType::TEXTURE_2D, binding, (void *)view.get(), 1});
  }
  void addUniformBuffer(uint32 binding, const shared_ptr<GpuBuffer> &buffer) override
  {
    _bindings.push_back({ResourceType::UNIFORM_BUFFER, binding, (void *)buffer.get(), 1});
  }
  void addStorageBuffer(uint32, const shared_ptr<GpuBuffer> &) override { /* not used here */ }
  void addSampler(uint32 binding, const shared_ptr<SamplerState> &sampler) override
  {
    _bindings.push_back({ResourceType::SAMPLER, binding, (void *)sampler.get(), 1});
  }
  void addTextureArray(uint32 binding, const std::vector<shared_ptr<Texture>> &textures) override
  {
    for (auto &t : textures)
      _bindings.push_back({ResourceType::TEXTURE_2D, binding, (void *)t.get(), 1});
  }
  bool build(const std::shared_ptr<RenderDevice> &) override
  {
    _built = true;
    return true;
  }
  void bind(const std::shared_ptr<RenderDevice> &, uint32) const override { /* no-op */ }
  bool isBuilt() const override { return _built; }
  void reset() override
  {
    _bindings.clear();
    _built = false;
  }

  struct Rec
  {
    ResourceType type;
    uint32 binding;
    void *resource;
    uint32 count;
  };
  const std::vector<Rec> &records() const { return _bindings; }

private:
  std::vector<Rec> _bindings;
  bool _built = false;
};

TEST_CASE("GpuBufferUsage bitmask operations work as expected", "[renderapi][phase5]")
{
  auto flags = GpuBufferUsage::Vertex | GpuBufferUsage::Index;
  REQUIRE((flags & GpuBufferUsage::Vertex));
  REQUIRE((flags & GpuBufferUsage::Index));
  REQUIRE_FALSE((flags & GpuBufferUsage::Uniform));
}

TEST_CASE("ImageView and BufferView report invalid when underlying resource is null", "[renderapi][phase5]")
{
  // ImageView with null image
  ImageViewDesc ivd{}; // image == nullptr
  ivd.viewFormat = TextureFormat::RGBA8;
  ImageView iv{ivd};
  REQUIRE_FALSE(iv.isValid());
  REQUIRE(iv.getNativeHandle() == nullptr);

  // BufferView with null buffer
  BufferViewDesc bvd{}; // buffer == nullptr
  BufferView bv{bvd};
  REQUIRE_FALSE(bv.isValid());
  REQUIRE(bv.getNativeHandle() == nullptr);
}

TEST_CASE("IResourceSet can accept an ImageView without dereferencing backend handles", "[renderapi][phase5]")
{
  auto layout = std::make_unique<TestResourceSetLayout>();
  // Bindings are advisory for this mock; build layout
  layout->addBinding(0, ResourceType::TEXTURE_2D, 1);
  REQUIRE(layout->build(nullptr));
  REQUIRE(layout->isBuilt());

  TestResourceSet set;

  // Create a minimal ImageView with null image (no backend handle required)
  ImageViewDesc ivd{};
  ivd.viewFormat = TextureFormat::RGBA8;
  auto iv = std::make_shared<ImageView>(ivd);
  set.addImageView(0, iv);

  // Also verify uniform buffer binding path compiles with a mock buffer
  GpuBufferDesc bd{};
  bd.ByteCount = 64;
  bd.BufferType = BufferType::Constant;
  auto tb = std::make_shared<TestBuffer>(bd);
  set.addUniformBuffer(1, tb);

  REQUIRE(set.build(nullptr));
  REQUIRE(set.isBuilt());

  // Check that our records captured both inserts
  const auto &recs = set.records();
  REQUIRE(recs.size() == 2);
  REQUIRE(recs[0].type == ResourceType::TEXTURE_2D);
  REQUIRE(recs[0].binding == 0);
  REQUIRE(recs[0].resource == (void *)iv.get());
  REQUIRE(recs[1].type == ResourceType::UNIFORM_BUFFER);
  REQUIRE(recs[1].binding == 1);
  REQUIRE(recs[1].resource == (void *)tb.get());
}

TEST_CASE("Default GpuBuffer::map throws when not implemented by backend", "[renderapi][phase5]")
{
  GpuBufferDesc bd{};
  bd.ByteCount = 16;
  bd.BufferType = BufferType::Vertex;
  TestBuffer tb{bd};
  // TestBuffer doesn't override map/unmap, so base should throw
  REQUIRE_THROWS_AS(tb.map(0, 8, AccessType::ReadOnly), std::runtime_error);
}
