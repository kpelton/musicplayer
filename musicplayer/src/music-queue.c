/* Music-queue.c */
#include "music-store.h"
#include "music-queue.h"
#include "jump-window.h"
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
static gboolean check_for_folders(GSList *list);
static void file_chooser_cb(GtkWidget *data, 
                            gint response,
                            gpointer user_data);

static void traverse_folders(gpointer data,gpointer user_data);
static void destroy_hash_element(gpointer data);
static void add (GtkWidget *widget,gpointer user_data);
static void add_columns (MusicQueue *self);
static void init_widgets (MusicQueue *self);
//static void playfile (GtkTreeSelection *selection, gpointer data);
static void nextFile (GsPlayer *player,gpointer user_data);
static void
onDragDataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata);

static void playfile (GtkTreeView *treeview,
		      GtkTreePath        *path,
		      GtkTreeViewColumn  *col,
		      gpointer data);


static void  rowchanged  (GtkTreeModel *tree_model,
		      GtkTreePath  *path,
		      GtkTreeIter  *iter,
		   gpointer      user_data);


static gboolean  grabfocuscb (GtkWidget *widget,
			 GdkEventButton *event,
			 gpointer user_date);


static gboolean lostgrabfocuscb (GtkWidget *widget,
			 GdkEventButton *event,
			 gpointer user_date);


static void dragend (GtkWidget *widget,
			  GdkDragContext *contex,
			 gpointer user_data);

static void dragbegin (GtkWidget *widget,
			  GdkDragContext *contex,
			   gpointer user_data);

static GtkWidget * getcontextmenu(gpointer user_data);

static void remove_files(GtkMenuItem *item, gpointer    
                         	callback_data);

static gboolean remove_files_via_press(GtkWidget *widget,
			 GdkEventKey *key,
				   gpointer user_data);

static void add_file(gpointer data,gpointer user_data);

gboolean has_selected(gpointer user_data);

static void set_font   (gpointer    callback_data,
			 guint       callback_action,
			GtkWidget  *widget);
static void set_repeat   (GtkCheckMenuItem *widget,
                          	gpointer user_data);

static GList * get_list(gpointer user_data);

static void gotJump(JumpWindow *jwindow,
			    GtkTreePath* path,gpointer user_data);

static gint compare_sort_nodes(sortnode *node1, sortnode *node2,gpointer userdata);

static sort_by_artist(gpointer    callback_data,
                        gpointer user_data);
static gint compare_sort_nodes_by_date(sortnodedate *node1, 
                                       sortnodedate *node2,
                                       gpointer userdata);
static sort_by_date(gpointer    callback_data,
                        gpointer user_data);
gboolean            traverse_tree_by_date                       (
                                                         gpointer data,
                                                         gpointer userdata);
gboolean            traverse_tree                       (
                                                         gpointer data,
                                                         gpointer userdata);

