#pragma once

// Internal headers.
#include "handle.hpp"

// External dependencies.
#include <imgui.h>

namespace boni {
namespace ImGui {
using context = boni::handle<ImGuiContext*, ::ImGui::DestroyContext>;
} // namespace ImGui
} // namespace boni