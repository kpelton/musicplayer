#include <glib.h>

#include "utils.h"

static gboolean
count_callback (gpointer user_data)
{
	guint *calls = user_data;

	(*calls)++;
	return G_SOURCE_REMOVE;
}

static void
test_callback_runs_once (void)
{
	guint calls = 0;

	music_main_context_invoke (count_callback, &calls);
	while (g_main_context_pending (NULL))
		g_main_context_iteration (NULL, FALSE);

	g_assert_cmpuint (calls, ==, 1);
}

static void
test_multiple_callbacks_run_once_each (void)
{
	guint calls = 0;

	music_main_context_invoke (count_callback, &calls);
	music_main_context_invoke (count_callback, &calls);
	while (g_main_context_pending (NULL))
		g_main_context_iteration (NULL, FALSE);

	g_assert_cmpuint (calls, ==, 2);
}

int
main (int argc, char **argv)
{
	g_test_init (&argc, &argv, NULL);
	g_test_add_func ("/main-context/callback-runs-once",
	                test_callback_runs_once);
	g_test_add_func ("/main-context/multiple-callbacks-run-once-each",
	                test_multiple_callbacks_run_once_each);
	return g_test_run ();
}
