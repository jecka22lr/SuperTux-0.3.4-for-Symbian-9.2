PhysicsFS 2.0.3 for Symbian S60 3rd Edition (zlib licence, see ..\LICENSE.txt)

Only the parts SuperTux needs: core, directory and zip archivers.
Symbian changes:
- physfs_platforms.h: Symbian platform (POSIX file calls, no CD-ROM, no threads).
- platform\symbian.c: new. Base dir = <drive>:\resource\apps\supertux2 on the
  drive the game was installed to; no symlinks; single-threaded locking stubs.
- platform\posix.c: '\' as the directory separator (PhysicsFS '/' paths are
  converted), no passwd database, user dir = c:\data.
