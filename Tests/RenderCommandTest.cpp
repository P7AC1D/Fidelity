#include "catch.hpp"

#include <algorithm>
#include <vector>

#include "RenderCommand.hpp"

TEST_CASE("RenderCommand Constructor and Accessor")
{
  uint32 gpuProgramId = 4;
  uint32 materialId = 52;
  uint32 depth = 299385;
  bool isTransparent = false;
  uint32 vboId = 42256;
  uint32 iboId = 3830;
  uint32 fboId = 28372;
  RenderCommand renderCommand(gpuProgramId, materialId, depth, isTransparent, vboId, iboId, fboId);
  
  auto renderCommandKey = renderCommand.GetKey();
  REQUIRE(renderCommandKey == 329177288685125696);
}

TEST_CASE("RenderCommand sorting")
{
  RenderCommand renderCommandA(255, 5553, 3532531, false, 1, 2, 3);
  RenderCommand renderCommandB(221, 5553, 3532531, false, 4, 5, 6);
  RenderCommand renderCommandC(221, 5000, 3532531, false, 7, 8, 9);
  RenderCommand renderCommandD(221, 5000, 2000000, false, 10, 12, 13);
  RenderCommand renderCommandE(200, 5000, 3532531, true, 14, 15, 16);
  RenderCommand renderCommandF(190, 6000, 2323826, true, 17, 18, 19);
  RenderCommand renderCommandG(343, 2355, 7366318, true, 20, 21, 22);
  
  std::vector<RenderCommand> renderQueue;
  renderQueue.push_back(renderCommandC);
  renderQueue.push_back(renderCommandA);
  renderQueue.push_back(renderCommandE);
  renderQueue.push_back(renderCommandD);
  renderQueue.push_back(renderCommandB);
  renderQueue.push_back(renderCommandF);
  renderQueue.push_back(renderCommandG);
  std::sort(renderQueue.begin(), renderQueue.end());
  REQUIRE(renderQueue[0] == renderCommandD);
  REQUIRE(renderQueue[1] == renderCommandC);
  REQUIRE(renderQueue[2] == renderCommandB);
  REQUIRE(renderQueue[3] == renderCommandA);
  REQUIRE(renderQueue[4] == renderCommandF);
  REQUIRE(renderQueue[5] == renderCommandE);
  REQUIRE(renderQueue[6] == renderCommandG);
}