static gboolean check_type_supported(const gchar *type);
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
			g_value_set_string (value, self->font);
			break;

		case PROP_MUSICQUEUE_LASTDIR:
			g_value_set_string (value, self->lastdir);
			break;

		case PROP_MUSICQUEUE_REPEAT:
			g_value_set_boolean (value, self->repeat);
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
			g_free (self->font);
			self->font = g_value_dup_string (value);   
			break;	

		case PROP_MUSICQUEUE_LASTDIR:
			g_free (self->lastdir);
			self->lastdir = g_value_dup_string (value);

			break;

		case PROP_MUSICQUEUE_REPEAT:
			self->repeat = g_value_get_boolean (value);

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

        if (self->client)
        {

            if((list = get_list(self)) != NULL)
            {
                plist_reader_write_list("/home/kyle/test.xspf",list,self->read);
                g_list_free(list);
            }


            g_object_get(G_OBJECT(self),"musicqueue-font",&font,NULL);
            g_object_get(G_OBJECT(self),"musicqueue-repeat",&repeat,NULL);

            //save all the props we want to gconf
            gconf_client_set_string              (self->client,
                                                  "/apps/musicplayer/font",
                                                  font,
                                                  NULL);

            gconf_client_set_bool           (self->client,
                                             "/apps/musicplayer/repeat",
                                             repeat,
                                             NULL);


            g_object_unref(self->client);
            self->client = NULL;
            g_free(font);
            //g_object_unref(self->store);
            //g_object_unref(self->read);


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
static void foreach_xspf(gpointer data,gpointer user_data)
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
		self->i++;

		gtk_list_store_append(self->store, &iter);

		g_strchomp((gchar *)track->uri);
		out = gnome_vfs_get_local_path_from_uri((gchar *)track->uri);


		gtk_list_store_set(self->store,&iter,COLUMN_URI,track->uri,-1);  

		g_snprintf(buffer,10,"%i",self->i);
		gtk_list_store_set(self->store,&iter,COLUMN_ID,buffer,-1);

        file =g_file_new_for_uri(track->uri);
         info= g_file_query_info(file,G_FILE_ATTRIBUTE_TIME_MODIFIED,
                            G_FILE_QUERY_INFO_NONE,
                            NULL,
                            NULL);    
        
        
        mod = g_file_info_get_attribute_uint64(info,
                                               G_FILE_ATTRIBUTE_TIME_MODIFIED); 

      g_snprintf(buffer,20,"%lu",(unsigned long int)mod);

        
	  gtk_list_store_set(self->store,&iter,COLUMN_MOD,buffer,-1);
       
        g_object_unref(file);
        g_object_unref(info);
		tokens=g_strsplit(out,toke,10);
        
		

		 //take out the '/' in the uri
		 for(i=1; tokens[i] != NULL; i++);

		 if(track->title!= NULL && track->artist !=NULL)
		 {	
			 gtk_list_store_set(self->store,&iter,COLUMN_TITLE,track->title,-1);
			 gtk_list_store_set(self->store,&iter,COLUMN_ARTIST,track->artist,-1);
			 g_snprintf(buffer,strlen(track->artist)+strlen(track->title) +4
                       ,"%s - %s",track->artist,track->title);
			 
		     gtk_list_store_set(self->store,&iter,COLUMN_SONG,buffer,-1);
			 
		 }
		 else
		 {
			 //without
			 gtk_list_store_set(self->store,&iter,COLUMN_SONG,(gpointer *)tokens[i-1]);   
		 }  

		 ts_metadata_free(track);
		 g_strfreev(tokens);  
		 g_free(out);
	 }
}

static void music_queue_read_xspf(gchar *location,MusicQueue *self)
{
     GList *list = g_list_alloc();
     plist_xspf_read(location,&list,self->read);

     g_list_foreach(list,foreach_xspf,self);

     g_list_free(list);
}

static void
music_queue_init (MusicQueue *self)
{
	gboolean repeat=FALSE;
		gchar *font;
 //need to pull in gconf stuff here
    
     //g_object_set(G_OBJECT (self), "musicqueue-font","verdanna bold 7",NULL);
	 g_object_set(G_OBJECT (self), "musicqueue-lastdir",g_get_home_dir(),NULL);

	 self->client = gconf_client_get_default();

	repeat=gconf_client_get_bool (self->client,"/apps/musicplayer/repeat",NULL);
		
	 g_object_set(G_OBJECT (self), "musicqueue-repeat",repeat,NULL);
	
     init_widgets(self);
     self->changed =FALSE;
     self->i=0;
     self->drag_started=FALSE;
     self->ts = NULL;
     self->read = plist_reader_new();
     
     music_queue_read_xspf("/home/kyle/test.xspf",self);
     
     
  
} 



static void init_widgets(MusicQueue *self)
{
     GtkTreeSelection *select;
     GtkTreeSortable *sortable;
	 



     self->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);

     gtk_box_pack_start (GTK_BOX (self),self->scrolledwindow, TRUE, TRUE, 0);
     
    
     
     gtk_widget_show (self->scrolledwindow);

     self->store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,-1);

    

       

     //add model to widget we want the jump window to have the filter store and the queue
     // to have the regular list store
     self->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(self->store));
     
     add_columns(self);

     select = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->treeview));

     gtk_widget_show (self->treeview);
     gtk_container_add (GTK_CONTAINER (self->scrolledwindow), self->treeview);

     //gtk_box_pack_start (GTK_BOX (self),self->treeview, TRUE, TRUE, 0);
     //gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (self->treeview), TRUE);

    g_object_set (G_OBJECT (self->treeview),"headers-visible"  ,FALSE,
	          	"headers-clickable",FALSE,NULL);
     
     self->openbutton = gtk_button_new_from_stock(GTK_STOCK_ADD);
     gtk_box_pack_start (GTK_BOX (self),self->openbutton, FALSE, TRUE, 0);
     gtk_widget_show(self->openbutton);


