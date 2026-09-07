#include "stats.h"
#include "music-plugin.h"
#include "music-queue.h"
#include "tag-scanner.h"

#include <glib/gstdio.h>
#include <time.h>

static const char PLUGIN_NAME[] = "Stats";
static const char DESC[] = "Track playback statistics";
static const char COPYRIGHT[] = "Kyle Pelton";
static const char WEBSITE[] = "www.squidman.net";

enum {
	SHOW_REPORT,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

static gboolean draw_stats(gpointer data);
static void stats_new_file(GsPlayer *player,
                           metadata *track,
                           gpointer user_data);
static void stats_eof(GsPlayer *player,
                      gpointer user_data);
static void show_stats(StatsPlugin *self);
static void show_stats_button(GtkButton *button,
                              gpointer user_data);

static gboolean db_exec(StatsPlugin *self,
                        const gchar *sql);
static gboolean db_create_schema(StatsPlugin *self);
static sqlite3_int64 db_track_id(StatsPlugin *self,
                                 const gchar *uri,
                                 metadata *track);
static void stats_update_current(StatsPlugin *self);
static void stats_close_current(StatsPlugin *self,
                                const gchar *outcome,
                                gboolean completed,
                                gboolean count_skip);
static gchar *format_seconds(guint64 seconds);

G_DEFINE_TYPE (StatsPlugin, stats_plugin, MUSIC_TYPE_PLUGIN);

static gboolean
db_exec(StatsPlugin *self,
        const gchar *sql)
{
	char *error_message = NULL;
	int result;

	result = sqlite3_exec(self->db, sql, NULL, NULL, &error_message);
	if (result != SQLITE_OK)
	{
		g_warning("Stats database error: %s",
		          error_message ? error_message : sqlite3_errmsg(self->db));
		sqlite3_free(error_message);
		return FALSE;
	}

	return TRUE;
}

static gboolean
db_create_schema(StatsPlugin *self)
{
	static const gchar schema[] =
		"PRAGMA foreign_keys = ON;"
		"CREATE TABLE IF NOT EXISTS tracks ("
		"id INTEGER PRIMARY KEY,"
		"uri TEXT NOT NULL UNIQUE,"
		"title TEXT, artist TEXT, album TEXT, genre TEXT, codec TEXT,"
		"duration_ns INTEGER, first_seen INTEGER NOT NULL,"
		"last_seen INTEGER NOT NULL"
		");"
		"CREATE TABLE IF NOT EXISTS play_sessions ("
		"id INTEGER PRIMARY KEY,"
		"track_id INTEGER NOT NULL,"
		"started_at INTEGER NOT NULL, ended_at INTEGER,"
		"listened_ms INTEGER NOT NULL DEFAULT 0,"
		"last_position_ns INTEGER NOT NULL DEFAULT 0,"
		"outcome TEXT NOT NULL DEFAULT 'playing' CHECK "
		"(outcome IN ('playing','completed','skipped','stopped')),"
		"counted_play INTEGER NOT NULL DEFAULT 0,"
		"FOREIGN KEY(track_id) REFERENCES tracks(id)"
		");"
		"CREATE TABLE IF NOT EXISTS track_stats ("
		"track_id INTEGER PRIMARY KEY,"
		"play_count INTEGER NOT NULL DEFAULT 0,"
		"completion_count INTEGER NOT NULL DEFAULT 0,"
		"skip_count INTEGER NOT NULL DEFAULT 0,"
		"listened_ms INTEGER NOT NULL DEFAULT 0,"
		"first_played INTEGER, last_played INTEGER,"
		"FOREIGN KEY(track_id) REFERENCES tracks(id)"
		");"
		"CREATE INDEX IF NOT EXISTS sessions_track_idx "
		"ON play_sessions(track_id);"
		"CREATE INDEX IF NOT EXISTS sessions_started_idx "
		"ON play_sessions(started_at);";

	return db_exec(self, schema);
}

static gboolean
bind_text_or_null(sqlite3_stmt *statement,
                  int index,
                  const gchar *value)
{
	if (value == NULL || value[0] == '\0')
		return sqlite3_bind_null(statement, index) == SQLITE_OK;

	return sqlite3_bind_text(statement, index, value, -1, SQLITE_TRANSIENT)
	       == SQLITE_OK;
}

static sqlite3_int64
db_track_id(StatsPlugin *self,
            const gchar *uri,
            metadata *track)
{
	sqlite3_stmt *statement = NULL;
	sqlite3_int64 id = 0;
	gint64 now = (gint64) time(NULL);
	int result;

	result = sqlite3_prepare_v2(self->db,
	                            "INSERT OR IGNORE INTO tracks "
	                            "(uri,first_seen,last_seen) VALUES(?,?,?)",
	                            -1, &statement, NULL);
	if (result == SQLITE_OK)
	{
		sqlite3_bind_text(statement, 1, uri, -1, SQLITE_TRANSIENT);
		sqlite3_bind_int64(statement, 2, now);
		sqlite3_bind_int64(statement, 3, now);
		sqlite3_step(statement);
	}
	if (statement)
		sqlite3_finalize(statement);

	result = sqlite3_prepare_v2(self->db,
	                            "UPDATE tracks SET title=COALESCE(?,title),"
	                            "artist=COALESCE(?,artist),album=COALESCE(?,album),"
	                            "genre=COALESCE(?,genre),codec=COALESCE(?,codec),"
	                            "duration_ns=CASE WHEN ? > 0 THEN ? ELSE duration_ns END,"
	                            "last_seen=? WHERE uri=?",
	                            -1, &statement, NULL);
	if (result == SQLITE_OK)
	{
		bind_text_or_null(statement, 1, track ? track->title : NULL);
		bind_text_or_null(statement, 2, track ? track->artist : NULL);
		bind_text_or_null(statement, 3, track ? track->album : NULL);
		bind_text_or_null(statement, 4, track ? track->genre : NULL);
		bind_text_or_null(statement, 5, track ? track->codec : NULL);
		sqlite3_bind_int64(statement, 6, track ? track->duration : 0);
		sqlite3_bind_int64(statement, 7, track ? track->duration : 0);
		sqlite3_bind_int64(statement, 8, now);
		sqlite3_bind_text(statement, 9, uri, -1, SQLITE_TRANSIENT);
		sqlite3_step(statement);
	}
	if (statement)
		sqlite3_finalize(statement);

	result = sqlite3_prepare_v2(self->db,
	                            "SELECT id FROM tracks WHERE uri=?",
	                            -1, &statement, NULL);
	if (result == SQLITE_OK)
	{
		sqlite3_bind_text(statement, 1, uri, -1, SQLITE_TRANSIENT);
		if (sqlite3_step(statement) == SQLITE_ROW)
			id = sqlite3_column_int64(statement, 0);
	}
	if (statement)
		sqlite3_finalize(statement);

	return id;
}

static void
stats_begin_track(StatsPlugin *self,
                  metadata *track)
{
	const gchar *uri = NULL;
	sqlite3_stmt *statement = NULL;

	if (track && track->uri)
		uri = track->uri;
	else if (self->player)
		uri = self->player->uri;
	if (uri == NULL || uri[0] == '\0')
		return;

	self->current_track_id = db_track_id(self, uri, track);
	if (self->current_track_id == 0)
		return;

	if (sqlite3_prepare_v2(self->db,
	                       "INSERT OR IGNORE INTO track_stats(track_id) VALUES(?)",
	                       -1, &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, self->current_track_id);
		sqlite3_step(statement);
	}
	if (statement)
		sqlite3_finalize(statement);

	if (sqlite3_prepare_v2(self->db,
	                       "INSERT INTO play_sessions(track_id,started_at) "
	                       "VALUES(?,?)", -1, &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, self->current_track_id);
		sqlite3_bind_int64(statement, 2, (sqlite3_int64) time(NULL));
		if (sqlite3_step(statement) == SQLITE_DONE)
			self->current_session_id = sqlite3_last_insert_rowid(self->db);
	}
	if (statement)
		sqlite3_finalize(statement);

	self->current_duration = track ? track->duration : 0;
	self->current_counted = FALSE;
	self->current_last_tick = g_get_monotonic_time();
	self->current_listen_usec = 0;
}

static guint64
current_threshold(StatsPlugin *self)
{
	if (self->current_duration > 0)
		return MIN((guint64) (30 * GST_SECOND),
	           (guint64) self->current_duration / 2);
	return 30 * GST_SECOND;
}

static void
stats_update_current(StatsPlugin *self)
{
	gint64 now;
	gint64 position = 0;
	gint64 duration = 0;
	gint64 delta;
	gint64 listened_ms;
	GstFormat format = GST_FORMAT_TIME;
	sqlite3_stmt *statement = NULL;

	if (self->current_session_id == 0 || self->player == NULL)
		return;

	now = g_get_monotonic_time();
	if (self->current_last_tick > 0 && isPlaying(self->player))
	{
		delta = now - self->current_last_tick;
		if (delta > 0 && delta < 10 * G_USEC_PER_SEC)
			self->current_listen_usec += delta;
	}
	self->current_last_tick = now;

	if (gst_element_query_position(self->player->play, format, &position)
	    && gst_element_query_duration(self->player->play, format, &duration)
	    && duration > 0)
		self->current_duration = duration;

	if (!self->current_counted
	    && ((guint64) position >= current_threshold(self)
	        || (guint64) self->current_listen_usec >= current_threshold(self)))
	{
		if (sqlite3_prepare_v2(self->db,
		                       "UPDATE track_stats SET play_count=play_count+1,"
		                       "first_played=COALESCE(first_played,?),last_played=? "
		                       "WHERE track_id=?", -1, &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, (sqlite3_int64) time(NULL));
			sqlite3_bind_int64(statement, 2, (sqlite3_int64) time(NULL));
			sqlite3_bind_int64(statement, 3, self->current_track_id);
			sqlite3_step(statement);
		}
		if (statement)
			sqlite3_finalize(statement);
		self->current_counted = TRUE;
	}

	listened_ms = self->current_listen_usec / 1000;
	if (sqlite3_prepare_v2(self->db,
	                       "UPDATE play_sessions SET listened_ms=?,"
	                       "last_position_ns=? WHERE id=?", -1, &statement, NULL)
	    == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, listened_ms);
		sqlite3_bind_int64(statement, 2, position);
		sqlite3_bind_int64(statement, 3, self->current_session_id);
		sqlite3_step(statement);
	}
	if (statement)
		sqlite3_finalize(statement);

