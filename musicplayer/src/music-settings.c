#include "music-settings.h"

GSettings *
music_settings_new (void)
{
	return g_settings_new(MUSIC_SETTINGS_SCHEMA_ID);
}