//signals

	g_signal_connect ((gpointer) self->openbutton, "released",
	                  G_CALLBACK (add),
	                  (self));
	g_signal_connect (G_OBJECT (self->treeview), "row-activated",
	                  G_CALLBACK (playfile),
	                  self);

	g_signal_connect (G_OBJECT (self->store), "row-changed",
	                  G_CALLBACK (rowchanged),
	                  self);

	g_signal_connect (G_OBJECT (self->treeview), "button_press_event",
	                  G_CALLBACK (grabfocuscb),
	                  self);

	g_signal_connect (G_OBJECT (self->treeview), "key_press_event",
	                  G_CALLBACK (remove_files_via_press),
	                  self);


	g_signal_connect (G_OBJECT (self->treeview), "button_release_event",
	                  G_CALLBACK (lostgrabfocuscb),
	                  self);

	g_signal_connect (G_OBJECT (self->treeview), "drag_begin",
	                  G_CALLBACK (dragbegin), 
	                  self);

	g_signal_connect (G_OBJECT (self->treeview), "drag_end",
	                  G_CALLBACK (dragend), 
	                  self);

	g_signal_connect(self->treeview, "drag_data_received",
	                 G_CALLBACK(onDragDataReceived),
	                 self);

	
     
//set policy
     
     

     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self->scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
     
     //gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (self->scrolledwindow), GTK_SHADOW_ETCHED_IN);

     gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->treeview),TRUE);
     
//DnD stuff
     gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->treeview),TRUE);
     gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (self->treeview),
						      targetentries, G_N_ELEMENTS (targetentries),
						      GDK_ACTION_COPY | GDK_ACTION_MOVE);

     

    

      self->currselection = gtk_tree_view_get_selection(GTK_TREE_VIEW (self->treeview));
      gtk_tree_selection_set_mode(self->currselection,GTK_SELECTION_MULTIPLE);




	self->menu = getcontextmenu(self);
	gtk_widget_show_all(self->menu);
	gtk_menu_attach_to_widget(GTK_MENU(self->menu),self->treeview,NULL);
}







static void
onDragDataReceived(GtkWidget *wgt, GdkDragContext *context, int x, int y,
                        GtkSelectionData *seldata, guint info, guint time,
                        gpointer userdata)
{
    
    MusicQueue *self = (MusicQueue *) userdata;
    char **list;
    int i;

     //add to list;
    self->ts = tag_scanner_new();

     //check to see if it was internal
    if(gtk_drag_get_source_widget (context)  == NULL){
	 list = g_uri_list_extract_uris ((char *)seldata->data);
	 for(i=0; list[i] != NULL; i++)
	 {
	      //for each selected item
	      add_file(list[i],self);
	 }
	 // gtk_drag_finish (context, TRUE, FALSE, time);

	g_strfreev (list);  
    }
    //internal drag 
    else{
     self->changed = TRUE;
    }
    
    g_object_unref(self->ts);
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
	     self->curr = iter;
	     gtk_tree_model_get (model, &iter, COLUMN_ID, &id, -1);
	     self->currid = atoi(id);

                gtk_tree_model_get (model, &iter, COLUMN_URI, &uri, -1);
		gs_playFile(self->player,uri);
                
                g_free (uri);
		        g_free(id);
                  
        }
}
void music_queue_play_selected (MusicQueue *self)
{
 //	playfile(self->treeview,self->path,
//	          NULL,(gpointer)self);
}