	if (self->current_duration > 0
	    && sqlite3_prepare_v2(self->db,
	                           "UPDATE tracks SET duration_ns=? WHERE id=?",
	                           -1, &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, self->current_duration);
		sqlite3_bind_int64(statement, 2, self->current_track_id);
		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}
}

static void
stats_close_current(StatsPlugin *self,
                    const gchar *outcome,
                    gboolean completed,
                    gboolean count_skip)
{
	sqlite3_stmt *statement = NULL;
	gint64 now;
	gint64 listened_ms;

	if (self->current_session_id == 0)
		return;

	stats_update_current(self);
	now = (gint64) time(NULL);
	listened_ms = self->current_listen_usec / 1000;

	if (completed || (count_skip && self->current_counted))
	{
		const gchar *sql;
		if (completed)
			sql = "UPDATE track_stats SET completion_count=completion_count+1 "
			      "WHERE track_id=?";
		else
			sql = "UPDATE track_stats SET skip_count=skip_count+1 WHERE track_id=?";
		if (sqlite3_prepare_v2(self->db, sql, -1, &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, self->current_track_id);
			sqlite3_step(statement);
		}
		if (statement)
			sqlite3_finalize(statement);
	}

	if (completed && !self->current_counted)
	{
		if (sqlite3_prepare_v2(self->db,
		                       "UPDATE track_stats SET play_count=play_count+1,"
		                       "first_played=COALESCE(first_played,?),last_played=? "
		                       "WHERE track_id=?", -1, &statement, NULL) == SQLITE_OK)
		{
			sqlite3_bind_int64(statement, 1, now);
			sqlite3_bind_int64(statement, 2, now);
			sqlite3_bind_int64(statement, 3, self->current_track_id);
			sqlite3_step(statement);
		}
		if (statement)
			sqlite3_finalize(statement);
	}

	if (sqlite3_prepare_v2(self->db,
	                       "UPDATE play_sessions SET ended_at=?,listened_ms=?,"
	                       "outcome=?,counted_play=? WHERE id=?", -1,
	                       &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, now);
		sqlite3_bind_int64(statement, 2, listened_ms);
		sqlite3_bind_text(statement, 3, outcome, -1, SQLITE_STATIC);
		sqlite3_bind_int(statement, 4, self->current_counted || completed);
		sqlite3_bind_int64(statement, 5, self->current_session_id);
		sqlite3_step(statement);
	}
	if (statement)
		sqlite3_finalize(statement);

	if (sqlite3_prepare_v2(self->db,
	                       "UPDATE track_stats SET listened_ms=listened_ms+? "
	                       "WHERE track_id=?", -1, &statement, NULL) == SQLITE_OK)
	{
		sqlite3_bind_int64(statement, 1, listened_ms);
		sqlite3_bind_int64(statement, 2, self->current_track_id);
		sqlite3_step(statement);
	}
	if (statement)
		sqlite3_finalize(statement);

	self->current_track_id = 0;
	self->current_session_id = 0;
	self->current_duration = 0;
	self->current_counted = FALSE;
	self->current_last_tick = 0;
	self->current_listen_usec = 0;
}

