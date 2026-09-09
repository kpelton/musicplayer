#include <glib.h>

#include "music-settings.h"

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
	g_test_add_func("/settings/defaults", test_default_settings);
	g_test_add_func("/settings/round-trip", test_settings_round_trip);
	return g_test_run();
}
