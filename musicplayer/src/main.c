#include <gtk/gtk.h>


#include "utils.h"
#include "player.h"
#include "music-queue.h"
#include "music-main-window.h"
#include "xspf-reader.h"
#include "pl-reader.h"
#include "plugin-engine.h"
#include "music-plugin-manager.h"
#include <gconf/gconf-client.h>
#include <string.h>




int main (int argc, char *argv[])
{
	GtkWidget *mainwindow;

    g_thread_init(NULL);
	gdk_threads_init();
	g_type_init(); 
	gtk_init (&argc, &argv);

		make_pref_folder();

		gst_init (&argc, &argv);

		gconf_init(argc, argv, NULL);

		mainwindow = music_main_window_new ();
		gtk_widget_show (mainwindow);

        //		g_object_set(G_OBJECT (mainwindow), "title","squid player",NULL);


		if(argc >1)
			music_main_play_file(MUSIC_MAIN_WINDOW(mainwindow),argv[1]);

		gdk_threads_enter();

		gtk_main ();
		gdk_threads_leave();	



    
	return 0;

}
