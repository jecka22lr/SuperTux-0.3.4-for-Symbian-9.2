/*
 * Symbian S60 (3rd Edition, Open C) support routines for PhysicsFS.
 * File access itself is in posix.c; this file holds the parts that
 * unix.c would normally provide.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_SYMBIAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "physfs_internal.h"

int __PHYSFS_platformInit(void)
{
    return(1);  /* always succeed. */
} /* __PHYSFS_platformInit */


int __PHYSFS_platformDeinit(void)
{
    return(1);  /* always succeed. */
} /* __PHYSFS_platformDeinit */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
    /* phones have no CD drives */
} /* __PHYSFS_platformDetectAvailableCDs */


/*
 * Executables live in the protected \sys\bin, so argv0 says nothing useful.
 * The base dir is where the game's data was installed: <drive>:\resource\apps\supertux2
 * on whichever drive the user picked (memory card first).
 */
char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
    static const char drives[] = "efdgc";
    char path[64];
    char *retval;
    int i;

    for (i = 0; drives[i] != '\0'; i++)
    {
        sprintf(path, "%c:\\resource\\apps\\supertux2", drives[i]);
        if (access(path, F_OK) == 0)
            break;
    } /* for */

    if (drives[i] == '\0')
        strcpy(path, "c:\\resource\\apps\\supertux2");

    retval = (char *) allocator.Malloc(strlen(path) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, path);
    return(retval);
} /* __PHYSFS_platformCalcBaseDir */


/* Symbian has no symlinks or "." / ".." tricks to resolve in our paths */
char *__PHYSFS_platformRealPath(const char *path)
{
    char *retval = (char *) allocator.Malloc(strlen(path) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, path);
    return(retval);
} /* __PHYSFS_platformRealPath */


char *__PHYSFS_platformCurrentDir(void)
{
    char buf[256];
    char *retval;

    if (getcwd(buf, sizeof(buf)) == NULL)
        strcpy(buf, "c:\\data");

    retval = (char *) allocator.Malloc(strlen(buf) + 1);
    BAIL_IF_MACRO(retval == NULL, ERR_OUT_OF_MEMORY, NULL);
    strcpy(retval, buf);
    return(retval);
} /* __PHYSFS_platformCurrentDir */


int __PHYSFS_platformSetDefaultAllocator(PHYSFS_Allocator *a)
{
    return(0);  /* just use malloc() and friends. */
} /* __PHYSFS_platformSetDefaultAllocator */


/* SuperTux only touches PhysicsFS from one thread, so no locking needed */
void *__PHYSFS_platformGetThreadID(void) { return((void *) 0x0001); }
void *__PHYSFS_platformCreateMutex(void) { return((void *) 0x0001); }
void __PHYSFS_platformDestroyMutex(void *mutex) {}
int __PHYSFS_platformGrabMutex(void *mutex) { return(1); }
void __PHYSFS_platformReleaseMutex(void *mutex) {}

#endif  /* PHYSFS_PLATFORM_SYMBIAN */

/* end of symbian.c ... */
