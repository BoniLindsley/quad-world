#pragma once

// Internal headers.
#include "handle.hpp"

// External dependencies.
#include <SDL.h>

namespace boni {
namespace SDL2 {
using gl_context = boni::handle<SDL_GLContext, SDL_GL_DeleteContext>;
using renderer = boni::handle<SDL_Renderer*, SDL_DestroyRenderer>;
using window = boni::handle<SDL_Window*, SDL_DestroyWindow>;
} // namespace SDL2
} // namespace boni