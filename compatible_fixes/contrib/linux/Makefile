BUILDDIR=build
LIBDIR=lib
INCLUDEDIR=include

export APBUILD_CC = gcc-4.0
export APBUILD_CXX1 = g++-4.0

LIBOGG=libogg.so.0.5.3
LIBVORBIS=libvorbis.so.0.3.1
LIBVORBISFILE=libvorbisfile.so.3.1.1
LIBSDL=libSDL-1.2.so.0.11.0

all: install

distclean: clean

clean:
	-rm -f bin/*
	-rm -f lib/*
	-make -C lndir distclean
	make -C $(BUILDDIR) clean

install: $(LIBDIR)/$(LIBOGG) $(LIBDIR)/$(LIBVORBIS) $(LIBDIR)/$(LIBSDL)

$(LIBDIR)/$(LIBOGG): build/created
	make -C $(BUILDDIR)/libogg-1.1.3 -f preMake
	make -C $(BUILDDIR)/libogg-1.1.3
	make -C $(BUILDDIR)/libogg-1.1.3 install
	cp $(BUILDDIR)/bin/lib/$(LIBOGG) $(LIBDIR)
	cp -ra $(BUILDDIR)/bin/include/ogg $(INCLUDEDIR)

$(LIBDIR)/$(LIBVORBIS) $(BUILDDIR)/bin/lib/$(LIBVORBISFILE): build/created
	make -C $(BUILDDIR)/libvorbis-1.1.2 -f preMake
	make -C $(BUILDDIR)/libvorbis-1.1.2
	make -C $(BUILDDIR)/libvorbis-1.1.2 install
	cp $(BUILDDIR)/bin/lib/$(LIBVORBIS) $(BUILDDIR)/bin/lib/$(LIBVORBISFILE) $(LIBDIR)
	cp -ra $(BUILDDIR)/bin/include/vorbis $(INCLUDEDIR)

$(LIBDIR)/$(LIBSDL): build/created
	make -C $(BUILDDIR)/SDL-1.2.11 -f preMake
	make -C $(BUILDDIR)/SDL-1.2.11
	make -C $(BUILDDIR)/SDL-1.2.11 install
	cp $(BUILDDIR)/bin/lib/$(LIBSDL) $(LIBDIR)
	cp -ra $(BUILDDIR)/bin/include/SDL $(INCLUDEDIR)

build/created bin/fastdep: bin/lndir
	make -C $(BUILDDIR)
	make -C $(BUILDDIR)/fastdep-0.15 -f preMake
	make -C $(BUILDDIR)/fastdep-0.15 CC=apgcc CXX=apg++
	install -s $(BUILDDIR)/fastdep-0.15/fastdep bin

bin/lndir:
	make -C lndir -f preMake
	install -s lndir/lndir bin

