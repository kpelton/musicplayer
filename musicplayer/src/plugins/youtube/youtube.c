#include "youtube.h"

#include "music-queue.h"

#include <glib/gstdio.h>

/* The plugin engine uses this name as the persisted plugin identifier, so it
 * must not contain spaces. The description supplies the user-facing wording. */
static const char PLUGIN_NAME[] = "YouTube";
static const char DESC[] = "Download audio from a YouTube URL into the playlist";
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";
static const gchar *const audio_quality_values[] = {
	"0", "320K", "256K", "192K", "128K"
};
static const gchar *const audio_quality_labels[] = {
	"Best quality", "320 kbps", "256 kbps", "192 kbps", "128 kbps"
};
#define AUDIO_QUALITY_COUNT G_N_ELEMENTS(audio_quality_values)

static gboolean youtube_plugin_activate(MusicPlugin *plugin,
                                        MusicMainWindow *mw);
static gboolean youtube_plugin_deactivate(MusicPlugin *plugin);
static GtkWidget *youtube_plugin_get_config_window(MusicPlugin *plugin);
static void youtube_download_clicked(GtkButton *button,
                                     gpointer user_data);
static void youtube_download_finished(GObject *source,
                                      GAsyncResult *result,
                                      gpointer user_data);
static void youtube_context_menu_item_activate(GtkMenuItem *item,
                                               gpointer user_data);
static void youtube_dialog_destroyed(GtkWidget *dialog,
                                     gpointer user_data);
static void youtube_quality_changed(GtkComboBox *combo,
                                    gpointer user_data);
static void youtube_plugin_dispose(GObject *object);

G_DEFINE_TYPE(YoutubePlugin, youtube_plugin, MUSIC_TYPE_PLUGIN);

typedef struct {
	YoutubePlugin *plugin;
	GSubprocess *process;
	gchar *download_path;
	GtkWidget *row;
	GtkWidget *url_label;
	GtkWidget *status_label;
	GtkWidget *progress_bar;
	guint progress_source;
	gint references;
} YoutubeDownload;

static YoutubeDownload *youtube_download_ref(YoutubeDownload *download);
static void youtube_download_unref(YoutubeDownload *download);
static void youtube_download_set_status(YoutubeDownload *download,
                                        const gchar *message);
static void youtube_download_set_progress(YoutubeDownload *download,
                                          gdouble fraction,
                                          const gchar *text);
static gboolean youtube_download_progress_pulse(gpointer user_data);
static void youtube_remove_finished_downloads(YoutubePlugin *plugin);
static void youtube_schedule_downloads_hide(YoutubePlugin *plugin);
static gboolean youtube_hide_downloads(gpointer user_data);
static YoutubeDownload *youtube_download_new(YoutubePlugin *plugin,
                                             const gchar *url);
static gboolean youtube_start_download(YoutubePlugin *self,
                                       const gchar *url);

static void
youtube_debug_command(const gchar *const *argv)
{
	guint index;

	g_printerr("youtube: executing:");
	for (index = 0; argv[index] != NULL; index++)
	{
		gchar *quoted = g_shell_quote(argv[index]);

		g_printerr(" %s", quoted);
		g_free(quoted);
	}
	g_printerr("\n");
}

static gchar *
youtube_download_directory(void)
{
	const gchar *music_directory;

	music_directory = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
	if (music_directory == NULL || music_directory[0] == '\0')
		music_directory = g_get_home_dir();

	return g_build_filename(music_directory, "MusicPlayer", "YouTube", NULL);
}

static void
youtube_set_status(YoutubePlugin *self,
                   const gchar *message)
{
	if (self->status_label != NULL)
		gtk_label_set_text(GTK_LABEL(self->status_label), message);
}

static YoutubeDownload *
youtube_download_ref(YoutubeDownload *download)
{
	download->references++;
	return download;
}

static void
youtube_download_free(YoutubeDownload *download)
{
	if (download->progress_source != 0)
	{
		g_source_remove(download->progress_source);
		download->progress_source = 0;
	}
	if (download->row != NULL)
	{
		gtk_widget_destroy(download->row);
		download->row = NULL;
		download->url_label = NULL;
		download->status_label = NULL;
		download->progress_bar = NULL;
	}
	g_clear_object(&download->process);
	g_free(download->download_path);
	g_object_unref(download->plugin);
	g_free(download);
}

