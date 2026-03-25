#include "about_page.h"
#include "page_signals.h"

struct _AboutPage {
  GtkBox parent_instance;
};

G_DEFINE_TYPE (AboutPage, about_page, GTK_TYPE_BOX)

static void
about_page_class_init (AboutPageClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Example/about_page.ui");

  register_page_signals(G_TYPE_FROM_CLASS(klass));
}

static void
about_page_init (AboutPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
