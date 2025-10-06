#include "minesweeper.h"

guint num_len(gint num) {
  return (num == 0)?1:(floor(log10(abs(num)))+(num < 0)?2:1);
}

dimension repeate_dim(guint value, guint amount) {
  dimension dim;
  dim.len = amount;
  dim.dim = malloc(sizeof(guint)*amount);
  for (guint i = 0; i < amount; i++) dim.dim[i] = value;
  return dim;
}

static void minesweeper_field_dispose (GObject *gobject);
static void minesweeper_field_finalize (GObject *gobject);

struct _MinesweeperField {
  GtkWidget parent_instance;

  GtkWidget* child;
  GtkWidget* overlay;

  dimension dim;
  guint spacing_multiplier;
  guint uncovered_cells;
  guint area;
  guint bombs;
  dimension tmpdim;
  guint tmpseed;
  guint tmpbombs;
  guint seed;
  guint is_rel : 1;
  guint state : 3;
};

enum {
  RUNNING,
  PAUSED,
  LOST,
  WON,
  FORFEIT
};

G_DEFINE_TYPE (MinesweeperField, minesweeper_field, GTK_TYPE_WIDGET)

static void minesweeper_cell_dispose (GObject *gobject);
static void minesweeper_cell_finalize (GObject *gobject);

struct _MinesweeperCell {
  GtkWidget parent_instance;

  GtkWidget* child;
  GtkWidget* value;
  GtkWidget* bomb;
  GtkWidget* flag;
  GtkWidget* highlight;
  MinesweeperField* field;

  //GtkActionHelper *action_helper;

  dimension loc;
  char* display;

  guint is_bomb : 1;
  guint state : 2;
  guint abs;
  gint rel;
};

G_DEFINE_TYPE (MinesweeperCell, minesweeper_cell, GTK_TYPE_WIDGET)

static void minesweeper_field_execute_at_all(MinesweeperField* field, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data);
void minesweeper_cell_uncover(MinesweeperCell* cell);
static gboolean minesweeper_cell_uncover_wrapper(MinesweeperCell* cell, void* data) {
  minesweeper_cell_uncover(cell);
  return 1;
}

void minesweeper_field_game_running(MinesweeperField* field) {
  if (field->state) {
    gtk_overlay_remove_overlay(GTK_OVERLAY(field->child), field->overlay);
    field->state = RUNNING;
  }
}

void minesweeper_field_game_lost(MinesweeperField* field) {
  if (!field->state) {
    field->overlay = gtk_label_new("Game Over\nYou lost");
    gtk_overlay_add_overlay(GTK_OVERLAY(field->child), field->overlay);
    field->state = LOST;
    minesweeper_field_execute_at_all(field, minesweeper_cell_uncover_wrapper, NULL);
  }
}

void minesweeper_field_game_won(MinesweeperField* field) {
  if (!field->state) {
    field->overlay = gtk_label_new("Game Over\nYou won");
    gtk_overlay_add_overlay(GTK_OVERLAY(field->child), field->overlay);
    field->state = WON;
  }
}

void minesweeper_field_game_forfeit(MinesweeperField* field) {
  if (!field->state) {
    field->overlay = gtk_label_new("Game Over\nYou forfeit");
    gtk_overlay_add_overlay(GTK_OVERLAY(field->child), field->overlay);
    field->state = FORFEIT;
  }
}

void minesweeper_field_game_paused(MinesweeperField* field) {
  if (!field->state) {
    field->overlay = gtk_label_new("Game Paused");
    gtk_overlay_add_overlay(GTK_OVERLAY(field->child), field->overlay);
    field->state = PAUSED;
  }
}

static gboolean minesweeper_field_execute_at(MinesweeperField* field, dimension loc, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data);
static void minesweeper_field_execute_at_influenced_area(MinesweeperField* field, dimension loc, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data);

static gboolean minesweeper_cell_add_influenced_highlight(MinesweeperCell* cell, void* data) {
  gtk_image_set_from_file(GTK_IMAGE(cell->highlight), "./assets/influenced_highlight.png");
  gtk_image_set_pixel_size(GTK_IMAGE(cell->highlight), 40);
}

static void minesweeper_cell_highlight_influenced_area(MinesweeperCell* cell) {
  minesweeper_field_execute_at_influenced_area(cell->field, cell->loc, minesweeper_cell_add_influenced_highlight, NULL);
  gtk_image_set_from_file(GTK_IMAGE(cell->highlight), "./assets/current_highlight.png");
  gtk_image_set_pixel_size(GTK_IMAGE(cell->highlight), 40);
}