static void
youtube_download_unref(YoutubeDownload *download)
{
	download->references--;
	if (download->references == 0)
		youtube_download_free(download);
}

static void
youtube_download_set_status(YoutubeDownload *download,
                             const gchar *message)
{
	if (download->status_label != NULL)
		gtk_label_set_text(GTK_LABEL(download->status_label), message);
}

static void
youtube_download_set_progress(YoutubeDownload *download,
                               gdouble fraction,
                               const gchar *text)
{
	if (download->progress_bar == NULL)
		return;

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(download->progress_bar),
	                              fraction);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(download->progress_bar), text);
}

static gboolean
youtube_download_progress_pulse(gpointer user_data)
{
	YoutubeDownload *download = user_data;

	if (download->process == NULL || download->progress_bar == NULL)
	{
		download->progress_source = 0;
		return G_SOURCE_REMOVE;
	}

	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(download->progress_bar));
	return G_SOURCE_CONTINUE;
}

static void
youtube_remove_finished_downloads(YoutubePlugin *plugin)
{
	GList *node;
	GList *next;

	for (node = plugin->downloads; node != NULL; node = next)
	{
		YoutubeDownload *download = node->data;

		next = node->next;
		if (download->process == NULL)
		{
			plugin->downloads = g_list_delete_link(plugin->downloads, node);
			youtube_download_unref(download);
		}
	}
}

static gboolean
youtube_hide_downloads(gpointer user_data)
{
	YoutubePlugin *plugin = YOUTUBE_PLUGIN(user_data);
	GList *node;

	plugin->downloads_hide_source = 0;
	for (node = plugin->downloads; node != NULL; node = node->next)
	{
		YoutubeDownload *download = node->data;

		if (download->process != NULL)
			return G_SOURCE_REMOVE;
	}

	youtube_remove_finished_downloads(plugin);
	if (plugin->downloads_box != NULL)
		gtk_widget_hide(plugin->downloads_box);
	return G_SOURCE_REMOVE;
}

static void
youtube_schedule_downloads_hide(YoutubePlugin *plugin)
{
	GList *node;

	if (plugin->mw == NULL || plugin->downloads_box == NULL)
		return;

	for (node = plugin->downloads; node != NULL; node = node->next)
	{
		YoutubeDownload *download = node->data;

		if (download->process != NULL)
			return;
	}

	if (plugin->downloads_hide_source == 0)
		plugin->downloads_hide_source = g_timeout_add_seconds(
			3, youtube_hide_downloads, plugin);
}

static void
youtube_quality_changed(GtkComboBox *combo,
                        gpointer user_data)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(user_data);
	GSettings *settings;
	gint active;

	active = gtk_combo_box_get_active(combo);
	if (active < 0 || active >= (gint) AUDIO_QUALITY_COUNT)
		return;

	self->audio_quality = active;
	settings = music_settings_new();
	g_settings_set_int(settings, "youtube-quality", active);
	g_object_unref(settings);
}

static YoutubeDownload *
youtube_download_new(YoutubePlugin *plugin,
                     const gchar *url)
{
	YoutubeDownload *download;
	GtkWidget *row;
	GtkWidget *url_label;
	GtkWidget *progress_bar;
	GtkWidget *status_label;

	download = g_new0(YoutubeDownload, 1);
	download->plugin = g_object_ref(plugin);
	download->references = 1;
	youtube_remove_finished_downloads(plugin);
	if (plugin->downloads_hide_source != 0)
	{
		g_source_remove(plugin->downloads_hide_source);
		plugin->downloads_hide_source = 0;
	}
	if (plugin->downloads_box != NULL)
		gtk_widget_show_all(plugin->downloads_box);

	row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	url_label = gtk_label_new(url);
	gtk_label_set_ellipsize(GTK_LABEL(url_label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_size_request(url_label, 260, -1);
	gtk_widget_set_halign(url_label, GTK_ALIGN_START);
	progress_bar = gtk_progress_bar_new();
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), "Starting...");
	status_label = gtk_label_new("Starting...");
	gtk_widget_set_halign(status_label, GTK_ALIGN_START);

	gtk_box_pack_start(GTK_BOX(row), url_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(row), progress_bar, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(row), status_label, FALSE, FALSE, 0);

	download->row = row;
	download->url_label = url_label;
	download->progress_bar = progress_bar;
	download->status_label = status_label;
	plugin->downloads = g_list_append(plugin->downloads, download);
	if (plugin->downloads_box != NULL)
		gtk_box_pack_start(GTK_BOX(plugin->downloads_box), row,
		                   FALSE, FALSE, 0);
	gtk_widget_show_all(row);

	return download;
}

