/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * musicplayer
 * Copyright (C)  2009 <>
 * 
 */

#ifndef _JUMP_WINDOW_H_
#define _JUMP_WINDOW_H_

#include <glib-object.h>
#include <gtk/gtk.h>
#include "music-side-queue.h"

G_BEGIN_DECLS

#define JUMP_TYPE_WINDOW             (jump_window_get_type ())
#define JUMP_WINDOW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), JUMP_TYPE_WINDOW, JumpWindow))
#define JUMP_WINDOW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), JUMP_TYPE_WINDOW, JumpWindowClass))
#define JUMP_IS_WINDOW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), JUMP_TYPE_WINDOW))
#define JUMP_IS_WINDOW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), JUMP_TYPE_WINDOW))
#define JUMP_WINDOW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), JUMP_TYPE_WINDOW, JumpWindowClass))

typedef struct _JumpWindowClass JumpWindowClass;
typedef struct _JumpWindow JumpWindow;

struct _JumpWindowClass
{
	GtkWindowClass parent_class;
};

struct _JumpWindow
{
	GtkWindow parent_instance;
	GtkWidget *mainvbox;
    GtkWidget *mainhbox;
	GtkWidget *entry;    
	GtkWidget *treeview;
	GtkWidget *openbutton;
	GtkWidget *scrolledwindow;
	GtkWidget *jumpbutton;
	GtkWidget *queuebutton;
	GtkTreeModelFilter *filter;
	MusicSideQueue *squeue;
	
};

GType jump_window_get_type (void) G_GNUC_CONST;

GtkWidget*
jump_window_new_with_model_squeue (GtkTreeModel *model,MusicSideQueue *squeue);
G_END_DECLS

#endif /* _JUMP_WINDOW_H_ */
