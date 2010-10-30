/* Music-queue.c */
#include "music-store.h"
#include "music-queue.h"
#include "jump-window.h"
#include "utils.h"
#include "music-plugin-manager.h"
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gio/gio.h>
#include <string.h>
G_DEFINE_TYPE (MusicQueue, music_queue, GTK_TYPE_VBOX)

struct
{
	gchar *title;
	guint type;
	gchar *datestr; 
	guint  date;
	gint id;    
}typedef sortnode;


struct
{
	GHashTable *htable;
	gint *order;
	gint curr;
}typedef traversestr;

struct
{
	MusicQueue *self;
	gpointer user_data;
}typedef threadstr;

typedef enum
{
	SORT_DATE,
	SORT_TITLE


}sorttype;

enum
{
	COLUMN_ARTIST,
	COLUMN_TITLE,
	COLUMN_SONG,
	COLUMN_WEIGHT,
	COLUMN_PLAYING,
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

typedef enum {
	NEWFILE,
	REMOVE	
}SIGNALS;

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
add_from_dialog (GtkWidget *widget,
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
play_file (GtkTreeView *treeview,
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
handle_key_input(GtkWidget *widget,
                 GdkEventKey *key,
                 gpointer user_data);
static
gpointer add_threaded_folders(gpointer user_data);

static void 
add_file(const gchar *uri,MusicQueue *self,metadata *track);

GtkTreePath *
muisc_queue_path_from_id(MusicQueue *self,guint terms);

gboolean 
has_selected(MusicQueue *self);

/*
 static void 
 set_font   (gpointer    callback_data,
             guint       callback_action,
             GtkWidget  *widget);
			 */
static void 
set_repeat   (GtkCheckMenuItem *widget,
              gpointer user_data);


static void 
got_jump(JumpWindow *jwindow,
         GtkTreePath* path,gpointer user_data);

static gint 
compare_sort_nodes(sortnode *node1, 
                   sortnode *node2,
                   gpointer userdata);

static void
sort_by_artist(gpointer    callback_data,
               gpointer user_data);


static void 
sort_by_date(gpointer    callback_data,
             gpointer user_data);

static void
sort_list(gpointer    callback_data,
          gpointer user_data,int sorttype);

static gboolean
traverse_tree (gpointer data,
               gpointer userdata);



static void 
plugins_item_selected  (gpointer    callback_data,
                        guint       callback_action,
                        GtkWidget  *widget);
static void 
jump_to_current_song(gpointer    callback_data,
                     gpointer user_data);

static void remove_files_from_list(GList * rows,
                                   MusicQueue *self);

static void 
remove_duplicates(GtkMenuItem *item, 
                  gpointer  callback_data);

static gpointer 
add_threaded_dlist(gpointer user_data);

static gpointer 
add_threaded_slist(gpointer user_data);

static void 
add_to_side_queue(gpointer    callback_data,
                  gpointer user_data);
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
	guint currid;
	guint size;
	gboolean changed;
	gchar *font;
	gchar *lastdir;
	gboolean drag_started;
	gboolean repeat;
	GSList *list;
	GList *dlist;
	GMutex *mutex;
	GThread *thread;
	MusicSideQueue *sidequeue;
};

//end private varibles

//globals
static int signals[5];
//

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
		const char *home;
		char *outputdir;




		home = g_getenv ("HOME");

		outputdir = g_strdup_printf("%s/.musicplayer/pl.xspf",home);

		self->priv->read = PLAYLIST_READER(xspf_reader_new());

		if (self->priv->client)
		{

			if((list = music_queue_get_list(self)) != NULL)
			{
				playlist_reader_write_list(self->priv->read,outputdir,list);
				free(outputdir);
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
			g_object_unref(self->priv->read);
			g_object_unref(self->priv->sidequeue);
			g_object_unref(self->priv->ts);

			if(self->priv->thread == NULL)
				g_mutex_free(self->priv->mutex);	


			G_OBJECT_CLASS (music_queue_parent_class)->dispose (object);
		}
	}
}
static void
music_queue_finalize (GObject *object)
{


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


	signals[NEWFILE]= g_signal_new ("new-file",
	                                G_TYPE_FROM_CLASS (klass),
	                                G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                                0 /* closure */,
	                                NULL /* accumulator */,
	                                NULL /* accumulator data */,
	                                g_cclosure_marshal_VOID__POINTER,                            
	                                G_TYPE_NONE /* return_tpe */,
	                                1,
	                                G_TYPE_POINTER);

	signals[REMOVE]= g_signal_new ("remove-file",
	                               G_TYPE_FROM_CLASS (klass),
	                               G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                               0 /* closure */,
	                               NULL /* accumulator */,
	                               NULL /* accumulator data */,
	                               g_cclosure_marshal_VOID__POINTER,                            
	                               G_TYPE_NONE /* return_tpe */,
	                               1,
	                               G_TYPE_POINTER);

	g_object_class_install_property (object_class,
	                                 PROP_MUSICQUEUE_REPEAT,
	                                 pspec);

}
static void 
foreach_playlist_file(gpointer data,
                      gpointer user_data)
{

	metadata *track = (metadata *)data;
	if(data)
		add_file(track->uri,user_data,track);
}

static void 
music_queue_read_start_playlist(gchar *location,
                                MusicQueue *self)
{
	self->priv->dlist =NULL;
	threadstr *str = g_malloc(sizeof(threadstr));	
	GList *list = NULL;
	str->self = self;


	playlist_reader_read_list(self->priv->read,location,&list);
	str->user_data = list;
	g_thread_create(add_threaded_dlist,str,TRUE,NULL);  

	g_object_unref(self->priv->read);

}

static void
music_queue_init (MusicQueue *self)
{
	gboolean repeat=FALSE;
	char *outputdir;
	const char *home;


	home = g_getenv ("HOME");

	outputdir = g_strdup_printf("%s/.musicplayer/pl.xspf",home);



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
	self->priv->i=1;
	self->priv->size=0;
	self->priv->drag_started=FALSE;
	self->priv->ts = NULL;
	self->priv->read = PLAYLIST_READER(xspf_reader_new());
	self->priv->mutex = g_mutex_new ();
	self->priv->thread = NULL;
	self->priv->sidequeue = music_side_queue_new ();

	self->priv->ts = tag_scanner_new ();

	music_queue_read_start_playlist(outputdir,self);

	g_free(outputdir);   

} 



static void 
init_widgets(MusicQueue *self)
{
	GtkTreeSelection *select;

	self->priv->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);