static void
youtube_download_finished(GObject *source,
                           GAsyncResult *result,
                           gpointer user_data)
{
	YoutubeDownload *download = user_data;
	YoutubePlugin *self = download->plugin;
	GSubprocess *process = G_SUBPROCESS(source);
	GError *error = NULL;
	gchar *stdout_data = NULL;
	gchar *stderr_data = NULL;
	gchar *downloaded_path = NULL;
	gchar **output_lines = NULL;
	guint output_index;
	gboolean successful;

	if (!g_subprocess_communicate_utf8_finish(process, result,
	                                           &stdout_data, &stderr_data,
	                                           &error))
	{
		if (error != NULL)
		{
			g_printerr("youtube: communicate failed: %s\n", error->message);
			youtube_download_set_status(download, error->message);
			youtube_download_set_progress(download, 0.0, "Download failed");
		}
		else
		{
			g_printerr("youtube: communicate failed: unknown error\n");
			youtube_download_set_status(download,
		                             "youtube-dl could not download that URL");
			youtube_download_set_progress(download, 0.0, "Download failed");
		}
		g_clear_error(&error);
		goto cleanup;
	}

	successful = g_subprocess_get_successful(process);
	if (stdout_data != NULL && stdout_data[0] != '\0')
		g_printerr("youtube: stdout:\n%s\n", stdout_data);
	if (stderr_data != NULL && stderr_data[0] != '\0')
		g_printerr("youtube: stderr:\n%s\n", stderr_data);
	g_printerr("youtube: command exited successfully: %s\n",
	           successful ? "yes" : "no");

	if (!successful)
	{
		g_printerr("youtube: downloader exited unsuccessfully\n");
		youtube_download_set_status(download,
		                             "youtube-dl could not download that URL");
		youtube_download_set_progress(download, 0.0, "Download failed");
		goto cleanup;
	}

	/* --print after_move:filepath gives us the final title-based path after
	 * yt-dlp has converted the download to MP3. */
	if (stdout_data != NULL)
	{
		output_lines = g_strsplit(stdout_data, "\n", -1);
		for (output_index = 0; output_lines[output_index] != NULL;
		     output_index++)
		{
			gchar *candidate = g_strstrip(output_lines[output_index]);

			if (candidate[0] != '\0' &&
			    g_file_test(candidate, G_FILE_TEST_IS_REGULAR))
			{
				downloaded_path = g_strdup(candidate);
				break;
			}
		}
	}
	if (downloaded_path == NULL)
	{
		youtube_download_set_status(download,
		                             "Download finished, but no MP3 was found");
		youtube_download_set_progress(download, 0.0, "Download failed");
		goto cleanup;
	}

	if (self->mw != NULL && self->mw->queue != NULL)
	{
		gchar *uri = g_filename_to_uri(downloaded_path, NULL, &error);

		if (uri != NULL)
		{
			add_file_ext(uri, self->mw->queue);
			g_free(uri);
			youtube_download_set_status(download,
		                             "Downloaded and added to the playlist");
			youtube_download_set_progress(download, 1.0, "Download succeeded");
		}
		else
		{
			youtube_download_set_status(download, error ? error->message :
				"Downloaded, but could not add the file");
			youtube_download_set_progress(download, 0.0, "Download failed");
			g_clear_error(&error);
		}
	}
	else
	{
		youtube_download_set_status(download, "Downloaded");
		youtube_download_set_progress(download, 1.0, "Download succeeded");
	}

	cleanup:
	if (download->progress_source != 0)
	{
		g_source_remove(download->progress_source);
		download->progress_source = 0;
	}
	g_clear_object(&download->process);
	youtube_schedule_downloads_hide(self);
	g_free(downloaded_path);
	g_strfreev(output_lines);
	g_free(stdout_data);
	g_free(stderr_data);
	youtube_download_unref(download);
}

