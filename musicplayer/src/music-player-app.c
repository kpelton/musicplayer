/* music-player-app.c */

#include "music-player-app.h"
#include "utils.h"
#include "player.h"
#include "music-queue.h"
#include "music-main-window.h"
#include "xspf-reader.h"
#include "pl-reader.h"
#include "plugin-engine.h"
#include "music-plugin-manager.h"
#include <gconf/gconf-client.h>
#include <unique/unique.h>
#include <string.h>


G_DEFINE_TYPE (MusicPlayerApp, music_player_app, MUSIC_TYPE_PLAYER_APP)

#define GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), MUSIC_TYPE_PLAYER_APP, MusicPlayerAppPrivate))



struct _MusicPlayerAppPrivate {
	int dummy;
	GtkWidget *mainwindow;
	UniqueApp* app;
};

static void
music_player_app_get_property (GObject *object, guint property_id,
                               GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
music_player_app_set_property (GObject *object, guint property_id,
                               const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
music_player_app_dispose (GObject *object)
{
	G_OBJECT_CLASS (music_player_app_parent_class)->dispose (object);
}

static void
music_player_app_finalize (GObject *object)
{
	G_OBJECT_CLASS (music_player_app_parent_class)->finalize (object);
}

static void
music_player_app_class_init (MusicPlayerAppClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MusicPlayerAppPrivate));

	object_class->get_property = music_player_app_get_property;
	object_class->set_property = music_player_app_set_property;
	object_class->dispose = music_player_app_dispose;
	object_class->finalize = music_player_app_finalize;
}

static void
music_player_app_init (MusicPlayerApp *self)
{
	self->priv =  G_TYPE_INSTANCE_GET_PRIVATE (self, MUSIC_TYPE_PLAYER_APP, 
	                                           MusicPlayerApp);
}

MusicPlayerApp*
music_player_app_new (void)
{
	return g_object_new (MUSIC_TYPE_PLAYER_APP, NULL);
}
