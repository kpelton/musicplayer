
#include <string.h>
#include <gio/gio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>

#endif
#include "plugin-engine.h"

typedef  void  (*plugininfo)(char *);

struct _MusicPluginInfo
{
	gchar        *file;

	gchar        *location;
	GModule  *module;

	gchar        *name;
	gchar        *desc;
	gchar        **authors;
	gchar        *copyright;
	gchar        *website;

	//gchar        *icon_name;
	//GdkPixbuf    *icon_pixbuf;

	//MusicPlugin   *plugin;

	gboolean     builtin;
	gboolean     active;
	gboolean     visible;
	guint        active_notification_id;
	guint        visible_notification_id;
};
static GHashTable *music_plugins = NULL;
static MusicMainWindow * mainwindow;


static 
gboolean music_plugins_load_all ();
static GList * 
music_plugins_get_dirs ();
static void  
music_plugins_find_plugins (gchar * start,
                            GList **list);
static gboolean
load_file(gchar *location);

gboolean 
music_plugins_engine_init (MusicMainWindow * mainwindows)
{
    
    music_plugins = g_hash_table_new_full (g_str_hash, g_str_equal, NULL,NULL);
    mainwindow = mainwindows;
    g_object_ref(mainwindow);
    music_plugins_load_all ();
    
    return TRUE;

}

static 
gboolean music_plugins_load_all ()
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

        load_file(list1->data);
        g_free(list1->data);
    }
    
    g_list_free(list);
    
}

static gboolean
load_file(gchar *location)
{
    MusicPluginInfo *info;

    void * (*register_func)();
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
    plugin_obj = g_object_new  (type,
                                NULL,
                                NULL);
    
    
    g_module_close(info->module);

    
    return TRUE;
    
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
