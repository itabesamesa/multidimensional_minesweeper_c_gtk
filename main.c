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
  dim.len = 2;
  dim.dim = malloc(sizeof(guint)*dim.len);
  dim.dim[0] = 4;
  dim.dim[1] = 4;
  //dim.dim[2] = 4;
  /*dim.dim[3] = 4;
  dim.dim[4] = 2;
  dim.dim[5] = 2;
  dim.dim[6] = 2;*/

  GtkWidget* field = minesweeper_field_new();
  minesweeper_field_generate(MINESWEEPER_FIELD(field), dim);
  minesweeper_field_populate(MINESWEEPER_FIELD(field), 1, 1);
  gtk_window_set_child(GTK_WINDOW(window), field);

  gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication* app = gtk_application_new("com.example.GtkApplication", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
