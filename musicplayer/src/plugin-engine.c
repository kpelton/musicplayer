
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

static gboolean
load_file(gchar *location,MusicMainWindow * mainwindows);

static gboolean
plugin_is_enabled(const gchar *name)
{
	GSettings *settings;
	gchar **enabled;
	gboolean result;

	settings = music_settings_new();
	enabled = g_settings_get_strv(settings, "enabled-plugins");
	result = g_strv_contains((const gchar * const *) enabled, name);
	g_strfreev(enabled);
	g_object_unref(settings);
	return result;
}

static void
plugin_set_enabled(const gchar *name,
                   gboolean enabled)
{
	GSettings *settings;
	gchar **current;
	GPtrArray *updated;
	guint i;

	settings = music_settings_new();
	current = g_settings_get_strv(settings, "enabled-plugins");
	updated = g_ptr_array_new_with_free_func(g_free);
	for (i = 0; current[i] != NULL; i++)
	{
		if (g_strcmp0(current[i], name) != 0)
			g_ptr_array_add(updated, g_strdup(current[i]));
	}
	if (enabled)
		g_ptr_array_add(updated, g_strdup(name));
	g_ptr_array_add(updated, NULL);
	g_settings_set_strv(settings, "enabled-plugins",
	                    (const gchar * const *) updated->pdata);
	g_ptr_array_free(updated, TRUE);
	g_strfreev(current);
	g_object_unref(settings);
}

//static void
//music_plugins_free_details(MusicPluginDetails *details);

//static void
//music_plugins_engine_unload_plugin(MusicPluginInfo *info);

gboolean 
music_plugins_engine_init (MusicMainWindow * mainwindow)
{
	mw = mainwindow;

	music_plugins = g_hash_table_new (g_str_hash, g_str_equal);//, NULL,NULL);
	music_plugins_load_all (mainwindow);


	return TRUE;

}

void
music_plugins_engine_shutdown (void)
{
	GHashTableIter iter;
	gpointer value;

	if (music_plugins == NULL)
		return;

	g_hash_table_iter_init (&iter, music_plugins);
	while (g_hash_table_iter_next (&iter, NULL, &value))
	{
		MusicPluginInfo *info = value;

		if (info->plugin != NULL)
		{
			music_plugin_deactivate (info->plugin);
			g_object_unref (info->plugin);
			info->plugin = NULL;
		}
		info->active = FALSE;

		if (info->details != NULL)
		{
			g_free (info->details->name);
			g_free (info->details->desc);
			g_strfreev (info->details->authors);
			g_free (info->details->copyright);
			g_free (info->details->website);
			g_free (info->details);
			info->details = NULL;
		}
		if (info->module != NULL)
		{
			g_module_close (info->module);
			info->module = NULL;
		}
		g_free (info->location);
		g_free (info);
	}

	g_hash_table_destroy (music_plugins);
	music_plugins = NULL;
	mw = NULL;
}

static 
gboolean music_plugins_load_all (MusicMainWindow * mainwindow)
{
	GList *list=g_list_alloc();
	GList *list1;
	GList *listbeg;
	listbeg = list;
#ifdef TOTEM_RUN_IN_SOURCE_TREE
	music_plugins_find_plugins("src/plugins",&list);
#else
	music_plugins_find_plugins(PLUGIN_DIR,&list);
#endif
	list = listbeg;

	for(list1 = list->next; list1!=NULL; list1 = list1->next)
	{

		load_file(list1->data,mainwindow);
		g_free(list1->data);
	}

	g_list_free(list);
	return TRUE;
}

static gboolean
load_file(gchar*            location,
          MusicMainWindow * mainwindow)
{
	MusicPluginInfo *info;

	GType (*register_func)();
	MusicPluginDetails * (*get_details_func)();

	info = g_malloc(sizeof(MusicPluginInfo));

	if(!(info->module = g_module_open(location,G_MODULE_BIND_LAZY)))
		g_warning ("%s", g_module_error ());

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

	info->details = get_details_func();

	g_hash_table_insert (music_plugins, info->location, info);

	if(plugin_is_enabled(info->details->name))
	{
		info->active = TRUE;
		music_plugins_engine_activate_plugin(info);

	}
	else
	{
		info->active = FALSE;
	}




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
	plugin_set_enabled(info->details->name, TRUE);

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
	plugin_set_enabled(info->details->name, FALSE);

	info->active = FALSE;
	music_plugin_deactivate(info->plugin);
	g_object_unref(info->plugin);
	info->plugin=NULL;
	return TRUE;
}
/*
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
				 */
GList *
music_plugins_get_list()
{
	if(music_plugins) 
		return g_hash_table_get_values (music_plugins);
	else
		return NULL;
}
void 
music_plugins_find_plugins (gchar * start,
                            GList **list)
{
	GFileEnumerator *enumer=NULL; 
	GFileInfo *info=NULL;
	GFile *file=NULL;
	const gchar *filetype;
	gchar *buffer=NULL;
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
