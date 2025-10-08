/*
 * Copyright/Licensing information.
 */

/* inclusion guard */
#pragma once

#include <gtk/gtk.h>
#include <math.h>
#include "mtwister.h"

typedef struct _dimension {
  guint len;
  guint* dim;
} dimension;

guint num_len(gint num);
dimension repeate_dim(guint value, guint amount);

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

void minesweeper_cell_redraw(MinesweeperCell* cell);
void minesweeper_cell_set_state(MinesweeperCell* cell, guint state);
void minesweeper_cell_set_is_bomb(MinesweeperCell* cell, guint is_bomb);
void minesweeper_cell_uncover(MinesweeperCell* cell);
void minesweeper_cell_uncover_unless_flag(MinesweeperCell* cell);
void minesweeper_cell_flag(MinesweeperCell* cell);

guint minesweeper_cell_get_is_bomb(MinesweeperCell* cell);

GtkWidget* minesweeper_field_new(void);

void minesweeper_field_game_running(MinesweeperField* field);
void minesweeper_field_game_lost(MinesweeperField* field);
void minesweeper_field_game_won(MinesweeperField* field);
void minesweeper_field_game_forfeit(MinesweeperField* field);
void minesweeper_field_game_paused(MinesweeperField* field);
void minesweeper_field_generate(MinesweeperField* field);
void minesweeper_field_populate(MinesweeperField* field);
void minesweeper_field_toggle_delta_mode(MinesweeperField* field);
void minesweeper_field_toggle_obfuscate_on_pause(MinesweeperField* field);
void minesweeper_field_set_dim(MinesweeperField* field, dimension dim);
void minesweeper_field_set_seed(MinesweeperField* field, guint seed);
void minesweeper_field_set_bombs(MinesweeperField* field, guint bombs);
void minesweeper_field_set_tmpdim(MinesweeperField* field, dimension dim);
void minesweeper_field_set_tmpseed(MinesweeperField* field, guint seed);
void minesweeper_field_set_tmpbombs(MinesweeperField* field, guint bombs);
void minesweeper_field_set_generate_populate(MinesweeperField* field, dimension dim, guint seed, guint bombs);
void minesweeper_field_set_tmp(MinesweeperField* field, dimension dim, guint seed, guint bombs);
void minesweeper_field_apply_tmp_generate_populate(MinesweeperField* field);
void minesweeper_field_empty(MinesweeperField* field);
void minesweeper_field_empty_apply_tmp_generate_populate(MinesweeperField* field);
guint minesweeper_field_get_tmpseed(MinesweeperField* field);

G_END_DECLS
