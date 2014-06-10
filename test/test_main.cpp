#include "pch.hpp"

#define CATCH_CONFIG_RUNNER
#include <catch/catch.hpp>

//int SDL_main(int argc, char* argv[])
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  // global setup...

  int result = Catch::Session().run(0, 0);

  // global clean-up...

  return result;
}