static gchar *
format_seconds(guint64 seconds)
{
	guint64 hours = seconds / 3600;
	guint64 minutes = (seconds % 3600) / 60;

	if (hours > 0)
		return g_strdup_printf("%" G_GUINT64_FORMAT "h %" G_GUINT64_FORMAT "m",
		                       hours, minutes);
	return g_strdup_printf("%" G_GUINT64_FORMAT "m", minutes);
}

static void
stats_total_values(StatsPlugin *self,
                   guint64 *plays,
                   guint64 *listening_ms)
{
	sqlite3_stmt *statement = NULL;

	*plays = 0;
	*listening_ms = 0;
	if (sqlite3_prepare_v2(self->db,
	                       "SELECT COALESCE(SUM(play_count),0),"
	                       "COALESCE(SUM(listened_ms),0) FROM track_stats",
	                       -1, &statement, NULL) == SQLITE_OK)
	{
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			*plays = (guint64) sqlite3_column_int64(statement, 0);
			*listening_ms = (guint64) sqlite3_column_int64(statement, 1);
		}
	}
	if (statement)
		sqlite3_finalize(statement);
	if (self->current_session_id)
		*listening_ms += self->current_listen_usec / 1000;
}

static gboolean
draw_stats(gpointer data)
{
	StatsPlugin *self = data;
	guint64 plays;
	guint64 listening_ms;
	gchar *playlist_time;
	gchar *listening_time;
	gchar *summary;
	gchar *detail;

	stats_update_current(self);
	stats_total_values(self, &plays, &listening_ms);
	playlist_time = format_seconds(music_queue_get_length(self->queue) /
	                               GST_SECOND);
	listening_time = format_seconds(listening_ms / 1000);
	summary = g_strdup_printf("Files: %u  |  Plays: %" G_GUINT64_FORMAT,
	                          music_queue_get_size(self->queue), plays);
	detail = g_strdup_printf("Playlist: %s  |  Listening: %s",
	                         playlist_time, listening_time);
	gtk_label_set_text(GTK_LABEL(self->text), summary);
	gtk_label_set_text(GTK_LABEL(self->text2), detail);
	g_free(summary);
	g_free(detail);
	g_free(playlist_time);
	g_free(listening_time);
	return TRUE;
}

