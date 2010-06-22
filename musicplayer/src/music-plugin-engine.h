/* music-plugin-engine.h */

#ifndef _MUSIC_PLUGIN_ENGINE
#define _MUSIC_PLUGIN_ENGINE

#include <glib-object.h>

G_BEGIN_DECLS

#define MUSIC_TYPE_PLUGIN_ENGINE music_plugin_engine_get_type()

#define MUSIC_PLUGIN_ENGINE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_PLUGIN_ENGINE, MusicPluginEngine))

#define MUSIC_PLUGIN_ENGINE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_PLUGIN_ENGINE, MusicPluginEngineClass))

#define MUSIC_IS_PLUGIN_ENGINE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_PLUGIN_ENGINE))

#define MUSIC_IS_PLUGIN_ENGINE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_PLUGIN_ENGINE))

#define MUSIC_PLUGIN_ENGINE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_PLUGIN_ENGINE, MusicPluginEngineClass))

typedef struct _MusicPluginEnginePrivate MusicPluginEnginePrivate;
typedef struct {
  GObject parent;
  MusicPluginEnginePrivate *priv; 
} MusicPluginEngine;

typedef struct {
  GObjectClass parent_class;
} MusicPluginEngineClass;

GType music_plugin_engine_get_type (void);

MusicPluginEngine* music_plugin_engine_new (void);

G_END_DECLS
#endif