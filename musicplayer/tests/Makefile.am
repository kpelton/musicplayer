check_PROGRAMS = shuffle-test side-queue-test settings-test
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
