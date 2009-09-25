/* plugin.h */

#ifndef _MUSIC_PLUGIN
#define _MUSIC_PLUGIN

#include <glib-object.h>
#include "music-main-window.h"
#include "plugin-engine.h"

G_BEGIN_DECLS

#define MUSIC_TYPE_PLUGIN music_plugin_get_type()

#define MUSIC_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_PLUGIN, MusicPlugin))

#define MUSIC_IS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_PLUGIN))

#define MUSIC_PLUGIN_GET_INTERFACE(inst) \
(G_TYPE_INSTANCE_GET_INTERFACE ((inst), MUSIC_TYPE_PLUGIN, MusicPluginInterface))


typedef struct _MusicPlugin MusicPlugin;
typedef struct _MusicPluginInterface MusicPluginInterface;

struct _MusicPluginInterface {
    
  GTypeInterface parent_iface;

    gboolean (*music_plugin_deactivate) ( MusicPlugin *self);
    gboolean (*music_plugin_activate) ( MusicPlugin *self,MusicMainWindow *mw);
    MusicPluginDetails * (*music_plugin_get_info) ( MusicPlugin *self);
};

GType
music_plugin_get_type (void);

gboolean 
music_plugin_activate ( MusicPlugin *self,MusicMainWindow *mw);

gboolean 
music_plugin_deactivate ( MusicPlugin *self);

MusicPluginDetails*
music_plugin_get_info( MusicPlugin *self);
G_END_DECLS

#endif /* plugin.h */