static gboolean
youtube_start_download(YoutubePlugin *self,
                       const gchar *url)
{
	const gchar *program_name;
	gchar *program;
	gchar *directory;
	gchar *output_path;
	gchar *status;
	GError *error = NULL;
	const gchar *argv[19];
	GSubprocess *process;
	YoutubeDownload *download;

	program = g_find_program_in_path("youtube-dl");
	program_name = "youtube-dl";
	if (program == NULL)
	{
		program = g_find_program_in_path("yt-dlp");
		program_name = "yt-dlp";
	}
	if (program == NULL)
	{
		youtube_set_status(self, "Install youtube-dl or yt-dlp first");
		return FALSE;
	}

	directory = youtube_download_directory();
	if (g_mkdir_with_parents(directory, 0700) != 0)
	{
		youtube_set_status(self, "Could not create the YouTube download folder");
		g_free(program);
		g_free(directory);
		return FALSE;
	}

	download = youtube_download_new(self, url);
	output_path = g_build_filename(directory, "%(title)s.%(ext)s", NULL);
	g_free(directory);

	argv[0] = program;
	argv[1] = "--no-playlist";
	argv[2] = "--extract-audio";
	argv[3] = "--audio-format";
	argv[4] = "mp3";
	argv[5] = "--audio-quality";
	argv[6] = audio_quality_values[
		CLAMP(self->audio_quality, 0, (gint) AUDIO_QUALITY_COUNT - 1)];
	argv[7] = "--newline";
	argv[8] = "--embed-metadata";
	argv[9] = "--parse-metadata";
	argv[10] = "%(uploader)s:%(meta_artist)s";
	argv[11] = "--print";
	argv[12] = "after_move:filepath";
	argv[13] = "--output";
	argv[14] = output_path;
	/* YouTube's default web client is currently returning SABR-only formats
	 * to yt-dlp. The Android client still exposes a downloadable format. */
	argv[15] = "--extractor-args";
	argv[16] = "youtube:player_client=android";
	argv[17] = url;
	argv[18] = NULL;

	youtube_debug_command(argv);
	process = g_subprocess_newv(argv,
	                            G_SUBPROCESS_FLAGS_STDOUT_PIPE |
	                            G_SUBPROCESS_FLAGS_STDERR_PIPE,
	                            &error);
	g_free(program);
	if (process == NULL)
	{
		g_printerr("youtube: failed to start command: %s\n",
		           error ? error->message : "unknown error");
		youtube_set_status(self, error ? error->message : "Could not start downloader");
		youtube_download_set_status(download,
	                             error ? error->message : "Could not start downloader");
	youtube_download_set_progress(download, 0.0, "Download failed");
		youtube_schedule_downloads_hide(self);
		g_clear_error(&error);
		g_free(output_path);
		return FALSE;
	}

	download->process = process;
	download->download_path = output_path;
	youtube_download_set_status(download, "Downloading...");
	download->progress_source = g_timeout_add(100,
	                                           youtube_download_progress_pulse,
	                                           download);
	status = g_strdup_printf("Downloading with %s...", program_name);
	youtube_download_set_status(download, status);
	g_free(status);
	g_subprocess_communicate_utf8_async(process, NULL, NULL,
	                                    youtube_download_finished,
	                                    youtube_download_ref(download));
	return TRUE;
}

static void
youtube_download_clicked(GtkButton *button,
                          gpointer user_data)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(user_data);
	const gchar *url;

	(void) button;
	url = gtk_entry_get_text(GTK_ENTRY(self->url_entry));
	if (url == NULL || url[0] == '\0' ||
	    !(g_str_has_prefix(url, "http://") ||
	      g_str_has_prefix(url, "https://")))
	{
		youtube_set_status(self, "Enter an http:// or https:// YouTube URL");
		return;
	}

	if (!youtube_start_download(self, url))
		return;
	gtk_entry_set_text(GTK_ENTRY(self->url_entry), "");
	youtube_set_status(self, "Download started; see the main window");
}

