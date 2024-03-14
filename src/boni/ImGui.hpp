#pragma once

// Internal headers.
#include "./memory.hpp"

// External dependencies.
#include <imgui.h>

namespace boni::ImGui {

using context =
    boni::memory::handle<ImGuiContext*, void, ::ImGui::DestroyContext>;

} // namespace boni::ImGui