static void add(GtkWidget *widget,gpointer user_data)
{

    MusicQueue *self = (MusicQueue *) user_data;

    GtkWidget *dialog;
    GtkFileFilter *filter;
    gchar *lastdir = NULL;	
    gint response;

    filter = gtk_file_filter_new ();

    gtk_file_filter_set_name(filter,"Music Files");  
    gtk_file_filter_add_pattern(filter,"*.mp3");
    gtk_file_filter_add_pattern(filter,"*.flac");
    gtk_file_filter_add_pattern(filter,"*.ogg");
    gtk_file_filter_add_pattern(filter,"*.wma");


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
    self->ts = tag_scanner_new();
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),lastdir);

    g_signal_connect_swapped (dialog,
                             "response", 
                             (GCallback)file_chooser_cb,
                             self);

    gtk_widget_show_all (dialog);
    g_free(lastdir);


}
static void file_chooser_cb(GtkWidget *data, 
                            gint response,
                            gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) data;
    gboolean b = TRUE;
    GSList *list;
    GtkFileFilter *filter;
    GtkWidget *dialog = GTK_WIDGET(user_data);
    
    gchar *lastdir = NULL;	
       
    if ( response  == GTK_RESPONSE_ACCEPT )
    {

        list =  gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
        //set our last dir to one they chose
        
        g_object_set(G_OBJECT(self),"musicqueue-lastdir",
                     gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog))
                     ,NULL);	


        gtk_widget_destroy (dialog);
        g_slist_foreach (list,add_file,self);
        g_slist_free (list);
        g_object_unref(self->ts);
    }
   else if(response == 1) //folder(s) selected
    {

        list = gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
        if(check_for_folders(list))
        {
            gtk_widget_destroy (dialog);
            //need to check if the selection is a folder
            g_slist_foreach (list,traverse_folders,self);
            g_object_unref(self->ts);
        }
        g_slist_free (list);
    }


}
void add_file_ext(gpointer data,gpointer user_data)
{
	 MusicQueue *self = (MusicQueue *) user_data;
	 self->ts = tag_scanner_new();
	 add_file(data,user_data);   
	 g_object_unref(self->ts) ;
}
static gboolean check_for_folders(GSList *list)
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
static void traverse_folders(gpointer data,gpointer user_data)
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
static gboolean check_type_supported(const gchar *type)
{
    if(strcmp(type,"audio/mpeg") == 0)
        return TRUE;
    if(strcmp(type,"audio/ogg")  == 0)
        return TRUE;
    if(strcmp(type,"audio/flac") == 0)
        return TRUE;
    if(strcmp(type,"audio/wma")  == 0)
         return TRUE;

    return FALSE;
}

//uri and musicqueuestatic void add_file(gpointer data,gpointer user_data);
static void add_file(gpointer data,gpointer user_data)
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

	self->i++;


	valid = gnome_vfs_make_uri_from_input((gchar *)data);

	gtk_list_store_append(self->store, &iter);



    // gtk_list_store_set(self->store,&iter,COLUMN_TITLE,out,-1);    
    gtk_list_store_set(self->store,&iter,COLUMN_URI,valid,-1);  

    g_snprintf(buffer,10,"%i",self->i);
    gtk_list_store_set(self->store,&iter,COLUMN_ID,buffer,-1);  

    file =g_file_new_for_uri(valid);
    info= g_file_query_info(file,G_FILE_ATTRIBUTE_TIME_MODIFIED,
                            G_FILE_QUERY_INFO_NONE,
                            NULL,
                            NULL);    


    mod = g_file_info_get_attribute_uint64(info,
                                           G_FILE_ATTRIBUTE_TIME_MODIFIED); 

    g_snprintf(buffer,20,"%lu",(unsigned long int)mod);
  
        
		gtk_list_store_set(self->store,&iter,COLUMN_MOD,buffer,-1);
       
        g_object_unref(file);
        g_object_unref(info);
    
	//get meta data info
	md=ts_get_metadata(valid,self->ts);
	//printf("%s\n",(gchar *)data);
	if(md != NULL && md->title != NULL && md->artist !=NULL)
	{	  
		gtk_list_store_set(self->store,&iter,COLUMN_TITLE,md->title,-1);
		gtk_list_store_set(self->store,&iter,COLUMN_ARTIST,md->artist,-1);
		
	   	g_snprintf(buffer,strlen(md->artist)+strlen(md->title)+4,
                      "%s - %s",md->artist,md->title);
			gtk_list_store_set(self->store,&iter,COLUMN_SONG,buffer,-1);
		
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
			gtk_list_store_set(self->store,&iter,COLUMN_SONG,(gpointer *)tokens2[0]);

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
    self->player =player;
    
    g_signal_connect (player, "eof",
                      G_CALLBACK(nextFile),
                      (gpointer)self);
    
    
    
    
    return GTK_WIDGET(self);
}
static void nextFile              (GsPlayer      *player,
                                   gpointer         user_data)
{
	MusicQueue *self = (MusicQueue *) user_data;
	GtkTreePath *path;
	GtkTreeModel *model;
	GtkTreeIter iter;

	gboolean test;	

	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);


	if (self->currid > 0)
	{
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->treeview));

		gtk_tree_selection_unselect_iter(self->currselection,&self->curr);  
		if(gtk_tree_model_iter_next(model,&self->curr))
		{
			//there is a next file 
			gtk_tree_selection_select_iter(self->currselection,&self->curr); 
			playfile(GTK_TREE_VIEW(self->treeview),gtk_tree_model_get_path(model,&self->curr),NULL,user_data);
		} else{
			//repeat code check to make sure property is true
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->store),&iter) && test)
			{
				gtk_tree_selection_select_iter(self->currselection,&iter); 
				playfile(GTK_TREE_VIEW(self->treeview),gtk_tree_model_get_path(model,&iter),NULL,user_data);
			}


		}


	}
}



