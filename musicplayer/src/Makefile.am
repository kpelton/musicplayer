#SUBDIRS = plugins
bin_PROGRAMS = musicplayer 

common_defines =-DPLUGIN_DIR=\""$(libdir)/musicplayer/plugins"\"

musicplayer_CPPFLAGS =$(common_defines) $(AM_CPPFLAGS)

musicplayer_CFLAGS =   $(GST_CFLAGS) $(GLIB_CFLAGS) $(GTK_CFLAGS) $(WARN_CFLAGS)	
musicplayer_LDADD =  $(GLIB_LIBS) $(INTI_LIBS) $(GST_LIBS) $(AM_LDFLAGS)	




musicplayer_SOURCES = music-main-window.c musicseek.c player.c main.c music-queue.c music-volume.c  tag-scanner.c music-queue.h  music-volume.h  music-main-window.h music-seek.h player.h tag-scanner.h  music-store.c music-store.h jump-window.h jump-window.c pl-reader.h  pl-reader.c  xspf-reader.h  xspf-reader.c m3u-reader.h  m3u-reader.c \
	plugin-engine.c  plugin-engine.h utils.c  utils.h plugins/music-plugin.h plugins/music-plugin.c

SUBDIRS = \
	plugins


