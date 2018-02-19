/* music-plugin-engine.c */

#include "music-plugin-engine.h"
#include <gtk/gtk.h>
#include <gio/gio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#include "music-main-window.h"
#include "plugin-engine.h"
#endif

G_DEFINE_TYPE (MusicPluginEngine, music_plugin_engine, MUSIC_TYPE_PLUGIN_ENGINE)

#define GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), MUSIC_TYPE_PLUGIN_ENGINE, MusicPluginEnginePrivate))


static gboolean 
load_all (MusicMainWindow * mainwindow);

struct _MusicPluginEnginePrivate {
	int dummy;
	GHashTable *music_plugins;

};

static void
music_plugin_engine_get_property (GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
music_plugin_engine_set_property (GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec)
{
	switch (property_id) {
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}

static void
music_plugin_engine_dispose (GObject *object)
{
	G_OBJECT_CLASS (music_plugin_engine_parent_class)->dispose (object);
}

static void
music_plugin_engine_finalize (GObject *object)
{
	G_OBJECT_CLASS (music_plugin_engine_parent_class)->finalize (object);
}

static void
music_plugin_engine_class_init (MusicPluginEngineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MusicPluginEnginePrivate));

	object_class->get_property = music_plugin_engine_get_property;
	object_class->set_property = music_plugin_engine_set_property;
	object_class->dispose = music_plugin_engine_dispose;
	object_class->finalize = music_plugin_engine_finalize;
}

static void
music_plugin_engine_init (MusicPluginEngine *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, MUSIC_TYPE_PLUGIN_ENGINE, 
	                                          MusicPluginEnginePrivate);
	self->priv->music_plugins = g_hash_table_new (g_str_hash, g_str_equal);//, NULL,NULL);

}

static
gboolean load_all (MusicMainWindow * mainwindow)
{
	GList *list=g_list_alloc();
	GList *list1;
	GList *listbeg;
	listbeg = list;
#ifdef TOTEM_RUN_IN_SOURCE_TREE
	music_plugins_find_plugins("plugins",&list);
#else
	music_plugins_find_plugins(PLUGIN_DIR,&list);
#endif
	list = listbeg;

	for(list1 = list->next; list1!=NULL; list1 = list1->next)
	{   printf("file to load: %s\n",(gchar *)list1->data);

		//need to check if it has a gconf entry to save it
		//load_file(list1->data,mainwindow);
		g_free(list1->data);
	}

	g_list_free(list);
	return TRUE;
}



MusicPluginEngine*
music_plugin_engine_new (void)
{
	return g_object_new (MUSIC_TYPE_PLUGIN_ENGINE, NULL);
}