static void add_columns(MusicQueue *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gchar *font = NULL;
    g_object_get(G_OBJECT(self),"musicqueue-font",&font,NULL);
    
    printf("font:%s\n",font);
    
    renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"font",self->font,NULL);
    column = gtk_tree_view_column_new_with_attributes ("Artist",
											renderer,
											"text",
											COLUMN_ARTIST,
											NULL);
    
    gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN (column),TRUE);
    
    //gtk_tree_view_append_column (GTK_TREE_VIEW(self->treeview), column);
    //gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 5);
    
    //set sortable
   // gtk_tree_view_column_set_sort_column_id(column,SORTID_ARTIST);
    
    
    
    renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    
    g_object_set(G_OBJECT(renderer),"font",font,NULL);
    
    
    column = gtk_tree_view_column_new_with_attributes ("Title",
											renderer,
											"text",
											COLUMN_TITLE,
											NULL);

	// gtk_tree_view_column_set_sort_column_id(column,SORTID_TITLE);

	
 	renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    
    g_object_set(G_OBJECT(renderer),"font",font,NULL);
    


	
    column = gtk_tree_view_column_new_with_attributes ("Songs",
											renderer,
											"text",
											COLUMN_SONG,
											NULL);

	

	gtk_tree_view_append_column (GTK_TREE_VIEW(self->treeview), column);

	
    //gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN (column),TRUE);
    
    //gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 20);
    
    //sort
  
    
    //gtk_tree_view_append_column (GTK_TREE_VIEW(self->treeview), column);

	
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
static void  rowchanged  (GtkTreeModel *tree_model,
					 GtkTreePath  *path,
					 GtkTreeIter  *iter,
					 gpointer      user_data) 
{
    MusicQueue *self = (MusicQueue *) user_data;
    gchar *id;
    //Curr was deleted
    
    if(self->changed)
    {
	   gtk_tree_model_get (tree_model, iter, COLUMN_ID, &id, -1);
	   
	   if(atoi(id) == self->currid)
	   {
		  self->curr = *iter;
		  
	   }
	   
	   g_free (id);
	   self->changed = FALSE;
    }
    
}


static gboolean grabfocuscb (GtkWidget *widget,
					    GdkEventButton *event,
					    gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    GtkWidget *menu;
    GtkTreeModel *model;					      
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->treeview));
    
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->treeview),TRUE);

	if((event->button ==3) && (event->type == GDK_BUTTON_PRESS))
	{
		if(has_selected(user_data))
			gtk_widget_set_sensitive(self->delete,TRUE);
		else
			gtk_widget_set_sensitive(self->delete,FALSE);
			
		gtk_menu_popup(GTK_MENU(self->menu),NULL,NULL,
                       NULL,NULL,event->button,event->time);
		return FALSE;
	}
    
    

    return FALSE;
}


static gboolean lostgrabfocuscb (GtkWidget *widget,
						   GdkEventButton *event,
						   gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    
    if(!self->drag_started)
    {
	   gtk_drag_dest_unset (self->treeview);
	   gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->treeview),FALSE);
	   gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (self->treeview),
									 targetentries, G_N_ELEMENTS (targetentries),
									 GDK_ACTION_COPY | GDK_ACTION_MOVE);
	   
    }
    return FALSE;
    
}


static void dragbegin (GtkWidget *widget,
				   GdkDragContext *contex,
				   gpointer user_data)

{
    
    MusicQueue *self = (MusicQueue *) user_data;
    self->drag_started = TRUE;
    
}


