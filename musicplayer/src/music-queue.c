/* Music-queue.c */
#include "music-store.h"
#include "music-queue.h"
#include "jump-window.h"
#include "music-plugin-manager.h"
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
G_DEFINE_TYPE (MusicQueue, music_queue, GTK_TYPE_VBOX)

 struct
{
    gchar *title;
    gint id;    
}typedef sortnode;

struct
{
     gchar *datestr; 
     guint  date;
    gint id;    
}typedef sortnodedate;
 struct
{
    GHashTable *htable;
    gint *order;
    gint curr;
}typedef traversestr;


enum
{
    COLUMN_ARTIST,
    COLUMN_TITLE,
    COLUMN_SONG,
    COLUMN_URI,
    COLUMN_ID,
    COLUMN_MOD,
    N_COLUMNS,

};
enum
{
    TARGET_STRING,
    TARGET_URL
};

enum
{
    SORTID_ARTIST,
    SORTID_TITLE
};
enum
{
    PROP_0,

    PROP_MUSICQUEUE_FONT,
    PROP_MUSICQUEUE_LASTDIR,
    PROP_MUSICQUEUE_REPEAT
};


//priv fuctions
static gboolean 
check_for_folders(GSList *list);

static void 
file_chooser_cb(GtkWidget *data, 
                gint response,
                gpointer user_data);
static void
choose_file_action(gchar * uri,
                   const gchar *type, 
                   gpointer user_data);
static void 
scan_file_action(gpointer data,
                 gpointer user_data);

static void 
traverse_folders(gpointer data,
                 gpointer user_data);
static void 
destroy_hash_element(gpointer data);

static void 
add (GtkWidget *widget,
gpointer user_data);

static void 
add_columns (MusicQueue *self);

static void 
init_widgets (MusicQueue *self);

static void 
music_queue_read_start_playlist(gchar *location,
                                MusicQueue *self);
//static void playfile (GtkTreeSelection *selection, gpointer data);
static void 
next_file (GsPlayer *player,
          gpointer user_data);
static void
on_drag_data_received(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata);

static void 
playfile (GtkTreeView *treeview,
          GtkTreePath        *path,
          GtkTreeViewColumn  *col,
          gpointer data);


static void  
row_changed  (GtkTreeModel *tree_model,
             GtkTreePath  *path,
             GtkTreeIter  *iter,
             gpointer      user_data);


static gboolean  
grab_focus_cb (GtkWidget *widget,
             GdkEventButton *event,
             gpointer user_date);

static gboolean 
lost_grab_focus_cb(GtkWidget *widget,
			 GdkEventButton *event,
			 gpointer user_date);

static void 
drag_end (GtkWidget *widget,
         GdkDragContext *contex,
         gpointer user_data);

static void 
drag_begin (GtkWidget *widget,
           GdkDragContext *contex,
           gpointer user_data);

static GtkWidget * 
get_context_menu(gpointer user_data);

static void 
remove_files(GtkMenuItem *item, 
             gpointer callback_data);

static gboolean 
remove_files_via_press(GtkWidget *widget,
                       GdkEventKey *key,
                       gpointer user_data);

static void 
add_file(gpointer data,gpointer user_data);

gboolean 
has_selected(gpointer user_data);

static void 
set_font   (gpointer    callback_data,
            guint       callback_action,
            GtkWidget  *widget);

static void 
set_repeat   (GtkCheckMenuItem *widget,
              gpointer user_data);

static GList * 
get_list(gpointer user_data);

static void 
gotJump(JumpWindow *jwindow,
        GtkTreePath* path,gpointer user_data);

static gint 
compare_sort_nodes(sortnode *node1, 
                   sortnode *node2,
                   gpointer userdata);

static gint 
sort_by_artist(gpointer    callback_data,
               gpointer user_data);

static gint
compare_sort_nodes_by_date(sortnodedate *node1, 
                           sortnodedate *node2,
                           gpointer userdata);

static void 
sort_by_date(gpointer    callback_data,
             gpointer user_data);

static gboolean            
traverse_tree_by_date (gpointer data,
                       gpointer userdata);

static gboolean 
traverse_tree (gpointer data,
               gpointer userdata);

static gboolean 
check_type_supported(const gchar *type);

static void 
plugins_item_selected  (gpointer    callback_data,
                        guint       callback_action,
                        GtkWidget  *widget);


//end priv functions

//private varibles
struct _MusicQueuePrivate{
	GtkWidget* treeview;
	GtkWidget* openbutton;
	GtkWidget* scrolledwindow;
	GtkWidget *menu;
	GtkWidget *delete;
    GtkListStore *store;
    GtkTreeModel *musicstore;
    GtkTreeIter  curr;
	GtkTreeSelection *currselection;
    GConfClient* client;
	GsPlayer *player;
    TagScanner *ts;
    PlaylistReader *read;
	guint i;
	gint currid;
    gboolean changed;
	gchar *font;
    gchar *lastdir;
	gboolean drag_started;
	gboolean repeat;
};

//end private varibles








const static  GtkTargetEntry targetentries[] =
{
     { "STRING",        0, TARGET_STRING },
     { "text/plain",    0, TARGET_STRING }
};



static void
music_queue_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
     
     MusicQueue *self = MUSIC_QUEUE(object);

	switch (property_id) {
		case PROP_MUSICQUEUE_FONT:
			g_value_set_string (value, self->priv->font);
			break;

		case PROP_MUSICQUEUE_LASTDIR:
			g_value_set_string (value, self->priv->lastdir);
			break;

		case PROP_MUSICQUEUE_REPEAT:
			g_value_set_boolean (value, self->priv->repeat);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


static void
music_queue_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
	MusicQueue *self = MUSIC_QUEUE(object);
	switch (property_id) {

		case PROP_MUSICQUEUE_FONT:
			g_free (self->priv->font);
			self->priv->font = g_value_dup_string (value);   
			break;	

		case PROP_MUSICQUEUE_LASTDIR:
			g_free (self->priv->lastdir);
			self->priv->lastdir = g_value_dup_string (value);

			break;

		case PROP_MUSICQUEUE_REPEAT:
			self->priv->repeat = g_value_get_boolean (value);

			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
	}
}


