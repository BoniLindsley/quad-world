// Internal headers.
#include "boni/ImGui.hpp"
#include "boni/SDL2.hpp"

// External dependencies.
#include <boost/numeric/conversion/cast.hpp>

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
#include <cmath>
#include <cstdint>
#include <cstdio>

class RenderState {
public:
  std::vector<SDL_Point> points;
  std::vector<SDL_Point> draw_positions;
  SDL_Point draw_offset{0, 0};
  float draw_scale{1.f};
  bool is_right_dragging{false};
  SDL_Point last_right_press_position{0, 0};
  SDL_Point last_right_press_draw_offset{0, 0};
};

void process_gui(RenderState& state) {
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

auto render(boni::SDL2::renderer& renderer, RenderState& state) -> int {
  if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) != 0) {
    return -1;
  }
  if (SDL_RenderClear(renderer) != 0) {
    return -1;
  }

  const auto& points = state.points;
  const auto point_count = points.size();
  if (point_count > 0) {
    const auto& draw_offset = state.draw_offset;
    auto& draw_positions = state.draw_positions;
    draw_positions.clear();
    std::transform(
        points.cbegin(), points.cend(),
        std::back_inserter(draw_positions),
        [draw_offset](SDL_Point point) -> SDL_Point {
          return {point.x + draw_offset.x, point.y + draw_offset.y};
        });

    constexpr auto max_value = std::numeric_limits<std::uint8_t>::max();
    if (SDL_SetRenderDrawColor(
            renderer, max_value, max_value, max_value, max_value) != 0) {
      return -1;
    }
    const auto draw_scale = state.draw_scale;
    if (SDL_RenderSetScale(renderer, draw_scale, draw_scale) != 0) {
      return -1;
    }
    if (SDL_RenderDrawPoints(
            renderer, draw_positions.data(),
            boost::numeric_cast<int>(draw_positions.size())) != 0) {
      return -1;
    }
    if (SDL_RenderSetScale(renderer, 1, 1) != 0) {
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
  {
    auto renderer_output_size = SDL_Point{0, 0};
    if (SDL_GetRendererOutputSize(
            renderer, &renderer_output_size.x,
            &renderer_output_size.y) != 0) {
      return -1;
    }
    render_state.draw_offset = SDL_Point{
        renderer_output_size.x / 2, renderer_output_size.y / 2};
  }

  while (true) {
    SDL_Event event;
    if (SDL_WaitEvent(&event) == 0) {
      SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
      return 1;
    }
    auto redraw_needed = false;
    do {
      auto is_event_processed = false;
      switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
        if (!io.WantCaptureMouse) {
          const auto& button_event = event.button;
          switch (button_event.button) {
          case SDL_BUTTON_LEFT: {
            const auto button_x = static_cast<float>(button_event.x);
            const auto button_y = static_cast<float>(button_event.y);
            const auto draw_offset = render_state.draw_offset;
            const auto draw_scale = render_state.draw_scale;
            const auto point_x =
                boost::numeric_cast<int>(button_x / draw_scale) -
                draw_offset.x;
            const auto point_y =
                boost::numeric_cast<int>(button_y / draw_scale) -
                draw_offset.y;
            render_state.points.push_back({point_x, point_y});
            is_event_processed = true;
            redraw_needed = true;
          } break;
          case SDL_BUTTON_RIGHT:
            render_state.last_right_press_position = {
                button_event.x, button_event.y};
            render_state.last_right_press_draw_offset =
                render_state.draw_offset;
            render_state.is_right_dragging = true;
            is_event_processed = true;
            break;
          default:
            break;
          }
        }
        break;
      case SDL_MOUSEBUTTONUP: {
        const auto& button_event = event.button;
        auto& is_right_dragging = render_state.is_right_dragging;
        if (is_right_dragging &&
            button_event.button == SDL_BUTTON_RIGHT) {
          is_right_dragging = false;
          is_event_processed = true;
        }
      } break;
      case SDL_MOUSEMOTION:
        if (render_state.is_right_dragging) {
          const auto& motion_event = event.motion;
          const auto& last_right_press_position =
              render_state.last_right_press_position;
          const auto draw_scale = render_state.draw_scale;
          const auto movemnt_x =
              static_cast<float>(
                  motion_event.x - last_right_press_position.x) /
              draw_scale;
          const auto movemnt_y =
              static_cast<float>(
                  motion_event.y - last_right_press_position.y) /
              draw_scale;
          const auto& last_right_press_draw_offset =
              render_state.last_right_press_draw_offset;
          render_state.draw_offset = {
              last_right_press_draw_offset.x +
                  static_cast<int>(movemnt_x),
              last_right_press_draw_offset.y +
                  static_cast<int>(movemnt_y),
          };
          is_event_processed = true;
          redraw_needed = true;
        }
        break;
      case SDL_MOUSEWHEEL:
        if (!io.WantCaptureMouse) {
          auto& wheel_event = event.wheel;
          auto scale_ratio = std::pow(0.9f, wheel_event.y);
          render_state.draw_scale *= static_cast<float>(scale_ratio);
          is_event_processed = true;
          redraw_needed = true;
        }
        break;
      case SDL_QUIT:
        return 0;
      case SDL_WINDOWEVENT: {
        auto& window_event = event.window;
        if (window_event.event == SDL_WINDOWEVENT_CLOSE &&
            window_event.windowID == SDL_GetWindowID(window)) {
          return 0;
        }
      } break;
      default:
        break;
      }
      if (!is_event_processed) {
        redraw_needed |= ImGui_ImplSDL2_ProcessEvent(&event);
      }
    } while (SDL_PollEvent(&event) != 0);

    if (redraw_needed) {
      ImGui_ImplSDL2_NewFrame();
      ImGui_ImplSDLRenderer_NewFrame();
      ImGui::NewFrame();

      process_gui(render_state);
      if (render(renderer, render_state) != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
        return 1;
      }

      ImGui::Render();
      ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
      SDL_RenderPresent(renderer);
    }
  }

  return 0;
}
