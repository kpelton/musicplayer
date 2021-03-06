/* plugin.h */

#ifndef _MUSIC_PLUGIN
#define _MUSIC_PLUGIN

#include <glib-object.h>
#include "music-main-window.h"


G_BEGIN_DECLS

#define MUSIC_TYPE_PLUGIN music_plugin_get_type()


#define MUSIC_PLUGIN(obj) \
(G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_PLUGIN, MusicPlugin))

#define MUSIC_PLUGIN_CLASS(klass) \
(G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_PLUGIN, MusicPluginClass))

#define MUSIC_IS_PLUGIN(obj) \
(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_PLUGIN))

#define MUSIC_IS_PLUGIN_CLASS(klass) \
(G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_PLUGIN))

#define MUSIC_PLUGIN_GET_CLASS(obj) \
(G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_PLUGIN, MusicPluginClass))

typedef struct MusicPluginDetails{

	gchar        *name;
	gchar        *desc;
	gchar        **authors;
	gchar        *copyright;
	gchar        *website;
	gboolean     is_configurable;

}MusicPluginDetails;

typedef struct MusicPlugin{
	GObject parent;

} MusicPlugin;

typedef struct {
	GObjectClass parent_class;
	gboolean (*music_plugin_deactivate) ( MusicPlugin *self);
	gboolean (*music_plugin_activate) ( MusicPlugin *self,MusicMainWindow *mw);
	GtkWidget *(*music_plugin_get_config_window)(MusicPlugin *self);
} MusicPluginClass;


GType
music_plugin_get_type (void);

gboolean 
music_plugin_activate ( MusicPlugin *self,MusicMainWindow *mw);

gboolean 
music_plugin_deactivate ( MusicPlugin *self);

GtkWidget *
music_plugin_get_config_window(MusicPlugin *self);


G_END_DECLS

#endif /* plugin.h */
