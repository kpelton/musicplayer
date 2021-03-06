/* real-test.c */

#include "visual.h"
#include "music-plugin.h"
#include "tag-scanner.h"



static const char PLUGIN_NAME[] = "Visualize";
static const char DESC[] = "various visualizations ";
//tatic const char AUTHORS[][] = {"Kyle Pelton","\0"};
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
static gboolean is_configurable = TRUE;

gboolean 
visual_plugin_activate ( MusicPlugin  *self,MusicMainWindow *mw);

gboolean 
visual_plugin_deactivate( MusicPlugin *self);



G_DEFINE_TYPE (VisualPlugin, visual_plugin, MUSIC_TYPE_PLUGIN);



MusicPluginDetails * 
get_details()
{   

	MusicPluginDetails *info;

	info = g_malloc(sizeof(MusicPluginDetails));

	info->name = g_strdup(PLUGIN_NAME);
	info->desc = g_strdup(DESC);
	info->copyright = g_strdup(COPYRIGHT);
	info->website = g_strdup(WEBSITE);
	info->is_configurable = is_configurable;


	return info;


}

gboolean visual_plugin_activate (MusicPlugin *user_data,MusicMainWindow *mw)
{
	VisualPlugin * self = (VisualPlugin *)user_data;
	self->mw = mw;

	GstElement *vis_capsfilter;
	GstPad *pad;
	GstElement *vis_bin;
	GstCaps *caps = NULL;
	guint flags;
	



	vis_capsfilter = gst_element_factory_make ("capsfilter",
	                                           "vis_capsfilter");


	vis_bin = gst_bin_new("vis_bin");
	//self->goom = gst_element_factory_make("goom2k1","sink");


	self->goom = gst_element_factory_make("monoscope","sink");

	gst_bin_add_many (GST_BIN (vis_bin), self->goom,vis_capsfilter,NULL);
	/* Sink ghostpad */
	pad = gst_element_get_static_pad (self->goom, "sink");
	gst_element_add_pad (vis_bin, gst_ghost_pad_new ("sink", pad));
	gst_object_unref (pad);


	pad = gst_element_get_static_pad (vis_capsfilter, "src");
	gst_element_add_pad (vis_bin, gst_ghost_pad_new ("src", pad));
	gst_element_link_pads (self->goom, "src", vis_capsfilter, "sink");

	pad = gst_element_get_static_pad (self->goom, "src");
	caps = gst_pad_get_allowed_caps (pad);

	gst_object_unref (pad);


	caps = gst_caps_make_writable (caps);


	/* Get visualization size */
	GstStructure *s = gst_caps_get_structure (caps, 0);

	/* Fixate */
	gst_structure_fixate_field_nearest_int (s, "width", 1024);
	gst_structure_fixate_field_nearest_int (s, "height", 768);


	/* set this */
	g_object_set (vis_capsfilter, "caps", caps, NULL);



	self->bin = vis_bin;
	g_object_set(G_OBJECT(self->mw->player->play),"vis-plugin",vis_bin,NULL);
	 g_object_get (G_OBJECT(self->mw->player->play), "flags", &flags, NULL);
	flags |= (1<<3);
	g_object_set (G_OBJECT(self->mw->player->play), "flags", flags, NULL);
	return TRUE;
}



gboolean visual_plugin_deactivate ( MusicPlugin *user_data)
{
	VisualPlugin * self = (VisualPlugin *)user_data;


	gst_element_set_state (self->bin, GST_STATE_NULL);
	gst_element_set_state (self->mw->player->play, GST_STATE_NULL);
	

	g_object_set(G_OBJECT(self->mw->player->play),"vis-plugin",NULL,NULL);

	g_object_unref(self->bin);
	return TRUE;

}

GType 
register_music_plugin()
{
	return visual_plugin_get_type();
}


static void
visual_plugin_dispose (GObject *object)
{

	G_OBJECT_CLASS (visual_plugin_parent_class)->dispose (object);

}


static void
visual_plugin_init (VisualPlugin *self)
{



}
static void
visual_plugin_class_init (VisualPluginClass *klass)
{
	MusicPluginClass  *class = MUSIC_PLUGIN_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	/* implement pure virtual class function. */
	class->music_plugin_activate=visual_plugin_activate;
	class->music_plugin_deactivate=visual_plugin_deactivate;



	object_class->dispose = visual_plugin_dispose;



}
VisualPlugin* visual_plugin_new ()
{
	return g_object_new (VISUAL_TYPE_PLUGIN, NULL);
}

