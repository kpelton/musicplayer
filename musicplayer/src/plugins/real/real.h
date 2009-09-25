/* real-test.h */

#ifndef _REAL_TEST
#define _REAL_TEST

#include <glib-object.h>
#include "music-main-window.h"

G_BEGIN_DECLS

#define REAL_TYPE_TEST real_test_get_type()

#define REAL_TEST(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), REAL_TYPE_TEST, RealTest))

#define REAL_TEST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), REAL_TYPE_TEST, RealTestClass))

#define REAL_IS_TEST(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), REAL_TYPE_TEST))

#define REAL_IS_TEST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), REAL_TYPE_TEST))

#define REAL_TEST_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), REAL_TYPE_TEST, RealTestClass))

typedef struct {
  GObject parent;
    MusicMainWindow *mw;
    gint id;
} RealTest;

typedef struct {
  GObjectClass parent_class;
} RealTestClass;

GType real_test_get_type (void);

RealTest* real_test_new (void);

G_END_DECLS

#endif /* _REAL_TEST */
