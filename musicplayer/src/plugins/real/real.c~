/* real-test.c */

#include "real.h"

G_DEFINE_TYPE (RealTest, real_test, G_TYPE_OBJECT)
void *
register_music_plugin()
{
    return real_test_get_type();
}

static void
real_test_class_init (RealTestClass *klass)
{
}

static void
real_test_init (RealTest *self)
{
  printf("it's working\n\n");
}

RealTest*
real_test_new (void)
{
  return g_object_new (REAL_TYPE_TEST, NULL);
}