	gtk_box_pack_start (GTK_BOX (self),self->priv->scrolledwindow, TRUE, TRUE, 0);

	gtk_widget_show (self->priv->scrolledwindow);

	self->priv->store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT,G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,-1);

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
	                  G_CALLBACK (add_from_dialog),
	                  (self));
	g_signal_connect (G_OBJECT (self->priv->treeview), "row-activated",
	                  G_CALLBACK (play_file),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->store), "row-changed",
	                  G_CALLBACK (row_changed),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->treeview), "button_press_event",
	                  G_CALLBACK (grab_focus_cb),
	                  self);

	g_signal_connect (G_OBJECT (self->priv->treeview), "key_press_event",
	                  G_CALLBACK (handle_key_input),
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
	char **list=NULL;
	int i;
	GSList *slist = NULL;
	threadstr *str = g_malloc(sizeof(threadstr));	

	str->self = self;
	//add to list;


	//check to see if it was internal
	if(gtk_drag_get_source_widget (context)  == NULL){



		list = g_uri_list_extract_uris ((char *)seldata->data);
		for(i=0; list[i] != NULL; i++)
		{
			slist = g_slist_append (slist,list[i]);
		}

		str->user_data = slist;
		g_thread_create(add_threaded_slist,str,TRUE,NULL);  
	}

	//internal drag 
	else{
		self->priv->changed = TRUE;
	}


}


static void play_file (GtkTreeView *treeview,
                       GtkTreePath        *path,
                       GtkTreeViewColumn  *col,
                       gpointer data)

{
	MusicQueue *self = (MusicQueue *) data;
	gchar * uri=NULL;
	GtkTreeIter iter;
	GtkTreeModel *model=NULL;
	gchar *id=NULL;
	model = gtk_tree_view_get_model(treeview);

	if(self->priv->currid > 0)
		gtk_list_store_set(self->priv->store,&self->priv->curr,COLUMN_PLAYING,FALSE,-1);
	if (gtk_tree_model_get_iter (model, &iter,path))
	{
		self->priv->curr = iter;
		gtk_tree_model_get (model, &iter, COLUMN_ID, &id, -1);
		self->priv->currid = atoi(id);
		gtk_list_store_set(self->priv->store,&iter,COLUMN_PLAYING,TRUE,-1);
		gtk_list_store_set(self->priv->store,&iter,COLUMN_WEIGHT,PANGO_WEIGHT_BOLD,-1);
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

		play_file(GTK_TREE_VIEW(self->priv->treeview),list->data,
		          NULL,(gpointer)self);
	}
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (list);

}




