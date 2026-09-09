/***************************************************************************
 *            utils.h
 *
 *  Tue Sep 22 09:32:18 2009
 *  Copyright  2009  kyle
 *  <kyle@<host>>
 ****************************************************************************/
#include <glib.h>
#include <gio/gio.h>
void make_pref_folder();
gchar* parse_file_name(GFile *file);
void music_main_context_invoke (GSourceFunc callback, gpointer data);
