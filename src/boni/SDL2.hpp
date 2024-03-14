#pragma once

// Internal headers.
#include "./memory.hpp"

// External dependencies.
#include <SDL.h>

namespace boni::SDL2 {

using gl_context =
    memory::handle<SDL_GLContext, void, SDL_GL_DeleteContext>;
using renderer =
    memory::handle<SDL_Renderer*, void, SDL_DestroyRenderer>;
using window = memory::handle<SDL_Window*, void, SDL_DestroyWindow>;

} // namespace boni::SDL2