#include "music-plugin.h"


static void
music_plugin_base_init (gpointer g_class)
{
  static gboolean is_initialized = FALSE;

  if (!is_initialized)
    {
      /* add properties and signals to the interface here */

      is_initialized = TRUE;
    }
}

GType
music_plugin_get_type (void)
{
  static GType iface_type = 0;
  if (iface_type == 0)
    {
      static const GTypeInfo info = {
        sizeof (MusicPluginInterface),
        music_plugin_base_init,   /* base_init */
        NULL,   /* base_finalize */
      };

      iface_type = g_type_register_static (G_TYPE_INTERFACE, "MusicPlugin",
                                           &info, 0);
    }

  return iface_type;
}

gboolean music_plugin_deactivate ( MusicPlugin *self)
{
       g_return_if_fail (MUSIC_IS_PLUGIN(self));
    
     return MUSIC_PLUGIN_GET_INTERFACE (self)->music_plugin_deactivate(self);
}

gboolean music_plugin_activate ( MusicPlugin *self,MusicMainWindow *mw)
{
       g_return_if_fail (MUSIC_IS_PLUGIN(self));
    
     return MUSIC_PLUGIN_GET_INTERFACE (self)->music_plugin_activate(self,mw);
   
}
MusicPluginDetails * music_plugin_get_info ( MusicPlugin *self)
{
   g_return_if_fail (MUSIC_IS_PLUGIN(self));
    
     return MUSIC_PLUGIN_GET_INTERFACE (self)->music_plugin_get_info(self); 
}