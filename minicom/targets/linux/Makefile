# -*- Makefile -*- for Defcon

#
# In the parent directory of source/ contrib/ targets/
#
# make -f targets/linux/Makefile.release
# or
# make -f targets/linux/Makefile.debug

CXXFLAGS=$(CXXFLAGS_BASE) -pipe -fmessage-length=0 -fvisibility=hidden 
CFLAGS=${CXXFLAGS} -DENABLE_BINRELOC

CPPFLAGS=$(CPPFLAGS_BASE)\
-DTARGET_FULLGAME \
-DTARGET_OS_LINUX \
-DHAVE_SOCKET \
-DHAVE_SELECT \
-DHAVE_GETHOSTBYNAME \
-DHAVE_INET_NTOA \
-DSILENT \
$(INCLUDES:%=-I%)

SYSTEMIV_PATH=contrib/systemIV

INCLUDES=\
source \
source/app \
source/lib \
$(SYSTEMIV_PATH) \
$(SYSTEMIV_PATH)/contrib \
$(SYSTEMIV_PATH)/contrib/unrar \
$(SYSTEMIV_PATH)/contrib/binreloc \
contrib/linux/include \
$(SYSTEMIV_PATH)/lib/gucci \
$(SYSTEMIV_PATH)/lib/netlib \
$(SYSTEMIV_PATH)/lib/eclipse \
contrib/binreloc

LIBS=\
-dynamic \
-Lcontrib/linux/lib \
-L/usr/X11R6/lib \
-lSDL -lGL -lGLU \
-lvorbisfile -lvorbis -logg \

LINK=$(CXX)

