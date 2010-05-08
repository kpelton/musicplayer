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

static void stats_plugin_new_file(MusicQueue *queue,
				  metadata* p_track,gpointer user_data);
static void stats_plugin_remove_file(MusicQueue *queue,
				     metadata* p_track,gpointer user_data);
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


/*
  static gboolean stats_plugin_eof(gpointer player,StatsPlugin * self)
  {
  g_signal_handler_unblock(player,self->id2);
  return TRUE;
  }
*/
gboolean stats_plugin_music_plugin_activate (MusicPlugin *self,MusicMainWindow *mw)
{
	StatsPlugin * stats = (StatsPlugin *)self;
	stats->mw = mw;
	printf("it's working\n\n");
	g_object_ref(stats->mw);
	GList *list=NULL;
	GList *node=NULL;
    
	stats->id1 = g_signal_connect (mw->queue, "new-file",
				       G_CALLBACK(stats_plugin_new_file),
				       (gpointer)stats);
    
	stats->id2 = g_signal_connect (mw->queue, "remove-file",
				       G_CALLBACK(stats_plugin_remove_file),
				       (gpointer)stats);
    
    	stats->hbox = gtk_hbox_new(FALSE,0);
      	stats->text = gtk_label_new ("");
    	gtk_box_pack_start (GTK_BOX (stats->hbox), stats->text, FALSE, FALSE,0);
	gtk_box_pack_start (GTK_BOX (stats->mw->mainvbox), stats->hbox, FALSE, FALSE,0);
    
	gtk_widget_show_all(stats->hbox);


    
	list = music_queue_get_list(mw->queue);
     
     
	for(node = list; node!=NULL; node=node->next){
	     
		if(node->data)
		{
			stats->count++;
			g_free(node->data);
		}
	}
	g_list_free(list);    
	stats->buffer = g_strdup_printf("Files:%i",stats->count); 

    
	
	draw_stats(stats);


   	stats->id3 = g_timeout_add_seconds(1,(GSourceFunc)draw_stats,self);   
	return TRUE;
}

static gboolean 
draw_stats(gpointer data)
{
	StatsPlugin * self = (StatsPlugin *)data;
    	gtk_label_set_text(GTK_LABEL(self->text),self->buffer);
    	return TRUE;
}
static void stats_plugin_new_file(MusicQueue *queue,
				  metadata* p_track,gpointer user_data)
{
	StatsPlugin * self = (StatsPlugin *)user_data;
	self->count++;

	g_free(self->buffer);
	self->buffer = g_strdup_printf("Files:%i",self->count); 

    
  
}

static void stats_plugin_remove_file(MusicQueue *queue,
				     metadata* p_track,gpointer user_data)
{
	StatsPlugin * self = (StatsPlugin *)user_data;
	self->count--;

    	g_free(self->buffer);
	self->buffer = g_strdup_printf("Files:%i",self->count); 


	draw_stats(self);
	
    
  
}

gboolean stats_plugin_music_plugin_deactivate ( MusicPlugin *user_data)
{
	StatsPlugin * self = (StatsPlugin *)user_data;

    
	g_signal_handler_disconnect (G_OBJECT (self->mw->queue),
				     self->id1);
        g_signal_handler_disconnect (G_OBJECT (self->mw->queue),
				     self->id2);
    	g_source_remove (self->id3);
   

	printf("destruction \n");
	g_object_unref(self->mw);
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