static void dragend (GtkWidget *widget,
				 GdkDragContext *contex,
				 gpointer user_data)

{
    
    MusicQueue *self = (MusicQueue *) user_data;
    self->drag_started = FALSE;
    
    //gtk_drag_dest_unset (self->treeview);
    //gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->treeview),FALSE);
    
    //bug:
    // user has to have a row selected for external dnd to work
}


static GtkWidget * getcontextmenu(gpointer user_data)
{
    
    GtkItemFactory *item_factory;
    GtkWidget  *menu,*font,*repeat,*sort,*sort2,*seperator;
	gboolean test;
	
	MusicQueue *self = (MusicQueue *) user_data;
	
	menu = gtk_menu_new();

		
    self->delete = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE,NULL);
	font   = gtk_image_menu_item_new_from_stock(GTK_STOCK_SELECT_FONT,NULL);
    repeat =  gtk_check_menu_item_new_with_label("Repeat");
    seperator = gtk_separator_menu_item_new ();
    sort   = gtk_menu_item_new_with_label("Sort By Artist");
    sort2   = gtk_menu_item_new_with_label("Sort By Date");
	


	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);

	   	if(test)
	   		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(repeat),TRUE);
	
	g_signal_connect (G_OBJECT (self->delete), "activate",
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

    
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),self->delete);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),font);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),repeat);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),seperator);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),sort);
    gtk_menu_shell_append (GTK_MENU_SHELL(menu),sort2);
	
	   
    return menu;
    
}
static gboolean remove_files_via_press(GtkWidget *widget,
							    GdkEventKey *event,
							    gpointer user_data)
{
    GtkWidget *jumpwindow;
    MusicQueue *self = (MusicQueue *) user_data;
    if(event->keyval == GDK_Delete && has_selected(user_data) == TRUE)
	   remove_files(NULL,user_data);

  if(event->keyval == GDK_j)
    {
       self->musicstore = music_store_new_with_model(GTK_TREE_MODEL(self->store),NULL);
       jumpwindow = jump_window_new_with_model(self->musicstore);

    g_signal_connect(jumpwindow, "jump",
	                 G_CALLBACK(gotJump),
	                 self);
           
    gtk_widget_show(jumpwindow);
       return TRUE;
    }
    
    return FALSE;
}  

static void gotJump(JumpWindow *jwindow,
			    GtkTreePath* path,gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    gtk_tree_selection_unselect_all(self->currselection);
    gtk_tree_selection_select_path(self->currselection,path); 

    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(self->treeview),path,NULL,TRUE,0.5,0.5);   
    playfile(GTK_TREE_VIEW(self->treeview),path,NULL,user_data);
}
    

static void set_font   (gpointer    callback_data,
				         guint       callback_action,
				         GtkWidget  *widget)
{
    MusicQueue *self = (MusicQueue *) callback_data;
    
    GtkWidget *   font_window = gtk_font_selection_dialog_new       ("Select playlist font"); 
    
    gtk_widget_show(font_window); 
    
}
static void set_repeat   (GtkCheckMenuItem *widget,
                          	gpointer user_data
				   		)
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


static void remove_files(GtkMenuItem *item, gpointer    
                         	callback_data)
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
    
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->treeview));
    
    
    rows = gtk_tree_selection_get_selected_rows(self->currselection,&model);
    
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
	   if(atoi(id) == self->currid)
	   {
		  if(!gtk_tree_model_iter_next(model,&self->curr))
		  {
			 //last one in tree view
			 self->currid = -1;
		  }
		  else
		  { //update the ID to the next one 
			 g_free(id);
			 gtk_tree_model_get (model, &self->curr, 
							 COLUMN_ID, &id, -1);    	 
			 self->currid = atoi(id);
		  }
	   }

      
	   gtk_list_store_remove(GTK_LIST_STORE(self->store),&iter);
	   
    }
    //free everything
    g_list_foreach(rowref_list, (GFunc) gtk_tree_row_reference_free, NULL);
    g_list_free(rowref_list);
    g_list_free(rows);
    g_free(id);
   
} 


gboolean has_selected(gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    
    GList *rows;
    GtkTreeModel *model;
     self->currselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->treeview));

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->treeview));
    rows = gtk_tree_selection_get_selected_rows(self->currselection,&model);	
    
    if(rows)
    {	
		g_list_free(rows);
	   return TRUE;
		
    }
    return FALSE;
}

