// Internal headers.
#include "boni/ImGui.hpp"
#include "boni/SDL2.hpp"

// External dependencies.
#include <boost/numeric/conversion/cast.hpp>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <SDL.h>

// Standard libraries.
#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// C Standard libraries.
#include <cmath>
#include <cstdint>
#include <cstdio>

constexpr auto ZOOM_PER_LEVEL{0.9f};

using Position = std::array<int, 2>;

struct Drag {
  SDL_Point start_mouse_point{0, 0};
  Position start_position{0, 0};
};

struct Camera {
  Position position;
  int zoom_level{};
  std::optional<Drag> drag;
};

auto get_zoom(const Camera& camera) {
  return std::pow(ZOOM_PER_LEVEL, camera.zoom_level);
}

auto viewport_to_world(
    const Camera& camera, const SDL_Point viewport_point) -> Position {
  const auto zoom = get_zoom(camera);
  const auto world_point_relative_to_camera_x =
      static_cast<float>(viewport_point.x) / zoom;
  const auto world_point_relative_to_camera_y =
      static_cast<float>(viewport_point.y) / zoom;
  const auto camera_position = camera.position;
  return {
      boost::numeric_cast<int>(world_point_relative_to_camera_x) +
          camera_position[0],
      boost::numeric_cast<int>(world_point_relative_to_camera_y) +
          camera_position[1]};
}

auto world_to_viewport(const Camera& camera, const Position world_point)
    -> SDL_Point {
  const auto camera_position = camera.position;
  const auto world_point_relative_to_camera_x =
      static_cast<float>(world_point[0] - camera_position[0]);
  const auto world_point_relative_to_camera_y =
      static_cast<float>(world_point[1] - camera_position[1]);
  const auto zoom = get_zoom(camera);
  return {
      boost::numeric_cast<int>(world_point_relative_to_camera_x * zoom),
      boost::numeric_cast<int>(world_point_relative_to_camera_y * zoom)

  };
}

struct RenderState {
  std::vector<Position> positions;
  std::vector<SDL_Point> draw_points;
  Camera camera;
};

void refresh_positions_render_cache(RenderState& state) {
  const auto& positions = state.positions;
  const auto& camera = state.camera;
  auto& draw_positions = state.draw_points;
  draw_positions.clear();
  std::transform(
      positions.cbegin(), positions.cend(),
      std::back_inserter(draw_positions),
      [&camera](Position point) -> SDL_Point {
        return world_to_viewport(camera, point);
      });
}

void process_gui(RenderState& state) {
  const auto is_shown = ImGui::Begin("Positions");
  struct WindowCleanup {
    ~WindowCleanup() { ImGui::End(); }
  } _window_cleanup;
  // const boni::cleanup<ImGui::End> _window_cleanup{};
  if (is_shown) {
    constexpr auto dimension = 2;
    const auto is_shown = ImGui::BeginTable("PositionTable", dimension);
    if (is_shown) {
      struct TableCleanup {
        ~TableCleanup() { ImGui::EndTable(); }
      } _table_cleanup;
      // const boni::cleanup<ImGui::EndTable> _table_cleanup{};
      auto& positions = state.positions;
      for (auto row = 0; row < positions.size(); ++row) {
        ImGui::TableNextRow();
        ImGui::PushID(row);
        struct IdCleanup {
          ~IdCleanup() { ImGui::PopID(); }
        } _id_cleanup;
        // const boni::cleanup<ImGui::PopID> _id_cleanup{};
        ImGui::TableNextColumn();
        ImGui::InputInt2("", positions[row].data());
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      const auto is_clicked = ImGui::Button("+##AddRow");
      if (is_clicked) {
        positions.push_back({0, 0});
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
  refresh_positions_render_cache(state);
  auto& draw_points = state.draw_points;
  auto draw_count = boost::numeric_cast<int>(draw_points.size());
  if (draw_count > 0) {
    constexpr auto max_value = std::numeric_limits<std::uint8_t>::max();
    if (SDL_SetRenderDrawColor(
            renderer, max_value, max_value, max_value, max_value) != 0) {
      return -1;
    }
    if (SDL_RenderDrawPoints(renderer, draw_points.data(), draw_count) !=
        0) {
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
  struct Sdl2Cleanup {
    ~Sdl2Cleanup() { SDL_Quit(); }
  } _sdl2_cleanup;
  // const boni::cleanup<SDL_Quit> _sdl2_cleanup{};

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

  struct PlatformCleanup {
    ~PlatformCleanup() { ImGui_ImplSDL2_Shutdown(); }
  } _platform_cleanup;
  // const boni::cleanup<ImGui_ImplSDL2_Shutdown> _platform_cleanup{};
  ImGui_ImplSDLRenderer2_Init(renderer);

  struct RendererCleanup {
    ~RendererCleanup() { ImGui_ImplSDLRenderer2_Shutdown(); }
  } _renderer_cleanup;
  // const boni::cleanup<ImGui_ImplSDLRenderer2_Shutdown>
  //     _renderer_cleanup{};

  auto render_state = RenderState{};
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
            const auto new_point = viewport_to_world(
                render_state.camera, {button_event.x, button_event.y});
            render_state.positions.push_back(new_point);
            is_event_processed = true;
            redraw_needed = true;
          } break;
          case SDL_BUTTON_RIGHT: {
            auto& camera = render_state.camera;
            camera.drag.emplace(
                Drag{{button_event.x, button_event.y}, camera.position});
            is_event_processed = true;
          } break;
          default:
            break;
          }
        }
        break;
      case SDL_MOUSEBUTTONUP: {
        auto& drag = render_state.camera.drag;
        const auto& button_event = event.button;
        if (drag.has_value() &&
            button_event.button == SDL_BUTTON_RIGHT) {
          drag.reset();
          is_event_processed = true;
        }
      } break;
      case SDL_MOUSEMOTION: {
        auto& camera = render_state.camera;
        auto& drag_maybe = camera.drag;
        if (drag_maybe.has_value()) {
          auto& drag = camera.drag.value();
          camera.position = drag.start_position;
          const auto& motion_event = event.motion;
          const auto start_mouse_point = drag.start_mouse_point;
          const auto drag_displacement = SDL_Point{
              motion_event.x - start_mouse_point.x,
              motion_event.y - start_mouse_point.y};
          camera.position = viewport_to_world(
              camera, {-drag_displacement.x, -drag_displacement.y});
          is_event_processed = true;
          redraw_needed = true;
        }
      } break;
      case SDL_MOUSEWHEEL:
        if (!io.WantCaptureMouse) {
          auto& wheel_event = event.wheel;
          render_state.camera.zoom_level += wheel_event.y;
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
      ImGui_ImplSDLRenderer2_NewFrame();
      ImGui::NewFrame();

      process_gui(render_state);
      if (render(renderer, render_state) != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_RENDER, "%s", SDL_GetError());
        return 1;
      }

      ImGui::Render();
      ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
      SDL_RenderPresent(renderer);
    }
  }

  return 0;
}
