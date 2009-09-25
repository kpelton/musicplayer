#ifndef PLUGIN_ENGINE_H
#define PLUGIN_ENGINE_H
#include <glib.h>
#include <glib-object.h>
#include "music-main-window.h"
typedef struct _MusicPluginInfo MusicPluginInfo;
typedef struct{
    
    gchar        *name;
	gchar        *desc;
	gchar        **authors;
	gchar        *copyright;
	gchar        *website;
}MusicPluginDetails;

gboolean	 music_plugins_engine_init 		(MusicMainWindow *mainwindow);
#endif