static void
music_queue_dispose (GObject *object)
{
    if(object){
        MusicQueue *self = MUSIC_QUEUE(object) ;
        GList *list;
        gboolean repeat;	
        gchar *font;

        if (self->priv->client)
        {

            if((list = get_list(self)) != NULL)
            {
                playlist_reader_write_list(self->priv->read,"/home/kyle/test.xspf",list);
                g_list_free(list);
            }


            g_object_get(G_OBJECT(self),"musicqueue-font",&font,NULL);
            g_object_get(G_OBJECT(self),"musicqueue-repeat",&repeat,NULL);

            //save all the props we want to gconf
            //gconf_client_set_string              (self->priv->client,
            //                                      "/apps/musicplayer/font",
            //                                      font,
            //                                      NULL);

            gconf_client_set_bool           (self->priv->client,
                                             "/apps/musicplayer/repeat",
                                             repeat,
                                             NULL);


            g_object_unref(self->priv->client);
            self->priv->client = NULL;
            g_free(font);
            //g_object_unref(self->priv->store);
            //g_object_unref(self->priv->read);


            G_OBJECT_CLASS (music_queue_parent_class)->dispose (object);
        }
    }
}
static void
music_queue_finalize (GObject *object)
{
     MusicQueue *self = MUSIC_QUEUE(object);
     G_OBJECT_CLASS (music_queue_parent_class)->finalize (object);
}
  
static void
music_queue_class_init (MusicQueueClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	GParamSpec *pspec;
    g_type_class_add_private (klass, sizeof (MusicQueuePrivate));
	object_class->dispose = music_queue_dispose;
	object_class->finalize = music_queue_finalize;
	object_class->get_property = music_queue_get_property;
	object_class->set_property = music_queue_set_property;

	pspec = g_param_spec_string ("musicqueue-font",
	                             "font",
	                             "Set font face",
	                             NULL /* default value */,
	                             G_PARAM_READWRITE);

	g_object_class_install_property (object_class,
	                                 PROP_MUSICQUEUE_FONT,
	                                 pspec);


	pspec = g_param_spec_string ("musicqueue-lastdir",
	                             "lastdir",
	                             "Set last directory opened",
	                             NULL /* default value */,
	                             G_PARAM_READWRITE);

	g_object_class_install_property (object_class,
	                                 PROP_MUSICQUEUE_LASTDIR,
	                                 pspec);

	pspec = g_param_spec_boolean ("musicqueue-repeat",
	                              "repeat",
	                              "Set the playlist to repeat",
	                              FALSE /* default value */,
	                              G_PARAM_READWRITE);

	g_object_class_install_property (object_class,
	                                 PROP_MUSICQUEUE_REPEAT,
	                                 pspec);





}
static void 
foreach_playlist_file(gpointer data,
                      gpointer user_data)
{

	metadata *track = (metadata *)data;
	MusicQueue *self = (MusicQueue *)user_data;
	GtkTreeIter iter;
	gchar *out;
	gchar **tokens;
	const gchar toke[] ="/";
	gchar buffer[60000];
    GFile *file;
    GFileInfo *info;
    guint64 mod;
	int i;

	if(data)
	{
		self->priv->i++;

		gtk_list_store_append(self->priv->store, &iter);

		g_strchomp((gchar *)track->uri);
		out = gnome_vfs_get_local_path_from_uri((gchar *)track->uri);


		gtk_list_store_set(self->priv->store,&iter,COLUMN_URI,track->uri,-1);  

		g_snprintf(buffer,10,"%i",self->priv->i);
		gtk_list_store_set(self->priv->store,&iter,COLUMN_ID,buffer,-1);

        file =g_file_new_for_uri(track->uri);
        info= g_file_query_info(file,G_FILE_ATTRIBUTE_TIME_MODIFIED,
                            G_FILE_QUERY_INFO_NONE,
                            NULL,
                            NULL);    
        
        
        mod = g_file_info_get_attribute_uint64(info,
                                               G_FILE_ATTRIBUTE_TIME_MODIFIED); 

      g_snprintf(buffer,20,"%lu",(unsigned long int)mod);

        
	  gtk_list_store_set(self->priv->store,&iter,COLUMN_MOD,buffer,-1);
       
        g_object_unref(file);
        g_object_unref(info);
		tokens=g_strsplit(out,toke,10);
        
		

		 //take out the '/' in the uri
		 for(i=1; tokens[i] != NULL; i++);

		 if(track->title!= NULL && track->artist !=NULL)
		 {	
			 gtk_list_store_set(self->priv->store,&iter,COLUMN_TITLE,track->title,-1);
			 gtk_list_store_set(self->priv->store,&iter,COLUMN_ARTIST,track->artist,-1);
			 g_snprintf(buffer,strlen(track->artist)+strlen(track->title) +4
                       ,"%s - %s",track->artist,track->title);
			 
		     gtk_list_store_set(self->priv->store,&iter,COLUMN_SONG,buffer,-1);
			 
		 }
		 else
		 {
			 //without
			 gtk_list_store_set(self->priv->store,&iter,COLUMN_SONG,(gpointer *)tokens[i-1]);   
		 }  

		 ts_metadata_free(track);
		 g_strfreev(tokens);  
		 g_free(out);
	 }
}

static void 
music_queue_read_start_playlist(gchar *location,
                                MusicQueue *self)
{
     GList *list = g_list_alloc();
    
     playlist_reader_read_list(self->priv->read,location,&list);

     g_list_foreach(list,foreach_playlist_file,self);

     g_list_free(list);
     
}

