#include "minesweeper.h"

enum {
  COVERED,
  UNCOVERED,
  COVERED_FLAG,
  UNCOVERED_FLAG,
};

enum {
  SAFE,
  BOMB,
};

static int grid_spacing_multiplier = 10;
static char covered_icon[20] = "./assets/covered.png";
static char uncovered_icon[22] = "./assets/uncovered.png";

typedef struct _cell {
  short state; //0 covered, 1 uncovered, 2 covered flagged, 3 uncovered flagged
short bomb; //0 not a bomb, 1 is a bomb
  int abs;
  int rel;
} cell;

typedef struct _field {
  dimension dim;
  int bombs;
  int covered;
  int uncovered;
  cell* cells;
} field;
/*
struct _CellClass {
  GtkContainerClass parent_class;

  void (* pressed)  (GtkButton *button);
  void (* released) (GtkButton *button);
  void (* clicked)  (GtkButton *button);
  void (* enter)    (GtkButton *button);
  void (* leave)    (GtkButton *button);
};

struct _Cell {
  GtkContainer container;

  GtkWidget *child;

  guint in_button : 1;
  guint button_down : 1;
  guint is_bomb : 1;
  guint state : 2;
  guint abs;
  guint rel;
};

guint          cell_get_type        (void);
GtkWidget*     cell_new             (void);
void           cell_clear           (cell* c);

enum {
  CELL_SIGNAL,
  LAST_SIGNAL
};

static gint cell_signals[LAST_SIGNAL] = { 0 };

static void cell_class_init (CellClass *class) {
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) class;

  cell_signals[CELL_SIGNAL] = gtk_signal_new ("cell",
                                         GTK_RUN_FIRST,
                                         object_class->type,
                                         GTK_SIGNAL_OFFSET (CellClass, cell),
                                         gtk_signal_default_marshaller, GTK_ARG_NONE, 0);


  gtk_object_class_add_signals (object_class, tictactoe_signals, LAST_SIGNAL);

  class->tictactoe = NULL;
}

guint cell_get_type () {
  static guint cell_type = 0;

  if (!cell_type) {
    GtkTypeInfo cell_info = {
      "Cell",
      sizeof (Cell),
      sizeof (CellClass),
      (GtkClassInitFunc) cell_class_init,
      (GtkObjectInitFunc) cell_init,
      (GtkArgFunc) NULL,
    };

    cell_type = gtk_type_unique (gtk_vbox_get_type (), &cell_info);
  }

  return cell_type;
}
*/

static void
print_hello (GtkWidget *widget,
             gpointer   data)
{
  g_print ("Hello World\n");
}

static void on_activate (GtkApplication *app) {
  // Create a new window
  GtkWidget *window = gtk_application_window_new (app);
  // Create a new button
  GtkWidget *button = gtk_button_new_with_label ("Hello, World!");
  // When the button is clicked, close the window passed as an argument
  g_signal_connect_swapped (button, "clicked", G_CALLBACK (gtk_window_close), window);
  gtk_window_set_child (GTK_WINDOW (window), button);

  GtkWidget* grid = gtk_grid_new();
  gtk_window_set_child(GTK_WINDOW(window), grid);
  char number[5];
  for (short x = 0; x < 9; x++) {
    sprintf(number, "%d\n", x);
    button = gtk_button_new_with_label(number);
    g_signal_connect(button, "clicked", G_CALLBACK(print_hello), NULL);
    gtk_grid_attach(GTK_GRID(grid), button, x%3, x/3, 1, 1);
  }

  dimension dim;
  dim.len = 7;
  dim.dim = malloc(sizeof(int)*dim.len);
  dim.dim[0] = 2;
  dim.dim[1] = 2;
  dim.dim[2] = 2;
  dim.dim[3] = 2;
  dim.dim[4] = 2;
  dim.dim[5] = 2;
  dim.dim[6] = 2;

  GtkWidget* field = minesweeper_field_new();
  minesweeper_field_generate(MINESWEEPER_FIELD(field), dim);
  gtk_window_set_child(GTK_WINDOW(window), field);

  gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication* app = gtk_application_new("com.example.GtkApplication", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
