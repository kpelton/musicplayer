/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * musicplayer
 * Copyright (C)  2009 <>
 * 
 */

#include "jump-window.h"

typedef enum
{
  COLUMN_ARTIST,
  COLUMN_TITLE,
  COLUMN_SONG,
  COLUMN_URI,
  COLUMN_ID,
  N_COLUMNS,
  
}COLUMNS;


typedef enum {
	JUMP
}SIGNALS;

static signals[5];

//prototypes
static void init_widgets(JumpWindow *self, GtkTreeModel *model);
static void add_columns(JumpWindow *self);
static void jump_button_pressed(gpointer user_data);
static void row_activated(GtkTreeView *treeview,
                      GtkTreePath        *path,
                      GtkTreeViewColumn  *col,
                      gpointer data);
G_DEFINE_TYPE (JumpWindow, jump_window, GTK_TYPE_WINDOW);

static void
jump_window_init (JumpWindow *object)
{
	/* TODO: Add initialization code here */


}

static void
jump_window_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (jump_window_parent_class)->finalize (object);
}

static void
jump_window_class_init (JumpWindowClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkWindowClass* parent_class = GTK_WINDOW_CLASS (klass);

	object_class->finalize = jump_window_finalize;

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
jump_window_new_with_model (GtkTreeModel *model)
{
	GtkWidget *retval;
	JumpWindow *self;
	
    self = g_object_new (JUMP_TYPE_WINDOW, NULL);

	init_widgets(self,model);
	
	return GTK_WIDGET(self);
}

static void init_widgets(JumpWindow *self, GtkTreeModel *model)
{
 gtk_window_set_title (GTK_WINDOW (self), ("Jump"));

	self->mainhbox = gtk_hbox_new (FALSE, 0);
	self->mainvbox = gtk_vbox_new (FALSE, 0);

	self->entry = 	gtk_entry_new();

	self->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
	self->jumpbutton = gtk_button_new_with_label("Jump");
	
	 
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self->scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
     

	//main containers
	self->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	gtk_container_add (GTK_CONTAINER (self->scrolledwindow), self->treeview);
	gtk_container_add (GTK_CONTAINER (self), self->mainvbox);

	add_columns(self);
	//pack all
	gtk_box_pack_start (GTK_BOX (self->mainvbox),self->entry, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (self->mainvbox),self->scrolledwindow, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (self->mainvbox),self->jumpbutton, FALSE, FALSE, 0);

	//properties
	  gtk_window_set_default_size         (GTK_WINDOW(self),
					  250,
					  350);
	
	gtk_widget_show_all(GTK_WIDGET(self->mainvbox));
	gtk_widget_show_all(GTK_WIDGET(self->scrolledwindow));

	//signals

	g_signal_connect ((gpointer) self->jumpbutton, "released",
	                  G_CALLBACK (jump_button_pressed),
	                  (self));

	g_signal_connect (G_OBJECT (self->treeview), "row-activated",
	                  G_CALLBACK (row_activated),
	                  self);
	
}

static void jump_button_pressed(gpointer data)
{
	JumpWindow *self = JUMP_WINDOW(data);
}
static void row_activated(GtkTreeView *treeview,
                      GtkTreePath        *path,
                      GtkTreeViewColumn  *col,
                      gpointer data)
{
	JumpWindow *self = JUMP_WINDOW(data);
	
}


static void add_columns(JumpWindow *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new ();
    
    column = gtk_tree_view_column_new_with_attributes ("Artist",
											renderer,
											"text",
											COLUMN_ARTIST,
											NULL);
    
    gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN (column),TRUE);
       
    renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    
  
    
    column = gtk_tree_view_column_new_with_attributes ("Title",
											renderer,
											"text",
											COLUMN_TITLE,
											NULL);

	
 	renderer = gtk_cell_renderer_text_new ();
    g_object_set(G_OBJECT(renderer),"ellipsize",PANGO_ELLIPSIZE_END,NULL);

    column = gtk_tree_view_column_new_with_attributes ("Songs",
											renderer,
											"text",
											COLUMN_SONG,
											NULL);

	

	gtk_tree_view_append_column (GTK_TREE_VIEW(self->treeview), column);

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

}