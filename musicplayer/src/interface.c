#include <gtk/gtk.h>
#include "music-queue.h"
#include "music-seek.h"
#include "music-volume.h"
#include "player.h"
GtkWidget * create_mainwindow ()
{

     GtkWidget *mainwindow;
     GsPlayer *player;
     GtkWidget *mainvbox;
     GtkWidget *songlabel;
     GtkWidget *musicseek;


     //init player window
     player = gs_player_new();
     mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);

     gtk_window_set_resizable (GTK_WINDOW(mainwindow),FALSE);
     gtk_window_set_title (GTK_WINDOW (mainwindow), ("test"));

     
     

     //add mainvbox to mainwindow
     mainvbox = gtk_vbox_new(FALSE,0);

     gtk_container_add (GTK_CONTAINER (mainwindow), mainvbox);

     
     //song label

     songlabel = gtk_label_new ("songlabel");

     gtk_box_pack_start (GTK_BOX (mainvbox), songlabel, FALSE, FALSE,0);

     //seek widget
     musicseek = music_seek_new_with_adj_and_player(GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 1)),player);
     
  player->scroll=musicseek;
  gtk_box_pack_start (GTK_BOX (mainvbox), musicseek, FALSE, FALSE,0);

     //hbox 


  //show all

  gtk_widget_show(mainvbox);
  gtk_widget_show(songlabel);
  gtk_widget_show(musicseek);
  
  
  return mainwindow;





}
