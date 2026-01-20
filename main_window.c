#include "main_window.h"

struct _MainWindow {
  AdwApplicationWindow parent_instance;

  GtkButton *sidebar_toggle;
  AdwOverlaySplitView *split_view;
  GtkLabel  *main_label;
};

G_DEFINE_FINAL_TYPE (MainWindow, main_window, ADW_TYPE_APPLICATION_WINDOW)

static void
toggle_sidebar (GtkButton *button, MainWindow *self)
{
  gboolean shown = adw_overlay_split_view_get_show_sidebar (self->split_view);
  adw_overlay_split_view_set_show_sidebar (self->split_view, !shown);
}

static void
main_window_class_init (MainWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Example/main_window.ui");

  gtk_widget_class_bind_template_child (widget_class, MainWindow, sidebar_toggle);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, main_label);

  gtk_widget_class_bind_template_callback (widget_class, toggle_sidebar);
}

static void
main_window_init (MainWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