static gboolean minesweeper_cell_remove_influenced_highlight(MinesweeperCell* cell, void* data) {
  gtk_image_clear(GTK_IMAGE(cell->highlight));
}

static void minesweeper_cell_unhighlight_influenced_area(MinesweeperCell* cell) {
  minesweeper_field_execute_at_influenced_area(cell->field, cell->loc, minesweeper_cell_remove_influenced_highlight, NULL);
}

static void minesweeper_cell_init(MinesweeperCell* self) {
	GtkWidget* widget = GTK_WIDGET(self);

  self->child = gtk_overlay_new();
  gtk_widget_set_parent(self->child, widget);

  gtk_overlay_set_child(GTK_OVERLAY(self->child), gtk_image_new_from_file("./assets/covered.png"));
  gtk_image_set_pixel_size(GTK_IMAGE(gtk_overlay_get_child(GTK_OVERLAY(self->child))), 40);

  self->value = gtk_label_new(":3");
  gtk_widget_set_parent(self->value, self->child);
  gtk_widget_set_visible(self->value, 0);

  self->flag = gtk_image_new_from_file("./assets/flag.png");
  gtk_widget_set_parent(self->flag, self->child);
  gtk_image_set_pixel_size(GTK_IMAGE(self->flag), 40);
  gtk_widget_set_visible(self->flag, 0);

  self->bomb = gtk_image_new_from_file("./assets/bomb.png");
  gtk_widget_set_parent(self->bomb, self->child);
  gtk_image_set_pixel_size(GTK_IMAGE(self->bomb), 40/1.5);
gtk_widget_set_visible(self->bomb, 0);

  self->highlight = gtk_image_new();
  gtk_overlay_add_overlay(GTK_OVERLAY(self->child), self->highlight);

  GtkGesture* left_click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(left_click), 1);
  g_signal_connect_object(left_click, "pressed", G_CALLBACK(minesweeper_cell_uncover), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(left_click));

  GtkGesture* right_click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(right_click), 3);
  g_signal_connect_object(right_click, "pressed", G_CALLBACK(minesweeper_cell_flag), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(right_click));

  GtkEventController* mouse_enter = gtk_event_controller_motion_new();
  g_signal_connect_object(mouse_enter, "enter", G_CALLBACK(minesweeper_cell_highlight_influenced_area), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(mouse_enter));

  GtkEventController* mouse_leave = gtk_event_controller_motion_new();
  g_signal_connect_object(mouse_leave, "leave", G_CALLBACK(minesweeper_cell_unhighlight_influenced_area), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(mouse_leave));
}

static void minesweeper_cell_dispose(GObject *gobject) {
	MinesweeperCell* self = MINESWEEPER_CELL(gobject);

	g_clear_pointer (&self->child, gtk_widget_unparent);
  free(self->display);
  free(self->loc.dim);

	G_OBJECT_CLASS(minesweeper_cell_parent_class)->dispose (gobject);
}

static void minesweeper_cell_finalize(GObject *gobject) {
  G_OBJECT_CLASS(minesweeper_cell_parent_class)->finalize (gobject);
}

static void minesweeper_cell_class_init(MinesweeperCellClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = minesweeper_cell_dispose;
	object_class->finalize = minesweeper_cell_finalize;

	gtk_widget_class_set_layout_manager_type (GTK_WIDGET_CLASS(klass), GTK_TYPE_BOX_LAYOUT);
}

GtkWidget* minesweeper_cell_new(void) {
	return g_object_new (MINESWEEPER_TYPE_CELL, NULL);
}

static gboolean minesweeper_cell_inc_rel(MinesweeperCell* cell, void* data) {
  cell->rel++;
  return 1;
}

static gboolean minesweeper_cell_dec_rel(MinesweeperCell* cell, void* data) {
  cell->rel--;
  return 1;
}

static void minesweeper_cell_set_display(MinesweeperCell* cell) {
  if (cell->field->is_rel) {
    cell->display = realloc(cell->display, sizeof(char)*num_len(cell->rel));
    if (cell->rel) {
      sprintf(cell->display, "%d", cell->rel);
    } else {
      cell->display[0] = '\0';
    }
  } else {
    cell->display = realloc(cell->display, sizeof(char)*num_len(cell->abs));
    if (cell->abs) {
      sprintf(cell->display, "%d", cell->abs);
    } else {
      cell->display[0] = '\0';
    }
  }
}

