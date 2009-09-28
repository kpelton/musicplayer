#ifndef PLUGIN_ENGINE_H
#define PLUGIN_ENGINE_H
#include <glib.h>
#include <glib-object.h>
#include "music-main-window.h"
#include "plugins/music-plugin.h"

typedef struct MusicPluginInfo{
	gchar        *location;
	GModule  *module;
    GType   type;
    MusicPluginDetails *details;
    MusicPluginDetails * (*get_details_func)();
	MusicPlugin   *plugin;

	gboolean     builtin;
	gboolean     active;
	gboolean     visible;
	guint        active_notification_id;
	guint        visible_notification_id;
}MusicPluginInfo;

gboolean	 
music_plugins_engine_init 		(MusicMainWindow *mainwindow);
GList *
music_plugins_get_list();
gboolean
music_plugins_engine_activate_plugin(MusicPluginInfo *info);
gboolean
music_plugins_engine_deactivate_plugin(MusicPluginInfo *info);
gboolean
music_plugins_engine_plugin_is_active(MusicPluginInfo *info);

#endif