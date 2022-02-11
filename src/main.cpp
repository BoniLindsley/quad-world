// Internal headers.
#include "boni/ImGui.hpp"
#include "boni/SDL2.hpp"

// External dependencies.
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <SDL.h>

// Standard libraries.
#include <stdexcept>
#include <string>

// C Standard libraries.
#include <cstdint>
#include <cstdio>

const auto default_clear_colour = ImVec4{0.45F, 0.55F, 0.60F, 1.00F};

class RenderState {
public:
  bool done = false;
};

void render(RenderState& state, boni::SDL2::renderer& renderer) {
  auto& done = state.done;
  const auto is_shown = ImGui::Begin("Hello, world!");
  const auto _window_cleanup = boni::cleanup<ImGui::End>();
  if (is_shown) {
    const auto is_button_pressed = ImGui::Button("Goodbye, world!");
    if (is_button_pressed) {
      done = true;
    }
  }
  SDL_RenderClear(renderer);
}

auto main(int /*argc*/, char** /*argv*/) -> int {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
    return 1;
  }
  const auto _sdl2_cleanup = boni::cleanup<SDL_Quit>();

  constexpr auto window_width = 1280;
  constexpr auto window_height = 720;
  auto window = boni::SDL2::window{SDL_CreateWindow(
      "quad-world", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      window_width, window_height, 0)};
  if (window.get() == nullptr) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
    return 1;
  }

  auto renderer = boni::SDL2::renderer{SDL_CreateRenderer(
      window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED)};
  if (renderer.get() == nullptr) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
    return 1;
  }

  IMGUI_CHECKVERSION();
  const auto imgui_context =
      boni::ImGui::context{ImGui::CreateContext()};
  if (imgui_context.get() == nullptr) {
    SDL_LogCritical(
        SDL_LOG_CATEGORY_VIDEO, "Failed to create ImGui context.");
    return 1;
  }
  ImGuiIO& io = ImGui::GetIO();

  ImGui_ImplSDL2_InitForSDLRenderer(window);
  const auto platform_cleanup = boni::cleanup<ImGui_ImplSDL2_Shutdown>();
  ImGui_ImplSDLRenderer_Init(renderer);
  const auto renderer_cleanup =
      boni::cleanup<ImGui_ImplSDLRenderer_Shutdown>();

  auto render_state = RenderState{};

  auto& done = render_state.done;
  while (!done) {
    SDL_Event event;
    if (SDL_WaitEvent(&event) == 0) {
      SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
      return 1;
    }
    do {
      const auto is_event_used = ImGui_ImplSDL2_ProcessEvent(&event);
      if (is_event_used) {
        continue;
      }
      if (event.type == SDL_QUIT) {
        done = true;
      }
      if (event.type == SDL_WINDOWEVENT) {
        auto& window_event = event.window;
        if (window_event.event == SDL_WINDOWEVENT_CLOSE &&
            window_event.windowID == SDL_GetWindowID(window)) {
          done = true;
        }
      }
    } while (SDL_PollEvent(&event) != 0);
    if (done) {
      continue;
    }

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui::NewFrame();

    render(render_state, renderer);

    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
  }

  return 0;
}