static void
music_queue_init (MusicQueue *self)
{
	gboolean repeat=FALSE;
    gchar *font;
    //need to pull in gconf stuff here

     self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, MUSIC_TYPE_QUEUE, 
                                               MusicQueuePrivate);
     //g_object_set(G_OBJECT (self), "musicqueue-font","verdanna bold 7",NULL);
	 g_object_set(G_OBJECT (self), "musicqueue-lastdir",g_get_home_dir(),NULL);

	 self->priv->client = gconf_client_get_default();

	repeat=gconf_client_get_bool (self->priv->client,"/apps/musicplayer/repeat",NULL);
		
	 g_object_set(G_OBJECT (self), "musicqueue-repeat",repeat,NULL);
	
     init_widgets(self);
     self->priv->changed =FALSE;
     self->priv->i=0;
     self->priv->drag_started=FALSE;
     self->priv->ts = NULL;
     self->priv->read = PLAYLIST_READER(xspf_reader_new());
     
     music_queue_read_start_playlist("/home/kyle/test.xspf",self);
     
     
  
} 



static void 
init_widgets(MusicQueue *self)
{
     GtkTreeSelection *select;
     GtkTreeSortable *sortable;
	 



     self->priv->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);

     gtk_box_pack_start (GTK_BOX (self),self->priv->scrolledwindow, TRUE, TRUE, 0);
     
    
     
     gtk_widget_show (self->priv->scrolledwindow);

     self->priv->store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,-1);

    

       

     //add model to widget we want the jump window to have the filter store and the queue
     // to have the regular list store
     self->priv->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->priv->store));
     
     add_columns(self);

     select = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->treeview));

     gtk_widget_show (self->priv->treeview);
     gtk_container_add (GTK_CONTAINER (self->priv->scrolledwindow), self->priv->treeview);

     //gtk_box_pack_start (GTK_BOX (self),self->priv->treeview, TRUE, TRUE, 0);
     //gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (self->priv->treeview), TRUE);

    g_object_set (G_OBJECT (self->priv->treeview),"headers-visible"  ,FALSE,
	          	"headers-clickable",FALSE,NULL);
     
     self->priv->openbutton = gtk_button_new_from_stock(GTK_STOCK_ADD);
     gtk_box_pack_start (GTK_BOX (self),self->priv->openbutton, FALSE, TRUE, 0);
     gtk_widget_show(self->priv->openbutton);


//signals

	g_signal_connect ((gpointer) self->priv->openbutton, "released",
	                  G_CALLBACK (add),
	                  (self));
	g_signal_connect (G_OBJECT (self->priv->treeview), "row-activated",
	                  G_CALLBACK (playfile),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->store), "row-changed",
	                  G_CALLBACK (row_changed),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->treeview), "button_press_event",
	                  G_CALLBACK (grab_focus_cb),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->treeview), "key_press_event",
	                  G_CALLBACK (remove_files_via_press),
	                  self);


	g_signal_connect (G_OBJECT (self->priv->treeview), "button_release_event",
	                  G_CALLBACK (lost_grab_focus_cb),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->treeview), "drag_begin",
	                  G_CALLBACK (drag_begin), 
	                  self);

	g_signal_connect (G_OBJECT (self->priv->treeview), "drag_end",
	                  G_CALLBACK (drag_end), 
	                  self);

	g_signal_connect(self->priv->treeview, "drag_data_received",
	                 G_CALLBACK(on_drag_data_received),
	                 self);

	
     
//set policy
     
     

     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self->priv->scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
     
     //gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (self->priv->scrolledwindow), GTK_SHADOW_ETCHED_IN);

     gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->priv->treeview),TRUE);
     
//DnD stuff
     gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->priv->treeview),TRUE);
     gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (self->priv->treeview),
						      targetentries, G_N_ELEMENTS (targetentries),
						      GDK_ACTION_COPY | GDK_ACTION_MOVE);

     

    

      self->priv->currselection = gtk_tree_view_get_selection(GTK_TREE_VIEW (self->priv->treeview));
      gtk_tree_selection_set_mode(self->priv->currselection,GTK_SELECTION_MULTIPLE);




	self->priv->menu = get_context_menu(self);
	gtk_widget_show_all(self->priv->menu);
	gtk_menu_attach_to_widget(GTK_MENU(self->priv->menu),self->priv->treeview,NULL);
}







static void
on_drag_data_received(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata)
{
    
    MusicQueue *self = (MusicQueue *) userdata;
    char **list;
    int i;

     //add to list;
    self->priv->ts = tag_scanner_new();

     //check to see if it was internal
    if(gtk_drag_get_source_widget (context)  == NULL){
	 list = g_uri_list_extract_uris ((char *)seldata->data);
	 for(i=0; list[i] != NULL; i++)
	 {
	      
	    scan_file_action (list[i],self);
	 }
	 // gtk_drag_finish (context, TRUE, FALSE, time);

	g_strfreev (list);  
    }
    //internal drag 
    else{
     self->priv->changed = TRUE;
    }
    
    g_object_unref(self->priv->ts);
}


static void playfile (GtkTreeView *treeview,
                      GtkTreePath        *path,
                      GtkTreeViewColumn  *col,
                      gpointer data)
	
{
     MusicQueue *self = (MusicQueue *) data;
     gchar * uri;
     GtkTreeIter iter;
     GtkTreeModel *model;
     gchar *id;
     model = gtk_tree_view_get_model(treeview);
    
     if (gtk_tree_model_get_iter (model, &iter,path))
        {
	     self->priv->curr = iter;
	     gtk_tree_model_get (model, &iter, COLUMN_ID, &id, -1);
	     self->priv->currid = atoi(id);

                gtk_tree_model_get (model, &iter, COLUMN_URI, &uri, -1);
		gs_playFile(self->priv->player,uri);
                
                g_free (uri);
		        g_free(id);
                  
        }
}
void music_queue_play_selected (MusicQueue *self)
{
 GList *list;
    GtkTreeModel *model= GTK_TREE_MODEL(self->priv->store); 
    
       list = gtk_tree_selection_get_selected_rows (self->priv->currselection,
                                                    &model);
    
 	
    if(list){
      
        playfile(GTK_TREE_VIEW(self->priv->treeview),list->data,
	          NULL,(gpointer)self);
    }
    g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (list);
  
}



