#include <nwge/engine.hpp>
#include "PlayState.hpp"

s32 main([[maybe_unused]] s32 argc, [[maybe_unused]] CStr *argv) {
  nwge::startPtr(getPlayState(), {
    .appName = "AwdawdAI",
    .windowAspectRatio = {1, 1}
  });
  return 0;
}
