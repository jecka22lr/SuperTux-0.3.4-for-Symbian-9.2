/*
 * SuperTux - Symbian port
 * Functions SuperTux calls that the Symbian SDL 1.2.14 build doesn't export.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDL's own headers are deliberately not included: they declare these as
 * DLL imports, which can't be defined here.
 */
#include <stddef.h>
#include <string.h>

/* ---- Joystick: phones have none, so report zero joysticks ---- */

int SDL_NumJoysticks(void) { return 0; }
const char* SDL_JoystickName(int index) { (void) index; return NULL; }
void* SDL_JoystickOpen(int index) { (void) index; return NULL; }
int SDL_JoystickIndex(void* joystick) { (void) joystick; return -1; }
int SDL_JoystickNumAxes(void* joystick) { (void) joystick; return 0; }
int SDL_JoystickNumHats(void* joystick) { (void) joystick; return 0; }
int SDL_JoystickNumButtons(void* joystick) { (void) joystick; return 0; }
void SDL_JoystickClose(void* joystick) { (void) joystick; }

/*
 * ---- Character set conversion, used by tinygettext for translations ----
 * SuperTux's translations and its fonts are all UTF-8, and tinygettext skips
 * conversion when both sides match, so this is only reached for unusual
 * charsets; bytes are passed through unchanged.
 */

void* SDL_iconv_open(const char* tocode, const char* fromcode)
{
  (void) tocode; (void) fromcode;
  return (void*) 1;
}

int SDL_iconv_close(void* cd)
{
  (void) cd;
  return 0;
}

size_t SDL_iconv(void* cd, const char** inbuf, size_t* inbytesleft,
                 char** outbuf, size_t* outbytesleft)
{
  size_t n;
  (void) cd;
  if (inbuf == NULL || *inbuf == NULL || inbytesleft == NULL)
    return 0;  /* reset request */
  n = *inbytesleft < *outbytesleft ? *inbytesleft : *outbytesleft;
  memcpy(*outbuf, *inbuf, n);
  *inbuf += n;
  *outbuf += n;
  *inbytesleft -= n;
  *outbytesleft -= n;
  return 0;
}