static sort_by_artist(gpointer    callback_data,
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
     if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->store),&iter))
    {
       list = g_list_alloc();   
        htable = g_hash_table_new_full(g_int_hash,g_int_equal,
                                       destroy_hash_element,
                                       destroy_hash_element);
        
        do
	   {
            //need a struture that holds artist name and ID of the element
            node = g_malloc(sizeof(sortnode));
		    gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
						  &iter,COLUMN_ARTIST, &(node->title), -1); 
            gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
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
								GTK_TREE_MODEL(self->store),
								&iter));
     
        
                //need to write compare function that compares the two artist strings
        str.htable=htable;
        str.order = g_malloc(sizeof(gint)*i+1);        
        str.curr=0; //reset our counter for our new order

          g_list_foreach(list,(GFunc)traverse_tree,&str); 
        

        gtk_list_store_reorder(GTK_LIST_STORE(self->store),str.order);

        
        //finally call the rearange function on the List store with our new order 
     
    //free list of nodes
    //free our order
    g_free(str.order);

    //destroy data structures
    g_list_free(list);
    g_hash_table_destroy(htable);

    
    }
}

static sort_by_date(gpointer    callback_data,
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
    
     
     if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->store),&iter) )
    {
       list = g_list_alloc();   
        htable = g_hash_table_new_full(g_int_hash,g_int_equal,
                                       destroy_hash_element,
                                       destroy_hash_element);
        
        do
	   {
            //need a struture that holds artist name and ID of the element
            node = g_malloc(sizeof(sortnodedate));
		    gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
						  &iter,COLUMN_MOD, &(node->datestr), -1); 
            gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
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
								GTK_TREE_MODEL(self->store),
								&iter));
     
        
                //need to write compare function that compares the two artist strings
        str.htable=htable;
        str.order = g_malloc(sizeof(gint)*i+1);        
        str.curr=0; //reset our counter for our new order

          g_list_foreach(list,(GFunc)traverse_tree_by_date,&str); 
        

        gtk_list_store_reorder(GTK_LIST_STORE(self->store),str.order);

        
        //finally call the rearange function on the List store with our new order 
     
    //free list of nodes
    //free our order
    g_free(str.order);

    //destroy data structures
    g_list_free(list);
    g_hash_table_destroy(htable);
    

    
    }
}

static void destroy_hash_element(gpointer data)
{
    g_free((gint *)data);
}




gboolean            traverse_tree                       (
                                                         gpointer data,
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
gboolean            traverse_tree_by_date                       (
                                                         gpointer data,
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
static gint compare_sort_nodes(sortnode *node1, 
                               sortnode *node2,
                               gpointer userdata)
{
    int ret=0;

    gchar*title1=NULL; 
    if(node1)
        title1= node1->title; 
    gchar *title2 = NULL;
    if(node2)
        title2 = node2->title;

    //title is empty
    if(title1 == NULL || title2 == NULL)
    {
        if (title1 == NULL && title1 == NULL)
            return 0;

        ret = (title1== NULL) ? -1 : 1;

    }else
    {
        //ret =g_utf8_collate(title1,title2);
          ret =strcmp(title1,title2);
    }
    return ret;
}

static gint compare_sort_nodes_by_date(sortnodedate *node1, 
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

                                  
static GList* get_list(gpointer user_data)
{
    MusicQueue *self = (MusicQueue *) user_data;
    metadata *track = NULL;
    GtkTreeIter iter;
    GList *list = NULL;
    gchar *artist, *title, *uri;
    
    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->store),&iter))
    {
	   list = g_list_alloc();
	   do
	   {
		  track = ts_metadata_new();
		  
		  gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
						  &iter,COLUMN_TITLE, &title, -1); 
		  
		  track->title = title;
		  gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
						  &iter,COLUMN_ARTIST, &artist, -1); 
		  
		  track->artist = artist;
		  
		  gtk_tree_model_get (GTK_TREE_MODEL(self->store), 
						  &iter,COLUMN_URI, &uri, -1);
		  track->uri = uri;
		  
		  list =  g_list_append(list,(gpointer)track); 
	   }while(gtk_tree_model_iter_next(
								GTK_TREE_MODEL(self->store),
					 			&iter));
    }
    
    
    return list;
    
}