static void 
add_from_dialog(GtkWidget *widget,
                gpointer user_data)
{

	MusicQueue *self = (MusicQueue *) user_data;
	GtkWidget *dialog=NULL;
	GtkFileFilter *filter=NULL;
	gchar *lastdir = NULL;	


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

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),lastdir);

	g_signal_connect_swapped (dialog,
	                          "response", 
	                          (GCallback)file_chooser_cb,
	                          self);

	gtk_widget_show_all (dialog);
	g_free(lastdir);


}
static
gpointer add_threaded_folders(gpointer user_data)
{
	threadstr *str = user_data;
	MusicQueue *self = (MusicQueue *) str->self;
	GSList *node = str->user_data;



	for(; node != NULL; node=node->next)
	{
		traverse_folders (node->data,self);
		if(node->data)
			g_free(node->data);
	}
	g_slist_free(self->priv->list);
	g_free(str);
	return NULL;

}
//takes list with raw uri
static
gpointer add_threaded_slist(gpointer user_data)
{
	threadstr *str = user_data;
	MusicQueue *self = (MusicQueue *) str->self;
	GSList *node = str->user_data;	

	for(; node != NULL; node=node->next)
	{
		scan_file_action (node->data,self);
		if(node->data)
			g_free(node->data);
	}
	g_slist_free(self->priv->list);
	g_free(str);

	return NULL;

}


//this function takes a dlist that already has the md struct
static
gpointer add_threaded_dlist(gpointer user_data)

{
	threadstr *str = user_data;
	MusicQueue *self = (MusicQueue *) str->self;
	GSList *node = str->user_data;	
	metadata *md = NULL;

	for(; node != NULL; node=node->next)
	{
		if(node->data)
		{
			md = (metadata *) node->data;
			add_file(md->uri,self,md); //add_file will free md
		}
	}
	g_list_free(self->priv->dlist);
	g_free(str);

	return NULL;

}


static void 
file_chooser_cb(GtkWidget *data, 
                gint response,
                gpointer user_data)


{
	MusicQueue *self = (MusicQueue *) data;
	GtkWidget *dialog = GTK_WIDGET(user_data);
	gchar *current_folder = NULL;
	GSList *slist = NULL;
	threadstr *str = g_malloc(sizeof(threadstr));	
	str->self = self;

	if(response == GTK_RESPONSE_CANCEL)
	{
		gtk_widget_destroy (dialog);
	}

	else if (response  == GTK_RESPONSE_ACCEPT)
	{
		g_mutex_lock(self->priv->mutex); 
		slist =  gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));
		//set our last dir to one they chose


		current_folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));
		if(current_folder)
		{
			g_object_set(G_OBJECT(self),"musicqueue-lastdir",
			             gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog))
			             ,NULL);	
			g_free(current_folder);
		}

		gtk_widget_destroy (dialog);

		str->user_data = slist;
		g_thread_create(add_threaded_slist,str,TRUE,NULL);  

		g_mutex_unlock(self->priv->mutex); 

	}
	else if(response == 1) //folder(s) selected
	{
		current_folder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog));
		if(current_folder)
		{
			g_object_set(G_OBJECT(self),"musicqueue-lastdir",
			             gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (dialog))
			             ,NULL);	
			g_free(current_folder);
		}

		g_mutex_lock(self->priv->mutex); 
		slist= gtk_file_chooser_get_uris (GTK_FILE_CHOOSER (dialog));

		if(check_for_folders(self->priv->list))
		{
			gtk_widget_destroy (dialog);
			str->user_data = slist;
			self->priv->thread = g_thread_create(add_threaded_folders,str,TRUE,NULL);  

		}
		g_mutex_unlock(self->priv->mutex); 
	}
}


