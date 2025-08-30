/*
 * Copyright/Licensing information.
 */

/* inclusion guard */
#pragma once

#include <gtk/gtk.h>

typedef struct _dimension {
  guint len;
  guint* dim;
} dimension;

/*
 * Potentially, include other headers on which this header depends.
 */

G_BEGIN_DECLS

/*
 * Type declaration.
 */
#define MINESWEEPER_TYPE_CELL minesweeper_cell_get_type()
G_DECLARE_FINAL_TYPE (MinesweeperCell, minesweeper_cell, MINESWEEPER, CELL, GtkWidget)

#define MINESWEEPER_TYPE_FIELD minesweeper_field_get_type()
G_DECLARE_FINAL_TYPE (MinesweeperField, minesweeper_field, MINESWEEPER, FIELD, GtkWidget)

/*
 * Method definitions.
 */
GtkWidget* minesweeper_cell_new(void);
void minesweeper_cell_set_state(MinesweeperCell* cell, guint state);

GtkWidget* minesweeper_field_new(void);
void minesweeper_field_generate(MinesweeperField* field, dimension dim);

G_END_DECLS
