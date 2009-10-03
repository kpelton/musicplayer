#include <gtk/gtk.h>



#include "player.h"
#include "music-queue.h"
#include "music-main-window.h"
#include "xspf-reader.h"
#include "pl-reader.h"
#include "plugin-engine.h"
#include "music-plugin-manager.h"
#include <gconf/gconf-client.h>

main (int argc, char *argv[])
{
  GtkWidget *mainwindow;
  MusicMainWindow *m;

    g_type_init();   

    
  gst_init (&argc, &argv);
  
  gtk_init (&argc, &argv);
    
  gconf_init(argc, argv, NULL);
    mainwindow = music_main_window_new ();

    
  
  
  gtk_widget_show (mainwindow);

music_plugins_engine_init(MUSIC_MAIN_WINDOW(mainwindow));
  g_object_set(G_OBJECT (mainwindow), "title","squid player",NULL);
    

 if (argc >1) //command line arugment to file
	music_main_play_file(MUSIC_MAIN_WINDOW(mainwindow),argv[1]);


   gtk_main ();

  
  return 0;
  
}
