#include <glib.h>
#include <glib/gstdio.h>
#include <unistd.h>

#include "pl-reader.h"
#include "tag-scanner.h"
#include "m3u-reader.h"
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

static void
test_xspf_writer_exports_tracks (void)
{
	GError *error = NULL;
	GList *tracks = NULL;
	GList *loaded = NULL;
	metadata *first;
	metadata *second;
	XspfReader *writer;
	XspfReader *reader;
	gchar *path;
	gint fd;

	fd = g_file_open_tmp ("musicplayer-export-XXXXXX.xspf", &path, &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);

	first = ts_metadata_new ();
	first->uri = g_strdup ("file:///music/export-first.ogg");
	first->title = g_strdup ("Export First");
	first->artist = g_strdup ("Export Artist");
	first->duration = 111000;
	second = ts_metadata_new ();
	second->uri = g_strdup ("file:///music/export-second.flac");
	second->title = g_strdup ("Export Second");
	second->artist = g_strdup ("Another Artist");
	second->duration = 222000;
	tracks = g_list_append (tracks, first);
	tracks = g_list_append (tracks, second);

	writer = xspf_reader_new ();
	g_assert_true (playlist_reader_write_list (PLAYLIST_READER (writer),
	                                           path,
	                                           tracks));
	g_object_unref (writer);
	g_list_free (tracks);
	tracks = NULL;

	reader = xspf_reader_new ();
	g_assert_true (playlist_reader_read_list (PLAYLIST_READER (reader),
	                                          path,
	                                          &loaded));
	g_assert_cmpuint (g_list_length (loaded), ==, 2);

	first = loaded->data;
	second = loaded->next->data;
	g_assert_cmpstr (first->uri, ==, "file:///music/export-first.ogg");
	g_assert_cmpstr (first->title, ==, "Export First");
	g_assert_cmpstr (first->artist, ==, "Export Artist");
	g_assert_cmpint (first->duration, ==, 111000);
	g_assert_cmpstr (second->uri, ==, "file:///music/export-second.flac");
	g_assert_cmpstr (second->title, ==, "Export Second");
	g_assert_cmpstr (second->artist, ==, "Another Artist");
	g_assert_cmpint (second->duration, ==, 222000);

	ts_metadata_list_free (loaded);
	g_list_free (loaded);
	g_object_unref (reader);
	g_remove (path);
	g_free (path);
}

static void
test_m3u_reader_loads_empty_playlist (void)
{
	GError *error = NULL;
	GList *tracks = NULL;
	M3uReader *reader;
	gchar *path;
	gint fd;

	fd = g_file_open_tmp ("musicplayer-playlist-XXXXXX.m3u", &path, &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);
	g_assert_true (g_file_set_contents (path, "#EXTM3U\n", -1, &error));
	g_assert_no_error (error);

	reader = m3u_reader_new ();
	g_assert_cmpstr (playlist_reader_mime_supported (PLAYLIST_READER (reader)),
	                ==, "audio/x-mpegurl");
	g_assert_true (playlist_reader_read_list (PLAYLIST_READER (reader),
	                                         path,
	                                         &tracks));
	g_assert_null (tracks);

	g_object_unref (reader);
	g_remove (path);
	g_free (path);
}

static void
test_m3u_writer_exports_paths (void)
{
	GError *error = NULL;
	GList *tracks = NULL;
	metadata *first;
	metadata *second;
	M3uReader *writer;
	gchar *path;
	gchar *contents = NULL;
	gsize length;
	gint fd;

	fd = g_file_open_tmp ("musicplayer-export-XXXXXX.m3u", &path, &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);

	first = ts_metadata_new ();
	first->uri = g_strdup ("file:///music/export-first.ogg");
	second = ts_metadata_new ();
	second->uri = g_strdup ("file:///music/export-second.flac");
	tracks = g_list_append (tracks, first);
	tracks = g_list_append (tracks, second);

	writer = m3u_reader_new ();
	g_assert_true (playlist_reader_write_list (PLAYLIST_READER (writer),
	                                           path,
	                                           tracks));
	g_object_unref (writer);
	g_list_free (tracks);
	tracks = NULL;
	g_assert_true (g_file_get_contents (path, &contents, &length, &error));
	g_assert_no_error (error);
	g_assert_cmpstr (contents, ==,
	                "#EXTM3U\n"
	                "/music/export-first.ogg\n"
	                "/music/export-second.flac\n");

	g_free (contents);
	g_remove (path);
	g_free (path);
}

static void
test_m3u_reader_loads_absolute_paths (void)
{
	GError *error = NULL;
	GList *tracks = NULL;
	M3uReader *reader;
	gchar *playlist_path;
	gchar *first_path;
	gchar *second_path;
	gchar *first_uri;
	gchar *second_uri;
	gchar *contents;
	gint fd;

	fd = g_file_open_tmp ("musicplayer-m3u-first-XXXXXX.ogg", &first_path,
	                      &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);
	fd = g_file_open_tmp ("musicplayer-m3u-second-XXXXXX.flac", &second_path,
	                      &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);
	fd = g_file_open_tmp ("musicplayer-m3u-XXXXXX.m3u", &playlist_path,
	                      &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);

	contents = g_strdup_printf ("#EXTM3U\n%s\n%s\n",
	                           first_path, second_path);
	g_assert_true (g_file_set_contents (playlist_path, contents, -1, &error));
	g_assert_no_error (error);
	g_free (contents);

	reader = m3u_reader_new ();
	g_assert_true (playlist_reader_read_list (PLAYLIST_READER (reader),
	                                         playlist_path,
	                                         &tracks));
	g_assert_cmpuint (g_list_length (tracks), ==, 2);
	first_uri = g_filename_to_uri (first_path, NULL, &error);
	g_assert_no_error (error);
	second_uri = g_filename_to_uri (second_path, NULL, &error);
	g_assert_no_error (error);
	g_assert_cmpstr (((metadata *) tracks->data)->uri, ==, first_uri);
	g_assert_cmpstr (((metadata *) tracks->next->data)->uri, ==, second_uri);

	ts_metadata_list_free (tracks);
	g_list_free (tracks);
	g_free (first_uri);
	g_free (second_uri);
	g_object_unref (reader);
	g_remove (playlist_path);
	g_remove (first_path);
	g_remove (second_path);
	g_free (playlist_path);
	g_free (first_path);
	g_free (second_path);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/playlist-loading/xspf-reader-loads-tracks",
	                test_xspf_reader_loads_tracks);
	g_test_add_func ("/playlist-loading/xspf-writer-exports-tracks",
	                test_xspf_writer_exports_tracks);
	g_test_add_func ("/playlist-loading/m3u-reader-loads-empty-playlist",
	                test_m3u_reader_loads_empty_playlist);
	g_test_add_func ("/playlist-loading/m3u-writer-exports-paths",
	                test_m3u_writer_exports_paths);
	g_test_add_func ("/playlist-loading/m3u-reader-loads-absolute-paths",
	                test_m3u_reader_loads_absolute_paths);
	return g_test_run ();
}