void minesweeper_cell_uncover_unless_flag(MinesweeperCell* cell) {
  if (!cell->state) minesweeper_cell_set_state(cell, 1);
}

static gboolean minesweeper_cell_uncover_unless_flag_wrapper(MinesweeperCell* cell, void* data) {
  minesweeper_cell_uncover_unless_flag(cell);
  return 1;
}

static gboolean minesweeper_field_execute_at_recursor(GtkWidget* grid, dimension loc, guint pos, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data) {
  pos -= 2;
  if (pos) {
    return minesweeper_field_execute_at_recursor(gtk_grid_get_child_at(GTK_GRID(grid), loc.dim[pos], loc.dim[pos+1]), loc, pos, func, data);
  } else {
    GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid), loc.dim[0], loc.dim[1]);
    return (*func)(MINESWEEPER_CELL(cell), data);
  }
}

static gboolean minesweeper_field_execute_at(MinesweeperField* field, dimension loc, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data) {
  if (loc.len == 1) {
    return minesweeper_field_execute_at_recursor(gtk_overlay_get_child(GTK_OVERLAY(field->child)), loc, 0, func, data);
  } else if (loc.len%2) {
    return minesweeper_field_execute_at_recursor(gtk_grid_get_child_at(GTK_GRID(gtk_overlay_get_child(GTK_OVERLAY(field->child))), loc.dim[loc.len-1], 0), loc, loc.len-1, func, data);
  } else {
    return minesweeper_field_execute_at_recursor(gtk_overlay_get_child(GTK_OVERLAY(field->child)), loc, loc.len, func, data);
  }
}

static void minesweeper_field_execute_at_influenced_area_recursor(MinesweeperField* field, dimension loc, dimension loc2, guint pos, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data) {
  if (pos) {
    for (guint d = 0; d < 3; d++) {
      gint tmpd = loc.dim[pos]-1+d;
      if (tmpd >= 0 && tmpd < field->dim.dim[pos]) {
        loc2.dim[pos] = tmpd;
        minesweeper_field_execute_at_influenced_area_recursor(field, loc, loc2, pos-1, func, data);
      }
    }
  } else {
    for (guint d = 0; d < 3; d++) {
      gint tmpd = loc.dim[pos]-1+d;
      if (tmpd >= 0 && tmpd < field->dim.dim[pos]) {
        loc2.dim[pos] = tmpd;
        minesweeper_field_execute_at(field, loc2, func, data);
      }
    }
  }
}

static void minesweeper_field_execute_at_influenced_area(MinesweeperField* field, dimension loc, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data) {
  dimension loc2;
  loc2.len = loc.len;
  loc2.dim = malloc(sizeof(guint)*loc2.len);
  for (guint i = 0; i < loc2.len; i++) loc2.dim[i] = loc.dim[i];
  minesweeper_field_execute_at_influenced_area_recursor(field, loc, loc2, loc.len-1, func, data);
  free(loc2.dim);
}

void minesweeper_cell_redraw(MinesweeperCell* cell) {
  if (cell->state%2) {
    gtk_image_set_from_file(GTK_IMAGE(gtk_overlay_get_child(GTK_OVERLAY(cell->child))), "./assets/uncovered.png");
    cell->field->uncovered_cells++;
    if (cell->is_bomb) {
      //GtkWidget* bomb = gtk_image_new_from_file("./assets/bomb.png");
      //gtk_image_set_pixel_size(GTK_IMAGE(bomb), 40/1.5);
      //gtk_overlay_add_overlay(GTK_OVERLAY(cell->child), bomb);
      gtk_widget_set_visible(cell->bomb, 1);
      minesweeper_field_game_lost(cell->field);
    } else {
      minesweeper_cell_set_display(cell);
      //gtk_overlay_add_overlay(GTK_OVERLAY(cell->child), gtk_label_new(cell->display));
      gtk_widget_set_visible(cell->value, 1);
      gtk_label_set_label(GTK_LABEL(cell->value), cell->display);
      if (!cell->abs) minesweeper_field_execute_at_influenced_area(cell->field, cell->loc, minesweeper_cell_uncover_unless_flag_wrapper, NULL);
      if (!(cell->field->area-cell->field->bombs-cell->field->uncovered_cells)) minesweeper_field_game_won(cell->field);
    }
  } else {
    gtk_image_set_from_file(GTK_IMAGE(gtk_overlay_get_child(GTK_OVERLAY(cell->child))), "./assets/covered.png");
  }
}