SOURCES= \
source/app/tutorial.cpp \
source/app/tutorial_defcon.cpp \
source/app/modsystem.cpp \
source/app/defcon_soundinterface.cpp \
source/app/version_manager.cpp \
source/app/app.cpp \
source/app/game.cpp \
source/app/statusicon.cpp \
source/app/x11icon.cpp \
source/app/game_history.cpp \
source/lib/debug_utils_linux.cpp \
source/lib/multiline_text.cpp \
source/lib/spawn.cpp \
source/lib/universal_include.cpp \
source/renderer/map_renderer.cpp \
source/renderer/animated_icon.cpp \
source/renderer/lobby_renderer.cpp \
source/world/radarstation.cpp \
source/world/movingobject.cpp \
source/world/team.cpp \
source/world/date.cpp \
source/world/explosion.cpp \
source/world/fighter.cpp \
source/world/battleship.cpp \
source/world/silo.cpp \
source/world/gunfire.cpp \
source/world/saucer.cpp \
source/world/whiteboard.cpp \
source/world/world.cpp \
source/world/worldobject.cpp \
source/world/worldoption.cpp \
source/world/blip.cpp \
source/world/sub.cpp \
source/world/earthdata.cpp \
source/world/radargrid.cpp \
source/world/airbase.cpp \
source/world/votingsystem.cpp \
source/world/city.cpp \
source/world/carrier.cpp \
source/world/fleet.cpp \
source/world/nuke.cpp \
source/world/node.cpp \
source/world/depthcharge.cpp \
source/world/bomber.cpp \
source/world/tornado.cpp \
source/network/ClientToServer.cpp \
source/network/Server.cpp \
source/network/ServerToClient.cpp \
source/defcon.cpp \
source/interface/mod_window.cpp \
source/interface/badkey_window.cpp \
source/interface/network_window.cpp \
source/interface/connecting_window.cpp \
source/interface/tutorial_window.cpp \
source/interface/fading_window.cpp \
source/interface/profile_window.cpp \
source/interface/chat_window.cpp \
source/interface/lobby_window.cpp \
source/interface/interface.cpp \
source/interface/mainmenu.cpp \
source/interface/side_panel.cpp \
source/interface/toolbar.cpp \
source/interface/authkey_window.cpp \
source/interface/alliances_window.cpp \
source/interface/resynchronise_window.cpp \
source/interface/fleet_placement_icon.cpp \
source/interface/casualties_window.cpp \
source/interface/serverbrowser_window.cpp \
source/interface/serverdetails_window.cpp \
source/interface/debug_window.cpp \
source/interface/demo_window.cpp \
source/interface/whiteboard_panel.cpp \
source/interface/worldstatus_window.cpp \
source/interface/worldobject_window.cpp \
source/interface/info_window.cpp \
source/interface/placement_icon.cpp \
$(SYSTEMIV_PATH)/contrib/binreloc/binreloc.c \
$(SYSTEMIV_PATH)/contrib/unrar/archive.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/arcread.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/cmddata.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/consio.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/crc.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/crypt.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/encname.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/errhnd.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/extinfo.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/extract.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/filcreat.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/file.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/filefn.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/filestr.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/find.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/getbits.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/global.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/int64.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/isnt.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/list.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/match.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/options.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/pathfn.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/rarresource.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/rarvm.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/rawread.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/rdwrfn.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/recvol.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/rijndael.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/rs.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/savepos.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/scantree.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/sha1.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/smallfn.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/strfn.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/strlist.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/system.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/timefn.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/ulinks.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/unicode.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/unpack.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/unrar.cpp \
$(SYSTEMIV_PATH)/contrib/unrar/volume.cpp \
$(SYSTEMIV_PATH)/interface/components/checkbox.cpp \
$(SYSTEMIV_PATH)/interface/components/core.cpp \
$(SYSTEMIV_PATH)/interface/components/drop_down_menu.cpp \
$(SYSTEMIV_PATH)/interface/components/filedialog.cpp \
$(SYSTEMIV_PATH)/interface/components/inputfield.cpp \
$(SYSTEMIV_PATH)/interface/components/message_dialog.cpp \
$(SYSTEMIV_PATH)/interface/components/scrollbar.cpp \
$(SYSTEMIV_PATH)/interface/options/screenoptions_window.cpp \
$(SYSTEMIV_PATH)/interface/options/soundoptions_window.cpp \
$(SYSTEMIV_PATH)/interface/soundsystem/soundeditor_window.cpp \
$(SYSTEMIV_PATH)/interface/soundsystem/soundstats_window.cpp \
$(SYSTEMIV_PATH)/interface/soundsystem/soundparameter_window.cpp \
$(SYSTEMIV_PATH)/interface/styleeditor_window.cpp \
$(SYSTEMIV_PATH)/lib/eclipse/eclbutton.cpp \
$(SYSTEMIV_PATH)/lib/eclipse/eclipse.cpp \
$(SYSTEMIV_PATH)/lib/eclipse/eclwindow.cpp \
$(SYSTEMIV_PATH)/lib/filesys/binary_stream_readers.cpp \
$(SYSTEMIV_PATH)/lib/filesys/file_system.cpp \
$(SYSTEMIV_PATH)/lib/filesys/filesys_utils.cpp \
$(SYSTEMIV_PATH)/lib/filesys/text_file_writer.cpp \
$(SYSTEMIV_PATH)/lib/filesys/text_stream_readers.cpp \
$(SYSTEMIV_PATH)/lib/gucci/input.cpp \
$(SYSTEMIV_PATH)/lib/gucci/input_sdl.cpp \
$(SYSTEMIV_PATH)/lib/gucci/window_manager.cpp \
$(SYSTEMIV_PATH)/lib/gucci/window_manager_sdl.cpp \
$(SYSTEMIV_PATH)/lib/math/fixed_float.cpp \
$(SYSTEMIV_PATH)/lib/math/fixed_64.cpp \
$(SYSTEMIV_PATH)/lib/math/hash.cpp \
$(SYSTEMIV_PATH)/lib/math/math_utils.cpp \
$(SYSTEMIV_PATH)/lib/math/random_number.cpp \
$(SYSTEMIV_PATH)/lib/math/vector2.cpp \
$(SYSTEMIV_PATH)/lib/math/vector3.cpp \
$(SYSTEMIV_PATH)/lib/metaserver/authentication.cpp \
$(SYSTEMIV_PATH)/lib/metaserver/matchmaker.cpp \
$(SYSTEMIV_PATH)/lib/metaserver/metaserver.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_lib.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_mutex_linux.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_socket.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_socket_listener.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_socket_session.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_thread_linux.cpp \
$(SYSTEMIV_PATH)/lib/netlib/net_udp_packet.cpp \
$(SYSTEMIV_PATH)/lib/render/colour.cpp \
$(SYSTEMIV_PATH)/lib/render/renderer.cpp \
$(SYSTEMIV_PATH)/lib/render/styletable.cpp \
$(SYSTEMIV_PATH)/lib/resource/bitmap.cpp \
$(SYSTEMIV_PATH)/lib/resource/bitmapfont.cpp \
$(SYSTEMIV_PATH)/lib/resource/image.cpp \
$(SYSTEMIV_PATH)/lib/resource/resource.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_blueprint_manager.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_filter.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_instance.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_library_2d.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_library_2d_sdl.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_library_3d.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_library_3d_software.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_parameter.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_sample_bank.cpp \
$(SYSTEMIV_PATH)/lib/sound/sound_sample_decoder.cpp \
$(SYSTEMIV_PATH)/lib/sound/soundsystem.cpp \
$(SYSTEMIV_PATH)/lib/sound/soundsystem_interface.cpp \
$(SYSTEMIV_PATH)/lib/tosser/directory.cpp \
$(SYSTEMIV_PATH)/lib/debug_utils.cpp \
$(SYSTEMIV_PATH)/lib/hi_res_time.cpp \
$(SYSTEMIV_PATH)/lib/language_table.cpp \
$(SYSTEMIV_PATH)/lib/preferences.cpp \
$(SYSTEMIV_PATH)/lib/profiler.cpp \
$(SYSTEMIV_PATH)/lib/string_utils.cpp

# Building 

OBJDIR_BASE=.linux-objs$(SUFFIX)
FULL_OBJDIR=$(OBJDIR_BASE)/full

ALL_OBJECTS_1=$(SOURCES:%.cpp=%.o)
ALL_OBJECTS=$(ALL_OBJECTS_1:%.c=%.o)
FULL_OBJECTS=$(ALL_OBJECTS:%.o=$(FULL_OBJDIR)/%.o)

all: defcon.full$(SUFFIX)

$(OBJDIR_BASE)/full/%.o: %.cpp
	@echo Compiling $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR_BASE)/full/%.o: %.c
	@echo Compiling $<
	@mkdir -p $(dir $@)
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

defcon.full$(SUFFIX): $(FULL_OBJECTS)
	@echo -n "Linking... "
	@$(LINK) -o $@ $+ $(LIBS) $(LDFLAGS)
	@echo "Done."

clean:
	rm -f defcon.full$(SUFFIX)
	rm -rf $(OBJDIR_BASE)
	rm -f full$(SUFFIX).dep

FASTDEP = contrib/linux/bin/fastdep

%.dep: TARGET=full

%.dep:
	@test -d $(OBJDIR_BASE) || mkdir $(OBJDIR_BASE)
	@$(FASTDEP) -O $(OBJDIR_BASE)/$(TARGET) $(INCLUDES:%=-I %) --remakedeptarget=$@ $(SOURCES) > $@

-include full$(SUFFIX).dep
