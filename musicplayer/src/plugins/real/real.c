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


gboolean 
real_test_music_plugin_activate ( MusicPlugin *self,MusicMainWindow *mw);

gboolean 
real_test_music_plugin_deactivate ( MusicPlugin *self);

MusicPluginDetails * 
real_test_get_info(MusicPlugin *parent);

static void real_test_new_file(GsPlayer *player,
			     metadata* p_track,gpointer user_data);

static void
real_test_interface_init(MusicPluginInterface *iface);

G_DEFINE_TYPE_WITH_CODE (RealTest, real_test, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (MUSIC_TYPE_PLUGIN,real_test_interface_init));

static void
real_test_interface_init(MusicPluginInterface *iface)
{

    iface->music_plugin_activate = real_test_music_plugin_activate;
    iface->music_plugin_deactivate = real_test_music_plugin_deactivate;
    iface->music_plugin_get_info = real_test_get_info;
}

MusicPluginDetails * 
real_test_get_info(MusicPlugin *parent)
{   
    RealTest * self = (RealTest *)parent;
    MusicPluginDetails *info;

    info = g_malloc(sizeof(MusicPluginDetails));

    info->name = g_strdup(PLUGIN_NAME);
    info->desc = g_strdup(DESC);
    info->copyright = g_strdup(COPYRIGHT);
    info->website = g_strdup(WEBSITE);

    return info;
    
    
}
/*
typedef struct{
    
    gchar        *name;
	gchar        *desc;
	gchar        **authors;
	gchar        *copyright;
	gchar        *website;
}MusicPluginDetails;
*/

static gboolean real_test_eof(gpointer player,RealTest * self)
{
    g_signal_handler_unblock(player,self->id);
}
gboolean real_test_music_plugin_activate ( MusicPlugin *self,MusicMainWindow *mw)
{
RealTest * real = (RealTest *)self;
     real->mw = mw;
     printf("it's working\n\n");
    g_object_ref(real->mw);

    g_signal_connect(mw->player, "eof",
				 G_CALLBACK(real_test_eof),
				 self);
    
   real->id = g_signal_connect (mw->player, "new-file",
                      G_CALLBACK(real_test_new_file),
                      (gpointer)real);
    
    g_signal_handler_block(mw->player,real->id); 
}

static void real_test_new_file(GsPlayer *player,
			     metadata* p_track,gpointer user_data)
{
    NotifyNotification *example;
    RealTest * self = (RealTest *)user_data;
    if(p_track->artist && p_track->title)
    {
    // initiate notify
  

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
    g_signal_handler_block(player,self->id); 
  
}
gboolean real_test_music_plugin_deactivate ( MusicPlugin *self)
{
    notify_uninit();  
    RealTest * real = (RealTest *)self;
     printf("destruction \n");
         g_object_unref(real->mw);
        g_object_unref(real);
}

GType 
register_music_plugin()
{
    return real_test_get_type();
}

static void
real_test_class_init (RealTestClass *klass)
{
}

static void
real_test_init (RealTest *self)
{
   notify_init("mplayer");
}

RealTest*
real_test_new (void)
{
  return g_object_new (REAL_TYPE_TEST, NULL);
}

