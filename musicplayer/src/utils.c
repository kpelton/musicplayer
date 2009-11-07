#include <glib.h>
#include <string.h>


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