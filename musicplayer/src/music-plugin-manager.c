/* music-plugin-manager.c */

#include "music-plugin-manager.h"
#include "plugin-engine.h"
#include "plugins/music-plugin.h"

G_DEFINE_TYPE (MusicPluginManager, music_plugin_manager, GTK_TYPE_WINDOW)


struct _MusicPluginManagerPrivate {
	GtkWidget	*tree;
	GtkWidget	*about;
	GtkWidget	*config;
	GtkWidget	*popup_menu;
	int dummy;
};

enum
{
	ACTIVE_COLUMN,
	INFO_COLUMN,
	N_COLUMNS
};

static void 
music_plugin_init_widgets(MusicPluginManager *self);
static void
music_plugin_manager_populate_lists (MusicPluginManager *pm);

static void
music_plugin_manager_view_info_cell_cb (GtkTreeViewColumn *tree_column,
                                        GtkCellRenderer   *cell,
                                        GtkTreeModel      *tree_model,
                                        GtkTreeIter       *iter,
                                        gpointer           data);
static void
active_toggled_cb (GtkCellRendererToggle *cell,
                   gchar                 *path_str,
                   MusicPluginManager  *self);
static void
row_activated_cb(GtkTreeView       *treeview,            
                 gpointer data);

static gboolean
music_plugin_manager_set_active (MusicPluginManager *pm,
                                 GtkTreeIter        *iter,
                                 GtkTreeModel       *model,
                                 gboolean            active);

static void
about_button_cb(GtkButton *button,
                gpointer userdata);

static void
music_plugin_manager_finalize (GObject *object)
{
	G_OBJECT_CLASS (music_plugin_manager_parent_class)->finalize (object);
}


static void
music_plugin_manager_class_init (MusicPluginManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);



	object_class->finalize = music_plugin_manager_finalize;
	g_type_class_add_private (klass, sizeof (MusicPluginManagerPrivate));

}

static void
music_plugin_manager_construct_tree (MusicPluginManager *self);

static void
music_plugin_manager_init (MusicPluginManager *self)
{
	self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, MUSIC_TYPE_PLUGIN_MANAGER, 
	                                          MusicPluginManagerPrivate);
	music_plugin_init_widgets(self);
	music_plugin_manager_populate_lists(self);

}

