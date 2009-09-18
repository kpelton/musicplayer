bin_PROGRAMS = musicplayer

AM_CFLAGS = `pkg-config --cflags --libs gtk+-2.0 gstreamer-0.10 libgnome-2.0 gio-2.0` -O3		
AM_LDFLAGS = `pkg-config --libs gtk+-2.0 gstreamer-0.10 libgnome-2.0 gio-2.0` -O3

musicplayer_SOURCES = music-main-window.c musicseek.c player.c main.c music-queue.c music-volume.c  tag-scanner.c music-queue.h  music-volume.h  music-main-window.h music-seek.h player.h tag-scanner.h  music-store.c music-store.h jump-window.h jump-window.c \
	pl-reader.h  pl-reader.c  xspf-reader.h  xspf-reader.c 

musicplayer_LDADD = $(INTI_LIBS)
