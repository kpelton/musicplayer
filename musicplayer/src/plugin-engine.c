
#include <string.h>
#include <gio/gio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>

#endif
#include "plugin-engine.h"
#include "plugins/music-plugin.h"



struct _MusicPluginInfo
{
	gchar        *location;
	GModule  *module;

    MusicPluginDetails *details;
    
	MusicPlugin   *plugin;

	gboolean     builtin;
	gboolean     active;
	gboolean     visible;
	guint        active_notification_id;
	guint        visible_notification_id;
};



static GHashTable *music_plugins = NULL;



static 
gboolean music_plugins_load_all (MusicMainWindow * mainwindows);
static GList * 
music_plugins_get_dirs ();
static void  
music_plugins_find_plugins (gchar * start,
                            GList **list);
static gboolean
load_file(gchar *location,MusicMainWindow * mainwindows);

static void
music_plugins_free_details(MusicPluginDetails *details);

gboolean 
music_plugins_engine_init (MusicMainWindow * mainwindow)
{
    
    music_plugins = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,NULL);
    music_plugins_load_all (mainwindow);
    
    return TRUE;

}

static 
gboolean music_plugins_load_all (MusicMainWindow * mainwindow)
{
    GList *list;
    GList *list1;
    GList *listbeg;
    list = g_list_alloc();
    listbeg = list;
#ifdef TOTEM_RUN_IN_SOURCE_TREE
     music_plugins_find_plugins("plugins",&list);
#else
    music_plugins_find_plugins(PLUGIN_DIR,&list);
#endif
    list = listbeg;

  for(list1 = list->next; list1!=NULL; list1 = list1->next)
    {   printf("file to load: %s\n",(gchar *)list1->data);

        load_file(list1->data,mainwindow);
        g_free(list1->data);
    }
    
    g_list_free(list);
    
}

static gboolean
load_file(gchar*            location,
          MusicMainWindow * mainwindow)
{
    MusicPluginInfo *info;

    GType (*register_func)();
    GType type;
    gpointer plugin_obj;
    
   info = g_malloc(sizeof(MusicPluginInfo));
   info->module = g_module_open(location,G_MODULE_BIND_LAZY);
   

    /* extract symbols from the lib */
	if (!g_module_symbol (info->module, "register_music_plugin", (void *)&register_func)) {
		g_warning ("%s", g_module_error ());
	     g_module_close(info->module);
		return FALSE;
	}

    type =(GType )register_func();
    info->location = strdup(location);
    info->plugin = g_object_new  (type,
                                  NULL,
                                  NULL);

    g_hash_table_insert (music_plugins, info->location, info);
    
    music_plugin_activate(info->plugin,mainwindow); 
    
    info->details = music_plugin_get_info(info->plugin);
    
    music_plugins_free_details(info->details);
    return TRUE;
    
}

static void
music_plugins_free_details(MusicPluginDetails *details)
{
    //still needs authors
    if(details)
    {
        if(details->name)
            g_free(details->name);
        if(details->desc)
            g_free(details->desc);
        if(details->name)
            g_free(details->copyright);
        if(details->name)
            g_free(details->website);
    }
}


static void 
music_plugins_find_plugins (gchar * start,
                            GList **list)
{
    GFileEnumerator *enumer; 
    GFileInfo *info;
    GFile *file=NULL;
    const gchar *name;
    const gchar *filetype;
    gchar *buffer;
    gchar *escaped;
    GError *err=NULL;
    const gchar *target;

    file = g_file_new_for_path((gchar *)start);

    if(file)
    {
    enumer = g_file_enumerate_children (file,
                                        G_FILE_ATTRIBUTE_STANDARD_NAME ","
                                        G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                                        G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
                                        G_FILE_ATTRIBUTE_STANDARD_TARGET_URI ,
                                        0,NULL,
                                     &err);
    }
    
    if (err != NULL)
    {
        /* Report error to user, and free error */
        fprintf (stderr, "Unable to read file: %s\n ", err->message);
        g_error_free (err);
        return;
    }


    info = g_file_enumerator_next_file(enumer,NULL,&err);

    if (err != NULL)
    {
        /* Report error to user, and free error */
        fprintf (stderr, "Unable to read file: %s\n", err->message);
        g_error_free (err);
        return;
    }

    while(info != NULL)
    {
        target = g_file_info_get_attribute_byte_string (info, G_FILE_ATTRIBUTE_STANDARD_NAME);

        if (target != NULL)
        {
            buffer = g_malloc(sizeof(gchar) *strlen(start)+strlen(target)+10);
            g_snprintf(buffer,strlen(start)+strlen(target)+10,"%s/%s",start,target);

            if(g_file_info_get_file_type(info) ==  G_FILE_TYPE_DIRECTORY)
            {
                //call recursively for child directories 
                music_plugins_find_plugins(buffer,list);
            }
            else
            {
                filetype = g_file_info_get_attribute_string (info, 
                                                             G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE);
                if(filetype)
                {
                   
                    if(strcmp("application/x-sharedlib",filetype) == 0)
                    {

                        *list = g_list_append(*list,g_strdup(buffer));
                    }

                    g_free(buffer);
                }
            }


            g_object_unref(info);
            info = g_file_enumerator_next_file(enumer,NULL,NULL);
        }
    }
    g_object_unref(file);
    g_object_unref(enumer);

    
}