static void
youtube_context_menu_item_activate(GtkMenuItem *item,
                                   gpointer user_data)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(user_data);
	GtkWidget *dialog;
	gint response;

	(void) item;
	dialog = youtube_plugin_get_config_window(MUSIC_PLUGIN(self));
	if (dialog == NULL)
		return;

	if (GTK_IS_DIALOG(dialog))
	{
		do
			response = gtk_dialog_run(GTK_DIALOG(dialog));
		while (response == GTK_RESPONSE_NONE);
		gtk_widget_destroy(dialog);
	}
	else
		gtk_widget_show(dialog);
}

static void
youtube_dialog_destroyed(GtkWidget *dialog,
                         gpointer user_data)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(user_data);

	(void) dialog;
	self->dialog = NULL;
	self->url_entry = NULL;
	self->status_label = NULL;
	self->quality_combo = NULL;
	self->download_button = NULL;
}

static GtkWidget *
youtube_plugin_get_config_window(MusicPlugin *plugin)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(plugin);
	GtkWidget *dialog;
	GtkWidget *content;
	GtkWidget *box;
	GtkWidget *label;
	GtkWidget *download_button;
	GtkWidget *quality_label;
	GtkWidget *quality_combo;
	gchar *directory;
	gchar *directory_label;
	guint quality_index;

	dialog = gtk_dialog_new_with_buttons(
		"YouTube Downloader",
		self->mw ? GTK_WINDOW(self->mw) : NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		"_Close", GTK_RESPONSE_CLOSE,
		NULL);
	download_button = gtk_dialog_add_button(GTK_DIALOG(dialog),
	                                         "_Download", GTK_RESPONSE_NONE);
	content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
	gtk_container_set_border_width(GTK_CONTAINER(box), 12);
	gtk_box_pack_start(GTK_BOX(content), box, TRUE, TRUE, 0);

	label = gtk_label_new("YouTube URL:");
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	self->url_entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(self->url_entry),
	                               "https://www.youtube.com/watch?v=...");
	gtk_box_pack_start(GTK_BOX(box), self->url_entry, FALSE, FALSE, 0);

	quality_label = gtk_label_new("MP3 quality:");
	gtk_widget_set_halign(quality_label, GTK_ALIGN_START);
	gtk_box_pack_start(GTK_BOX(box), quality_label, FALSE, FALSE, 0);
	quality_combo = gtk_combo_box_text_new();
	for (quality_index = 0; quality_index < AUDIO_QUALITY_COUNT;
	     quality_index++)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(quality_combo),
		                               audio_quality_labels[quality_index]);
	gtk_combo_box_set_active(GTK_COMBO_BOX(quality_combo), self->audio_quality);
	gtk_box_pack_start(GTK_BOX(box), quality_combo, FALSE, FALSE, 0);

	directory = youtube_download_directory();
	directory_label = g_strdup_printf("Files are saved to: %s", directory);
	label = gtk_label_new(directory_label);
	gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_widget_set_halign(label, GTK_ALIGN_START);
	gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
	g_free(directory_label);
	g_free(directory);

	self->status_label = gtk_label_new("Ready");
	gtk_widget_set_halign(self->status_label, GTK_ALIGN_START);
	gtk_box_pack_start(GTK_BOX(box), self->status_label, FALSE, FALSE, 0);

	self->download_button = download_button;
	self->quality_combo = quality_combo;
	self->dialog = dialog;
	g_signal_connect(quality_combo, "changed",
	                 G_CALLBACK(youtube_quality_changed), self);
	g_signal_connect(download_button, "clicked",
	                 G_CALLBACK(youtube_download_clicked), self);
	g_signal_connect(dialog, "destroy",
	                 G_CALLBACK(youtube_dialog_destroyed), self);
	gtk_widget_show_all(dialog);
	return dialog;
}

MusicPluginDetails *
get_details(void)
{
	MusicPluginDetails *info = g_malloc0(sizeof(MusicPluginDetails));

	info->name = g_strdup(PLUGIN_NAME);
	info->desc = g_strdup(DESC);
	info->copyright = g_strdup(COPYRIGHT);
	info->website = g_strdup(WEBSITE);
	info->is_configurable = TRUE;
	return info;
}