void 
add_file_ext(gchar * data,
             gpointer user_data)
{

	scan_file_action(data,user_data);

}
static 
gboolean check_for_folders(GSList *list)
{
	GFile *file;
	GFileInfo *info;
	gboolean ret = TRUE;

	for(list=list; list!=NULL; list=list->next)
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
	GFileEnumerator *enumer=NULL; 
	GFileInfo *info=NULL;
	GFile *file=NULL;
	const gchar *target_uri;
	const gchar *uri = (gchar *) data;
	const gchar *filetype;
	gchar *buffer=NULL;
	gchar *escaped=NULL;
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
		return;
	}

	info = g_file_enumerator_next_file(enumer,NULL,&err);

	if (err != NULL)
	{
		/* Report error to user, and free error */
		fprintf (stderr, "Unable to read file: %s\n", err->message);
		g_error_free (err);
		return;
	}

	while(info != NULL)
	{
		target_uri = g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME);

		if (target_uri != NULL)
		{
			escaped = g_uri_escape_string(target_uri,NULL,TRUE);
			buffer = g_malloc(sizeof(gchar) *strlen(escaped)+strlen(uri)+2);
			g_snprintf(buffer,strlen(escaped)+strlen(uri)+2,"%s/%s",uri,escaped);

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
					add_file(buffer,user_data,NULL);
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


gboolean 
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
	GFile * file = g_file_new_for_uri((gchar *)data);
	GError *err=NULL;
	GFileInfo *info=NULL;
	gchar* uri = (gchar *)data; 

	const gchar *type;


	info =g_file_query_info (file,
	                         G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE ,
	                         0,NULL,
	                         &err);
	if (err != NULL)
	{
		/* Report error to user, and free error */
		fprintf (stderr, "Unable to read file: %s\n", err->message);
		g_error_free (err);   
		return;
	}
	else
	{
		type= g_file_info_get_attribute_string (info, 
		                                        G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
		choose_file_action(uri,type,user_data);
		g_object_unref(file);
		g_object_unref(info);
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
		add_file(uri,user_data,NULL);
	}
	else if(strcmp(type,"audio/ogg")  == 0)
	{
		add_file(uri,user_data,NULL);
	}
	else if(strcmp(type,"audio/x-flac") == 0)
	{
		add_file(uri,user_data,NULL);
	}
	else if(strcmp(type,"audio/wma")  == 0)
	{
		add_file(uri,user_data,NULL);
	}
	else if(strcmp(type,"application/xspf+xml")  == 0)
	{
		list = NULL;
		read = PLAYLIST_READER(xspf_reader_new());
		playlist_reader_read_list(read,uri,&list);
		g_list_foreach(list,foreach_playlist_file,self);
		g_object_unref(read);

		g_list_free(list);
	}

	else if(strcmp(type,"audio/x-mpegurl")  == 0)
	{
		list = NULL;
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
add_file(const gchar *uri,MusicQueue *self,metadata *track)
{
	GtkTreeIter   iter;
	gchar *name=NULL;
	GError *err =NULL;
	gchar buffer[1024];
	gchar *valid=NULL;
	GFile *file=NULL;
	GFileInfo *info=NULL;
	guint64 mod;
	metadata *md = NULL;


	self->priv->i++;

	file =g_file_new_for_commandline_arg(uri);
	info= g_file_query_info(file,"time::modified,standard::display-name",
	                        G_FILE_QUERY_INFO_NONE,  NULL,&err);    

	if(err != NULL)
	{       
		printf("%s\n",err->message);
		g_object_unref(file);
		g_object_unref(info);
		return;

	}
	valid =  g_file_get_uri(file);

	gdk_threads_enter();
	gtk_list_store_append(self->priv->store, &iter);

	//gtk_list_store_set(self->priv->store,&iter,COLUMN_TITLE,out,-1);    
	gtk_list_store_set(self->priv->store,&iter,COLUMN_URI,valid,-1);  

	g_snprintf(buffer,10,"%i",self->priv->i);
	gtk_list_store_set(self->priv->store,&iter,COLUMN_ID,buffer,-1);  


	mod = g_file_info_get_attribute_uint64(info,
	                                       G_FILE_ATTRIBUTE_TIME_MODIFIED); 

	g_snprintf(buffer,20,"%lu",(unsigned long int)mod);



	gtk_list_store_set(self->priv->store,&iter,COLUMN_MOD,buffer,-1);
	gdk_threads_leave();

	g_mutex_lock(self->priv->mutex);  //lock the tag scanner so we dont screw up the pipeline in ts
	if(!track)
		md=ts_get_metadata(valid,self->priv->ts);
	else
		md = track;
	g_mutex_unlock(self->priv->mutex); 
	gdk_threads_enter();

	if(md != NULL && md->title != NULL && md->artist !=NULL) //we have tags
	{	  
		gtk_list_store_set(self->priv->store,&iter,COLUMN_TITLE,md->title,-1);
		gtk_list_store_set(self->priv->store,&iter,COLUMN_ARTIST,md->artist,-1);

		g_snprintf(buffer,strlen(md->artist)+strlen(md->title)+4,
		           "%s - %s",md->artist,md->title);
		gtk_list_store_set(self->priv->store,&iter,COLUMN_SONG,buffer,-1);

		if (track) //either free value obtained from tag scanner or free passed in value
			ts_metadata_free(track);
		else
			ts_metadata_free(md);
		self->priv->size++;
	}
	else //no tags go by file name
	{
		name = (gchar *)parse_file_name(file);//some kind of error here so have to cast

		gtk_list_store_set(self->priv->store,&iter,COLUMN_SONG,name,-1);   
		gtk_list_store_set(self->priv->store,&iter,COLUMN_PLAYING,FALSE,-1);


		g_free(name);
		if(track)
			ts_metadata_free(track);	
		self->priv->size++;
	}


	gdk_threads_leave();

	g_signal_emit (self, signals[NEWFILE],0,NULL);
	g_free(valid);

	g_object_unref(file);
	g_object_unref(info);
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

	g_signal_connect (self->priv->player, "eof",
	                  G_CALLBACK(next_file),
	                  (gpointer)self);


	return GTK_WIDGET(self);
}

//need to free paths here
static void 
next_file            (GsPlayer      *player,
                      gpointer         user_data)
{
	MusicQueue *self = (MusicQueue *) user_data;
	GtkTreeModel *model=NULL;
	guint id=0;
	GtkTreeIter iter;
	GtkTreePath *path= NULL;

	gboolean test;	

	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);






	if (self->priv->currid > 0) //only when there are files in the playlist
	{
		gtk_tree_selection_unselect_all(self->priv->currselection); 
		gtk_list_store_set(self->priv->store,&self->priv->curr,COLUMN_PLAYING,FALSE,-1); 

		model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));
		id = music_side_queue_dequeue (self->priv->sidequeue);
		path  = muisc_queue_path_from_id(self,id);
		if(id > 0 && path != NULL) //has a sidequeue entry 
		{
			path  = muisc_queue_path_from_id(self,id);
			gtk_tree_model_get_iter(model,&self->priv->curr,path);
			gtk_tree_selection_select_iter(self->priv->currselection,&self->priv->curr);
			play_file(GTK_TREE_VIEW(self->priv->treeview),path,NULL,user_data);

			gtk_tree_path_free(path);

		}

		else if(gtk_tree_model_iter_next(model,&self->priv->curr)) //next file in list
		{
			//there is a next file 
			gtk_tree_selection_select_iter(self->priv->currselection,&self->priv->curr); 
			play_file(GTK_TREE_VIEW(self->priv->treeview),gtk_tree_model_get_path(model,&self->priv->curr),NULL,user_data);
		} else{ // go to top of the list 
			//repeat code check to make sure property is true
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter) && test)
			{
				gtk_tree_selection_select_iter(self->priv->currselection,&iter); 
				play_file(GTK_TREE_VIEW(self->priv->treeview),gtk_tree_model_get_path(model,&iter),NULL,user_data);
			}
		}
	}
}



