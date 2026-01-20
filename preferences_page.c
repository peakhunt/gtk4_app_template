#include "preferences_page.h"

struct _PreferencesPage {
  GtkBox parent_instance;

  /* Template children */
  AdwStatusPage *status;
  GtkSwitch     *dark_mode_switch;
};

G_DEFINE_TYPE (PreferencesPage, preferences_page, GTK_TYPE_BOX)

static void
preferences_page_class_init (PreferencesPageClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Example/preferences_page.ui");

  gtk_widget_class_bind_template_child (widget_class, PreferencesPage, status);
  gtk_widget_class_bind_template_child (widget_class, PreferencesPage, dark_mode_switch);
}

static void
preferences_page_init (PreferencesPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  /* Example: connect signal to dark_mode_switch */
  g_signal_connect (self->dark_mode_switch, "notify::active",
                    G_CALLBACK (gtk_widget_queue_draw), self);
}