static void 
music_plugin_init_widgets(MusicPluginManager *self)
{

	GtkWidget *label;
	GtkWidget *alignment;
	GtkWidget *viewport;
	GtkWidget *hbuttonbox;
	//GtkWidget *config;
	GtkWidget *pm;
	gchar *markup;



	label = gtk_label_new (NULL);
	markup = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>",
	                                  "Active plugins");
	gtk_label_set_markup (GTK_LABEL (label), markup);
	g_free (markup);

	pm = gtk_vbox_new(FALSE,0);

	gtk_container_add (GTK_CONTAINER (self), pm);

	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

	gtk_box_pack_start (GTK_BOX (pm), label, FALSE, TRUE, 0);

	alignment = gtk_alignment_new (0., 0., 1., 1.);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 0, 0, 12, 0);
	gtk_box_pack_start (GTK_BOX (pm), alignment, TRUE, TRUE, 0);

	viewport = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (viewport),
	                                GTK_POLICY_AUTOMATIC,
	                                GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (alignment), viewport);

	self->priv->tree = gtk_tree_view_new ();
	music_plugin_manager_construct_tree(self);
	gtk_container_add (GTK_CONTAINER (viewport), self->priv->tree);

	hbuttonbox = gtk_hbox_new (FALSE,10);
	gtk_box_pack_start (GTK_BOX (pm), hbuttonbox, FALSE, FALSE, 0);


	self->priv->about = gtk_button_new_from_stock (GTK_STOCK_ABOUT);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), self->priv->about);


	self->priv->config = gtk_button_new_from_stock( GTK_STOCK_PREFERENCES);
	gtk_container_add (GTK_CONTAINER (hbuttonbox), self->priv->config);


	gtk_widget_set_size_request (GTK_WIDGET (viewport), 270, 100);

	g_signal_connect (self->priv->about,
	                  "clicked",
	                  G_CALLBACK (about_button_cb),
	                  self);

	/*
	 g_signal_connect (pm->priv->configure_button,
	                   "clicked",
	                   G_CALLBACK (configure_button_cb),
					   pm);
					   */
	gtk_widget_show_all(pm);

}
static void
about_button_cb(GtkButton *button,
                gpointer userdata)
{

	MusicPluginManager *self = (MusicPluginManager *)userdata;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(self->priv->tree));
	MusicPluginInfo *info;
	GtkTreeIter iter;
	GtkWidget *dialog;
	GtkTreeSelection * selection;
	GtkTreePath *path;
	GList *list;

	selection   = gtk_tree_view_get_selection         (GTK_TREE_VIEW(self->priv->tree));


	list =gtk_tree_selection_get_selected_rows
		(selection,
		 &model);

	if(list)
	{
		path = list->data;

		if(gtk_tree_model_get_iter(model,&iter,path))
		{


			gtk_tree_model_get (model, &iter, INFO_COLUMN, &info, -1);

			g_return_if_fail (info != NULL);

			dialog = gtk_about_dialog_new();

			gtk_about_dialog_set_program_name   (GTK_ABOUT_DIALOG(dialog),
			                                     info->details->name);

			gtk_about_dialog_set_comments   (GTK_ABOUT_DIALOG(dialog),
			                                 info->details->desc);
			g_signal_connect_swapped (dialog,
			                          "response",
			                          G_CALLBACK (gtk_widget_destroy),
			                          dialog);

			gtk_dialog_run(GTK_DIALOG(dialog));

			g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
			g_list_free (list);
		}


	}
}

static void
music_plugin_manager_construct_tree (MusicPluginManager *self)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkListStore *model;


	model = gtk_list_store_new (N_COLUMNS,
	                            G_TYPE_BOOLEAN,
	                            G_TYPE_POINTER);

	gtk_tree_view_set_model (GTK_TREE_VIEW (self->priv->tree),
	                         GTK_TREE_MODEL (model));
	g_object_unref (model);

	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->priv->tree), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self->priv->tree), FALSE);

	/* first column */
        
	cell = gtk_cell_renderer_toggle_new ();
	g_object_set (cell, "xpad", 6, NULL);
	g_signal_connect (cell,
	                  "toggled",
	                  G_CALLBACK (active_toggled_cb),
	                  self);
	printf("test\n");
        column = gtk_tree_view_column_new_with_attributes ("Title",
	                                                   cell,
	                                                   "active",
                                                           ACTIVE_COLUMN,                                      
	                                                   NULL);
        printf("test\n");

	gtk_tree_view_column_set_spacing (column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->tree), column);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, "info");
	gtk_tree_view_column_set_resizable (column, TRUE);



	/* third column */
	cell = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, cell, TRUE);
	g_object_set (cell, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_set_cell_data_func (column, cell,
	                                         music_plugin_manager_view_info_cell_cb,
	                                         self, NULL);


	gtk_tree_view_column_set_spacing (column, 6);
	gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->tree), column);


	/*
	 g_signal_connect (pm->priv->tree,
	                   "cursor_changed",
	                   G_CALLBACK (cursor_changed_cb),
					   pm);
					   */

	g_signal_connect (self->priv->tree,
	                  "cursor-changed",
	                  G_CALLBACK (row_activated_cb),
	                  self);
	/*
	 g_signal_connect (pm->priv->tree,
	                   "button-press-event",
	                   G_CALLBACK (button_press_event_cb),
					   pm);
					   g_signal_connect (pm->priv->tree,
					                     "popup-menu",
					                     G_CALLBACK (popup_menu_cb),
										 pm);
										 */
	gtk_widget_show (self->priv->tree);
}
static void
row_activated_cb(GtkTreeView       *treeview,

                 gpointer data)
{
	MusicPluginManager *self = (MusicPluginManager *)data;
	GtkTreeModel *model = gtk_tree_view_get_model (treeview);
	MusicPluginInfo *info;
	GtkTreeIter iter;

	GtkTreeSelection * selection;
	GtkTreePath *path;
	GList *list;

	selection   = gtk_tree_view_get_selection         (treeview);


	list =gtk_tree_selection_get_selected_rows
		(selection,
		 &model);
	if(list)
	{
		path = list->data;

		if(gtk_tree_model_get_iter(model,&iter,path))
		{


			gtk_tree_model_get (model, &iter, INFO_COLUMN, &info, -1);

			g_return_if_fail (info != NULL);

			gtk_widget_set_sensitive (GTK_WIDGET (self->priv->config),
			                          info->details->is_configurable);

			g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
			g_list_free (list);
		}
	}

}