void minesweeper_cell_set_state(MinesweeperCell* cell, guint state) {
  if (cell->state == state || state > 3) {
    return;
  }
  if (state/2) {
    cell->flag = gtk_image_new_from_file("./assets/flag.png");
    gtk_image_set_pixel_size(GTK_IMAGE(cell->flag), 40);
    gtk_overlay_add_overlay(GTK_OVERLAY(cell->child), cell->flag);
    minesweeper_field_execute_at_influenced_area(cell->field, cell->loc, minesweeper_cell_dec_rel, NULL);
  } else if (cell->state > 1) {
    gtk_overlay_remove_overlay(GTK_OVERLAY(cell->child), cell->flag);
    minesweeper_field_execute_at_influenced_area(cell->field, cell->loc, minesweeper_cell_inc_rel, NULL);
  }
  if (state%2 == cell->state%2) {
    cell->state = state;
    return;
  }
  cell->state = state;
  minesweeper_cell_redraw(cell);
}

void minesweeper_cell_uncover(MinesweeperCell* cell) {
  if (!(cell->state%2)) {
    minesweeper_cell_set_state(cell, cell->state+1);
  }
}

void minesweeper_cell_flag(MinesweeperCell* cell) {
  if (cell->state/2) {
    minesweeper_cell_set_state(cell, cell->state-2);
  } else {
    minesweeper_cell_set_state(cell, cell->state+2);
  }
}

void minesweeper_cell_set_is_bomb(MinesweeperCell* cell, guint is_bomb) {
  cell->is_bomb = is_bomb;
}

guint minesweeper_cell_get_is_bomb(MinesweeperCell* cell) {
  return cell->is_bomb;
}

static gboolean minesweeper_cell_inc_all(MinesweeperCell* cell, void* data) {
  cell->abs++;
  cell->rel++;
  return 1;
}

static gboolean minesweeper_cell_dec_all(MinesweeperCell* cell, void* data) {
  cell->abs--;
  cell->rel--;
  return 1;
}

static void minesweeper_cell_copy_loc(MinesweeperCell* cell, dimension loc) {
  cell->loc.len = loc.len;
  cell->loc.dim = malloc(sizeof(guint)*loc.len);
  for (guint i = 0; i < loc.len; i++) cell->loc.dim[i] = loc.dim[i];
}

static void minesweeper_field_init(MinesweeperField* self) {
	GtkWidget* widget = GTK_WIDGET(self);

  self->child = gtk_overlay_new();
  gtk_widget_set_parent(self->child, widget);

  gtk_overlay_set_child(GTK_OVERLAY(self->child), gtk_grid_new());

  self->spacing_multiplier = 10;
}

static void minesweeper_field_dispose(GObject *gobject) {
	MinesweeperField* self = MINESWEEPER_FIELD(gobject);

	g_clear_pointer (&self->child, gtk_widget_unparent);

  free(self->dim.dim);
  free(self->tmpdim.dim);

	G_OBJECT_CLASS(minesweeper_field_parent_class)->dispose (gobject);
}

static void minesweeper_field_finalize(GObject *gobject) {
  G_OBJECT_CLASS(minesweeper_field_parent_class)->finalize (gobject);
}

static void minesweeper_field_class_init(MinesweeperFieldClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->dispose = minesweeper_field_dispose;
	object_class->finalize = minesweeper_field_finalize;

	gtk_widget_class_set_layout_manager_type(GTK_WIDGET_CLASS(klass), GTK_TYPE_BOX_LAYOUT);
}

GtkWidget* minesweeper_field_new (void) {
	return g_object_new(MINESWEEPER_TYPE_FIELD, NULL);
}

void minesweeper_field_set_dim(MinesweeperField* field, dimension dim) {
  field->dim.len = dim.len;
  field->dim.dim = realloc(field->dim.dim, sizeof(guint)*field->dim.len);
  for (guint i = 0; i < field->dim.len; i++) field->dim.dim[i] = dim.dim[i];
}

void minesweeper_field_set_tmpdim(MinesweeperField* field, dimension dim) {
  field->tmpdim.len = dim.len;
  field->tmpdim.dim = realloc(field->tmpdim.dim, sizeof(guint)*field->tmpdim.len);
  for (guint i = 0; i < field->tmpdim.len; i++) field->tmpdim.dim[i] = dim.dim[i];
}

