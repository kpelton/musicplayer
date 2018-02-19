/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * musicplayer
 * Copyright (C)  2009 <>
	 * 
 */
//strstrcase is not a standard function need GNU source define
#define _GNU_SOURCE
#include "jump-window.h"
#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <stdlib.h>
#include <string.h>
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


typedef enum {
	JUMP
}SIGNALS;

static int signals[5];

//prototypes
static void init_widgets(JumpWindow *self, GtkTreeModel *model);
static void add_columns(JumpWindow *self);
static void jump_button_pressed(GtkButton *button, gpointer user_data);
static void queue_button_pressed(GtkButton *button, gpointer user_data);

static void row_activated(GtkTreeView *treeview,
                          GtkTreePath        *path,
                          GtkTreeViewColumn  *col,
                          gpointer data);
static void               text_written                   (GtkEntry *entry,
                                                          gpointer  user_data);

gboolean	check_visible								(GtkTreeModel *model,
									                      GtkTreeIter *iter,
									                      gpointer data);


static void			clear_entry				(GtkEntry            *entry,
							                       GtkEntryIconPosition icon_pos,
							                       GdkEvent            *event,
							                       gpointer             data);  


static gboolean key_press_event_cb(GtkWidget *widget,
                                   GdkEventKey *event,
                                   gpointer user_data);




G_DEFINE_TYPE (JumpWindow, jump_window, GTK_TYPE_WINDOW);


static void
jump_window_init (JumpWindow *object)
{
	/* TODO: Add initialization code here */


}

static void
jump_window_finalize (GObject *object)
{
	JumpWindow *self = JUMP_WINDOW(object);

	g_object_unref(self->filter);


	G_OBJECT_CLASS (jump_window_parent_class)->finalize (object);
}

static void
jump_window_dispose (GObject *object)
{


	G_OBJECT_CLASS (jump_window_parent_class)->dispose (object);

}

static void
jump_window_class_init (JumpWindowClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);


	object_class->finalize = jump_window_finalize;
	object_class->dispose = jump_window_dispose;

	signals[JUMP]= g_signal_new ("jump",
	                             G_TYPE_FROM_CLASS (klass),
	                             G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
	                             0 /* closure */,
	                             NULL /* accumulator */,
	                             NULL /* accumulator data */,
	                             g_cclosure_marshal_VOID__POINTER,                            
	                             G_TYPE_NONE /* return_tpe */,
	                             1,
	                             G_TYPE_POINTER);

}
GtkWidget*
jump_window_new_with_model_squeue (GtkTreeModel *model,MusicSideQueue *squeue)
{

	JumpWindow *self;

	self = g_object_new (JUMP_TYPE_WINDOW, NULL);
	self->squeue = squeue;

	init_widgets(self,model);

	return GTK_WIDGET(self);
}

static void init_widgets(JumpWindow *self, GtkTreeModel *model)
{
	gtk_window_set_title (GTK_WINDOW (self), ("Jump"));

	self->mainhbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	self->mainvbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);

	self->entry = 	gtk_entry_new();
	self->filter =  GTK_TREE_MODEL_FILTER(model);

	self->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	self->jumpbutton = gtk_button_new_with_label("Jump");
	self->queuebutton = gtk_button_new_with_label("Queue");


	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self->scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);


	//main containers
	self->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_container_add (GTK_CONTAINER (self->scrolledwindow), self->treeview);
	gtk_container_add (GTK_CONTAINER (self), self->mainvbox);

	add_columns(self);
	//pack all
	gtk_box_pack_start (GTK_BOX (self->mainvbox),self->entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (self->mainvbox),self->scrolledwindow, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (self->mainvbox),self->mainhbox, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (self->mainhbox),self->jumpbutton, TRUE,TRUE, 0);
	gtk_box_pack_start (GTK_BOX (self->mainhbox),self->queuebutton, TRUE, TRUE, 0);
	//properties
	gtk_window_set_default_size         (GTK_WINDOW(self),
	                                     250,
	                                     350);


	g_object_set (G_OBJECT (self->treeview),"headers-visible"  ,FALSE,
	              "headers-clickable",FALSE,NULL);

	gtk_entry_set_icon_from_icon_name (GTK_ENTRY(self->entry),GTK_ENTRY_ICON_SECONDARY, "gtk-clear");

	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER(model),check_visible,self,NULL);

	gtk_widget_show_all(GTK_WIDGET(self->mainvbox));
	gtk_widget_show_all(GTK_WIDGET(self->scrolledwindow));

	//setup


	//signals

	g_signal_connect (G_OBJECT(self->jumpbutton), "released",
	                  G_CALLBACK (jump_button_pressed),
	                  (self));

	g_signal_connect (G_OBJECT(self->queuebutton), "released",
	                  G_CALLBACK (queue_button_pressed),
	                  (self));

	g_signal_connect (G_OBJECT (self), "key_press_event",
	                  G_CALLBACK (key_press_event_cb),
	                  self);

	g_signal_connect (G_OBJECT (self->treeview), "row-activated",
	                  G_CALLBACK (row_activated),
	                  self);

	g_signal_connect (G_OBJECT (self->entry), "changed",
	                  G_CALLBACK (text_written),
	                  self);

	g_signal_connect (G_OBJECT (self->entry), "icon-release",
	                  G_CALLBACK (clear_entry),
	                  self);

	gtk_tree_model_filter_refilter (self->filter);

}