static void
stats_new_file(GsPlayer *player,
               metadata *track,
               gpointer user_data)
{
	StatsPlugin *self = user_data;

	if (self->current_session_id)
		stats_close_current(self, "skipped", FALSE, TRUE);
	stats_begin_track(self, track);
	draw_stats(self);
}

static void
stats_eof(GsPlayer *player,
          gpointer user_data)
{
	StatsPlugin *self = user_data;

	stats_close_current(self, "completed", TRUE, FALSE);
	draw_stats(self);
}

static void
add_stats_column(GtkTreeView *view,
                 const gchar *title,
                 gint column,
                 gint sort_column)
{
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *view_column = gtk_tree_view_column_new_with_attributes(
		title, renderer, "text", column, NULL);

	gtk_tree_view_column_set_resizable(view_column, TRUE);
	gtk_tree_view_column_set_sort_column_id(view_column, sort_column);
	gtk_tree_view_append_column(view, view_column);
}

static void
show_stats_button(GtkButton *button,
                  gpointer user_data)
{
	show_stats(STATS_PLUGIN(user_data));
}

static void
show_stats(StatsPlugin *self)
{
	GtkWidget *dialog;
	GtkWidget *content;
	GtkWidget *summary;
	GtkWidget *scrolledwindow;
	GtkWidget *tree;
	GtkListStore *model;
	GtkTreeIter iter;
	sqlite3_stmt *statement = NULL;
	guint64 plays;
	guint64 listening_ms;
	guint64 completions = 0;
	guint64 skips = 0;
	gchar *listening_text;
	gchar *summary_text;

	stats_update_current(self);
	stats_total_values(self, &plays, &listening_ms);
	if (sqlite3_prepare_v2(self->db,
	                       "SELECT COALESCE(SUM(completion_count),0),"
	                       "COALESCE(SUM(skip_count),0) FROM track_stats",
	                       -1, &statement, NULL) == SQLITE_OK
	    && sqlite3_step(statement) == SQLITE_ROW)
	{
		completions = (guint64) sqlite3_column_int64(statement, 0);
		skips = (guint64) sqlite3_column_int64(statement, 1);
	}
	if (statement)
		sqlite3_finalize(statement);

	dialog = gtk_dialog_new_with_buttons("Playback Statistics",
	                                    GTK_WINDOW(self->mw),
	                                    GTK_DIALOG_MODAL,
	                                    "Close", GTK_RESPONSE_CLOSE,
	                                    NULL);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 400);
	content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	listening_text = format_seconds(listening_ms / 1000);
	summary_text = g_strdup_printf("Plays: %" G_GUINT64_FORMAT
	                              "   Completed: %" G_GUINT64_FORMAT
	                              "   Skipped: %" G_GUINT64_FORMAT
	                              "   Listening: %s",
	                              plays, completions, skips, listening_text);
	summary = gtk_label_new(summary_text);
	gtk_label_set_xalign(GTK_LABEL(summary), 0.0);
	gtk_box_pack_start(GTK_BOX(content), summary, FALSE, FALSE, 8);
	g_free(summary_text);

	model = gtk_list_store_new(10, G_TYPE_STRING, G_TYPE_STRING,
	                           G_TYPE_STRING, G_TYPE_STRING,
	                           G_TYPE_STRING, G_TYPE_STRING,
	                           G_TYPE_INT64, G_TYPE_INT64,
	                           G_TYPE_INT64, G_TYPE_INT64);
	if (sqlite3_prepare_v2(self->db,
	                       "SELECT t.title,t.uri,t.artist,s.play_count,"
	                       "s.completion_count,s.skip_count,s.listened_ms "
	                       "FROM track_stats s JOIN tracks t ON t.id=s.track_id "
	                       "ORDER BY s.play_count DESC,s.listened_ms DESC",
	                       -1, &statement, NULL) == SQLITE_OK)
	{
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			const gchar *title = (const gchar *) sqlite3_column_text(statement, 0);
			const gchar *uri = (const gchar *) sqlite3_column_text(statement, 1);
			const gchar *artist = (const gchar *) sqlite3_column_text(statement, 2);
			gchar *plays_text = g_strdup_printf("%" G_GINT64_FORMAT,
			                                    sqlite3_column_int64(statement, 3));
			gchar *completed_text = g_strdup_printf("%" G_GINT64_FORMAT,
			                                        sqlite3_column_int64(statement, 4));
			gchar *skips_text = g_strdup_printf("%" G_GINT64_FORMAT,
			                                    sqlite3_column_int64(statement, 5));
			gchar *time_text = format_seconds((guint64) sqlite3_column_int64(statement, 6) / 1000);

			gtk_list_store_append(model, &iter);
			gtk_list_store_set(model, &iter,
			                   0, title ? title : uri,
			                   1, artist ? artist : "",
			                   2, plays_text,
			                   3, completed_text,
			                   4, skips_text,
			                   5, time_text,
			                   6, sqlite3_column_int64(statement, 3),
			                   7, sqlite3_column_int64(statement, 4),
			                   8, sqlite3_column_int64(statement, 5),
			                   9, sqlite3_column_int64(statement, 6),
			                   -1);
			g_free(plays_text);
			g_free(completed_text);
			g_free(skips_text);
			g_free(time_text);
		}
	}
	if (statement)
		sqlite3_finalize(statement);

	scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
	add_stats_column(GTK_TREE_VIEW(tree), "Song", 0, 0);
	add_stats_column(GTK_TREE_VIEW(tree), "Artist", 1, 1);
	add_stats_column(GTK_TREE_VIEW(tree), "Plays", 2, 6);
	add_stats_column(GTK_TREE_VIEW(tree), "Completed", 3, 7);
	add_stats_column(GTK_TREE_VIEW(tree), "Skipped", 4, 8);
	add_stats_column(GTK_TREE_VIEW(tree), "Time", 5, 9);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), tree);
	gtk_box_pack_start(GTK_BOX(content), scrolledwindow, TRUE, TRUE, 8);

	gtk_widget_show_all(content);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	g_object_unref(model);
	g_free(listening_text);
}