static void minesweeper_field_generate_recursor(MinesweeperField* field, GtkWidget* grid, gint depth, dimension loc, guint spacing_multiplier) {
  depth -= 2;
  if (depth) {
    int spacing = (depth/2)*spacing_multiplier;
    gtk_grid_set_column_spacing(GTK_GRID(grid), spacing);
    gtk_grid_set_row_spacing(GTK_GRID(grid), spacing);
    for (int x = 0; x < field->dim.dim[depth]; x++) {
      loc.dim[depth] = x;
      for (int y = 0; y < field->dim.dim[depth+1]; y++) {
        GtkWidget* subgrid = gtk_grid_new();
        loc.dim[depth+1] = y;
        minesweeper_field_generate_recursor(field, subgrid, depth, loc, spacing_multiplier);
        gtk_grid_attach(GTK_GRID(grid), subgrid, x, y, 1, 1);
      }
    }
  } else {
    GtkWidget* cell;
    for (int x = 0; x < field->dim.dim[0]; x++) {
      loc.dim[0] = x;
      for (int y = 0; y < field->dim.dim[1]; y++) {
        loc.dim[1] = y;
        cell = minesweeper_cell_new();
        MinesweeperCell* c = MINESWEEPER_CELL(cell);
        minesweeper_cell_copy_loc(c, loc);
        c->field = field;
        gtk_grid_attach(GTK_GRID(grid), cell, x, y, 1, 1);
      }
    }
  }
}

void minesweeper_field_generate(MinesweeperField* field) {
  if (field->dim.len == 0) return;
  GtkWidget* grid = gtk_overlay_get_child(GTK_OVERLAY(field->child));
  dimension loc;
  loc.len = field->dim.len;
  loc.dim = malloc(sizeof(guint)*loc.len);
  if (field->dim.len == 1) {
    GtkWidget* cell;
    for (int x = 0; x < field->dim.dim[0]; x++) {
      cell = minesweeper_cell_new();
      MinesweeperCell* c = MINESWEEPER_CELL(cell);
      minesweeper_cell_copy_loc(c, loc);
      c->field = field;
      gtk_grid_attach(GTK_GRID(grid), cell, x, 0, 1, 1);
    }
  } else {
    if (field->dim.len%2) {
      int spacing = (field->dim.len/2+1)*field->spacing_multiplier;
      gtk_grid_set_column_spacing(GTK_GRID(grid), spacing);
      gtk_grid_set_row_spacing(GTK_GRID(grid), spacing);
      for (int x = 0; x < field->dim.dim[field->dim.len-1]; x++) {
        GtkWidget* subgrid = gtk_grid_new();
        loc.dim[field->dim.len-1] = x;
        minesweeper_field_generate_recursor(field, subgrid, field->dim.len-1, loc, field->spacing_multiplier);
        gtk_grid_attach(GTK_GRID(grid), subgrid, x, 0, 1, 1);
      }
    } else {
      minesweeper_field_generate_recursor(field, grid, field->dim.len, loc, field->spacing_multiplier);
    }
  }
  free(loc.dim);
}

static guint minesweeper_field_calc_area(dimension dim) {
  guint factor = 1;
  for (guint i = 0; i < dim.len; i++) {
    factor *= dim.dim[i];
  }
  return factor;
}

static gboolean minesweeper_cell_place_mine(MinesweeperCell* cell, void* data) {
  if (minesweeper_cell_get_is_bomb(MINESWEEPER_CELL(cell))) {
    return 0;
  } else {
    minesweeper_cell_set_is_bomb(MINESWEEPER_CELL(cell), 1);
    return 1;
  }
}

static gboolean minesweeper_cell_remove_mine(MinesweeperCell* cell, void* data) {
  if (minesweeper_cell_get_is_bomb(MINESWEEPER_CELL(cell))) {
    minesweeper_cell_set_is_bomb(MINESWEEPER_CELL(cell), 0);
    return 1;
  } else {
    return 0;
  }
}

static void minesweeper_field_execute_at_all_recursor(MinesweeperField* field, dimension loc, guint pos, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data) {
  if (pos) {
    for (guint d = 0; d < field->dim.dim[pos]; d++) {
      loc.dim[pos] = d;
      minesweeper_field_execute_at_all_recursor(field, loc, pos-1, func, data);
    }
  } else {
    for (guint d = 0; d < field->dim.dim[pos]; d++) {
      loc.dim[pos] = d;
      minesweeper_field_execute_at(field, loc, func, data);
    }
  }
}