static void 
add_columns(MusicQueue *self)
{
	GtkCellRenderer *renderer=NULL;
	GtkTreeViewColumn *column=NULL;
	gchar *font = NULL;
	g_object_get(G_OBJECT(self),"musicqueue-font",&font,NULL);

	printf("font:%s\n",font);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);

	g_object_set(G_OBJECT(renderer),"font",font,NULL);


	column = gtk_tree_view_column_new_with_attributes ("Songs",
	                                                   renderer,
	                                                   "text",
	                                                   COLUMN_SONG,
	                                                   "weight-set",
	                                                   COLUMN_PLAYING,
	                                                   "weight",
	                                                   COLUMN_WEIGHT,

	                                                   NULL);







	gtk_tree_view_append_column (GTK_TREE_VIEW(self->priv->treeview), column);


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
	GtkTreeModel *model;					      

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));

	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(self->priv->treeview),TRUE);

	if((event->button ==3) && (event->type == GDK_BUTTON_PRESS))
	{
		if(has_selected(self))
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

	GtkWidget  *menu,*repeat,*sort,*sort2,*seperator,*plugins,*current,*duplicates,*seperator2, *queue, *seperator3;
	gboolean test;

	MusicQueue *self = (MusicQueue *) user_data;

	menu = gtk_menu_new();


	self->priv->delete = gtk_image_menu_item_new_from_stock(GTK_STOCK_DELETE,NULL);

	plugins   = gtk_menu_item_new_with_label("Plugins");
	repeat =  gtk_check_menu_item_new_with_label("Repeat");
	seperator = gtk_separator_menu_item_new ();
	seperator3 = gtk_separator_menu_item_new ();
	duplicates = gtk_menu_item_new_with_label("Remove Duplicates");
	seperator2 = gtk_separator_menu_item_new ();
	current = gtk_menu_item_new_with_label("Jump To Current Song");
	sort   = gtk_menu_item_new_with_label("Sort By Artist");
	sort2   = gtk_menu_item_new_with_label("Sort By Date");
	queue = gtk_menu_item_new_with_label("Add To Side Queue");



	g_object_get(G_OBJECT(self),"musicqueue-repeat",&test,NULL);

	if(test)
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(repeat),TRUE);

	g_signal_connect (G_OBJECT (self->priv->delete), "activate",
	                  G_CALLBACK (remove_files),
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
	g_signal_connect (G_OBJECT (current), "activate",
	                  G_CALLBACK (jump_to_current_song),
	                  user_data);
	g_signal_connect (G_OBJECT (duplicates), "activate",
	                  G_CALLBACK (remove_duplicates),
	                  user_data);

	g_signal_connect (G_OBJECT (queue), "activate",
	                  G_CALLBACK (add_to_side_queue),
	                  user_data);


	gtk_menu_shell_append (GTK_MENU_SHELL(menu),self->priv->delete);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),queue);	
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),seperator3 );
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),repeat);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),plugins);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),seperator);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),current);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),duplicates);

	gtk_menu_shell_append (GTK_MENU_SHELL(menu),seperator2 );
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),sort);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu),sort2);



	return menu;

}

