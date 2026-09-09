#include <glib.h>
#include <glib/gstdio.h>

#include "music-settings.h"

static void
test_local_schema_path (void)
{
	const gchar *old_schema_dir = g_getenv ("GSETTINGS_SCHEMA_DIR");
	gchar *saved_schema_dir = g_strdup (old_schema_dir);
	gchar *root = g_dir_make_tmp ("musicplayer-settings-XXXXXX", NULL);
	gchar *src_dir = g_build_filename (root, "src", NULL);
	gchar *data_dir = g_build_filename (root, "data", NULL);
	gchar *compiled_schema = g_build_filename (data_dir, "gschemas.compiled", NULL);
	gchar *program = g_build_filename (src_dir, "musicplayer", NULL);

	g_assert_nonnull (root);
	g_assert_cmpint (g_mkdir_with_parents (src_dir, 0700), ==, 0);
	g_assert_cmpint (g_mkdir_with_parents (data_dir, 0700), ==, 0);
	g_assert_true (g_file_set_contents (compiled_schema, "", 0, NULL));
	g_unsetenv ("GSETTINGS_SCHEMA_DIR");

	music_settings_init (program);

	g_assert_cmpstr (g_getenv ("GSETTINGS_SCHEMA_DIR"), ==, data_dir);

	if (saved_schema_dir)
		g_setenv ("GSETTINGS_SCHEMA_DIR", saved_schema_dir, TRUE);
	else
		g_unsetenv ("GSETTINGS_SCHEMA_DIR");
	g_remove (compiled_schema);
	g_rmdir (data_dir);
	g_rmdir (src_dir);
	g_rmdir (root);
	g_free (program);
	g_free (compiled_schema);
	g_free (data_dir);
	g_free (src_dir);
	g_free (root);
	g_free (saved_schema_dir);
}

static void
test_default_settings (void)
{
	GSettings *settings = music_settings_new();

	g_assert_false(g_settings_get_boolean(settings, "repeat"));
	g_assert_false(g_settings_get_boolean(settings, "shuffle"));
	g_assert_cmpfloat(g_settings_get_double(settings, "volume"), ==, 1.0);
	g_assert_cmpint(g_settings_get_int(settings, "window-width"), ==, 350);
	g_assert_cmpint(g_settings_get_int(settings, "window-height"), ==, 250);
	g_assert_true(g_settings_get_boolean(settings, "expanded"));

	g_object_unref(settings);
}

static void
test_settings_round_trip (void)
{
	GSettings *settings = music_settings_new();

	g_settings_set_boolean(settings, "shuffle", TRUE);
	g_settings_set_double(settings, "volume", 0.42);
	g_settings_set_int(settings, "window-width", 800);

	g_assert_true(g_settings_get_boolean(settings, "shuffle"));
	g_assert_cmpfloat(g_settings_get_double(settings, "volume"), ==, 0.42);
	g_assert_cmpint(g_settings_get_int(settings, "window-width"), ==, 800);

	g_object_unref(settings);
}

int
main (int argc, char **argv)
{
	g_setenv("GSETTINGS_BACKEND", "memory", TRUE);
	g_test_init(&argc, &argv, NULL);
	g_test_add_func("/settings/local-schema-path", test_local_schema_path);
	g_test_add_func("/settings/defaults", test_default_settings);
	g_test_add_func("/settings/round-trip", test_settings_round_trip);
	return g_test_run();
}
