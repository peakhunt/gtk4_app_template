#include "dashboard_page.h"

struct _DashboardPage {
  GtkBox parent_instance;

  GtkButton     *refresh_button;
};

G_DEFINE_TYPE (DashboardPage, dashboard_page, GTK_TYPE_BOX)

  static void
dashboard_page_class_init (DashboardPageClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
      "/org/gnome/Example/dashboard_page.ui");

  gtk_widget_class_bind_template_child (widget_class, DashboardPage, refresh_button);
}

  static void
dashboard_page_init (DashboardPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  /* Example: connect signal to refresh_button */
  g_signal_connect (self->refresh_button, "clicked", G_CALLBACK (gtk_widget_queue_draw), self);
}
