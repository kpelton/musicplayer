/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * musicplayer
 * Copyright (C)  2009 <>
 * 
 */

#include "jump-window.h"



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