static void
active_toggled_cb (GtkCellRendererToggle *cell,
                   gchar                 *path_str,
                   MusicPluginManager  *self)
{    
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	gboolean active;

	path = gtk_tree_path_new_from_string (path_str);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->tree));
	g_return_if_fail (model != NULL);

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, ACTIVE_COLUMN, &active, -1);



	music_plugin_manager_set_active (self, &iter, model,active);

	gtk_tree_path_free (path);
}


static void
music_plugin_manager_view_info_cell_cb (GtkTreeViewColumn *tree_column,
                                        GtkCellRenderer   *cell,
                                        GtkTreeModel      *tree_model,
                                        GtkTreeIter       *iter,
                                        gpointer           data)
{
	MusicPluginDetails * details;
	gchar *text;
	MusicPluginInfo *info;



	gtk_tree_model_get (tree_model, iter, INFO_COLUMN, &info, -1);
	details = info->details;



	if (info == NULL)
		return;

	text = g_markup_printf_escaped ("<b>%s</b>\n%s",
	                                details->name,details->desc);

	g_object_set (G_OBJECT (cell),
	              "markup", text,
	              NULL);



	g_free (text);
}

static void
music_plugin_manager_populate_lists (MusicPluginManager *pm)
{
	GList *plugins;
	GtkListStore *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	MusicPluginInfo *info;
	//MusicPluginDetails * details;


	plugins = music_plugins_get_list();

	model = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (pm->priv->tree)));

	while (plugins)
	{

		info = (MusicPluginInfo *)plugins->data;


		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
		                    ACTIVE_COLUMN, music_plugins_engine_plugin_is_active (info),
		                    INFO_COLUMN, info,
		                    -1);

		plugins = plugins->next;
	}


	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{    

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (pm->priv->tree));
		g_return_if_fail (selection != NULL);

		gtk_tree_selection_select_iter (selection, &iter);

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
		                    INFO_COLUMN, &info, -1);

		gtk_widget_set_sensitive (GTK_WIDGET (pm->priv->config),
		                          info->details->is_configurable);
	}
	if(plugins)
		g_list_free(plugins);
}

static gboolean
music_plugin_manager_set_active (MusicPluginManager *pm,
                                 GtkTreeIter        *iter,
                                 GtkTreeModel       *model,
                                 gboolean            active)
{
	MusicPluginInfo *info;
	gboolean res = TRUE;



	gtk_tree_model_get (model, iter, INFO_COLUMN, &info, -1);

	g_return_val_if_fail (info != NULL, FALSE);

	if (active)
	{
		/* activate the plugin */
		if (music_plugins_engine_deactivate_plugin (info)) {
			res = FALSE;
		}

	}
	else
	{
		/* deactivate the plugin */
		if (music_plugins_engine_activate_plugin (info)) {
			res = FALSE;

		}

	}
	gtk_list_store_set (GTK_LIST_STORE (model), iter, ACTIVE_COLUMN,music_plugins_engine_plugin_is_active (info), -1);
	return res;
}

MusicPluginManager*
music_plugin_manager_new (void)
{
	MusicPluginManager* self = g_object_new (MUSIC_TYPE_PLUGIN_MANAGER, NULL);

	return self;
}

