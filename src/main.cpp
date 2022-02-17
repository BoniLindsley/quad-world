// Internal headers.
#include "boni/ImGui.hpp"
#include "boni/SDL2.hpp"

// External dependencies.
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>

#include <SDL.h>

// Standard libraries.
#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// C Standard libraries.
#include <cstdint>
#include <cstdio>

class RenderState {
public:
  bool done = false;
  std::vector<SDL_Point> points;
  std::vector<SDL_Point> draw_positions;
};

void process_gui(RenderState& state) noexcept {
  const auto is_shown = ImGui::Begin("Points");
  const boni::cleanup<ImGui::End> _window_cleanup{};
  if (is_shown) {
    constexpr auto dimension = 2;
    const auto is_shown = ImGui::BeginTable("PointTable", dimension);
    if (is_shown) {
      const boni::cleanup<ImGui::EndTable> _table_cleanup{};
      auto& points = state.points;
      for (auto row = 0; row < points.size(); ++row) {
        ImGui::TableNextRow();
        ImGui::PushID(row);
        const boni::cleanup<ImGui::PopID> _id_cleanup{};
        {
          ImGui::TableNextColumn();
          ImGui::InputInt("##x", static_cast<int*>(&points[row].x));
        }
        {
          ImGui::TableNextColumn();
          ImGui::InputInt("##y", static_cast<int*>(&points[row].y));
        }
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

int render(boni::SDL2::renderer& renderer, RenderState& state) noexcept {
  if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) != 0) {
    return -1;
  }
  if (SDL_RenderClear(renderer) != 0) {
    return -1;
  }

  const auto& points = state.points;
  const auto point_count = points.size();
  if (point_count > 0) {
    auto renderer_output_size = SDL_Point{0, 0};
    if (SDL_GetRendererOutputSize(
            renderer, &renderer_output_size.x,
            &renderer_output_size.y) != 0) {
      return -1;
    }
    const auto offset = SDL_Point{
        renderer_output_size.x / 2, renderer_output_size.y / 2};

    auto& draw_positions = state.draw_positions;
    draw_positions.clear();
    std::transform(
        points.cbegin(), points.cend(),
        std::back_inserter(draw_positions),
        [offset](SDL_Point point) -> SDL_Point {
          return {point.x + offset.x, point.y + offset.y};
        });
    const auto draw_count = draw_positions.size();
    if (draw_count > std::numeric_limits<int>::max()) {
      return SDL_SetError("Too many points to draw.");
    }
    const auto unsigned_draw_count = static_cast<int>(draw_count);

    constexpr auto max_value = std::numeric_limits<std::uint8_t>::max();
    if (SDL_SetRenderDrawColor(
            renderer, max_value, max_value, max_value, max_value) != 0) {
      return -1;
    }
    if (SDL_RenderDrawPoints(
            renderer, draw_positions.data(), unsigned_draw_count) != 0) {
      return -1;
    }
  }

  return 0;
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

  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  const boni::cleanup<ImGui_ImplSDL2_Shutdown> _platform_cleanup{};
  ImGui_ImplSDLRenderer_Init(renderer);
  const boni::cleanup<ImGui_ImplSDLRenderer_Shutdown>
      _renderer_cleanup{};

  auto render_state = RenderState{};

  auto& done = render_state.done;
  while (!done) {
    SDL_Event event;
    if (SDL_WaitEvent(&event) == 0) {
      SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
      return 1;
    }
    bool redraw_needed = false;
    do {
      const auto is_event_used = ImGui_ImplSDL2_ProcessEvent(&event);
      redraw_needed |= is_event_used;
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
    if (done || !redraw_needed) {
      continue;
    }

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui::NewFrame();

    process_gui(render_state);
    if (render(renderer, render_state) != 0) {
      SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
      render_state.done = true;
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer);
  }

  return 0;
}