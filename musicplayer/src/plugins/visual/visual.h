/* real-test.h */

#ifndef _VISUAL_PLUGIN
#define _VISUAL_PLUGIN

#include <glib-object.h>
#include "music-main-window.h"
#include "../music-plugin.h"
#include "player.h"
G_BEGIN_DECLS

#define VISUAL_TYPE_PLUGIN visual_plugin_get_type()

#define VISUAL_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), VISUAL_TYPE_PLUGIN, ViualPlugin))

#define VISUAL_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), VISUAL_TYPE_PLUGIN, VisualPluginClass))

#define VISUAL_IS_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), VISUAL_TYPE_PLUGIN))

#define VISUAL_IS_PLUGIN_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), VISUAL_TYPE_PLUGIN))

#define VISUAL_PLUGIN_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), VISUAL_TYPE_PLUGIN, VisualPluginClass))

typedef struct {
  MusicPlugin parent;
    MusicMainWindow *mw;
    GstElement *goom;
    GstElement *video;
    GstElement *bin;
  
} VisualPlugin;

typedef struct {
  MusicPluginClass parent_class;
} VisualPluginClass;

GType visual_plugin_get_type (void);

GType 
register_music_plugin();

G_END_DECLS

#endif  
