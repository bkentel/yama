#include "pch.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

int SDL_main(int argc, char* argv[]) {
  // global setup...

  int result = Catch::Session().run(argc, argv);

  // global clean-up...

  return result;
}
