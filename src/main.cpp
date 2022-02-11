// Internal headers.
#include "boni/ImGui.hpp"
#include "boni/SDL2.hpp"

// External dependencies.
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <SDL.h>

// Standard libraries.
#include <array>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// C Standard libraries.
#include <cstdint>
#include <cstdio>

using Point = std::array<int, 2>;

class RenderState {
public:
  bool done = false;
  std::vector<Point> points;
};

void render(RenderState& state) {
  const auto is_shown = ImGui::Begin("Points");
  const boni::cleanup<ImGui::End> _window_cleanup{};
  if (is_shown) {
    constexpr auto dimension = std::tuple_size<Point>::value;
    const auto is_shown = ImGui::BeginTable("PointTable", dimension);
    if (is_shown) {
      const boni::cleanup<ImGui::EndTable> _table_cleanup{};
      auto& points = state.points;
      for (auto row = 0; row < points.size(); ++row) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushID(row);
        const boni::cleanup<ImGui::PopID> _id_cleanup{};
        ImGui::InputInt2("", points[row].data());
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      const auto is_clicked = ImGui::Button("+##AddRow");
      if (is_clicked) {
        points.push_back({0, 0});
      }
    }
  }
}

auto main(int /*argc*/, char** /*argv*/) -> int {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
    return 1;
  }
  const boni::cleanup<SDL_Quit> _sdl2_cleanup{};

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
  const boni::cleanup<ImGui_ImplSDL2_Shutdown> _platform_cleanup{};
  ImGui_ImplSDLRenderer_Init(renderer);
  const boni::cleanup<ImGui_ImplSDLRenderer_Shutdown> _renderer_cleanup{};

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

    render(render_state);

    SDL_RenderClear(renderer);
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
  }

  return 0;
}