
#include <string.h>
#include <gio/gio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>

#endif
#include "plugin-engine.h"







static GHashTable *music_plugins = NULL;
static MusicMainWindow *mw = NULL;


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

static void
music_plugins_engine_unload_plugin(MusicPluginInfo *info);

gboolean 
music_plugins_engine_init (MusicMainWindow * mainwindow)
{
    
    music_plugins = g_hash_table_new (g_str_hash, g_str_equal);//, NULL,NULL);
    music_plugins_load_all (mainwindow);
    mw = mainwindow;
    
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

        //need to check if it has a gconf entry to save it
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
    MusicPluginDetails * (*get_details_func)();
    
   info = g_malloc(sizeof(MusicPluginInfo));
   info->module = g_module_open(location,G_MODULE_BIND_LAZY);
   

    /* extract symbols from the lib */
	if (!g_module_symbol (info->module, "register_music_plugin", (void *)&register_func)) {
		g_warning ("%s", g_module_error ());
	     g_module_close(info->module);
		return FALSE;
	}

        /* extract symbols from the lib */
	if (!g_module_symbol (info->module, "get_details",(void *) &get_details_func)) {
		g_warning ("%s", g_module_error ());
	     g_module_close(info->module);
		return FALSE;
	}

    info->type =(GType )register_func();
    info->location = strdup(location);
    info->active = FALSE;
    info->details = get_details_func();
    info->details->is_configurable = FALSE;
    g_hash_table_insert (music_plugins, info->location, info);
    
    
    return TRUE;
    
}


gboolean
music_plugins_engine_plugin_is_active(MusicPluginInfo *info)
{
    return info->active;
}
gboolean
music_plugins_engine_activate_plugin(MusicPluginInfo *info)
{


    info->active = TRUE;
    info->plugin = g_object_new  (info->type,
                                  NULL,
                                  NULL);
    
    music_plugin_activate(info->plugin,mw);

    return TRUE;
        
}
gboolean
music_plugins_engine_deactivate_plugin(MusicPluginInfo *info)
{
    info->active = FALSE;
    music_plugin_deactivate(info->plugin);
    g_object_unref(info->plugin);
    info->plugin=NULL;

     return TRUE;
}

static void
music_plugins_engine_unload_plugin(MusicPluginInfo *info)
{
    music_plugins_free_details(info->details);

    g_object_unref(info->plugin);
   
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

GList *
music_plugins_get_list()
{
       if(music_plugins) 
        return g_hash_table_get_values (music_plugins);
    else
        return NULL;
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
