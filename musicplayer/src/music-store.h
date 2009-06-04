/* music-store.h */

#ifndef _MUSIC_STORE
#define _MUSIC_STORE

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MUSIC_TYPE_STORE music_store_get_type()

#define MUSIC_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), MUSIC_TYPE_STORE, MusicStore))

#define MUSIC_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), MUSIC_TYPE_STORE, MusicStoreClass))

#define MUSIC_IS_STORE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MUSIC_TYPE_STORE))

#define MUSIC_IS_STORE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), MUSIC_TYPE_STORE))

#define MUSIC_STORE_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), MUSIC_TYPE_STORE, MusicStoreClass))

typedef struct {
  GtkTreeModelFilter parent;
} MusicStore;

typedef struct {
  GtkTreeModelFilterClass parent_class;
} MusicStoreClass;

GType music_store_get_type (void);

GtkTreeModel* music_store_new (void);
GtkTreeModel* music_store_new_with_model (GtkTreeModel *model,GtkTreePath  *root);

G_END_DECLS

#endif /* _MUSIC_STORE */