static void 
add(GtkWidget *widget,
    gpointer user_data)
{

    MusicQueue *self = (MusicQueue *) user_data;

    GtkWidget *dialog;
    GtkFileFilter *filter;
    gchar *lastdir = NULL;	
    gint response;

    filter = gtk_file_filter_new ();

    gtk_file_filter_set_name(filter,"Supported Types");  
    gtk_file_filter_add_pattern(filter,"*.mp3");
    gtk_file_filter_add_pattern(filter,"*.flac");
    gtk_file_filter_add_pattern(filter,"*.ogg");
    gtk_file_filter_add_pattern(filter,"*.wma");
    gtk_file_filter_add_pattern(filter,"*.xspf");
    gtk_file_filter_add_pattern(filter,"*.m3u");

    g_object_get(G_OBJECT(self),"musicqueue-lastdir",&lastdir,NULL);	

    dialog = gtk_file_chooser_dialog_new ("Open File",
                                          NULL,
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN,1,	   
                                          GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT,
                                          NULL);



    g_object_set(G_OBJECT(dialog),"select-multiple",TRUE,NULL);

    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(dialog),FALSE);
    self->priv->ts = tag_scanner_new();
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),lastdir);

    g_signal_connect_swapped (dialog,
                             "response", 
                             (GCallback)file_chooser_cb,
                             self);

    gtk_widget_show_all (dialog);
    g_free(lastdir);


}
static void 
file_chooser_cb(GtkWidget *data, 
                gint response,
                gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) data;
    gboolean b = TRUE;
    GSList *list;
    GtkFileFilter *filter;
    GtkWidget *dialog = GTK_WIDGET(user_data);
    
    gchar *lastdir = NULL;
    if(response == GTK_RESPONSE_CANCEL)
    {
        g_object_unref(self->priv->ts);
        gtk_widget_destroy (dialog);
    }

    else if (response  == GTK_RESPONSE_ACCEPT)
    {

        list =  gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
        //set our last dir to one they chose

        g_object_set(G_OBJECT(self),"musicqueue-lastdir",
                     gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog))
                     ,NULL);	


        gtk_widget_destroy (dialog);
        g_slist_foreach (list,scan_file_action,self);
        g_slist_free (list);
        g_object_unref(self->priv->ts);
    }
   else if(response == 1) //folder(s) selected
    {

        list = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
        if(check_for_folders(list))
        {
            gtk_widget_destroy (dialog);
            //need to check if the selection is a folder
            g_slist_foreach (list,traverse_folders,self);
            g_object_unref(self->priv->ts);
        }
        g_slist_free (list);
    }


}
void 
add_file_ext(gpointer data,
             gpointer user_data)
{
    GFile *file;
    gchar *uri;
    MusicQueue *self = (MusicQueue *) user_data;
    file = g_file_new_for_commandline_arg (data);
    uri = g_file_get_uri(file);
    self->priv->ts = tag_scanner_new(); 
    if(uri){
        scan_file_action (uri,self);
        g_free(uri);
    }
    g_object_unref(self->priv->ts);
    g_object_unref(file);
 }
static 
gboolean check_for_folders(GSList *list)
{
    GSList *beg = list;
    GFile *file;
    gchar *uri;
    GFileInfo *info;
    gboolean ret = TRUE;
    
    for(list; list!=NULL; list=list->next)
    {
        if(list->data){
        file = g_file_new_for_uri((gchar *)list->data);
        info = g_file_query_info(file,G_FILE_ATTRIBUTE_STANDARD_TYPE,0,NULL,NULL);

            if(g_file_info_get_file_type(info) != G_FILE_TYPE_DIRECTORY)
            {
                ret = FALSE;
            }
            g_object_unref(file);
            g_object_unref(info);
        }
    }
    return ret;
}
static void 
traverse_folders(gpointer data,
                 gpointer user_data)
{
    GFileEnumerator *enumer; 
    GFileInfo *info;
    GFile *file;
    const gchar *target_uri;
    const gchar *uri = (gchar *) data;
    const gchar *name;
    const gchar *filetype;
    gchar *buffer;
    gchar *escaped;
    GError *err=NULL;

    file = g_file_new_for_uri((gchar *)data);
    
    enumer = g_file_enumerate_children (file,
                                         G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                         G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                                         G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ","
                                         G_FILE_ATTRIBUTE_STANDARD_TARGET_URI ,
                                         0,NULL,
                                         &err);
    
   if (err != NULL)
  {
    /* Report error to user, and free error */
    fprintf (stderr, "Unable to read file: %s\n", err->message);
    g_error_free (err);
  }

    
    info = g_file_enumerator_next_file(enumer,NULL,&err);

       if (err != NULL)
  {
    /* Report error to user, and free error */
    fprintf (stderr, "Unable to read file: %s\n", err->message);
    g_error_free (err);
  }
    
    while(info != NULL)
    {
      target_uri = g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME);
       
              if (target_uri != NULL)
                {
                  escaped = g_uri_escape_string(target_uri,NULL,TRUE);
                  buffer = g_malloc(sizeof(gchar) *strlen(escaped)+strlen(uri)+10);
                  g_snprintf(buffer,strlen(escaped)+strlen(uri)+10,"%s/%s",uri,escaped);

                  if(g_file_info_get_file_type(info) ==  G_FILE_TYPE_DIRECTORY)
                    {
                         //call recursively
                        traverse_folders(buffer,user_data);
                    }
                    else
                    {
                        filetype = g_file_info_get_attribute_string (info, 
                                              G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
                        if(check_type_supported(filetype))
                        {
                            add_file(buffer,user_data);
                        }
                    }
                  
                  g_free(escaped);
                  g_free(buffer);
                 
                }

            
     g_object_unref(info);
    info = g_file_enumerator_next_file(enumer,NULL,NULL);
    }
    g_object_unref(file);
    g_object_unref(enumer);
    
                                            
}


static gboolean 
check_type_supported(const gchar *type)
{
    if(strcmp(type,"audio/mpeg") == 0)
        return TRUE;
    if(strcmp(type,"audio/ogg")  == 0)
        return TRUE;
    if(strcmp(type,"audio/x-flac") == 0)
        return TRUE;
    if(strcmp(type,"audio/wma")  == 0)
         return TRUE;

    return FALSE;
}

//uri and musicqueuestatic void add_file(gpointer data,gpointer user_data);

