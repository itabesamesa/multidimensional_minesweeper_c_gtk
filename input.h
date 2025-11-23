#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define INPUT_TYPE_ENTRYPOPOVER input_entrypopover_get_type()
G_DECLARE_FINAL_TYPE (InputEntrypopover, input_entrypopover, INPUT, ENTRYPOPOVER, GtkWidget)

GtkWidget* input_entrypopover_new(void);
GtkEntry* input_entrypopover_get_entry(InputEntrypopover* entrypopover);
GtkPopover* input_entrypopover_get_popover(InputEntrypopover* entrypopover);
void input_entrypopover_popdown(InputEntrypopover* entrypopover);
void input_entrypopover_popup(InputEntrypopover* entrypopover);

G_END_DECLS

  gtk_entry_set_input_purpose(GTK_ENTRY(bombs_entry), GTK_INPUT_PURPOSE_DIGITS);
GTK_INPUT_PURPOSE_DIGITS
