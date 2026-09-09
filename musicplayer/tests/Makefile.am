check_PROGRAMS = shuffle-test side-queue-test settings-test main-context-test playlist-loading-test core-components-test
TESTS = $(check_PROGRAMS)
AM_TESTS_ENVIRONMENT = GSETTINGS_SCHEMA_DIR=$(top_builddir)/data GSETTINGS_BACKEND=memory

shuffle_test_SOURCES = shuffle-test.c \
	$(top_srcdir)/src/shuffle-deck.c \
	$(top_srcdir)/src/shuffle-deck.h
shuffle_test_CPPFLAGS = -I$(top_srcdir)/src
shuffle_test_CFLAGS = $(GLIB_CFLAGS)
shuffle_test_LDADD = $(GLIB_LIBS)

side_queue_test_SOURCES = side-queue-test.c \
	$(top_srcdir)/src/music-side-queue.c \
	$(top_srcdir)/src/music-side-queue.h
side_queue_test_CPPFLAGS = -I$(top_srcdir)/src
side_queue_test_CFLAGS = $(GLIB_CFLAGS)
side_queue_test_LDADD = $(GLIB_LIBS)

settings_test_SOURCES = settings-test.c \
	$(top_srcdir)/src/music-settings.c \
	$(top_srcdir)/src/music-settings.h
settings_test_CPPFLAGS = -I$(top_srcdir)/src
settings_test_CFLAGS = $(GLIB_CFLAGS)
settings_test_LDADD = $(GLIB_LIBS)

main_context_test_SOURCES = main-context-test.c \
	$(top_srcdir)/src/utils.c \
	$(top_srcdir)/src/utils.h
main_context_test_CPPFLAGS = -I$(top_srcdir)/src
main_context_test_CFLAGS = $(GLIB_CFLAGS)
main_context_test_LDADD = $(GLIB_LIBS) $(GIO_LIBS)

playlist_loading_test_SOURCES = playlist-loading-test.c \
	$(top_srcdir)/src/m3u-reader.c \
	$(top_srcdir)/src/m3u-reader.h \
	$(top_srcdir)/src/pl-reader.c \
	$(top_srcdir)/src/pl-reader.h \
	$(top_srcdir)/src/tag-scanner.c \
	$(top_srcdir)/src/tag-scanner.h \
	$(top_srcdir)/src/xspf-reader.c \
	$(top_srcdir)/src/xspf-reader.h
playlist_loading_test_CPPFLAGS = -I$(top_srcdir)/src $(GTK_CFLAGS) $(GST_CFLAGS) $(LIBXML2_CFLAGS)
playlist_loading_test_CFLAGS = $(GLIB_CFLAGS)
playlist_loading_test_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(GST_LIBS) $(LIBXML2_LIBS)

core_components_test_SOURCES = core-components-test.c \
	$(top_srcdir)/src/music-store.c \
	$(top_srcdir)/src/music-store.h \
	$(top_srcdir)/src/player.c \
	$(top_srcdir)/src/player.h \
	$(top_srcdir)/src/plugins/music-plugin.c \
	$(top_srcdir)/src/plugins/music-plugin.h \
	$(top_srcdir)/src/tag-scanner.c \
	$(top_srcdir)/src/tag-scanner.h \
	$(top_srcdir)/src/utils.c \
	$(top_srcdir)/src/utils.h
core_components_test_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/plugins $(GTK_CFLAGS) $(GST_CFLAGS)
core_components_test_CFLAGS = $(GLIB_CFLAGS)
core_components_test_LDADD = $(GLIB_LIBS) $(GTK_LIBS) $(GST_LIBS)
