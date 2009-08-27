/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * musicplayer
 * Copyright (C)  2009 <>
 * 
 */

#include "jump-window.h"

enum
{
  COLUMN_ARTIST,
  COLUMN_TITLE,
  COLUMN_SONG,
  COLUMN_URI,
  COLUMN_ID,
  N_COLUMNS,
  
};
static void init_widgets(JumpWindow *self, GtkTreeModel *model);
static void add_columns(JumpWindow *self);


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
	
	gtk_widget_show_all(GTK_WIDGET(self->mainvbox));
	gtk_widget_show_all(GTK_WIDGET(self->scrolledwindow));

	
	
}
static void add_columns(JumpWindow *self)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    gchar *font = NULL;
    g_object_get(G_OBJECT(self),"musicqueue-font",&font,NULL);
    

    
    renderer = gtk_cell_renderer_text_new ();
    
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

}