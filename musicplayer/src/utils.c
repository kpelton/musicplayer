#include <glib.h>
#include <string.h>
#include <gio/gio.h>


void make_pref_folder()
{
       gchar *outputdir;
        const gchar *home;
        home = g_getenv ("HOME");

        outputdir = g_strdup_printf("%s/.musicplayer",home);


        if (!g_file_test(outputdir,G_FILE_TEST_EXISTS))
        {
            g_mkdir(outputdir,0700);
        }
         g_free(outputdir);
}

gchar* parse_file_name(GFile *file)
{
    GFileInfo *info;
    gchar *out;
    gchar **tokens;
    gchar *escaped;
    gchar * filetypes = ".mp3";

    info= g_file_query_info(file,"standard::display-name",
        G_FILE_QUERY_INFO_NONE,  NULL,NULL);    

    out = g_file_info_get_attribute_as_string(info,
        "standard::display-name"); 


    tokens=g_strsplit(out,filetypes,2);

   
    escaped = strndup(*tokens,strlen(*tokens)+10);
    	

    g_strfreev(tokens);  

    g_free(out);

    g_object_unref(info);

    return escaped;
    
	

    
    
}