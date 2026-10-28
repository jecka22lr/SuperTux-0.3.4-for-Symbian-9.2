//  SuperTux
//  Copyright (C) 2007 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SUPERTUX_OBSTACK_OBSTACKPP_HPP
#define HEADER_SUPERTUX_OBSTACK_OBSTACKPP_HPP

#include <obstack.h>
#include <stdlib.h>

inline void*
operator new (size_t bytes, struct obstack& obst)
{
  return obstack_alloc(&obst, bytes);
}

inline void*
operator new[] (size_t bytes, struct obstack& obst)
{
  return obstack_alloc(&obst, bytes);
}

static inline void* obstack_chunk_alloc(size_t size)
{
  // malloc returns NULL on failure, so obstack calls obstack_alloc_failed_handler;
  // an exception thrown here would have to pass through obstack.c (C code) and
  // kill the game. (std::nothrow needs a library Open C++ doesn't link.)
  return malloc(size);
}

static inline void obstack_chunk_free(void* data)
{
  free(data);
}

#endif

/* EOF */