MusicPluginDetails *
get_details(void)
{
	MusicPluginDetails *info = g_malloc0(sizeof(MusicPluginDetails));

	info->name = g_strdup(PLUGIN_NAME);
	info->desc = g_strdup(DESC);
	info->copyright = g_strdup(COPYRIGHT);
	info->website = g_strdup(WEBSITE);
	info->is_configurable = FALSE;
	return info;
}

gboolean
stats_plugin_music_plugin_activate(MusicPlugin *plugin,
                                   MusicMainWindow *mw)
{
	StatsPlugin *self = STATS_PLUGIN(plugin);

	if (self->db == NULL)
		return FALSE;

	self->mw = mw;
	self->player = mw->player;
	self->queue = MUSIC_QUEUE(mw->queue);
	g_object_ref(self->queue);
	self->new_file_handler = g_signal_connect(self->player, "new-file",
	                                          G_CALLBACK(stats_new_file), self);
	self->eof_handler = g_signal_connect(self->player, "eof",
	                                     G_CALLBACK(stats_eof), self);
	self->report_handler = g_signal_connect(self, "show-report",
	                                        G_CALLBACK(show_stats), NULL);

	self->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	self->text = gtk_label_new("");
	self->text2 = gtk_label_new("");
	self->details_button = gtk_button_new_with_label("Statistics");
	gtk_label_set_xalign(GTK_LABEL(self->text2), 1.0f);
	gtk_label_set_yalign(GTK_LABEL(self->text2), 1.0f);
	gtk_box_pack_start(GTK_BOX(self->hbox), self->text, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(self->hbox), self->text2, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(self->hbox), self->details_button,
	                  FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mw->mainvbox), self->hbox, FALSE, FALSE, 0);
	g_signal_connect(self->details_button, "clicked",
	                 G_CALLBACK(show_stats_button), self);
	gtk_widget_show_all(self->hbox);

	if (self->player->uri)
		stats_begin_track(self, mw->currsong);
	draw_stats(self);
	self->refresh_source = g_timeout_add_seconds(1,
	                                             (GSourceFunc) draw_stats,
	                                             self);
	return TRUE;
}