static void 
jump_to_current_song(gpointer    callback_data,
                     gpointer user_data)
{

	MusicQueue *self = (MusicQueue *) user_data;
	GtkTreePath *path=NULL;
	GtkTreeModel  *model = NULL;

	if(has_selected(self) == TRUE && (isPlaying(self->priv->player) || isPaused(self->priv->player)))
	{
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));

		path = gtk_tree_model_get_path (model,&self->priv->curr);
		gtk_tree_selection_unselect_all(self->priv->currselection);
		gtk_tree_selection_select_path(self->priv->currselection,path); 

		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(self->priv->treeview),path,NULL,TRUE,0.5,0.5);   
		gtk_tree_path_free (path);
	}
}
static void 
add_to_side_queue(gpointer    callback_data,
                  gpointer user_data)
{

	MusicQueue *self = (MusicQueue *) user_data;
	GtkTreeModel  *model = NULL;
	GList *list = NULL;
	GList *listptr  = NULL;
	GtkTreeIter iter;
	guint id;
	gchar *cid;




	model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));

	list = gtk_tree_selection_get_selected_rows(self->priv->currselection,&model);



	for(listptr = list; listptr != NULL; listptr=listptr->next)
	{  

		gtk_tree_model_get_iter (model, &iter,listptr->data);
		gtk_tree_model_get (model, &iter, COLUMN_ID, &cid, -1);

		id  = atoi(cid);
		music_side_queue_enqueue (self->priv->sidequeue,id);
	}


	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);      
	g_list_free (list);  



}

static gboolean 
handle_key_input(GtkWidget *widget,
                 GdkEventKey *event,
                 gpointer user_data)
{

	MusicQueue *self = (MusicQueue *) user_data;
	if(event->keyval == GDK_Delete && has_selected(self) == TRUE)
		remove_files(NULL,user_data);

	if(event->keyval == GDK_j || event->keyval == GDK_J)
	{

		make_jump_window(self);
		return TRUE;
	}
	if(event->keyval == GDK_c || event->keyval == GDK_C)
	{
		jump_to_current_song(NULL, user_data);
		return TRUE;
	}
	if(event->keyval == GDK_q || event->keyval == GDK_Q)
	{
		add_to_side_queue(NULL, user_data);
		return TRUE;
	}

	return FALSE;
}  

