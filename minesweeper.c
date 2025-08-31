#include "minesweeper.h"

static void minesweeper_cell_dispose (GObject *gobject);
static void minesweeper_cell_finalize (GObject *gobject);

struct _MinesweeperCell {
  GtkWidget parent_instance;

  GtkWidget* child;
  GtkWidget* flag;

  //GtkActionHelper *action_helper;

  guint is_bomb : 1;
  guint state : 2;
  guint abs;
  guint rel;
};

static void minesweeper_field_dispose (GObject *gobject);
static void minesweeper_field_finalize (GObject *gobject);

struct _MinesweeperField {
  GtkWidget parent_instance;

  GtkWidget* child;

  dimension dim;
  guint spacing_multiplier;
};

G_DEFINE_TYPE (MinesweeperCell, minesweeper_cell, GTK_TYPE_WIDGET)

static void minesweeper_cell_uncover(MinesweeperCell* self) {
  if (!self->state) {
    minesweeper_cell_set_state(self, 1);
  }
}

static void minesweeper_cell_flag(MinesweeperCell* self) {
  if (self->state/2) {
    minesweeper_cell_set_state(self, self->state-2);
  } else {
    minesweeper_cell_set_state(self, self->state+2);
  }
}

static void minesweeper_cell_init(MinesweeperCell* self) {
	GtkWidget* widget = GTK_WIDGET(self);

  self->child = gtk_overlay_new();
  gtk_widget_set_parent(self->child, widget);

  gtk_overlay_set_child(GTK_OVERLAY(self->child), gtk_image_new_from_file("./assets/covered.png"));

  GtkGesture* left_click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(left_click), 1);
  g_signal_connect_object(left_click, "pressed", G_CALLBACK(minesweeper_cell_uncover), self, G_CONNECT_SWAPPED);

  gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(left_click));

  GtkGesture* right_click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(right_click), 3);
  g_signal_connect_object(right_click, "pressed", G_CALLBACK(minesweeper_cell_flag), self, G_CONNECT_SWAPPED);

  gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(right_click));
}

