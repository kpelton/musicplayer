#include "music-plugin.h"


G_DEFINE_TYPE (MusicPlugin, music_plugin, G_TYPE_OBJECT)
static void
music_plugin_dispose (GObject *object)
{

     G_OBJECT_CLASS (music_plugin_parent_class)->dispose (object);
  
}

static void
music_plugin_finalize (GObject *object)
{
    
   
     G_OBJECT_CLASS (music_plugin_parent_class)->finalize (object);
}
static void
music_plugin_class_init (MusicPluginClass *klass)
{
     GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = music_plugin_dispose;
    object_class->finalize = music_plugin_finalize;
}

static void
music_plugin_init (MusicPlugin *self)
{
}



gboolean music_plugin_deactivate ( MusicPlugin *self)
{
       g_return_if_fail (MUSIC_IS_PLUGIN(self));
    
     return MUSIC_PLUGIN_GET_CLASS (self)->music_plugin_deactivate(self);
}

gboolean music_plugin_activate ( MusicPlugin *self,MusicMainWindow *mw)
{
       g_return_if_fail (MUSIC_IS_PLUGIN(self));
    
     return MUSIC_PLUGIN_GET_CLASS (self)->music_plugin_activate(self,mw);
   
}


                                       