void
make_jump_window(MusicQueue *self)
{

	GtkWidget *jumpwindow;

	self->priv->musicstore = music_store_new_with_model(GTK_TREE_MODEL(self->priv->store),NULL);
	jumpwindow = jump_window_new_with_model_squeue(self->priv->musicstore,self->priv->sidequeue);

	g_signal_connect(jumpwindow, "jump",
	                 G_CALLBACK(got_jump),
	                 self);

	gtk_widget_show(jumpwindow);

}
static void 
got_jump(JumpWindow *jwindow,
         GtkTreePath* path,
         gpointer user_data)
{

	MusicQueue *self = (MusicQueue *) user_data;
	gtk_tree_selection_unselect_all(self->priv->currselection);
	gtk_tree_selection_select_path(self->priv->currselection,path); 

	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(self->priv->treeview),path,NULL,TRUE,0.5,0.5);   
	play_file(GTK_TREE_VIEW(self->priv->treeview),path,NULL,user_data);
}

static void 
plugins_item_selected  (gpointer    callback_data,
                        guint       callback_action,
                        GtkWidget  *widget)
{
	GtkWidget* dialog = (GtkWidget *)music_plugin_manager_new ();

	gtk_widget_show(dialog);
}
/*
 static void 
 set_font   (gpointer    callback_data,
             guint       callback_action,
             GtkWidget  *widget)
			 {

				 GtkWidget *   font_window = gtk_font_selection_dialog_new       ("Select playlist font"); 

				 gtk_widget_show(font_window); 

				 }
				 */
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


//need help function that takes a list so we can remove files other than with a click or delete
static void remove_files_from_list(GList * rows,
                                   MusicQueue *self)
{
	GList * rowref_list = NULL;
	GtkTreeIter iter;
	GtkTreePath *path=NULL;
	GtkTreeModel *model= gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));
	gchar *id=NULL;
	GtkTreeRowReference  *rowref=NULL;
	GList *node = rows;




	for(; node != NULL; node=node->next)
	{
		if(node->data){
			path = (GtkTreePath *) node->data;
			rowref = gtk_tree_row_reference_new(model, path);

			rowref_list = g_list_append(rowref_list, rowref);
		}
	}
	node = rowref_list;
	for(; node != NULL; node=node->next)
	{
		if(node->data){
			path = gtk_tree_row_reference_get_path(
			                                       (GtkTreeRowReference*) 
			                                       node->data);
			gtk_tree_model_get_iter (model, &iter,path);
			gtk_tree_model_get (model, &iter, COLUMN_ID, &id, -1);

			//last one was deleted or all was deleted
			if(atoi(id) == self->priv->currid)
			{
				if(!gtk_tree_model_iter_next(model,&self->priv->curr))
				{
					//last one in tree view
					self->priv->currid = 0;
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
			g_signal_emit (self, signals[REMOVE],0,NULL);
		}
	}
	self->priv->size -=g_list_length(rows); //update size of queue  
	//free everything   
	g_list_foreach(rowref_list, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_free(rowref_list);
	g_list_foreach (rows,(GFunc) gtk_tree_path_free, NULL);
	g_list_free(rows);
	g_free(id);



}

static void 
remove_files(GtkMenuItem *item, 
             gpointer  callback_data)
{
	MusicQueue *self = (MusicQueue *) callback_data;
	GtkTreeModel *model=NULL;
	GList *rows = NULL;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->priv->treeview));


	rows = gtk_tree_selection_get_selected_rows(self->priv->currselection,&model);
	if(rows != NULL)

		remove_files_from_list(rows,self);
} 