static void 
scan_file_action(gpointer data,
                 gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    GFile * file = g_file_new_for_uri((gchar *)data);
    GError *err;
    GFileInfo *info;
    gchar* uri = (gchar *)data; 
    
    const gchar *type;
    
    
    info =g_file_query_info (file,
                                         G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                         G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                                         G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ,
                                         0,NULL,
                                         &err);
    if(info)
    {
       type= g_file_info_get_attribute_string (info, 
                                            G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
        choose_file_action(uri,type,user_data);
        
    }
    
    
}

static void 
choose_file_action(gchar * uri,
                   const gchar *type, 
                   gpointer user_data)
{
    PlaylistReader *read;
    GList *list;
    MusicQueue *self = (MusicQueue *) user_data;
    

    
    if(strcmp(type,"audio/mpeg") == 0)
    {
        add_file(uri,user_data);
    }
    else if(strcmp(type,"audio/ogg")  == 0)
    {
        add_file(uri,user_data);
    }
    else if(strcmp(type,"audio/x-flac") == 0)
    {
        add_file(uri,user_data);
    }
    else if(strcmp(type,"audio/wma")  == 0)
    {
        add_file(uri,user_data);
    }
    else if(strcmp(type,"application/xspf+xml")  == 0)
    {
       list = g_list_alloc();
        read = PLAYLIST_READER(xspf_reader_new());
        playlist_reader_read_list(read,uri,&list);
        g_list_foreach(list,foreach_playlist_file,self);
        g_object_unref(read);
        g_list_free(list);
    }
    
     else if(strcmp(type,"audio/x-mpegurl")  == 0)
    {
        list = g_list_alloc();
        read = PLAYLIST_READER(m3u_reader_new());
        playlist_reader_read_list(read,uri,&list);
        g_list_foreach(list,foreach_playlist_file,self);
        g_object_unref(read);
        g_list_free(list);
      
    }
    else
    {
        fprintf(stderr,"MIME type not supported\n");
    }
    
}
static void 
add_file(gpointer data,gpointer user_data)
{
	MusicQueue *self = (MusicQueue *) user_data;
	GtkTreeIter   iter;
	gchar *out;			       
	gchar **tokens;
	gchar **tokens2;
	const gchar toke[] ="/";
	const gchar toke2[] =".";
	gchar buffer[60000];
	gchar *valid;
     GFile *file;
    GFileInfo *info;
    guint64 mod;
    int i;
	metadata *md = NULL;

	self->priv->i++;


	valid = gnome_vfs_make_uri_from_input((gchar *)data);

	gtk_list_store_append(self->priv->store, &iter);



    // gtk_list_store_set(self->priv->store,&iter,COLUMN_TITLE,out,-1);    
    gtk_list_store_set(self->priv->store,&iter,COLUMN_URI,valid,-1);  

    g_snprintf(buffer,10,"%i",self->priv->i);
    gtk_list_store_set(self->priv->store,&iter,COLUMN_ID,buffer,-1);  

    file =g_file_new_for_uri(valid);
    info= g_file_query_info(file,G_FILE_ATTRIBUTE_TIME_MODIFIED,
                            G_FILE_QUERY_INFO_NONE,
                            NULL,
                            NULL);    


    mod = g_file_info_get_attribute_uint64(info,
                                           G_FILE_ATTRIBUTE_TIME_MODIFIED); 

    g_snprintf(buffer,20,"%lu",(unsigned long int)mod);
  
        
		gtk_list_store_set(self->priv->store,&iter,COLUMN_MOD,buffer,-1);
       
        g_object_unref(file);
        g_object_unref(info);
    
	//get meta data info
	md=ts_get_metadata(valid,self->priv->ts);
	//printf("%s\n",(gchar *)data);
	if(md != NULL && md->title != NULL && md->artist !=NULL)
	{	  
		gtk_list_store_set(self->priv->store,&iter,COLUMN_TITLE,md->title,-1);
		gtk_list_store_set(self->priv->store,&iter,COLUMN_ARTIST,md->artist,-1);
		
	   	g_snprintf(buffer,strlen(md->artist)+strlen(md->title)+4,
                      "%s - %s",md->artist,md->title);
			gtk_list_store_set(self->priv->store,&iter,COLUMN_SONG,buffer,-1);
		
		ts_metadata_free(md);
	}
	else
	{
		g_strchomp((gchar *)valid);
		out = gnome_vfs_get_local_path_from_uri(valid);
		tokens=g_strsplit(out,toke,10);

		if (tokens != NULL)
		{
			//take out the '/' in the ur
			for(i=1; tokens[i] != NULL; i++);			

			//take out the file extension;
			tokens2=g_strsplit(tokens[i-1],toke2,2);
			gtk_list_store_set(self->priv->store,&iter,COLUMN_SONG,(gpointer *)tokens2[0]);

		}
		g_strfreev(tokens);  
		g_strfreev(tokens2);  
		g_free(out);
		g_free(valid);
	}
    
}




GtkWidget*
music_queue_new (void)
{
    return (GTK_WIDGET(g_object_new (MUSIC_TYPE_QUEUE, NULL)));
}

GtkWidget*
music_queue_new_with_player(GsPlayer *player)
{
    MusicQueue *self;
    self =g_object_new (MUSIC_TYPE_QUEUE, NULL);
    self->priv->player =player;
    
    g_signal_connect (player, "eof",
                      G_CALLBACK(next_file),
                      (gpointer)self);
    
    
    
    
    return GTK_WIDGET(self);
}
static void 
next_file            (GsPlayer      *player,
                       gpointer         user_data)
{
	MusicQueue *self = (MusicQueue *) user_data;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;

	gboolean test;	

	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);


	if (self->priv->currid > 0)
	{
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));

		gtk_tree_selection_unselect_iter(self->priv->currselection,&self->priv->curr);  
		if(gtk_tree_model_iter_next(model,&self->priv->curr))
		{
			//there is a next file 
			gtk_tree_selection_select_iter(self->priv->currselection,&self->priv->curr); 
			playfile(GTK_TREE_VIEW(self->priv->treeview),gtk_tree_model_get_path(model,&self->priv->curr),NULL,user_data);
		} else{
			//repeat code check to make sure property is true
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter) && test)
			{
				gtk_tree_selection_select_iter(self->priv->currselection,&iter); 
				playfile(GTK_TREE_VIEW(self->priv->treeview),gtk_tree_model_get_path(model,&iter),NULL,user_data);
			}


		}


	}
}



