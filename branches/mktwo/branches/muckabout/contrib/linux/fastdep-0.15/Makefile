# CCDV=../../tools/ccdv/ccdv.bin
CXX=g++

all: fastdep

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
include config/config.me
endif
endif

include build/project.mk
include build/dependencies.mk
include build/release.mk
include build/gnugetopt.mk

include Makefile.common

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include .depends-all
endif
endif

BINARY=fastdep

CLASSES=$(foreach group,$(GROUPS),$($(call classgroup,fastdep,$(group))))
SOURCES=$(addsuffix .cc,$(CLASSES))
OBJECTS=$(SOURCES:.cc=.o)
OBJECTS+=$(BUILDSOURCES:.c=.o)

CFLAGS+=-Wall $(foreach include,$(INCLUDEDIRS),-I $(include))

$(OBJECTS): config/config.me $(ALLMAKEFILES)

$(BINARY): $(OBJECTS) $(ALLMAKEFILES)
	$(CCDV) $(CXX) $(CFLAGS) $(CXXFLAGS) -o $(BINARY) $(OBJECTS)
# Here is a trick on how to get fastdep linking on older cygwins.
# It's probably also needed to get it working on VC 6.0.
# However, since I am still reworking the build environment for now,
# this is left unmerged.
#ifeq ($(HOST_SYSTEM),cygwin)
#	$(foreach file, $(OBJECTS), $(shell echo $(file) >> linkfiles))
#	$(CCDV) $(CXX) $(CFLAGS) $(CXXFLAGS) -o $(BINARY) @linkfiles
#	-rm -f linkfiles
#endif

.cc.o: $(ALLMAKEFILES)
	$(CCDV) $(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	-rm -f $(OBJECTS) $(BINARY) .depends-all

distclean:
	-rm -f $(OBJECTS) $(BINARY) .depends-all config/config.me
#$(DEPENDS): $(SOURCES) $(MAKEFILE)
#ifeq ($(HOST_SYSTEM),cygwin)
#	$(foreach file, $(SOURCES), $(shell echo $(file) >> fastdepfiles))
#	$(BINARY) -o $(DEPENDS) @fastdepfiles > $(DEPENDS)
#	-rm -f fastdepfiles
#else
#	./$(BINARY) -o $(DEPENDS) $(SOURCES)
#	$(call depend_dir)
#	$(call depend_dir_test)
#endif

#include $(DEPENDS)

#$(DEPENDS): Makefile.real
.depends-%:
	$(call generate-depends)

config/config.me:
	./configure
