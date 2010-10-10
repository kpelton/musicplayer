/* music-plugin-manager.h */

#ifndef _MUSIC_PLUGIN_MANAGER
#define _MUSIC_PLUGIN_MANAGER

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MUSIC_TYPE_PLUGIN_MANAGER music_plugin_manager_get_type()

#define MUSIC_PLUGIN_MANAGER(obj) \
(G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_PLUGIN_MANAGER, MusicPluginManager))

#define MUSIC_PLUGIN_MANAGER_CLASS(klass) \
(G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_PLUGIN_MANAGER, MusicPluginManagerClass))

#define MUSIC_IS_PLUGIN_MANAGER(obj) \
(G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_PLUGIN_MANAGER))

#define MUSIC_IS_PLUGIN_MANAGER_CLASS(klass) \
(G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_PLUGIN_MANAGER))

#define MUSIC_PLUGIN_MANAGER_GET_CLASS(obj) \
(G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_PLUGIN_MANAGER, MusicPluginManagerClass))

typedef struct _MusicPluginManagerPrivate MusicPluginManagerPrivate;


typedef struct {

	GtkWindow parent;
	MusicPluginManagerPrivate *priv;

} MusicPluginManager;

typedef struct {
	GtkWindowClass parent_class;
} MusicPluginManagerClass;

GType music_plugin_manager_get_type (void);

MusicPluginManager* music_plugin_manager_new (void);

G_END_DECLS

#endif /* _MUSIC_PLUGIN_MANAGER */