static void 
add_columns(MusicQueue *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gchar *font = NULL;
    g_object_get(G_OBJECT(self),"musicqueue-font",&font,NULL);
    
    printf("font:%s\n",font);
    
    renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"font",self->priv->font,NULL);
    column = gtk_tree_view_column_new_with_attributes ("Artist",
											renderer,
											"text",
											COLUMN_ARTIST,
											NULL);
    
    gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN (column),TRUE);
    
    
    renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    
    g_object_set(G_OBJECT(renderer),"font",font,NULL);
    
    
    column = gtk_tree_view_column_new_with_attributes ("Title",
											renderer,
											"text",
											COLUMN_TITLE,
											NULL);

	
 	renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    
    g_object_set(G_OBJECT(renderer),"font",font,NULL);
    


	
    column = gtk_tree_view_column_new_with_attributes ("Songs",
											renderer,
											"text",
											COLUMN_SONG,
											NULL);

	

	gtk_tree_view_append_column (GTK_TREE_VIEW(self->priv->treeview), column);


    renderer = gtk_cell_renderer_text_new ();
    
    
    column = gtk_tree_view_column_new_with_attributes ("URI",
											renderer,
											"text",
											COLUMN_URI,
											NULL);
    renderer = gtk_cell_renderer_text_new ();
    
    
    column = gtk_tree_view_column_new_with_attributes ("ID",
											renderer,
                                                       "text",
                                                       COLUMN_ID,
                                                       NULL);
    renderer = gtk_cell_renderer_text_new ();

    column = gtk_tree_view_column_new_with_attributes ("MODIFCATION",
                                                       renderer,
                                                       "text",
                                                       COLUMN_MOD,
                                                       NULL);

    
}
static void  
row_changed  (GtkTreeModel *tree_model,
             GtkTreePath  *path,
             GtkTreeIter  *iter,
             gpointer      user_data) 
{
    MusicQueue *self = (MusicQueue *) user_data;
    gchar *id;
    //Curr was deleted
    
    if(self->priv->changed)
    {
	   gtk_tree_model_get (tree_model, iter, COLUMN_ID, &id, -1);
	   
	   if(atoi(id) == self->priv->currid)
	   {
		  self->priv->curr = *iter;
		  
	   }
	   
	   g_free (id);
	   self->priv->changed = FALSE;
    }
    
}


static 
gboolean grab_focus_cb (GtkWidget *widget,
                      GdkEventButton *event,
                      gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    GtkWidget *menu;
    GtkTreeModel *model;					      
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));
    
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->priv->treeview),TRUE);

	if((event->button ==3) && (event->type == GDK_BUTTON_PRESS))
	{
		if(has_selected(user_data))
			gtk_widget_set_sensitive(self->priv->delete,TRUE);
		else
			gtk_widget_set_sensitive(self->priv->delete,FALSE);
			
		gtk_menu_popup(GTK_MENU(self->priv->menu),NULL,NULL,
                       NULL,NULL,event->button,event->time);
		return FALSE;
	}
    
    

    return FALSE;
}


static gboolean 
lost_grab_focus_cb (GtkWidget *widget,
                 GdkEventButton *event,
                 gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    
    if(!self->priv->drag_started)
    {
	   gtk_drag_dest_unset (self->priv->treeview);
	   gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->priv->treeview),FALSE);
	   gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (self->priv->treeview),
									 targetentries, G_N_ELEMENTS (targetentries),
									 GDK_ACTION_COPY | GDK_ACTION_MOVE);
	   
    }
    return FALSE;
    
}


static void 
drag_begin (GtkWidget *widget,
           GdkDragContext *contex,
           gpointer user_data)
{
    
    MusicQueue *self = (MusicQueue *) user_data;
    self->priv->drag_started = TRUE;
    
}


static void 
drag_end (GtkWidget *widget,
         GdkDragContext *contex,
         gpointer user_data)
{
    
    MusicQueue *self = (MusicQueue *) user_data;
    self->priv->drag_started = FALSE;
    
    //gtk_drag_dest_unset (self->priv->treeview);
    //gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->priv->treeview),FALSE);
    
    //bug:
    // user has to have a row selected for external dnd to work
}


static GtkWidget * 
get_context_menu(gpointer user_data)
{
    
    GtkItemFactory *item_factory;
    GtkWidget  *menu,*font,*repeat,*sort,*sort2,*seperator,*plugins;
	gboolean test;
	
	MusicQueue *self = (MusicQueue *) user_data;
	
	menu = gtk_menu_new();

		
    self->priv->delete = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE,NULL);
	font   = gtk_image_menu_item_new_from_stock(GTK_STOCK_SELECT_FONT,NULL);
    plugins   = gtk_menu_item_new_with_label("Plugins");
    repeat =  gtk_check_menu_item_new_with_label("Repeat");
    seperator = gtk_separator_menu_item_new ();
    sort   = gtk_menu_item_new_with_label("Sort By Artist");
    sort2   = gtk_menu_item_new_with_label("Sort By Date");
	


	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);

	   	if(test)
	   		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(repeat),TRUE);
	
	g_signal_connect (G_OBJECT (self->priv->delete), "activate",
	                  G_CALLBACK (remove_files),
	                  user_data);
	g_signal_connect (G_OBJECT (font), "activate",
	                  G_CALLBACK (set_font),
	                  user_data);
	g_signal_connect (G_OBJECT (repeat), "activate",
	                  G_CALLBACK (set_repeat),
	                  user_data);
    g_signal_connect (G_OBJECT (sort), "activate",
                      G_CALLBACK (sort_by_artist),
                      user_data);
    g_signal_connect (G_OBJECT (sort2), "activate",
                      G_CALLBACK (sort_by_date),
                      user_data);
     g_signal_connect (G_OBJECT (plugins), "activate",
                      G_CALLBACK (plugins_item_selected),
                      user_data);

    
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),self->priv->delete);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),font);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),plugins);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),repeat);
    
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),seperator);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),sort);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),sort2);
    
	
	   
    return menu;
    
}
static gboolean 
remove_files_via_press(GtkWidget *widget,
                       GdkEventKey *event,
                       gpointer user_data)
{
    
    MusicQueue *self = (MusicQueue *) user_data;
    if(event->keyval == GDK_Delete && has_selected(user_data) == TRUE)
	   remove_files(NULL,user_data);

  if(event->keyval == GDK_j)
    {
       make_jump_window(self);
       return TRUE;
    }
    
    return FALSE;
}  