gboolean
stats_plugin_music_plugin_deactivate(MusicPlugin *plugin)
{
	StatsPlugin *self = STATS_PLUGIN(plugin);

	if (self->refresh_source)
	{
		g_source_remove(self->refresh_source);
		self->refresh_source = 0;
	}
	if (self->player)
	{
		if (self->new_file_handler)
			g_signal_handler_disconnect(self->player, self->new_file_handler);
		if (self->eof_handler)
			g_signal_handler_disconnect(self->player, self->eof_handler);
		self->new_file_handler = 0;
		self->eof_handler = 0;
	}
	if (self->report_handler)
	{
		g_signal_handler_disconnect(self, self->report_handler);
		self->report_handler = 0;
	}

	stats_close_current(self, "stopped", FALSE, FALSE);
	if (self->queue)
	{
		g_object_unref(self->queue);
		self->queue = NULL;
	}
	if (self->hbox)
	{
		gtk_widget_destroy(self->hbox);
		self->hbox = NULL;
	}
	self->player = NULL;
	self->mw = NULL;
	return TRUE;
}

GType
register_music_plugin(void)
{
	return stats_plugin_get_type();
}

static void
stats_plugin_dispose(GObject *object)
{
	StatsPlugin *self = STATS_PLUGIN(object);

	if (self->queue || self->hbox)
		stats_plugin_music_plugin_deactivate(MUSIC_PLUGIN(self));
	G_OBJECT_CLASS(stats_plugin_parent_class)->dispose(object);
}

