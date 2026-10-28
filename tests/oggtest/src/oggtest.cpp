// Ogg Vorbis test for SuperTux Milestone 2 on Symbian.
// 1. Measures how fast Tremor decodes test.ogg on this phone (logged as "x realtime").
// 2. Plays test.ogg (SuperTux 2's title theme, 44.1 kHz stereo) through SDL_mixer.
//    By default the whole file is loaded into memory first; press 2 to reload it
//    streamed from the file instead, press 1 to go back to memory.
// Screen: green = music playing, red = failed or stopped. A white bar sweeps across;
// if it stutters, the phone is struggling. Centre / 5 = Ogg sound effect, other keys quit.
// Log: c:\data\oggtest\log.txt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <tremor/ivorbisfile.h>

#include <e32std.h>
#include <eikstart.h>
#include <sdlepocapi.h>

class MSDLMainObs;
class SDLEnv
{
public:
	IMPORT_C static TInt SetMain(const TMainFunc& aFunc, TInt aFlags, MSDLMainObs* aObs, TInt aEnvFlags);
};

GLDEF_C TInt E32Main()
{
	return SDLEnv::SetMain(TMainFunc(SDL_main), CSDL::EAllowImageResizeKeepRatio, NULL, 0);
}

static FILE* g_log = NULL;
static void LOG(const char* a, const char* b = "")
{
	if (g_log) { fprintf(g_log, "%s%s\n", a, b); fflush(g_log); }
}
static void LOGN(const char* a, long n, const char* b = "")
{
	if (g_log) { fprintf(g_log, "%s%ld%s\n", a, n, b); fflush(g_log); }
}

/* the test files are installed on the drive the user picked */
static bool find_file(const char* name, char* out)
{
	const char drives[] = "cefd";
	for (int i = 0; drives[i]; i++)
	{
		sprintf(out, "%c:\\data\\oggtest\\%s", drives[i], name);
		if (access(out, F_OK) == 0)
			return true;
	}
	return false;
}

/* Decode 5 seconds of audio straight through Tremor and time it */
static void benchmark(const char* path)
{
	FILE* f = fopen(path, "rb");
	if (!f) { LOG("benchmark: cannot open file"); return; }
	OggVorbis_File vf;
	if (ov_open(f, &vf, NULL, 0) < 0) { LOG("benchmark: ov_open failed"); fclose(f); return; }
	vorbis_info* vi = ov_info(&vf, -1);
	LOGN("benchmark: rate ", vi->rate, " Hz");
	LOGN("benchmark: channels ", vi->channels);
	const long want = vi->rate * vi->channels * 2 * 5; /* bytes in 5 seconds */
	static char buf[4096];
	long got = 0;
	int section = 0;
	Uint32 t0 = SDL_GetTicks();
	while (got < want)
	{
		long n = ov_read(&vf, buf, sizeof(buf), &section);
		if (n <= 0) { LOGN("benchmark: ov_read returned ", n); break; }
		got += n;
	}
	Uint32 ms = SDL_GetTicks() - t0;
	LOGN("benchmark: decoded 5 s of audio in ", (long)ms, " ms");
	if (ms > 0)
		LOGN("benchmark: speed (x10) = ", 50000L / (long)ms, "  (10 = exactly realtime, higher is better)");
	ov_clear(&vf); /* also closes f */
}

/* Read a whole file into memory */
static void* load_file(const char* path, long* size)
{
	FILE* f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);
	void* data = malloc(*size);
	if (data && fread(data, 1, *size, f) != (size_t)*size) { free(data); data = NULL; }
	fclose(f);
	return data;
}

static Mix_Music* g_music = NULL;
static void* g_music_data = NULL;