static void 
remove_duplicates(GtkMenuItem *item, 
                  gpointer  callback_data)
{
	MusicQueue *self = MUSIC_QUEUE(callback_data);
	GList *list=NULL;
	GHashTable *htable=NULL;
	GtkTreeIter iter;
	gchar *old = NULL;
	gchar *song =NULL;
	GtkTreePath *path=NULL;
	gboolean found = FALSE;


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter) && music_queue_get_size(self)>1)
	{
		htable = g_hash_table_new_full(g_str_hash,g_str_equal,
		                               g_free,
		                               NULL);
		do
		{
			old = NULL;
			song = NULL;
			gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
			                    &iter,COLUMN_SONG, &song, -1);


			if(song) //if the song is being added by a thread this will be NULL
				old= g_hash_table_lookup(htable,song); 

			if(!old && song )
			{
				gtk_tree_model_get_path (GTK_TREE_MODEL(self->priv->store),&iter);

				g_hash_table_insert(htable,song,song);

			}
			else if (old && song)
			{
				path = gtk_tree_model_get_path (GTK_TREE_MODEL(self->priv->store),
				                                &iter);	
				found= TRUE;
				if(path){    					      

					list = g_list_insert(list,path,1);
					printf("%s\n",song);
				}
				g_free(song);
			}
			else{ //in the proccess of adding
				fprintf(stderr,"Error Removing All Duplicates... Probably adding\n");
			}
		}while(gtk_tree_model_iter_next(
		                                GTK_TREE_MODEL(self->priv->store),
		                                &iter));

		if(found)
			remove_files_from_list(list,self);
		g_hash_table_destroy(htable);

	}



}


gboolean //posible memory leak in this function needs more investigation
has_selected(MusicQueue *self)
{


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

static void 
sort_by_artist  (gpointer    callback_data,
                 gpointer user_data)
{
	sort_list(callback_data,user_data,SORT_TITLE);
}

static void 
sort_by_date  (gpointer    callback_data,
               gpointer user_data)
{
	sort_list(callback_data,user_data,SORT_DATE);
}


static void
sort_list(gpointer    callback_data,
          gpointer user_data,int sorttype)
{
	MusicQueue *self = (MusicQueue *) user_data;

	GtkTreeIter iter;

	GHashTable *htable=NULL;
	GList *list=NULL; 
	gchar *cid=NULL;
	gint *id=NULL;
	gint *curri=NULL;
	sortnode *node=NULL;
	gint i=0;
	traversestr str;


	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter))
	{
		htable = g_hash_table_new_full(g_int_hash,g_int_equal,
		                               g_free,
		                               g_free);

		do
		{
			node = g_malloc(sizeof(sortnode));
			switch(sorttype)
			{
				case SORT_TITLE:		

					node->type = SORT_TITLE;
					gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
					                    &iter,COLUMN_SONG, &(node->title), -1);

					break;
				case SORT_DATE:

					node->type = SORT_DATE;		        
					gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), 
					                    &iter,COLUMN_MOD, &(node->datestr), -1); 
					node->date = strtoul(node->datestr,NULL, 10);

					break;
			}

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

		if(node->type == SORT_TITLE)
		{
			if(node->title)
				g_free(node->title);
		}
		if(node->type == SORT_DATE)
		{
			if(node->datestr)
				g_free(node->datestr);
		}
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
	gchar*title1=NULL; 
	gchar *title2 = NULL;
	guint date1 =0; 
	guint date2= 0;

	if(node1->type == SORT_TITLE)
	{
		if(node1)
			title1=node1->title;   
		if(node2)
			title2 = node2->title;

		//title is empty
		if(title1 == NULL || title2 == NULL)
		{
			if (title1 == NULL && title2 == NULL)
				return 0;

			ret = (artist1== NULL) ? -1 : 1;

		}
		else
		{
			ret =strcmp(title1,title2);
		}

	}
	if(node1->type  == SORT_DATE)
	{
		if(node1)
			date1= node1->date; 

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
	}
	return ret;
}


GtkTreePath *
muisc_queue_path_from_id(MusicQueue *self,guint terms)
{
	GtkTreeIter iter;
	gchar *cid;
	guint id;
	GtkTreePath* path  = NULL;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter))
	{

		do
		{

			gtk_tree_model_get (GTK_TREE_MODEL(self->priv->store), &iter, COLUMN_ID,& cid, -1);
			id= atoi(cid);
			if(id == terms)
				path = gtk_tree_model_get_path (GTK_TREE_MODEL(self->priv->store),&iter); 


		}while(gtk_tree_model_iter_next(
		                                GTK_TREE_MODEL(self->priv->store),
		                                &iter) && id != terms);
	}

	return path;


}
GList* 
music_queue_get_list(MusicQueue *self)
{

	metadata *track = NULL;
	GtkTreeIter iter;
	GList *list = NULL;
	gchar *artist, *title, *uri;

	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(self->priv->store),&iter))
	{
		list = NULL;
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

guint
music_queue_get_size(MusicQueue *self)
{
	return self->priv->size;

}