static void
stats_plugin_finalize(GObject *object)
{
	StatsPlugin *self = STATS_PLUGIN(object);

	if (self->db)
		sqlite3_close(self->db);
	g_free(self->stats_path);
	G_OBJECT_CLASS(stats_plugin_parent_class)->finalize(object);
}

static void
stats_plugin_init(StatsPlugin *self)
{
	const gchar *data_dir = g_get_user_data_dir();
	gchar *directory;

	self->stats_path = g_build_filename(data_dir, "musicplayer",
	                                    "stats.db", NULL);
	directory = g_path_get_dirname(self->stats_path);
	g_mkdir_with_parents(directory, 0700);
	g_free(directory);

	if (sqlite3_open(self->stats_path, &self->db) != SQLITE_OK)
	{
		g_warning("Could not open playback statistics database: %s",
		          self->db ? sqlite3_errmsg(self->db) : "unknown error");
		if (self->db)
		{
			sqlite3_close(self->db);
			self->db = NULL;
		}
		return;
	}
	sqlite3_busy_timeout(self->db, 1000);
	if (!db_create_schema(self))
	{
		sqlite3_close(self->db);
		self->db = NULL;
		return;
	}
}

static void
stats_plugin_class_init(StatsPluginClass *klass)
{
	MusicPluginClass *plugin_class = MUSIC_PLUGIN_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	plugin_class->music_plugin_activate = stats_plugin_music_plugin_activate;
	plugin_class->music_plugin_deactivate = stats_plugin_music_plugin_deactivate;
	object_class->dispose = stats_plugin_dispose;
	object_class->finalize = stats_plugin_finalize;
	signals[SHOW_REPORT] = g_signal_new("show-report",
	                                   G_TYPE_FROM_CLASS(klass),
	                                   G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE |
	                                   G_SIGNAL_NO_HOOKS,
	                                   0, NULL, NULL,
	                                   g_cclosure_marshal_VOID__VOID,
	                                   G_TYPE_NONE, 0);
}

StatsPlugin *
stats_plugin_new(void)
{
	return g_object_new(STATS_TYPE_PLUGIN, NULL);
}
