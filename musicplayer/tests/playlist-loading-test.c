#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "pl-reader.h"
#include "tag-scanner.h"
#include "xspf-reader.h"

static gchar *
write_test_playlist (void)
{
	static const gchar contents[] =
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n"
		"  <trackList>\n"
		"    <track>\n"
		"      <location>file:///music/first.ogg</location>\n"
		"      <title>First Song</title>\n"
		"      <creator>First Artist</creator>\n"
		"      <duration>123456</duration>\n"
		"    </track>\n"
		"    <track>\n"
		"      <location>file:///music/second.ogg</location>\n"
		"      <title>Second Song</title>\n"
		"      <creator>Second Artist</creator>\n"
		"      <duration>654321</duration>\n"
		"    </track>\n"
		"  </trackList>\n"
		"</playlist>\n";
	GError *error = NULL;
	gchar *path;
	gint fd;

	fd = g_file_open_tmp ("musicplayer-playlist-XXXXXX.xspf", &path, &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);

	g_assert_true (g_file_set_contents (path, contents, -1, &error));
	g_assert_no_error (error);

	return path;
}

static void
test_xspf_reader_loads_tracks (void)
{
	XspfReader *reader;
	GList *tracks = NULL;
	metadata *first;
	metadata *second;
	gchar *path;

	path = write_test_playlist ();
	reader = xspf_reader_new ();

	g_assert_true (playlist_reader_read_list (PLAYLIST_READER (reader),
	                                          path,
	                                          &tracks));
	g_assert_cmpuint (g_list_length (tracks), ==, 2);

	first = tracks->data;
	second = tracks->next->data;
	g_assert_cmpstr (first->uri, ==, "file:///music/first.ogg");
	g_assert_cmpstr (first->title, ==, "First Song");
	g_assert_cmpstr (first->artist, ==, "First Artist");
	g_assert_cmpint (first->duration, ==, 123456);
	g_assert_cmpstr (second->uri, ==, "file:///music/second.ogg");
	g_assert_cmpstr (second->title, ==, "Second Song");
	g_assert_cmpstr (second->artist, ==, "Second Artist");
	g_assert_cmpint (second->duration, ==, 654321);

	ts_metadata_list_free (tracks);
	g_list_free (tracks);
	g_object_unref (reader);
	g_remove (path);
	g_free (path);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/playlist-loading/xspf-reader-loads-tracks",
	                test_xspf_reader_loads_tracks);
	return g_test_run ();
}