static gboolean
youtube_plugin_activate(MusicPlugin *plugin,
                         MusicMainWindow *mw)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(plugin);
	GtkWidget *heading;

	self->mw = mw;
	if (self->downloads_box == NULL && mw != NULL)
	{
		self->downloads_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
		heading = gtk_label_new("YouTube downloads");
		gtk_widget_set_halign(heading, GTK_ALIGN_START);
		gtk_box_pack_start(GTK_BOX(self->downloads_box), heading,
		                   FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(mw->mainvbox), self->downloads_box,
		                   FALSE, FALSE, 4);
		gtk_widget_show_all(self->downloads_box);
		gtk_widget_hide(self->downloads_box);
	}
	if (mw != NULL && mw->queue != NULL && self->context_menu_item == NULL)
	{
		self->context_menu_item = gtk_menu_item_new_with_label(
			"Download YouTube URL...");
		g_signal_connect(self->context_menu_item, "activate",
		                 G_CALLBACK(youtube_context_menu_item_activate), self);
		music_queue_register_context_menu_item(MUSIC_QUEUE(mw->queue),
		                                       self->context_menu_item);
	}
	return TRUE;
}

static gboolean
youtube_plugin_deactivate(MusicPlugin *plugin)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(plugin);

	if (self->downloads_hide_source != 0)
	{
		g_source_remove(self->downloads_hide_source);
		self->downloads_hide_source = 0;
	}

	if (self->context_menu_item != NULL && self->mw != NULL &&
	    self->mw->queue != NULL)
	{
		music_queue_unregister_context_menu_item(MUSIC_QUEUE(self->mw->queue),
	                                         self->context_menu_item);
		self->context_menu_item = NULL;
	}

	for (GList *node = self->downloads; node != NULL; node = node->next)
	{
		YoutubeDownload *download = node->data;

		if (download->process != NULL)
			g_subprocess_force_exit(download->process);
	}
	g_list_free_full(self->downloads, (GDestroyNotify) youtube_download_unref);
	self->downloads = NULL;
	if (self->downloads_box != NULL)
	{
		gtk_widget_destroy(self->downloads_box);
		self->downloads_box = NULL;
	}
	self->mw = NULL;
	return TRUE;
}

static void
youtube_plugin_dispose(GObject *object)
{
	YoutubePlugin *self = YOUTUBE_PLUGIN(object);

	if (self->mw != NULL || self->downloads != NULL ||
	    self->downloads_box != NULL || self->context_menu_item != NULL)
		youtube_plugin_deactivate(MUSIC_PLUGIN(self));
	if (self->dialog != NULL)
		gtk_widget_destroy(self->dialog);
	if (self->context_menu_item != NULL)
		gtk_widget_destroy(self->context_menu_item);
	G_OBJECT_CLASS(youtube_plugin_parent_class)->dispose(object);
}

GType
register_music_plugin(void)
{
	return youtube_plugin_get_type();
}

static void
youtube_plugin_class_init(YoutubePluginClass *klass)
{
	MusicPluginClass *plugin_class = MUSIC_PLUGIN_CLASS(klass);

	plugin_class->music_plugin_activate = youtube_plugin_activate;
	plugin_class->music_plugin_deactivate = youtube_plugin_deactivate;
	plugin_class->music_plugin_get_config_window =
		youtube_plugin_get_config_window;
	G_OBJECT_CLASS(klass)->dispose = youtube_plugin_dispose;
}

static void
youtube_plugin_init(YoutubePlugin *self)
{
	self->mw = NULL;
	self->dialog = NULL;
	self->url_entry = NULL;
	self->status_label = NULL;
	self->quality_combo = NULL;
	self->download_button = NULL;
	self->context_menu_item = NULL;
	self->downloads_box = NULL;
	self->downloads = NULL;
	self->downloads_hide_source = 0;
	self->audio_quality = 0;
	{
		GSettings *settings = music_settings_new();
		gint configured_quality = g_settings_get_int(settings,
		                                             "youtube-quality");

		if (configured_quality >= 0 &&
		    configured_quality < (gint) AUDIO_QUALITY_COUNT)
			self->audio_quality = configured_quality;
		g_object_unref(settings);
	}
}
