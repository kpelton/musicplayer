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
#include <unique/unique.h>
#include <string.h>
enum
{
  COMMAND_0, /* unused: 0 is an invalid command */
  COMMAND_PLAY
};


static UniqueResponse
message_received_cb (UniqueApp         *app,
                     UniqueCommand      command,
                     UniqueMessageData *message,
                     guint              time_,
                     gpointer           user_data);


main (int argc, char *argv[])
{
    GtkWidget *mainwindow;
    MusicMainWindow *m;
    UniqueApp* app;
 
    UniqueResponse response; /* the response to our command */
     UniqueMessageData *message; /* the payload for the command */
     UniqueCommand command; 

    g_type_init(); 
     gtk_init (&argc, &argv);
    app = unique_app_new_with_commands ("org.kyle.musicplayer", NULL,
                                      "play", COMMAND_PLAY,
                                      NULL);
  if (unique_app_is_running (app))
    {

        

         message = unique_message_data_new();
        unique_message_data_set_text (message,argv[1],strlen(argv[1])+1);
        /* send_message() will block until we get our response back */
        response = unique_app_send_message (app, COMMAND_PLAY, message);

        /* the message is copied, so we need to free it before returning */
        unique_message_data_free (message);
        
    }
    else
    {


        make_pref_folder();

        gst_init (&argc, &argv);

        gconf_init(argc, argv, NULL);
        mainwindow = music_main_window_new ();

        unique_app_watch_window (app, GTK_WINDOW (mainwindow));

        gtk_widget_show (mainwindow);

        music_plugins_engine_init(MUSIC_MAIN_WINDOW(mainwindow));
        g_object_set(G_OBJECT (mainwindow), "title","squid player",NULL);


         g_signal_connect (app, "message-received", 
                           G_CALLBACK (message_received_cb), 
                           mainwindow);



        gtk_main ();


    }
    
    g_object_unref (app);
    
  return 0;
  
}

static UniqueResponse
message_received_cb (UniqueApp         *app,
                     UniqueCommand      command,
                     UniqueMessageData *message,
                     guint              time_,
                     gpointer           user_data)
{
  UniqueResponse res;
  MusicMainWindow * mainwindow  = MUSIC_MAIN_WINDOW(user_data);
  gchar *text;

  switch (command)
    {
    case UNIQUE_ACTIVATE:
      /* move the main window to the screen that sent us the command */
     // gtk_window_set_screen (GTK_WINDOW (main_window), unique_message_data_get_screen (message));
     // gtk_window_present_with_time (GTK_WINDOW (main_window), time_);
      res = UNIQUE_RESPONSE_OK;
      break;
    case COMMAND_PLAY:
      /* "foo" is a command that can fail */
       text = unique_message_data_get_text(message);
            
        music_main_play_file(mainwindow,text);
        g_free(text);    
        res = UNIQUE_RESPONSE_OK;
      
      break;
      break;
    default:
      res = UNIQUE_RESPONSE_OK;
      break;
    }
  
  return res;
}

