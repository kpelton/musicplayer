#include <glib.h>
#include <glib/gstdio.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "music-store.h"
#include "player.h"
#include "plugins/music-plugin.h"
#include "tag-scanner.h"
#include "utils.h"

typedef struct {
	MusicPlugin parent_instance;
} TestPlugin;

typedef struct {
	MusicPluginClass parent_class;
} TestPluginClass;

G_DEFINE_TYPE (TestPlugin, test_plugin, MUSIC_TYPE_PLUGIN)

static gboolean
test_plugin_activate (MusicPlugin *plugin, MusicMainWindow *window)
{
	return plugin != NULL && window == NULL;
}

static gboolean
test_plugin_deactivate (MusicPlugin *plugin)
{
	return plugin != NULL;
}

static GtkWidget *
test_plugin_config (MusicPlugin *plugin)
{
	(void) plugin;
	return NULL;
}

static void
test_plugin_class_init (TestPluginClass *klass)
{
	MusicPluginClass *plugin_class = MUSIC_PLUGIN_CLASS (klass);

	plugin_class->music_plugin_activate = test_plugin_activate;
	plugin_class->music_plugin_deactivate = test_plugin_deactivate;
	plugin_class->music_plugin_get_config_window = test_plugin_config;
}

static void
test_plugin_init (TestPlugin *self)
{
}

static void
test_parse_file_name (void)
{
	GError *error = NULL;
	gchar *path;
	gchar *name;
	gint fd;
	GFile *file;

	fd = g_file_open_tmp ("musicplayer-track-XXXXXX.mp3", &path, &error);
	g_assert_no_error (error);
	g_assert_cmpint (fd, >=, 0);
	close (fd);

	file = g_file_new_for_path (path);
	name = parse_file_name (file);
	g_assert_nonnull (name);
	g_assert_true (g_str_has_prefix (name, "musicplayer-track-"));
	g_assert_false (g_str_has_suffix (name, ".mp3"));

	g_free (name);
	g_object_unref (file);
	g_remove (path);
	g_free (path);
}

static void
test_metadata_copy (void)
{
	metadata source = { 0 };
	metadata *copy = ts_metadata_new ();

	source.uri = "file:///song.ogg";
	source.title = "Song";
	source.artist = "Artist";
	source.album = "Album";
	source.genre = "Genre";
	source.codec = "Vorbis";
	source.duration = 987654;
	ts_metadata_copy (&source, copy);

	g_assert_cmpstr (copy->uri, ==, source.uri);
	g_assert_cmpstr (copy->title, ==, source.title);
	g_assert_cmpstr (copy->artist, ==, source.artist);
	g_assert_cmpstr (copy->album, ==, source.album);
	g_assert_cmpstr (copy->genre, ==, source.genre);
	g_assert_cmpstr (copy->codec, ==, source.codec);
	g_assert_cmpint (copy->duration, ==, source.duration);

	ts_metadata_free (copy);
}

static void
test_music_store_wraps_model (void)
{
	GtkListStore *source;
	GtkTreeModel *store;
	GtkTreeIter iter;
	gchar *value = NULL;
	guint rows = 0;

	source = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_list_store_append (source, &iter);
	gtk_list_store_set (source, &iter, 0, "first", -1);
	gtk_list_store_append (source, &iter);
	gtk_list_store_set (source, &iter, 0, "second", -1);

	store = music_store_new_with_model (GTK_TREE_MODEL (source), NULL);
	g_assert_true (gtk_tree_model_get_iter_first (store, &iter));
	do
	{
		gtk_tree_model_get (store, &iter, 0, &value, -1);
		g_assert_cmpstr (value, ==, rows == 0 ? "first" : "second");
		g_free (value);
		value = NULL;
		rows++;
	} while (gtk_tree_model_iter_next (store, &iter));

	g_assert_cmpuint (rows, ==, 2);
	g_object_unref (store);
	g_object_unref (source);
}

static void
test_player_volume_round_trip (void)
{
	GstElement *play;
	GsPlayer player = { 0 };

	play = gst_element_factory_make ("playbin", "test-playbin");
	if (play == NULL)
	{
		g_test_skip ("GStreamer playbin element is unavailable");
		return;
	}
	player.play = play;

	gs_Set_Volume (&player, 0.37);
	g_assert_cmpfloat (gs_Get_Volume (&player), ==, 0.37);
	gst_object_unref (play);
}

static void
test_plugin_virtual_dispatch (void)
{
	MusicPlugin *plugin = MUSIC_PLUGIN (g_object_new (test_plugin_get_type (), NULL));

	g_assert_true (music_plugin_activate (plugin, NULL));
	g_assert_true (music_plugin_deactivate (plugin));
	g_assert_null (music_plugin_get_config_window (plugin));
	g_object_unref (plugin);
	g_assert_false (music_plugin_activate (NULL, NULL));
	g_assert_false (music_plugin_deactivate (NULL));
	g_assert_null (music_plugin_get_config_window (NULL));
}

int
main (int argc, char **argv)
{
	gst_init (&argc, &argv);
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/utils/parse-file-name", test_parse_file_name);
	g_test_add_func ("/tag-scanner/metadata-copy", test_metadata_copy);
	g_test_add_func ("/music-store/wraps-model", test_music_store_wraps_model);
	g_test_add_func ("/player/volume-round-trip", test_player_volume_round_trip);
	g_test_add_func ("/plugin/virtual-dispatch", test_plugin_virtual_dispatch);
	return g_test_run ();
}
