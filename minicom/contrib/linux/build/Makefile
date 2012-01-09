LNDIR=../bin/lndir

all: sdl oggvorbis fastdep
	touch created

clean:
	-rm -rf SDL-1.2.11 libogg-1.1.3 libvorbis-1.1.2 fastdep-0.15 created

distclean: clean

sdl: SDL-1.2.11

oggvorbis: ogg vorbis

ogg: libogg-1.1.3

vorbis: libvorbis-1.1.2

fastdep: fastdep-0.15

SDL-1.2.11:
	-mkdir SDL-1.2.11
	$(LNDIR) ../../../../../../SystemIV/macosx/contrib/SDL-1.2.11 SDL-1.2.11

libogg-1.1.3:
	-mkdir libogg-1.1.3
	$(LNDIR) ../../../macosx/libogg-1.1.3 libogg-1.1.3

libvorbis-1.1.2:
	-mkdir libvorbis-1.1.2
	$(LNDIR) ../../../macosx/libvorbis-1.1.2 libvorbis-1.1.2

fastdep-0.15:
	-mkdir fastdep-0.15
	$(LNDIR) ../../fastdep-0.15 fastdep-0.15
