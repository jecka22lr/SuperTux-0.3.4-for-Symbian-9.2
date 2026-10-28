// PhysicsFS test for SuperTux Milestone 2 on Symbian.
// Mounts data.zip and a plain folder from <drive>:\resource\apps\supertux2, lists them,
// reads a compressed text file and a 2 MB Ogg out of the zip, writes a file to the
// write dir and reads it back, then plays the Ogg (loaded through PhysicsFS into memory).
// Screen: green = every check passed, red = something failed. Any key quits.
// Log: c:\data\physfstest\log.txt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <physfs.h>

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
static int g_failures = 0;
static void LOG(const char* a, const char* b = "")
{
	if (g_log) { fprintf(g_log, "%s%s\n", a, b); fflush(g_log); }
}
static void LOGN(const char* a, long n, const char* b = "")
{
	if (g_log) { fprintf(g_log, "%s%ld%s\n", a, n, b); fflush(g_log); }
}
static void CHECK(bool ok, const char* what)
{
	if (ok) LOG("ok:   ", what);
	else { g_failures++; LOG("FAIL: ", what); LOG("      PhysicsFS says: ", PHYSFS_getLastError() ? PHYSFS_getLastError() : "(nothing)"); }
}

/* Read a whole file through PhysicsFS */
static char* read_all(const char* name, long* size)
{
	PHYSFS_file* f = PHYSFS_openRead(name);
	if (!f) return NULL;
	*size = (long)PHYSFS_fileLength(f);
	char* data = (char*)malloc(*size + 1);
	if (data && PHYSFS_read(f, data, 1, *size) != *size) { free(data); data = NULL; }
	if (data) data[*size] = '\0';
	PHYSFS_close(f);
	return data;
}

static void list_dir(const char* dir)
{
	char** list = PHYSFS_enumerateFiles(dir);
	for (char** i = list; *i; i++)
	{
		char full[256], b[300];
		if (dir[0]) sprintf(full, "%s/%s", dir, *i); else strcpy(full, *i);
		sprintf(b, "  %s%s", full, PHYSFS_isDirectory(full) ? "/" : "");
		LOG(b);
	}
	PHYSFS_freeList(list);
}

int main(int argc, char* argv[])
{
	mkdir("c:\\data", 0777);
	mkdir("c:\\data\\physfstest", 0777);
	g_log = fopen("c:\\data\\physfstest\\log.txt", "w");
	LOG("physfstest start");

	CHECK(PHYSFS_init(argc > 0 ? argv[0] : NULL) != 0, "PHYSFS_init");
	LOG("base dir:  ", PHYSFS_getBaseDir());
	LOG("user dir:  ", PHYSFS_getUserDir());
	LOG("separator: ", PHYSFS_getDirSeparator());

	char path[256];
	sprintf(path, "%sdata.zip", PHYSFS_getBaseDir());
	CHECK(PHYSFS_addToSearchPath(path, 1) != 0, "mount data.zip");
	sprintf(path, "%sloose", PHYSFS_getBaseDir());
	CHECK(PHYSFS_addToSearchPath(path, 1) != 0, "mount loose folder");
	CHECK(PHYSFS_setWriteDir("c:\\data\\physfstest") != 0, "set write dir");

	LOG("files:");
	list_dir("");
	list_dir("music");
	list_dir("text");

	CHECK(PHYSFS_exists("music/test.ogg") != 0, "music/test.ogg exists (stored in zip)");
	CHECK(PHYSFS_exists("loose.txt") != 0, "loose.txt exists (plain folder)");
	CHECK(PHYSFS_isDirectory("music") != 0, "music is a directory");

	long size = 0;
	char* text = read_all("text/hello.txt", &size);
	CHECK(text != NULL && strncmp(text, "Hello from inside a zip", 23) == 0, "read deflated text/hello.txt");
	LOGN("  hello.txt bytes: ", size);
	free(text);

	char* loose = read_all("loose.txt", &size);
	CHECK(loose != NULL && strncmp(loose, "Loose file", 10) == 0, "read loose.txt");
	free(loose);

	PHYSFS_file* wf = PHYSFS_openWrite("written.txt");
	CHECK(wf != NULL, "open written.txt for writing");
	if (wf)
	{
		CHECK(PHYSFS_write(wf, "saved game data", 1, 15) == 15, "write 15 bytes");
		PHYSFS_close(wf);
	}
	PHYSFS_addToSearchPath("c:\\data\\physfstest", 1);
	char* back = read_all("written.txt", &size);
	CHECK(back != NULL && size == 15 && strcmp(back, "saved game data") == 0, "read written.txt back");
	free(back);

	Uint32 t0 = SDL_GetTicks();
	long ogg_size = 0;
	char* ogg = read_all("music/test.ogg", &ogg_size);
	CHECK(ogg != NULL, "read 2 MB music/test.ogg from the zip");
	LOGN("  ogg bytes: ", ogg_size);
	LOGN("  read took ", (long)(SDL_GetTicks() - t0), " ms");

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { LOG("SDL_Init failed: ", SDL_GetError()); return 1; }
	SDL_Surface* screen = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE);
	if (!screen) { LOG("SDL_SetVideoMode failed: ", SDL_GetError()); SDL_Quit(); return 1; }

	Mix_Music* music = NULL;
	if (Mix_OpenAudio(22050, AUDIO_S16, 2, 2048) == 0 && ogg)
	{
		music = Mix_LoadMUS_RW(SDL_RWFromConstMem(ogg, ogg_size));
		CHECK(music != NULL && Mix_PlayMusic(music, -1) == 0, "play the Ogg from the zip");
	}
	else
		CHECK(false, "open audio");

	LOGN("failures: ", g_failures);

	const Uint32 bg = g_failures == 0 ? SDL_MapRGB(screen->format, 0, 140, 0) : SDL_MapRGB(screen->format, 160, 0, 0);
	bool quit = false;
	while (!quit)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN)
				quit = true;
		SDL_FillRect(screen, NULL, bg);
		SDL_Flip(screen);
		SDL_Delay(50);
	}

	if (music) { Mix_HaltMusic(); Mix_FreeMusic(music); }
	Mix_CloseAudio();
	free(ogg);
	PHYSFS_deinit();
	SDL_Quit();
	LOG("physfstest end");
	if (g_log) fclose(g_log);
	return 0;
}
