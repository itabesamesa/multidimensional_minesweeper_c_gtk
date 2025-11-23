#include "input.h"

static void input_entrypopover_dispose(GObject* gobject);
static void input_entrypopover_finalize(GObject* gobject);

struct _InputEntrypopover {
  GtkEntry parent_instance;

  GtkWidget* popover;
};

struct _InputEntrypopoverClass {
  GtkEntryClass parent_class;
};

G_DEFINE_TYPE (InputEntrypopover, input_entrypopover, GTK_TYPE_WIDGET)
