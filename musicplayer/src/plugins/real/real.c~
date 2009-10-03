/* real-test.c */

#include "real.h"
#include "music-plugin.h"
#include "tag-scanner.h"

#include<libnotify/notify.h>

static const char PLUGIN_NAME[] = "real Plugin Test";
static const char DESC[] = "A real example of a working plugin";
//tatic const char AUTHORS[][] = {"Kyle Pelton","\0"};
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
static gboolean is_configurable = FALSE;

gboolean 
real_test_music_plugin_activate ( MusicPlugin  *self,MusicMainWindow *mw);

gboolean 
real_test_music_plugin_deactivate ( MusicPlugin *self);

MusicPluginDetails * 
real_test_get_info(MusicPlugin  *parent);

static void real_test_new_file(GsPlayer *player,
			     metadata* p_track,gpointer user_data);



G_DEFINE_TYPE (RealTest, real_test, MUSIC_TYPE_PLUGIN);



MusicPluginDetails * 
get_details()
{   

    MusicPluginDetails *info;

    info = g_malloc(sizeof(MusicPluginDetails));

    info->name = g_strdup(PLUGIN_NAME);
    info->desc = g_strdup(DESC);
    info->copyright = g_strdup(COPYRIGHT);
    info->website = g_strdup(WEBSITE);
    info->is_configurable = FALSE;

    return info;
  
    
}



static gboolean real_test_eof(gpointer player,RealTest * self)
{
    g_signal_handler_unblock(player,self->id2);
}
gboolean real_test_music_plugin_activate (MusicPlugin *self,MusicMainWindow *mw)
{
RealTest * real = (RealTest *)self;
     real->mw = mw;
     printf("it's working\n\n");
     g_object_ref(real->mw);

    real->id1 =g_signal_connect(mw->player, "eof",
				 G_CALLBACK(real_test_eof),
				 self);
    
   real->id2 = g_signal_connect (mw->player, "new-file",
                      G_CALLBACK(real_test_new_file),
                      (gpointer)real);
    
    g_signal_handler_block(mw->player,real->id2); 
}

static void real_test_new_file(GsPlayer *player,
			     metadata* p_track,gpointer user_data)
{
    NotifyNotification *example;
    RealTest * self = (RealTest *)user_data;
    if(p_track->artist && p_track->title)
    {
    // initiate notify
  
    notify_init("mplayer");
    // create a new notification
    
    example = notify_notification_new(p_track->artist,p_track->title,NULL,NULL);
 
 
    // attach that icon to the notification
 

    // set the timeout of the notification to 3 secs
    notify_notification_set_timeout(example,1500);

    // set the category so as to tell what kind it is
    char category[] = "test";
    notify_notification_set_category(example,category);

    // set the urgency level of the notification
    notify_notification_set_urgency (example,NOTIFY_URGENCY_NORMAL);

    GError *error = NULL;
    notify_notification_show(example,&error);
        
    }
    g_signal_handler_block(player,self->id2); 
  
}
gboolean real_test_music_plugin_deactivate ( MusicPlugin *user_data)
{
    RealTest * self = (RealTest *)user_data;
    notify_uninit();  
    g_signal_handler_disconnect (G_OBJECT (self->mw->player),
				    self->id1);
    g_signal_handler_disconnect (G_OBJECT (self->mw->player),
				    self->id2);

     printf("destruction \n");
    g_object_unref(self->mw);

}

GType 
register_music_plugin()
{
    return real_test_get_type();
}


static void
real_test_dispose (GObject *object)
{

     G_OBJECT_CLASS (real_test_parent_class)->dispose (object);
  
}

static void
real_test_finalize (GObject *object)
{
    
   
     G_OBJECT_CLASS (real_test_parent_class)->finalize (object);
}
static void
real_test_init (RealTest *self)
{
   
}
static void
real_test_class_init (RealTestClass *klass)
{
   MusicPluginClass  *class = MUSIC_PLUGIN_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
  /* implement pure virtual class function. */
  class->music_plugin_activate=real_test_music_plugin_activate;
  class->music_plugin_deactivate=real_test_music_plugin_deactivate;

    
   object_class->dispose = real_test_dispose;
    object_class->finalize = real_test_finalize;


}

RealTest*
real_test_new (void)
{
  return g_object_new (REAL_TYPE_TEST, NULL);
}

