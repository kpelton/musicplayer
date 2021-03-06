/* real-test.c */

#include "stats.h"
#include "music-plugin.h"
#include "music-queue.h"
#include "tag-scanner.h"


static const char PLUGIN_NAME[] = "Stats";
static const char DESC[] = "Information about the playlist";
//tatic const char AUTHORS[][] = {"Kyle Pelton","\0"};
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
//static gboolean is_configurable = FALSE;

gboolean 
stats_plugin_music_plugin_activate ( MusicPlugin  *self,MusicMainWindow *mw);

gboolean 
stats_plugin_music_plugin_deactivate ( MusicPlugin *self);

MusicPluginDetails * 
stats_plugin_get_info(MusicPlugin  *parent);


static gboolean 
draw_stats(gpointer data);


G_DEFINE_TYPE (StatsPlugin, stats_plugin, MUSIC_TYPE_PLUGIN);



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



gboolean stats_plugin_music_plugin_activate (MusicPlugin *self,MusicMainWindow *mw)
{
	StatsPlugin * stats = (StatsPlugin *)self;
	stats->queue = MUSIC_QUEUE(mw->queue);
	printf("it's working\n\n");
	g_object_ref(stats->queue);



	stats->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	stats->text = gtk_label_new ("");
	stats->text2 = gtk_label_new ("");
	gtk_label_set_xalign(GTK_LABEL(stats->text2),1.0f);
	gtk_label_set_yalign(GTK_LABEL(stats->text2),1.0f);
	gtk_box_pack_start (GTK_BOX (stats->hbox), stats->text, FALSE, FALSE,0);
	gtk_box_pack_start (GTK_BOX (stats->hbox), stats->text2, TRUE, TRUE,0);
	gtk_box_pack_start (GTK_BOX (mw->mainvbox), stats->hbox, FALSE, FALSE,0);

	gtk_widget_show_all(stats->hbox);





	draw_stats(self);

	stats->id3 = g_timeout_add_seconds(1,(GSourceFunc)draw_stats,self);   
	return TRUE;
}

static gboolean 
draw_stats(gpointer data)
{
	StatsPlugin * self = (StatsPlugin *)data;
	gint64 seconds = music_queue_get_length(self->queue)/GST_SECOND;
	gint64 minutes = 0;
	gint64 leftover =0;
	gint64 hours = 0;
	gchar str[51];


	minutes = seconds / 60.;
	hours = minutes/60.;
	minutes = (minutes - (60*hours));
	
	leftover = (seconds - (minutes *60));


	g_snprintf(str,50,"%li Hours %li Minutes" ,hours,minutes);
	
	gchar *buffer = g_strdup_printf("Files:%u", music_queue_get_size(self->queue));

	gtk_label_set_text(GTK_LABEL(self->text),buffer);
	gtk_label_set_text(GTK_LABEL(self->text2),str);
	g_free(buffer);
	return TRUE;
}




gboolean stats_plugin_music_plugin_deactivate ( MusicPlugin *user_data)
{
	StatsPlugin * self = (StatsPlugin *)user_data;


	g_source_remove (self->id3);


	printf("destruction \n");
	g_object_unref(self->queue);

	gtk_widget_destroy(self->hbox); 
	return TRUE;
}

GType 
register_music_plugin()
{
	return stats_plugin_get_type();
}


static void
stats_plugin_dispose (GObject *object)
{

	G_OBJECT_CLASS (stats_plugin_parent_class)->dispose (object);

}

static void
stats_plugin_finalize (GObject *object)
{


	G_OBJECT_CLASS (stats_plugin_parent_class)->finalize (object);
}
static void
stats_plugin_init (StatsPlugin *self)
{
	self->count = 0;

}
static void
stats_plugin_class_init (StatsPluginClass *klass)
{
	MusicPluginClass  *class = MUSIC_PLUGIN_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	/* implement pure virtual class function. */
	class->music_plugin_activate=stats_plugin_music_plugin_activate;
	class->music_plugin_deactivate=stats_plugin_music_plugin_deactivate;


	object_class->dispose = stats_plugin_dispose;
	object_class->finalize = stats_plugin_finalize;


}

StatsPlugin*
stats_plugin_new (void)
{
	return g_object_new (STATS_TYPE_PLUGIN, NULL);
}

