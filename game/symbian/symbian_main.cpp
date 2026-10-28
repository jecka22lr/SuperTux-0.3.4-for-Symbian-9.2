//  SuperTux - Symbian port entry point
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include <SDL.h>
#include <stdio.h>
#include <math.h>
#include <limits>
#include <sys/stat.h>
#include <iostream>
#include <streambuf>
#include <stdlib.h>
#include <obstack.h>

#include "supertux/main.hpp"
#include "build_options.h"

#include <e32std.h>
#include <eikstart.h>
#include <sdlepocapi.h>

class MSDLMainObs;
// Exported by sdlexe.dll; the SDK ships no header for it
class SDLEnv
{
public:
  IMPORT_C static TInt SetMain(const TMainFunc& aFunc, TInt aFlags, MSDLMainObs* aObs, TInt aEnvFlags);
};

// Our own entry point, replacing the one in sdlmain.lib: no "enter parameters"
// dialog and no virtual mouse (same as SuperTux 0.1.3 for S60)
GLDEF_C TInt E32Main()
{
  return SDLEnv::SetMain(TMainFunc(SDL_main), CSDL::EAllowImageResizeKeepRatio, NULL, 0);
}

#define ST2_LOG "c:\\data\\supertux2\\log.txt"

// Writes straight to the log file and closes it every time, so nothing is
// lost if the game crashes. std::cerr in Open C++ doesn't follow freopen().
class LogFileBuf : public std::streambuf
{
protected:
  virtual int overflow(int c)
  {
    if(c != EOF) {
      char ch = (char)c;
      xsputn(&ch, 1);
    }
    return c;
  }
  virtual std::streamsize xsputn(const char* s, std::streamsize n)
  {
    FILE* f = fopen(ST2_LOG, "a");
    if(f) {
      fwrite(s, 1, n, f);
      fclose(f);
    }
    return n;
  }
};

static LogFileBuf log_buf;

// Release build without logging: the game's messages are thrown away
class NullBuf : public std::streambuf
{
protected:
  virtual int overflow(int c) { return c; }
  virtual std::streamsize xsputn(const char*, std::streamsize n) { return n; }
};

static NullBuf null_buf;

static char* stack_top = 0;
static unsigned char* stack_bottom = 0;
static int stack_size = 0;
static const unsigned char STACK_PAINT = 0xA5;

// Fill the unused part of this thread's stack with a pattern, so checkpoints
// can tell how deep the stack has ever gone (Symbian kills a thread silently
// when it runs out of stack)
static void paint_stack()
{
  TThreadStackInfo info;
  if(RThread().StackInfo(info) != KErrNone)
    return;
  stack_bottom = (unsigned char*) info.iLimit;
  stack_size = (int)(info.iBase - info.iLimit);
  char here;
  unsigned char* p = stack_bottom + 16;
  unsigned char* end = (unsigned char*) &here - 512;
  while(p < end)
    *p++ = STACK_PAINT;
}

static int stack_peak()
{
  if(!stack_bottom)
    return 0;
  unsigned char* p = stack_bottom + 16;
  while(*p == STACK_PAINT)
    p++;
  return stack_size - (int)(p - stack_bottom);
}

static void checkpoint(const char* what)
{
#if !ST2_LOGGING
  (void) what;
  return;
#endif
  FILE* f = fopen(ST2_LOG, "a");
  if(f) {
    fprintf(f, "[symbian] %s (stack peak %d of %d)\n", what, stack_peak(), stack_size);
    fclose(f);
  }
}

void symbian_checkpoint(const char* what)
{
  checkpoint(what);
}

// The level/data file parser (lisp::Parser) allocates through obstack. When the
// heap runs out, obstack calls this instead of returning: log it, so a crash
// from a level that is too big for the phone says so instead of vanishing.
static void obstack_out_of_memory()
{
  checkpoint("OUT OF MEMORY while reading a level or data file - quitting");
  exit(1);
}

int main(int argc, char** argv)   // SDL.h turns this into SDL_main
{
  char top;
  stack_top = &top;
  paint_stack();
  // There is no console on a phone: the game's messages go to a log file
  mkdir("c:\\data", 0777);
  mkdir("c:\\data\\supertux2", 0777);
  obstack_alloc_failed_handler = obstack_out_of_memory;
#if ST2_LOGGING
  FILE* f = fopen(ST2_LOG, "w");  // start a fresh log
  if(f) fclose(f);
  checkpoint("main() reached");

  std::cerr.rdbuf(&log_buf);
  std::cout.rdbuf(&log_buf);
  std::clog.rdbuf(&log_buf);
  std::cerr << "[symbian] std::cerr goes to the log" << std::endl;
#else
  remove(ST2_LOG);  // no log in this build: drop one left by a debug build
  std::cerr.rdbuf(&null_buf);
  std::cout.rdbuf(&null_buf);
  std::clog.rdbuf(&null_buf);
#endif

#if ST2_LOGGING
  {
    // checks Open C's float maths (collisions depend on floorf/ceilf)
    volatile float a = 3.7f, b = 3.2f, c = -0.5f, d = -2.5f, e = 16.0f, g = 416.0f;
    volatile double h = 3.7;
    FILE* f2 = fopen(ST2_LOG, "a");
    if(f2) {
      fprintf(f2, "[symbian] maths: floorf(3.7)=%d ceilf(3.2)=%d floorf(-0.5)=%d fabsf(-2.5)*10=%d sqrtf(16)=%d floor(3.7)=%d 416/32=%d\n",
              (int) floorf(a), (int) ceilf(b), (int) floorf(c), (int) (fabsf(d) * 10), (int) sqrtf(e), (int) floor(h), (int) (g / 32));
      fprintf(f2, "[symbian] limits: has_infinity=%d (1000 < infinity)=%d (1000 < max)=%d\n", (int) std::numeric_limits<float>::has_infinity, (int) (1000.0f < std::numeric_limits<float>::infinity()), (int) (1000.0f < std::numeric_limits<float>::max()));
      fclose(f2);
    }
  }
#endif

  int result = 1;
  try {
    result = Main().run(argc, argv);
  } catch(...) {
    checkpoint("exception escaped Main::run");
  }
  checkpoint(result == 0 ? "Main::run returned 0" : "Main::run returned an error");
  return result;
}

/* EOF */
