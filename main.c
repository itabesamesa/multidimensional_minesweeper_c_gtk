#include <time.h>
#include "minesweeper.h"

MTRand r;
GtkWidget* field;

static gboolean check_entry_purity(GtkEntry* entry) {
  GtkEntryBuffer* entry_buf = gtk_entry_get_buffer(entry);
  const char* txt = gtk_entry_buffer_get_text(entry_buf);
  printf("\"%s\"\n", txt);
  for (guint i = 0; i < gtk_entry_buffer_get_length(entry_buf); i++) if (txt[i] <= '0' && txt[i] >= '9') return 0;
  return 1;
}

static void seed_from_entry(GtkEntry* entry, void* field) {
  if (check_entry_purity(entry)) {
    minesweeper_field_set_tmpseed((MinesweeperField*)field, atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry))));
  } else {
    printf("wrong input for seed\n");
  }
}

static void bombs_from_entry(GtkEntry* entry, void* field) {
  if (check_entry_purity(entry)) {
    minesweeper_field_set_tmpbombs((MinesweeperField*)field, atoi(gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry))));
  } else {
    printf("wrong input for bombs\n");
  }
}

static void dim_from_entry(GtkEntry* entry, MinesweeperField* field) {
  GtkEntryBuffer* entry_buf = gtk_entry_get_buffer(entry);
  char* txt = gtk_entry_buffer_get_text(entry_buf);
  printf("\"%s\"\n", txt);
  guint txt_len = gtk_entry_buffer_get_length(entry_buf);
  gboolean pure = 1;
  if (!txt_len) return;
  if (txt[0] == '^' || txt[txt_len-1] == '^') {
    printf("starting/ending carret\n");
    pure = 0;
  } else {
    for (guint i = 0; i < txt_len; i++) {
      if (!(txt[i] >= '0' && txt[i] <= '9') && txt[i] != ' ' && txt[i] != '^') {
        printf("contains non numbers/spaces/carrets: \"%c\"\n", txt[i]);
        pure = 0;
        break;
      }
    }
    for (guint i = 1; i < txt_len-1; i++) {
      if ((txt[i-1] == ' ' && txt[i] == '^') || (txt[i] == '^' && txt[i+1] == ' ')) {
        printf("wrong syntax for carret\n");
        pure = 0;
        break;
      }
    }
  }
  if (pure) {
    guint amount = 0;
    for (guint i = 1; i < txt_len; i++) {
      if ((txt[i-1] >= '0' && txt[i-1] <= '9') && (txt[i] == ' ')) {
        amount++;
      } else if (txt[i] == '^') {
        printf("amount: %d\n", amount);
        amount += atoi(&txt[i+1]);
        printf("amount: %d\n", amount);
      }
    }
    amount++; //for safety...
    printf("amount: %d\n", amount);
    dimension dim;
    dim.len = 0;
    dim.dim = malloc(sizeof(guint)*amount);
    while (txt[0] != '\0') {
      if (txt[0] == ' ') {
        txt = &txt[1];
      } else if (txt[0] == '^') {
        guint power = strtol(&txt[1], &txt, 10)-1;
        for (guint i = 0; i < power; i++) {
          printf("%d ", i);
          dim.dim[dim.len] = dim.dim[dim.len-1];
          dim.len++;
        }
        printf("repeated dim\n");
      } else {
        dim.dim[dim.len] = strtol(txt, &txt, 10);
        dim.len++;
      }
    }
    for (guint i = 0; i < dim.len; i++) printf("%d ", dim.dim[i]); printf("new dim %d\n", dim.len);
    minesweeper_field_set_tmpdim(field, dim);
    free(dim.dim);
  } else {
    printf("wrong input for dimension\n");
  }
}

typedef struct _new_seed_input {
  GtkEntry* entry;
  MinesweeperField* field;
} new_seed_input;

static void new_seed(new_seed_input* nsi) {
  minesweeper_field_set_tmpseed(nsi->field, genRandLong(&r));
  char* seed = malloc(sizeof(char)*num_len(minesweeper_field_get_tmpseed(nsi->field)));
  sprintf(seed, "%ld", minesweeper_field_get_tmpseed(nsi->field));
  printf("%s\n", seed);
  gtk_entry_buffer_delete_text(gtk_entry_get_buffer(nsi->entry), 0, -1);
  gtk_entry_set_placeholder_text(nsi->entry, seed);
  free(seed);
  gtk_widget_grab_focus(GTK_WIDGET(field));
}

static void generate_button_press(MinesweeperField* field) {
  minesweeper_field_empty_apply_tmp_generate_populate(field);
  gtk_widget_grab_focus(GTK_WIDGET(field));
}

static void delta_check_toggle(MinesweeperField* field) {
  minesweeper_field_toggle_delta_mode(field);
  gtk_widget_grab_focus(GTK_WIDGET(field));
}

static void grab_focus_wrapper(GtkEntry* entry, GtkWidget* field) {
  gtk_widget_grab_focus(field);
}

