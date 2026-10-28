# Repairs st2sdl.dll after a fresh link: the S60 3rd FP1 GCCE toolchain adds the
# export table onto the DLL's initialised data (same repair as fix_sdl_dll.py,
# in Perl because Perl is always on the build PATH).
# Usage: perl fix_st2sdl.pl S:\epoc32\release\gcce\urel\st2sdl.dll
# Safe to run every build: an already repaired DLL is left alone.
use strict;
my $fn = shift or die "usage: perl fix_st2sdl.pl st2sdl.dll\n";
open(my $in, '<', $fn) or die "fix_st2sdl: cannot open $fn\n";
binmode $in; local $/; my $d = <$in>; close $in;
die "fix_st2sdl: $fn is not an E32 image\n" unless substr($d, 16, 4) eq 'EPOC';
my @h = unpack('V25', substr($d, 0x14, 100));
my ($comp, $datasz, $expdir, $expcount, $dataoff) = @h[2, 8, 17, 18, 21];
if ($comp != 0) { print "fix_st2sdl: $fn is compressed, left alone\n"; exit 0; }
my $n = int($datasz / 4); $n = $expcount if $expcount < $n;
if ($n == 0) { print "fix_st2sdl: nothing to repair\n"; exit 0; }
sub w { unpack('V', substr($d, $_[0], 4)) }
my $M = 4294967296;
# SDL_CPUFeatures (data word 1) must come out as 0xFFFFFFFF
if ((w($dataoff + 4) - w($expdir + 4) + $M) % $M != 0xFFFFFFFF) {
  print "fix_st2sdl: $fn already repaired\n"; exit 0;
}
for my $i (0 .. $n - 1) {
  my $v = (w($dataoff + 4*$i) - w($expdir + 4*$i) + $M) % $M;
  substr($d, $dataoff + 4*$i, 4) = pack('V', $v);
}
open(my $out, '>', $fn) or die "fix_st2sdl: cannot write $fn\n";
binmode $out; print $out $d; close $out;
print "fix_st2sdl: repaired $n data words in $fn\n";
