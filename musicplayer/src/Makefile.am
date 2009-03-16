bin_PROGRAMS = musicplayer

AM_CFLAGS = `pkg-config --cflags --libs gtk+-2.0 gstreamer-0.10 libgnome-2.0` -g		
AM_LDFLAGS = `pkg-config --libs gtk+-2.0 gstreamer-0.10 libgnome-2.0` -g



musicplayer_SOURCES = interface.c music-main-window.c musicseek.c player.c main.c music-queue.c music-volume.c  plist-reader.c tag-scanner.c interface.h music-queue.h  music-volume.h  plist-reader.h music-main-window.h music-seek.h player.h tag-scanner.h

musicplayer_LDADD = $(INTI_LIBS)