static gboolean key_press_event_cb(GtkWidget *widget,
                                   GdkEventKey *event,
                                   gpointer data)
{
	JumpWindow *self = JUMP_WINDOW(data);
	if(event->keyval == GDK_KEY_Escape)
	{
		gtk_widget_destroy(GTK_WIDGET(self));
		return TRUE;
	}

	return FALSE;

}

static void			clear_entry				(GtkEntry            *entry,
							                       GtkEntryIconPosition icon_pos,
							                       GdkEvent            *event,
							                       gpointer             data)   
{
	JumpWindow *self = JUMP_WINDOW(data);

	gtk_entry_set_text(GTK_ENTRY(self->entry),"");

}


static void jump_button_pressed(GtkButton *button,  gpointer data)
{
	JumpWindow *self = JUMP_WINDOW(data);
	GtkTreeModel *model;

	GtkTreePath *child;
	GtkTreeSelection *selection;
	GtkTreeIter iter;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->treeview));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(self->treeview));

	if(gtk_tree_selection_get_selected(selection,&model,&iter))
	{
		child  =   gtk_tree_model_get_path   (model,&iter);

		g_signal_emit (self, signals[JUMP],0,child);

	}

	gtk_widget_destroy(GTK_WIDGET(self));

}

static void queue_button_pressed(GtkButton *button,  gpointer data)
{
	JumpWindow *self = JUMP_WINDOW(data);
	GtkTreeModel *model =NULL;
	GtkTreeSelection *selection=NULL;
	GtkTreeIter iter;
	GList *list = NULL;
	gchar *id = NULL;
	guint uid;
	GtkTreePath *path=NULL;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(self->treeview));
	model = gtk_tree_model_filter_get_model(self->filter);                                     
	list = gtk_tree_selection_get_selected_rows (selection,NULL);
	
	if(g_list_length(list) >0)
	{   
		path = gtk_tree_model_filter_convert_path_to_child_path (self->filter,(GtkTreePath *)list->data);
		if(path && gtk_tree_model_get_iter(model,&iter,path))
		{         
			gtk_tree_model_get (model, &iter, COLUMN_ID, &id, -1);
			uid = atoi(id);
			music_side_queue_enqueue (self->squeue,uid);
			g_free(id);
			gtk_tree_path_free(path);
		}
	}
	
	g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
	g_list_free (list);

}

static void row_activated(GtkTreeView *treeview,
                          GtkTreePath        *path,
                          GtkTreeViewColumn  *col,
                          gpointer data)
{
	JumpWindow *self = JUMP_WINDOW(data);
	GtkTreePath *child;
	GtkTreeModel *model;
	model = gtk_tree_view_get_model(treeview);

	child = gtk_tree_model_filter_convert_path_to_child_path
		(GTK_TREE_MODEL_FILTER(model),
		 path);

	g_signal_emit (self, signals[JUMP],0,child);

	gtk_widget_destroy(GTK_WIDGET(self));

}
static void               text_written                   (GtkEntry *entry,
                                                          gpointer  data)
{
	JumpWindow *self = JUMP_WINDOW(data);
	gtk_tree_model_filter_refilter (self->filter);

}

gboolean	check_visible								(GtkTreeModel *model,
									                      GtkTreeIter *iter,
									                      gpointer data)
{

	gchar *song=NULL;
	JumpWindow *self = JUMP_WINDOW(data);
	const gchar *text;

	text = gtk_entry_get_text(GTK_ENTRY(self->entry));

	if(text){

		gtk_tree_model_get (model, iter, COLUMN_SONG, &song, -1);	
			if(song){
				if(strcasestr(song,text))
				{
					g_free(song);
					return TRUE;
				}
				else
				{
					g_free(song);
					return FALSE;
				}
			}
	}
	return TRUE;
}

static void add_columns(JumpWindow *self)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();

	renderer = gtk_cell_renderer_text_new ();
	g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);


	column = gtk_tree_view_column_new_with_attributes ("Songs",
	                                                   renderer,
	                                                   "text",
	                                                   COLUMN_SONG,
	                                                   NULL);



	gtk_tree_view_append_column (GTK_TREE_VIEW(self->treeview), column);

}
