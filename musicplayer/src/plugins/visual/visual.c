/* real-test.c */

#include "visual.h"
#include "music-plugin.h"
#include "tag-scanner.h"

#include<libnotify/notify.h>

static const char PLUGIN_NAME[] = "Visualize";
static const char DESC[] = "various visualizations ";
//tatic const char AUTHORS[][] = {"Kyle Pelton","\0"};
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
static gboolean is_configurable = FALSE;

gboolean 
visual_plugin_activate ( MusicPlugin  *self,MusicMainWindow *mw);

gboolean 
visual_plugin_deactivate( MusicPlugin *self);



G_DEFINE_TYPE (VisualPlugin, visual_plugin, MUSIC_TYPE_PLUGIN);



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

gboolean visual_plugin_activate (MusicPlugin *user_data,MusicMainWindow *mw)
{
  VisualPlugin * self = (VisualPlugin *)user_data;
    self->mw = mw;


   self->goom = gst_element_factory_make("goom","sink");
   self->video  = gst_element_factory_make("xvimagesink","video-sink");

    

   g_object_set(G_OBJECT(self->mw->player->play),"video-sink",self->video,NULL);
   g_object_set(G_OBJECT(self->mw->player->play),"vis-plugin",self->goom,NULL);
}



gboolean visual_plugin_deactivate ( MusicPlugin *user_data)
{
    VisualPlugin * self = (VisualPlugin *)user_data;
    gpointer blah = NULL;
    
    // g_object_unref(self->video);
   
  
    
   g_object_set(G_OBJECT(self->mw->player->play),"video-sink",blah,NULL);
   g_object_set(G_OBJECT(self->mw->player->play),"vis-plugin",blah,NULL);


    

}

GType 
register_music_plugin()
{
    return visual_plugin_get_type();
}


static void
visual_plugin_dispose (GObject *object)
{

     G_OBJECT_CLASS (visual_plugin_parent_class)->dispose (object);
  
}


static void
visual_plugin_init (VisualPlugin *self)
{
 
    
    
}
static void
visual_plugin_class_init (VisualPluginClass *klass)
{
   MusicPluginClass  *class = MUSIC_PLUGIN_CLASS (klass);
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
  /* implement pure virtual class function. */
  class->music_plugin_activate=visual_plugin_activate;
  class->music_plugin_deactivate=visual_plugin_deactivate;

    
   object_class->dispose = visual_plugin_dispose;



}
VisualPlugin* visual_plugin_new ()
{
    return g_object_new (VISUAL_TYPE_PLUGIN, NULL);
}

