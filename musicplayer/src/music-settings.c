#include "music-settings.h"

void
music_settings_init (const gchar *program_path)
{
	const gchar *schema_dir = g_getenv ("GSETTINGS_SCHEMA_DIR");
	gchar *absolute_program;
	gchar *program_dir;
	gchar *schema_path;
	gchar *local_schema_dir;
	gchar *compiled_schema;

	if (schema_dir && *schema_dir)
		return;
	if (program_path == NULL || *program_path == '\0')
		return;

	absolute_program = g_canonicalize_filename (program_path, NULL);
	program_dir = g_path_get_dirname (absolute_program);
	schema_path = g_build_filename (program_dir, "..", "data", NULL);
	local_schema_dir = g_canonicalize_filename (schema_path, NULL);
	compiled_schema = g_build_filename (local_schema_dir, "gschemas.compiled", NULL);

	if (g_file_test (compiled_schema, G_FILE_TEST_IS_REGULAR))
		g_setenv ("GSETTINGS_SCHEMA_DIR", local_schema_dir, FALSE);

	g_free (compiled_schema);
	g_free (local_schema_dir);
	g_free (schema_path);
	g_free (program_dir);
	g_free (absolute_program);
}

GSettings *
music_settings_new (void)
{
	return g_settings_new(MUSIC_SETTINGS_SCHEMA_ID);
}