static void on_activate (GtkApplication *app) {
  r = seedRand(time(NULL));
  GtkWidget *window = gtk_application_window_new (app);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_hexpand(box, 1);
  gtk_widget_set_halign(box, GTK_ALIGN_FILL);
  GtkWidget* settings_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(settings_box, GTK_ALIGN_CENTER);
  GtkWidget* entry_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_valign(button_box, GTK_ALIGN_CENTER);
  GtkWidget* field_container = gtk_scrolled_window_new();
  gtk_widget_set_hexpand(field_container, 1);
  gtk_widget_set_vexpand(field_container, 1);
  gtk_widget_set_halign(field_container, GTK_ALIGN_FILL);
  gtk_widget_set_valign(field_container, GTK_ALIGN_FILL);
  field = minesweeper_field_new();
  gtk_widget_set_halign(field, GTK_ALIGN_CENTER);

  GtkWidget* dim_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(dim_box, GTK_ALIGN_END);
  GtkWidget* dim_label = gtk_label_new("Dimensions");
  gtk_box_append(GTK_BOX(dim_box), dim_label);
  GtkWidget* dim_entry = gtk_entry_new();
  g_signal_connect(dim_entry, "changed", G_CALLBACK(dim_from_entry), field); //updates on EVERY key press now. "activate" would only do it on "enter"
  g_signal_connect(dim_entry, "activate", G_CALLBACK(grab_focus_wrapper), field);
  gtk_entry_set_placeholder_text(GTK_ENTRY(dim_entry), "4 4 4 4");
  dimension default_dim = repeate_dim(4, 4);
  minesweeper_field_set_tmpdim(MINESWEEPER_FIELD(field), default_dim);
  free(default_dim.dim);
  gtk_box_append(GTK_BOX(dim_box), dim_entry);
  gtk_box_append(GTK_BOX(entry_box), dim_box);

  GtkWidget* seed_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(seed_box, GTK_ALIGN_END);
  GtkWidget* seed_label = gtk_label_new("Seed");
  gtk_box_append(GTK_BOX(seed_box), seed_label);
  GtkWidget* seed_entry = gtk_entry_new();
  g_signal_connect(seed_entry, "changed", G_CALLBACK(seed_from_entry), field);
  g_signal_connect(seed_entry, "activate", G_CALLBACK(grab_focus_wrapper), field);
  gtk_box_append(GTK_BOX(seed_box), seed_entry);
  gtk_entry_set_placeholder_text(GTK_ENTRY(seed_entry), "1");
  minesweeper_field_set_tmpseed(MINESWEEPER_FIELD(field), 1);
  gtk_box_append(GTK_BOX(entry_box), seed_box);

  GtkWidget* bombs_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_halign(bombs_box, GTK_ALIGN_END);
  GtkWidget* bombs_label = gtk_label_new("Bombs");
  gtk_box_append(GTK_BOX(bombs_box), bombs_label);
  GtkWidget* bombs_entry = gtk_entry_new();
  g_signal_connect(bombs_entry, "changed", G_CALLBACK(bombs_from_entry), field);
  g_signal_connect(bombs_entry, "activate", G_CALLBACK(grab_focus_wrapper), field);
  gtk_box_append(GTK_BOX(bombs_box), bombs_entry);
  gtk_entry_set_placeholder_text(GTK_ENTRY(bombs_entry), "20");
  minesweeper_field_set_tmpbombs(MINESWEEPER_FIELD(field), 20);
  gtk_box_append(GTK_BOX(entry_box), bombs_box);

  gtk_box_append(GTK_BOX(settings_box), entry_box);

  GtkWidget* random_seed_button = gtk_button_new_with_label("Random seed");
  new_seed_input* nsi = malloc(sizeof(new_seed_input));
  nsi->entry = GTK_ENTRY(seed_entry);
  nsi->field = MINESWEEPER_FIELD(field);
  g_signal_connect_swapped(random_seed_button, "clicked", G_CALLBACK(new_seed), nsi);
  gtk_box_append(GTK_BOX(button_box), random_seed_button);

  GtkWidget* generate_button = gtk_button_new_with_label("Generate");
  g_signal_connect_swapped(generate_button, "clicked", G_CALLBACK(generate_button_press), MINESWEEPER_FIELD(field));
  gtk_box_append(GTK_BOX(button_box), generate_button);

  gtk_box_append(GTK_BOX(settings_box), button_box);

  gtk_box_append(GTK_BOX(box), settings_box);

  GtkWidget* delta_check = gtk_check_button_new_with_label("Delta mode");
  gtk_widget_set_halign(delta_check, GTK_ALIGN_CENTER);
  g_signal_connect_swapped(delta_check, "toggled", G_CALLBACK(delta_check_toggle), MINESWEEPER_FIELD(field));
  gtk_box_append(GTK_BOX(box), delta_check);

  minesweeper_field_apply_tmp_generate_populate(MINESWEEPER_FIELD(field));

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(field_container), field);
  gtk_box_append(GTK_BOX(box), field_container);
  gtk_window_set_child(GTK_WINDOW(window), box);

  gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char *argv[]) {
  // Create a new application
  GtkApplication* app = gtk_application_new("com.example.GtkApplication", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  return g_application_run(G_APPLICATION(app), argc, argv);
}