static void minesweeper_field_execute_at_all(MinesweeperField* field, gboolean ((*func)(MinesweeperCell* cell, void* data)), void* data) {
  dimension loc;
  loc.len = field->dim.len;
  loc.dim = malloc(sizeof(guint)*loc.len);
  minesweeper_field_execute_at_all_recursor(field, loc, loc.len-1, func, data);
  free(loc.dim);
}

static gboolean minesweeper_cell_place_mine_and_inc_all(MinesweeperCell* cell, void* data) {
  minesweeper_cell_place_mine(cell, NULL);
  minesweeper_field_execute_at_influenced_area(cell->field, cell->loc, minesweeper_cell_inc_all, NULL);
}

void minesweeper_field_set_seed(MinesweeperField* field, guint seed) {
  field->seed = seed;
}

void minesweeper_field_set_bombs(MinesweeperField* field, guint bombs) {
  field->bombs = bombs;
}

void minesweeper_field_set_tmpseed(MinesweeperField* field, guint seed) {
  field->tmpseed = seed;
}

void minesweeper_field_set_tmpbombs(MinesweeperField* field, guint bombs) {
  field->tmpbombs = bombs;
}

void minesweeper_field_populate(MinesweeperField* field) {
  if (!field->dim.len) return;
  field->area = minesweeper_field_calc_area(field->dim);
  //if (field->bombs > field->area) return;
  dimension loc;
  loc.len = field->dim.len;
  loc.dim = malloc(sizeof(guint)*loc.len);
  MTRand r = seedRand(field->seed);
  gboolean success;
  if (field->bombs < field->area/2) {
    for (guint b = 0; b < field->bombs; b++) {
      do {
        for (guint d = 0; d < loc.len; d++) loc.dim[d] = genRand(&r)*field->dim.dim[d];
        success = minesweeper_field_execute_at(field, loc, minesweeper_cell_place_mine, NULL);
      } while (!success);
      minesweeper_field_execute_at_influenced_area(field, loc, minesweeper_cell_inc_all, NULL);
    }
  } else {
    minesweeper_field_execute_at_all(field, minesweeper_cell_place_mine_and_inc_all, NULL);
    if (field->area <= field->bombs) return;
    for (guint b = 0; b < field->area-field->bombs; b++) {
      do {
        for (guint d = 0; d < loc.len; d++) loc.dim[d] = genRand(&r)*field->dim.dim[d];
        success = minesweeper_field_execute_at(field, loc, minesweeper_cell_remove_mine, NULL);
      } while (!success);
      minesweeper_field_execute_at_influenced_area(field, loc, minesweeper_cell_dec_all, NULL);
    }
  }
}

static gboolean minesweeper_cell_redraw_wrapper(MinesweeperCell* cell, void* data) {
  minesweeper_cell_redraw(cell);
  return 1;
}

void minesweeper_field_toggle_delta_mode(MinesweeperField* field) {
  field->is_rel = (field->is_rel)?0:1;
  minesweeper_field_execute_at_all(field, minesweeper_cell_redraw_wrapper, NULL);
}

void minesweeper_field_set_generate_populate(MinesweeperField* field, dimension dim, guint seed, guint bombs) {
  minesweeper_field_set_dim(field, dim);
  minesweeper_field_set_seed(field, seed);
  minesweeper_field_set_bombs(field, bombs);
  minesweeper_field_generate(field);
  minesweeper_field_populate(field);
}

void minesweeper_field_set_tmp(MinesweeperField* field, dimension dim, guint seed, guint bombs) {
  minesweeper_field_set_tmpdim(field, dim);
  minesweeper_field_set_tmpseed(field, seed);
  minesweeper_field_set_tmpbombs(field, bombs);
}

void minesweeper_field_apply_tmp_generate_populate(MinesweeperField* field) {
  minesweeper_field_set_generate_populate(field, field->tmpdim, field->tmpseed, field->tmpbombs);
}

void minesweeper_field_empty(MinesweeperField* field) {
  GtkWidget* grid = gtk_overlay_get_child(GTK_OVERLAY(field->child));
	g_clear_pointer(&grid, gtk_widget_unparent);
  gtk_overlay_set_child(GTK_OVERLAY(field->child), gtk_grid_new());
}

void minesweeper_field_empty_apply_tmp_generate_populate(MinesweeperField* field) {
  minesweeper_field_game_running(field);
  minesweeper_field_empty(field);
  minesweeper_field_apply_tmp_generate_populate(field);
}

guint minesweeper_field_get_tmpseed(MinesweeperField* field) {
  return field->tmpseed;
}
