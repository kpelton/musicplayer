/* music-store.c */

#include "music-store.h"

static void
music_store_drag_dest_init (GtkTreeDragDestIface *iface);

static gboolean
music_store_row_drop_possible (GtkTreeDragDest  *drag_dest,
                               GtkTreePath      *dest_path,
	            			  GtkSelectionData *selection_data);
static gboolean
music_store_drag_data_received (GtkTreeDragDest   *drag_dest,
                                   GtkTreePath       *dest,
                                   GtkSelectionData  *selection_data);

G_DEFINE_TYPE_WITH_CODE (MusicStore, music_store, GTK_TYPE_TREE_MODEL_FILTER,
                            G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_DEST,
					        music_store_drag_dest_init))

#define GET_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MUSIC_TYPE_STORE, MusicStorePrivate))

typedef struct _MusicStorePrivate MusicStorePrivate;

struct _MusicStorePrivate {
    int dummy;
};

static void
music_store_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
music_store_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
music_store_dispose (GObject *object)
{
  G_OBJECT_CLASS (music_store_parent_class)->dispose (object);
}

static void
music_store_finalize (GObject *object)
{
  G_OBJECT_CLASS (music_store_parent_class)->finalize (object);
}

static void
music_store_class_init (MusicStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MusicStorePrivate));

  object_class->get_property = music_store_get_property;
  object_class->set_property = music_store_set_property;
  object_class->dispose = music_store_dispose;
  object_class->finalize = music_store_finalize;
}


static gboolean
music_store_row_drop_possible (GtkTreeDragDest  *drag_dest_parent,
                               GtkTreePath      *dest_path,
	            			  GtkSelectionData *selection_data)
{

gint *indices;
GtkTreeDragDest  *drag_dest = GTK_TREE_DRAG_DEST(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(drag_dest_parent)));
indices = gtk_tree_path_get_indices (dest_path);

  if (indices[0] <= g_sequence_get_length (GTK_LIST_STORE (drag_dest)->seq))
    return TRUE;

    return FALSE;
    

}
static gboolean
music_store_drag_data_received (GtkTreeDragDest   *drag_dest_parent,
                                   GtkTreePath       *dest,
                                   GtkSelectionData  *selection_data)
{
    GtkTreeDragDest  *drag_dest = GTK_TREE_DRAG_DEST(gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(drag_dest_parent)));
    GtkTreeIter src_iter;
    GtkTreeIter dest_iter;
    GtkTreePath *prev;
    GtkTreePath       *src;
    gboolean retval=FALSE;
    GtkTreeModel *src_model = GTK_TREE_MODEL(drag_dest);


    if(gtk_tree_get_row_drag_data (selection_data,
                                   &src_model,
                                   &src))
    {

        gtk_tree_model_get_iter (src_model,
                                    &src_iter,
                                    src);

        gtk_tree_model_get_iter (src_model, &dest_iter, dest);


        retval = TRUE;

        if(gtk_tree_path_is_descendant         (src,
                                                dest))
        {

            gtk_list_store_move_after (GTK_LIST_STORE(drag_dest),&src_iter,&dest_iter);
        }
        else
        {
            gtk_list_store_move_before (GTK_LIST_STORE(drag_dest),&src_iter,&dest_iter);
        }
//            gtk_tree_model_row_changed (drag_dest_parent, dest, &dest_iter);
            gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(drag_dest_parent));  
        }
    
                                 
 
    return FALSE;

}
static void
music_store_drag_dest_init (GtkTreeDragDestIface *iface)
{
  iface->drag_data_received = music_store_drag_data_received;
  iface->row_drop_possible = music_store_row_drop_possible;
}




static void
music_store_init (MusicStore *self)
{
}

GtkTreeModel*
music_store_new (void)
{
  return g_object_new (MUSIC_TYPE_STORE, NULL);
}

GtkTreeModel*
music_store_new_with_model (GtkTreeModel *model,GtkTreePath  *root)
{
  GObject *newobj = g_object_new (MUSIC_TYPE_STORE, "child-model", model,"virtual-root", root,NULL);

  
    return GTK_TREE_MODEL(newobj);
    
}