void
make_jump_window(MusicQueue *self)
{
    GtkWidget *jumpwindow;
    self->priv->musicstore = music_store_new_with_model(GTK_TREE_MODEL(self->priv->store),NULL);
    jumpwindow = jump_window_new_with_model(self->priv->musicstore);

    g_signal_connect(jumpwindow, "jump",
	                 G_CALLBACK(gotJump),
	                 self);
           
    gtk_widget_show(jumpwindow);
}
static void 
gotJump(JumpWindow *jwindow,
        GtkTreePath* path,
        gpointer user_data)
{
    
    MusicQueue *self = (MusicQueue *) user_data;
    gtk_tree_selection_unselect_all(self->priv->currselection);
    gtk_tree_selection_select_path(self->priv->currselection,path); 

    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(self->priv->treeview),path,NULL,TRUE,0.5,0.5);   
    playfile(GTK_TREE_VIEW(self->priv->treeview),path,NULL,user_data);
}
    
static void 
plugins_item_selected  (gpointer    callback_data,
                        guint       callback_action,
                        GtkWidget  *widget)
{
    GtkWidget* dialog = (GtkWidget *)music_plugin_manager_new ();

    gtk_widget_show(dialog);
}

static void 
set_font   (gpointer    callback_data,
            guint       callback_action,
            GtkWidget  *widget)
{
    MusicQueue *self = (MusicQueue *) callback_data;
    
    GtkWidget *   font_window = gtk_font_selection_dialog_new       ("Select playlist font"); 
    
    gtk_widget_show(font_window); 
    
}
static void 
set_repeat (GtkCheckMenuItem *widget,
            gpointer user_data)
{
	MusicQueue *self = (MusicQueue *) user_data;
	gboolean test;	

 	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);

		if(!test){
			g_object_set(G_OBJECT(self),"musicqueue-repeat",TRUE,NULL);
			gtk_check_menu_item_set_active (widget,TRUE);
		}
		else{
			g_object_set(G_OBJECT(self),"musicqueue-repeat",FALSE,NULL);
			gtk_check_menu_item_set_active (widget,FALSE);
		}
}


static void 
remove_files(GtkMenuItem *item, 
             gpointer  callback_data)
{
    MusicQueue *self = (MusicQueue *) callback_data;
    GList * rows;
    GList * rowref_list = g_list_alloc();
    gint i;
    GtkTreeIter iter;
    GtkTreeIter childiter;
    GtkTreePath *path;
    GtkTreeModel *model;
    gchar *id;
    GtkTreeRowReference  *rowref;
    
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));
    
    
    rows = gtk_tree_selection_get_selected_rows(self->priv->currselection,&model);
    
    for(i = 0; i < g_list_length (rows); i++) 
    {
	   path = (GtkTreePath *) g_list_nth_data(rows,i);
	   rowref = gtk_tree_row_reference_new(model, path);
	   rowref_list = g_list_append(rowref_list, rowref);
    }
    for(i = 1; i < g_list_length (rowref_list); i++) 
    {
	   path = gtk_tree_row_reference_get_path(
									  (GtkTreeRowReference*) 
									  g_list_nth_data(rowref_list,i));
	   gtk_tree_model_get_iter (model, &iter,path);
	   gtk_tree_model_get (model, &iter, COLUMN_ID, &id, -1);
	   
	   //last one was deleted or all was deleted
	   if(atoi(id) == self->priv->currid)
	   {
		  if(!gtk_tree_model_iter_next(model,&self->priv->curr))
		  {
			 //last one in tree view
			 self->priv->currid = -1;
		  }
		  else
		  { //update the ID to the next one 
			 g_free(id);
			 gtk_tree_model_get (model, &self->priv->curr, 
							 COLUMN_ID, &id, -1);    	 
			 self->priv->currid = atoi(id);
		  }
	   }

      
	   gtk_list_store_remove(GTK_LIST_STORE(self->priv->store),&iter);
	   
    }
    //free everything
    g_list_foreach(rowref_list, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free(rowref_list);
    g_list_free(rows);
    g_free(id);
   
} 


gboolean 
has_selected(gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    
    GList *rows;
    GtkTreeModel *model;
     self->priv->currselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->treeview));

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));
    rows = gtk_tree_selection_get_selected_rows(self->priv->currselection,&model);	
    
    if(rows)
    {	
		g_list_free(rows);
	   return TRUE;
		
    }
    return FALSE;
}

static 
sort_by_artist(gpointer    callback_data,
               gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    
    GtkTreeIter iter;
    
    GHashTable *htable;
    GList *list;
    gchar *title;
    gchar *cid;
    gint *id;
    gint *curri;
    sortnode *node;
    gint currid;
    gint i=0;
    traversestr str;
    
        //need to compare titles if the artists are the same
     if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter))
    {
       list = g_list_alloc();   
        htable = g_hash_table_new_full(g_int_hash,g_int_equal,
                                       destroy_hash_element,
                                       destroy_hash_element);
        
        do
	   {
            //need a struture that holds artist name and ID of the element
            node = g_malloc(sizeof(sortnode));
            gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_SONG, &(node->title), -1);
            gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_ID, &cid, -1); 

            id = g_malloc(sizeof(gint));
            curri = g_malloc(sizeof(gint));
            *id = atoi(cid);
            
            node->id = *id;
          
		   *curri=i;
           list = g_list_insert_sorted_with_data(list,(gpointer)node,
                                                 (GCompareDataFunc)compare_sort_nodes,
                                                 self);  
           g_hash_table_insert(htable,id,curri);
              ++i;
           
       }while(gtk_tree_model_iter_next(
								GTK_TREE_MODEL(self->priv->store),
								&iter));
     
        
                //need to write compare function that compares the two artist strings
        str.htable=htable;
        str.order = g_malloc(sizeof(gint)*i+1);        
        str.curr=0; //reset our counter for our new order

          g_list_foreach(list,(GFunc)traverse_tree,&str); 
        

        gtk_list_store_reorder(GTK_LIST_STORE(self->priv->store),str.order);

        
        //finally call the rearange function on the List store with our new order 
     
    //free list of nodes
    //free our order
    g_free(str.order);

    //destroy data structures
    g_list_free(list);
    g_hash_table_destroy(htable);

    
    }
}

