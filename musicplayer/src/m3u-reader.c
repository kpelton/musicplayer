/* m3u-reader.c */

#include "m3u-reader.h"
#include "pl-reader.h"
#include "tag-scanner.h"
#include <gio/gio.h>
#include <glib.h>
#include <string.h>

#define MIME_TYPE "audio/x-mpegurl"

static void
m3u_reader_playlist_interface_init(PlaylistReaderInterface *iface);

static const gchar *
m3u_reader_mime_type(PlaylistReader *plist);

struct _M3uReaderPrivate
{
};

G_DEFINE_TYPE_WITH_CODE (M3uReader, m3u_reader, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (PLAYLIST_TYPE_READER,
                                                m3u_reader_playlist_interface_init));

static gboolean
m3u_reader_read_list(PlaylistReader *plist,
                     gchar *location,
                     GList **list)
{
	gchar *buffer = NULL;
	gchar **lines = NULL;
	gsize count = 0;
	GFile *playlist_file;
	GFile *base_dir;
	GFile *track_file;
	GFileInfo *info;
	GError *error = NULL;
	metadata *md;
	gchar *uri;
	gint i;

	(void) plist;
	playlist_file = g_file_new_for_commandline_arg (location);
	if (!g_file_load_contents (playlist_file, NULL, &buffer, &count,
	                           NULL, &error))
	{
		if (error != NULL)
		{
			g_warning ("Unable to read M3U playlist: %s", error->message);
			g_error_free (error);
		}
		g_object_unref (playlist_file);
		return FALSE;
	}

	base_dir = g_file_get_parent (playlist_file);
	lines = g_strsplit (buffer, "\n", -1);
	for (i = 0; lines[i] != NULL; i++)
	{
		gchar *line = g_strstrip (lines[i]);

		if (line[0] == '\0' || line[0] == '#')
			continue;

		if (g_path_is_absolute (line) || strstr (line, "://") != NULL)
			track_file = g_file_new_for_commandline_arg (line);
		else if (base_dir != NULL)
			track_file = g_file_resolve_relative_path (base_dir, line);
		else
			track_file = g_file_new_for_commandline_arg (line);

		info = g_file_query_info (track_file,
		                          G_FILE_ATTRIBUTE_STANDARD_TYPE,
		                          0, NULL, &error);
		if (info == NULL)
		{
			if (error != NULL)
			{
				g_error_free (error);
				error = NULL;
			}
			g_object_unref (track_file);
			continue;
		}

		uri = g_file_get_uri (track_file);
		/* M3U carries paths, not metadata. Avoid synchronously running the
		 * GStreamer tag scanner while the file chooser is importing a list. */
		md = ts_metadata_new ();
		md->uri = g_strdup (uri);
		*list = g_list_append (*list, md);

		g_free (uri);
		g_object_unref (info);
		g_object_unref (track_file);
	}

	g_strfreev (lines);
	g_free (buffer);
	if (base_dir != NULL)
		g_object_unref (base_dir);
	g_object_unref (playlist_file);
	return TRUE;
}

static gboolean
m3u_reader_write_list(PlaylistReader *plist,
                      gchar *location,
                      GList *list)
{
	GString *contents;
	GList *node;
	GError *error = NULL;
	gboolean result;

	(void) plist;
	contents = g_string_new ("#EXTM3U\n");
	for (node = list; node != NULL; node = node->next)
	{
		metadata *track = node->data;
		GFile *file;
		gchar *path;

		if (track == NULL || track->uri == NULL)
			continue;

		file = g_file_new_for_commandline_arg (track->uri);
		path = g_file_get_path (file);
		g_string_append (contents, path != NULL ? path : track->uri);
		g_string_append_c (contents, '\n');
		g_free (path);
		g_object_unref (file);
	}

	result = g_file_set_contents (location, contents->str, contents->len, &error);
	if (error != NULL)
	{
		g_warning ("Unable to write M3U playlist: %s", error->message);
		g_error_free (error);
	}
	g_string_free (contents, TRUE);

	/* Playlist writers own metadata entries, but not the list container. */
	g_list_foreach (list, (GFunc) ts_metadata_free, NULL);
	return result;
}

static void
m3u_reader_playlist_interface_init(PlaylistReaderInterface *iface)
{
	iface->playlist_reader_read_list = m3u_reader_read_list;
	iface->playlist_reader_write_list = m3u_reader_write_list;
	iface->playlist_reader_mime_supported = m3u_reader_mime_type;
}

static const gchar *
m3u_reader_mime_type(PlaylistReader *plist)
{
	(void) plist;
	return MIME_TYPE;
}

static void
m3u_reader_finalize (GObject *object)
{
	G_OBJECT_CLASS (m3u_reader_parent_class)->finalize (object);
}

static void
m3u_reader_class_init (M3uReaderClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = m3u_reader_finalize;
}

static void
m3u_reader_init (M3uReader *self)
{
	(void) self;
}

M3uReader*
m3u_reader_new (void)
{
	return g_object_new (M3U_TYPE_READER, NULL);
}
