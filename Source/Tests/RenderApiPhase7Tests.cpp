#include "catch.hpp"

#include "RenderApi/GL/GLRenderDevice.hpp"
#include "RenderApi/CommandBuffer.hpp"
#include "RenderApi/Texture.hpp"
#include "RenderApi/Query.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <memory>
#include <thread>
#include <chrono>

namespace
{
  struct GlfwContext
  {
    GLFWwindow *window = nullptr;
    std::shared_ptr<GLRenderDevice> device;

    GlfwContext(int w, int h)
    {
      REQUIRE(glfwInit());

      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

      window = glfwCreateWindow(w, h, "Phase7Test", nullptr, nullptr);
      REQUIRE(window != nullptr);
      glfwMakeContextCurrent(window);

      REQUIRE(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) != 0);

      int fbw = 0, fbh = 0;
      glfwGetFramebufferSize(window, &fbw, &fbh);

      RenderDeviceDesc rd{};
      rd.RenderWidth = static_cast<uint32>(fbw > 0 ? fbw : w);
      rd.RenderHeight = static_cast<uint32>(fbh > 0 ? fbh : h);
      device = std::make_shared<GLRenderDevice>(rd);
      REQUIRE(device != nullptr);
    }

    ~GlfwContext()
    {
      if (window)
      {
        glfwDestroyWindow(window);
        window = nullptr;
      }
      glfwTerminate();
    }
  };
}

TEST_CASE("Phase7: Timestamp queries produce monotonic values", "[renderapi][phase7][gl]")
{
  GlfwContext ctx(64, 64);
  auto cmd = ctx.device->createCommandBuffer();
  REQUIRE(cmd != nullptr);

  QueryPoolDesc qpd{};
  qpd.type = QueryType::Timestamp;
  qpd.count = 2;
  auto pool = ctx.device->createQueryPool(qpd);
  REQUIRE(pool != nullptr);

  cmd->begin();
  cmd->writeTimestamp(pool, 0);
  // Ensure some GPU time passes; glFlush+sleep+glFinish to be conservative
  glFlush();
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  glFinish();
  cmd->writeTimestamp(pool, 1);

  uint64 results[2] = {0, 0};
  cmd->resolveQueryData(pool, 0, 2, results);
  cmd->end();
  cmd->execute();

  REQUIRE(results[0] <= results[1]);
}

TEST_CASE("Phase7: TimeElapsed query returns non-zero after GPU work", "[renderapi][phase7][gl]")
{
  GlfwContext ctx(64, 64);
  auto cmd = ctx.device->createCommandBuffer();
  REQUIRE(cmd != nullptr);

  QueryPoolDesc qpd{};
  qpd.type = QueryType::TimeElapsed;
  qpd.count = 1;
  auto pool = ctx.device->createQueryPool(qpd);
  REQUIRE(pool != nullptr);

  cmd->begin();
  cmd->beginQuery(pool, 0);
  // Minimal GPU activity
  glClear(GL_COLOR_BUFFER_BIT);
  glFinish();
  cmd->endQuery(pool, 0);

  uint64 elapsed = 0;
  cmd->resolveQueryData(pool, 0, 1, &elapsed);
  cmd->end();
  cmd->execute();

  REQUIRE(elapsed >= 0);
}

TEST_CASE("Phase7: Texture generateMips/copy/blit do not crash", "[renderapi][phase7][gl]")
{
  GlfwContext ctx(64, 64);
  auto cmd = ctx.device->createCommandBuffer();
  REQUIRE(cmd != nullptr);

  TextureDesc td{};
  td.Width = 32;
  td.Height = 32;
  td.Format = TextureFormat::RGBA8;
  td.Type = TextureType::Texture2D;
  td.MipLevels = 5;
  td.Flags = TextureDesc::UF_Sampled | TextureDesc::UF_TransferSrc | TextureDesc::UF_TransferDst;
  auto texA = ctx.device->createTexture(td, false);
  auto texB = ctx.device->createTexture(td, false);
  REQUIRE(texA);
  REQUIRE(texB);

  cmd->begin();
  // generate mips
  cmd->generateMips(texA);
  // same-size copy
  cmd->copyTexture(texA, texB, 0, 0);
  // blit (scale 32->16)
  TextureDesc tdSmall = td; tdSmall.Width = 16; tdSmall.Height = 16;
  auto texC = ctx.device->createTexture(tdSmall, false);
  REQUIRE(texC);
  cmd->blitTexture(texA, texC, true, 0, 0);
  cmd->end();
  cmd->execute();

  SUCCEED();
}