static void minesweeper_cell_dispose(GObject *gobject) {
	MinesweeperCell* self = MINESWEEPER_CELL(gobject);

	g_clear_pointer (&self->child, gtk_widget_unparent);

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

void minesweeper_cell_set_state(MinesweeperCell* cell, guint state) {
  if (cell->state == state || state > 3) {
    return;
  }
  if (state/2) {
    cell->flag = gtk_image_new_from_file("./assets/flag.png");
    gtk_overlay_add_overlay(GTK_OVERLAY(cell->child), cell->flag);
  } else if (cell->state > 1) {
    gtk_overlay_remove_overlay(GTK_OVERLAY(cell->child), cell->flag);
  }
  if (state%2 == cell->state%2) {
    cell->state = state;
    return;
  }
  if (state%2) {
    gtk_image_set_from_file(GTK_IMAGE(gtk_overlay_get_child(GTK_OVERLAY(cell->child))), "./assets/uncovered.png");
    if (cell->is_bomb) gtk_overlay_add_overlay(GTK_OVERLAY(cell->child), gtk_image_new_from_file("./assets/bomb.png"));
  } else {
    gtk_image_set_from_file(GTK_IMAGE(gtk_overlay_get_child(GTK_OVERLAY(cell->child))), "./assets/covered.png");
  }
  cell->state = state;
}

void minesweeper_cell_set_is_bomb(MinesweeperCell* cell, guint is_bomb) {
  cell->is_bomb = is_bomb;
}

guint minesweeper_cell_get_is_bomb(MinesweeperCell* cell) {
  return cell->is_bomb;
}

G_DEFINE_TYPE (MinesweeperField, minesweeper_field, GTK_TYPE_WIDGET)

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

static void minesweeper_field_generate_recursor(GtkWidget* grid, dimension dim, gint depth, guint spacing_multiplier) {
  depth -= 2;
  if (depth) {
    int spacing = (depth/2)*spacing_multiplier;
    gtk_grid_set_column_spacing(GTK_GRID(grid), spacing);
    gtk_grid_set_row_spacing(GTK_GRID(grid), spacing);
    for (int x = 0; x < dim.dim[depth]; x++) {
      for (int y = 0; y < dim.dim[depth+1]; y++) {
        GtkWidget* subgrid = gtk_grid_new();
        minesweeper_field_generate_recursor(subgrid, dim, depth, spacing_multiplier);
        gtk_grid_attach(GTK_GRID(grid), subgrid, x, y, 1, 1);
      }
    }
  } else {
    GtkWidget* cell;
    for (int x = 0; x < dim.dim[0]; x++) {
      for (int y = 0; y < dim.dim[1]; y++) {
        /*cell = gtk_button_new();
        gtk_button_set_child(GTK_BUTTON(button), gtk_image_new_from_file(covered_icon));
        gtk_widget_set_size_request(button, -1, -1);
        gtk_widget_set_vexpand(button, false);*/
        cell = minesweeper_cell_new();
        gtk_grid_attach(GTK_GRID(grid), cell, x, y, 1, 1);
      }
    }
  }
}

void minesweeper_field_generate(MinesweeperField* field, dimension dim) {
  if (dim.len == 0) {
    return;
  }
  GtkWidget* grid = gtk_overlay_get_child(GTK_OVERLAY(field->child));
  if (dim.len == 1) {
    GtkWidget* cell;
    for (int x = 0; x < dim.dim[0]; x++) {
      cell = minesweeper_cell_new();
      gtk_grid_attach(GTK_GRID(grid), cell, x, 0, 1, 1);
    }
  } else {
    if (dim.len%2) {
      int spacing = (dim.len/2+1)*field->spacing_multiplier;
      gtk_grid_set_column_spacing(GTK_GRID(grid), spacing);
      gtk_grid_set_row_spacing(GTK_GRID(grid), spacing);
      for (int x = 0; x < dim.dim[dim.len-1]; x++) {
        GtkWidget* subgrid = gtk_grid_new();
        minesweeper_field_generate_recursor(subgrid, dim, dim.len-1, field->spacing_multiplier);
        gtk_grid_attach(GTK_GRID(grid), subgrid, x, 0, 1, 1);
      }
    } else {
      minesweeper_field_generate_recursor(grid, dim, dim.len, field->spacing_multiplier);
    }
  }
  field->dim = dim;
  /*GtkWidget* button = gtk_button_new_with_label("0");
  for (int d = 0; d < dim.len/2; d++) {
    GtkWidget* grid = gtk_grid_new();
    for (int x = 0; x < dim.dim[d*2]; x++) {
      for (int y = 0; y < dim.dim[d*2+1]; y++) {
        gtk_grid_attach(GTK_GRID(grid), button, x, y, 1, 1);
      }
    }
    gtk_grid_set_column_spacing(GTK_GRID(grid), d);
    gtk_grid_set_row_spacing(GTK_GRID(grid), d);
    button = grid;
  }
  gtk_window_set_child(GTK_WINDOW(window), button);*/
}

static guint minesweeper_field_calc_area(dimension dim) {
  guint factor = 1;
  for (guint i = 0; i < dim.len; i++) {
    factor *= dim.dim[i];
  }
  return factor;
}

static gboolean minesweeper_field_place_mine_at_recursor(GtkWidget* grid, dimension loc, guint pos) {
  if (pos > 1) {
    return minesweeper_field_place_mine_at_recursor(gtk_grid_get_child_at(GTK_GRID(grid), loc.dim[pos-1], loc.dim[pos]), loc, pos-1);
  } else {
    GtkWidget* cell = gtk_grid_get_child_at(GTK_GRID(grid), loc.dim[pos-1], loc.dim[pos]);
    if (minesweeper_cell_get_is_bomb(MINESWEEPER_CELL(cell))) {
      return 0;
    } else {
      minesweeper_cell_set_is_bomb(MINESWEEPER_CELL(cell), 1);
      return 1;
    }
  }
}

static gboolean minesweeper_field_place_mine_at(MinesweeperField* field, dimension loc) {
  if (loc.len == 1) {
    return minesweeper_field_place_mine_at_recursor(gtk_overlay_get_child(GTK_OVERLAY(field->child)), loc, 0);
  } else if (loc.len%2) {
    return minesweeper_field_place_mine_at_recursor(gtk_grid_get_child_at(GTK_GRID(gtk_overlay_get_child(GTK_OVERLAY(field->child))), loc.dim[loc.len-1], 0), loc, loc.len-2);
  } else {
    return minesweeper_field_place_mine_at_recursor(gtk_overlay_get_child(GTK_OVERLAY(field->child)), loc, loc.len-1);
  }
}

void minesweeper_field_populate(MinesweeperField* field, guint seed, guint bombs) {
  if (!field->dim.len) return;
  guint area = minesweeper_field_calc_area(field->dim);
  if (bombs > area) return;
  dimension loc;
  loc.len = field->dim.len;
  loc.dim = malloc(sizeof(guint)*loc.len);
  MTRand r = seedRand(seed);
  gboolean success;
  if (bombs < area/2) {
    for (guint b = 0; b < bombs; b++) {
      do {
        for (guint d = 0; d < loc.len; d++) {
          loc.dim[d] = genRand(&r)*field->dim.dim[d];
          printf("%d ", loc.dim[d]);
        }
        printf("\n");
        success = minesweeper_field_place_mine_at(field, loc);
      } while (!success);
    }
  } else {
  }
}