static void 
sort_by_date(gpointer    callback_data,
             gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    
    GtkTreeIter iter;
    
    GHashTable *htable;
    GList *list;
    gchar *title;
    gchar *cid;
    gint *id;
    gint *curri;
    sortnodedate *node;
    gint currid;
    gint i=0;
    traversestr str;
    
     
     if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter) )
    {
       list = g_list_alloc();   
        htable = g_hash_table_new_full(g_int_hash,g_int_equal,
                                       destroy_hash_element,
                                       destroy_hash_element);
        
        do
	   {
            //need a struture that holds artist name and ID of the element
            node = g_malloc(sizeof(sortnodedate));
		    gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_MOD, &(node->datestr), -1); 
            gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_ID, &cid, -1); 

           
           
            node->date = strtoul(node->datestr,NULL, 10);

                       
            id = g_malloc(sizeof(gint));
            curri = g_malloc(sizeof(gint));
            *id = atoi(cid);
            
            node->id = *id;
          
		   *curri=i;
           list = g_list_insert_sorted_with_data(list,(gpointer)node,
                                                 (GCompareDataFunc)compare_sort_nodes_by_date,
                                                 self);  
           g_hash_table_insert(htable,id,curri);
              ++i;
           
       }while(gtk_tree_model_iter_next(
								GTK_TREE_MODEL(self->priv->store),
								&iter));
     
        
                //need to write compare function that compares the two artist strings
        str.htable=htable;
        str.order = g_malloc(sizeof(gint)*i+1);        
        str.curr=0; //reset our counter for our new order

          g_list_foreach(list,(GFunc)traverse_tree_by_date,&str); 
        

        gtk_list_store_reorder(GTK_LIST_STORE(self->priv->store),str.order);

        
        //finally call the rearange function on the List store with our new order 
     
    //free list of nodes
    //free our order
    g_free(str.order);

    //destroy data structures
    g_list_free(list);
    g_hash_table_destroy(htable);
    

    
    }
}

static void 
destroy_hash_element(gpointer data)
{
    g_free((gint *)data);
}




static gboolean 
traverse_tree  (gpointer data,
                gpointer userdata)
{
    if(data)
    {

        traversestr *str =(traversestr *) userdata;
        int currid=0;
        gint *old;
        sortnode * node = (sortnode *)data;
        currid= node->id;

        
        old= (gint *)g_hash_table_lookup(str->htable,&currid); 

       
        str->order[str->curr]=*old;

        str->curr++;
        
        if(node->title)
            g_free(node->title);
      
        g_free(node);
    }
    return FALSE;
}
static gboolean 
traverse_tree_by_date (gpointer data,
                       gpointer userdata)
{
    if(data)
    {

        traversestr *str =(traversestr *) userdata;
        int currid=0;
        gint *old;
        sortnodedate * node = (sortnodedate *)data;
        currid= node->id;

         
        old= (gint *)g_hash_table_lookup(str->htable,&currid); 
        
        str->order[str->curr]=*old;

        str->curr++;
     if(node->datestr)
            g_free(node->datestr);
      
        
        g_free(node);
    }
    return FALSE;
}
static gint 
compare_sort_nodes(sortnode *node1, 
                   sortnode *node2,
                   gpointer userdata)
{
    int ret=0;

    gchar*artist1=NULL; 
    gchar *artist2 = NULL;
    gchar*title1=NULL; 
    gchar *title2 = NULL;
    
    if(node1){
        title1=node1->title;   
    }
    
    if(node2)
    {
 
        title2 = node2->title;
    }
    //title is empty
    if(title1 == NULL || title2 == NULL)
    {
        if (title1 == NULL && title2 == NULL)
            return 0;

        ret = (artist1== NULL) ? -1 : 1;

    }else
    {
        //ret =g_utf8_collate(artist1,artist2);
          ret =strcmp(title1,title2);

       // if(ret == 0 && title1 && title2) //same artists so sort by title
       // {
      //      ret =strcmp(title1,title2);
       // }
    }
    return ret;
}

static gint 
compare_sort_nodes_by_date(sortnodedate *node1, 
                           sortnodedate *node2,
                           gpointer userdata)
{
    int ret=0;

    guint date1 =0; 
    if(node1)
        date1= node1->date; 
    guint date2= 0;
    if(node2)
        date2 = node2->date;

    //title is empty
    if(date1 == 0 || date2 == 0)
    {
        if (date1 == 0 && date2 == 0)
            return 0;

        ret = (date1== 0) ? -1 : 1;

    }else
    {
         ret = (date1 > date2) ? 1 : -1;
    }
    return ret;
}

                                  
static GList* 
get_list(gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    metadata *track = NULL;
    GtkTreeIter iter;
    GList *list = NULL;
    gchar *artist, *title, *uri;
    
    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter))
    {
	   list = g_list_alloc();
	   do
	   {
		  track = ts_metadata_new();
		  
		  gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_TITLE, &title, -1); 
		  
		  track->title = title;
		  gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_ARTIST, &artist, -1); 
		  
		  track->artist = artist;
		  
		  gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
						  &iter,COLUMN_URI, &uri, -1);
		  track->uri = uri;
		  
		  list =  g_list_append(list,(gpointer)track); 
	   }while(gtk_tree_model_iter_next(
								GTK_TREE_MODEL(self->priv->store),
					 			&iter));
    }
    
    
    return list;
    
}