static bool start_music(const char* path, bool from_memory)
{
	if (g_music) { Mix_HaltMusic(); Mix_FreeMusic(g_music); g_music = NULL; }
	if (g_music_data) { free(g_music_data); g_music_data = NULL; }

	Uint32 t0 = SDL_GetTicks();
	if (from_memory)
	{
		long size = 0;
		g_music_data = load_file(path, &size);
		if (!g_music_data) { LOG("could not read test.ogg into memory"); return false; }
		g_music = Mix_LoadMUS_RW(SDL_RWFromConstMem(g_music_data, size));
		LOG("music source: memory");
	}
	else
	{
		g_music = Mix_LoadMUS(path);
		LOG("music source: streamed from file");
	}
	LOGN("music load took ", (long)(SDL_GetTicks() - t0), " ms");
	if (!g_music) { LOG("Mix_LoadMUS failed: ", Mix_GetError()); return false; }
	if (Mix_PlayMusic(g_music, -1) < 0) { LOG("Mix_PlayMusic failed: ", Mix_GetError()); return false; }
	LOG("music playing");
	return true;
}

int main(int argc, char* argv[])
{
	mkdir("c:\\data", 0777);
	mkdir("c:\\data\\oggtest", 0777);
	g_log = fopen("c:\\data\\oggtest\\log.txt", "w");
	LOG("oggtest 2 start");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		LOG("SDL_Init failed: ", SDL_GetError());
		return 1;
	}
	SDL_Surface* screen = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE);
	if (!screen)
	{
		LOG("SDL_SetVideoMode failed: ", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	char music_path[256], sound_path[256];
	bool have_music = find_file("test.ogg", music_path);
	if (have_music) { LOG("music file: ", music_path); benchmark(music_path); }
	else LOG("test.ogg not found");

	bool ok = false;
	Mix_Chunk* sound = NULL;
	if (Mix_OpenAudio(22050, AUDIO_S16, 2, 2048) < 0)
		LOG("Mix_OpenAudio failed: ", Mix_GetError());
	else
	{
		LOG("audio open");
		if (have_music)
			ok = start_music(music_path, true);
		if (find_file("test_sound.ogg", sound_path))
		{
			sound = Mix_LoadWAV(sound_path);
			LOG(sound ? "ogg sound loaded" : "Mix_LoadWAV (ogg sound) failed: ", sound ? "" : Mix_GetError());
		}
	}

	const Uint32 green = SDL_MapRGB(screen->format, 0, 140, 0);
	const Uint32 red = SDL_MapRGB(screen->format, 160, 0, 0);
	const Uint32 white = SDL_MapRGB(screen->format, 255, 255, 255);
	int x = 0, frames = 0;
	Uint32 start = SDL_GetTicks(), last_report = start;
	bool quit = false;

	while (!quit)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				quit = true;
			else if (e.type == SDL_KEYDOWN)
			{
				SDLKey k = e.key.keysym.sym;
				if (k == SDLK_RETURN || k == SDLK_5)
				{
					if (sound) Mix_PlayChannel(-1, sound, 0);
					LOG("sound effect key");
				}
				else if (k == SDLK_1 && have_music)
					ok = start_music(music_path, true);
				else if (k == SDLK_2 && have_music)
					ok = start_music(music_path, false);
				else
					quit = true;
			}
		}

		/* once a second, note whether the music is still going */
		Uint32 now = SDL_GetTicks();
		if (now - last_report >= 1000)
		{
			last_report = now;
			LOGN("t=", (long)((now - start) / 1000), Mix_PlayingMusic() ? " s  music playing" : " s  music STOPPED");
		}

		SDL_FillRect(screen, NULL, (ok && Mix_PlayingMusic()) ? green : red);
		SDL_Rect bar = { (Sint16)x, 100, 16, 40 };
		SDL_FillRect(screen, &bar, white);
		SDL_Flip(screen);
		x = (x + 4) % (320 - 16);
		frames++;
		SDL_Delay(10);
	}

	LOGN("loop frames: ", frames);
	LOGN("loop ms: ", (long)(SDL_GetTicks() - start));
	if (sound) Mix_FreeChunk(sound);
	if (g_music) { Mix_HaltMusic(); Mix_FreeMusic(g_music); }
	if (g_music_data) free(g_music_data);
	Mix_CloseAudio();
	SDL_Quit();
	LOG("oggtest end");
	if (g_log) fclose(g_log);
	return 0;
}
