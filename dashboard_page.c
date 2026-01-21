#include "dashboard_page.h"
#include "page_signals.h"
#include "gauge_widget.h"

struct _DashboardPage {
  GtkBox parent_instance;

  GtkButton     *refresh_button;
  GaugeWidget   *test_gauge;   /* reference to gauge */
  guint          update_timer; /* timeout ID */
};

G_DEFINE_TYPE (DashboardPage, dashboard_page, GTK_TYPE_BOX)

/* --- Helpers --- */
static gboolean
update_gauge_cb(gpointer user_data)
{
  DashboardPage *self = DASHBOARD_PAGE(user_data);

  /* Generate random value between min and max */
  double min = 0.0, max = 100.0;
  double value = g_random_double_range(min, max);

  gauge_widget_set_value(self->test_gauge, value);

  return G_SOURCE_CONTINUE; /* keep repeating */
}

/* --- Page signals --- */
static void
on_page_activated(GObject *stack, GParamSpec *pspec, gpointer user_data)
{
  DashboardPage *self = DASHBOARD_PAGE(user_data);

  if (self->update_timer == 0) {
    self->update_timer = g_timeout_add(1000, update_gauge_cb, self);
  }
}

static void
on_page_deactivated(GObject *stack, GParamSpec *pspec, gpointer user_data)
{
  DashboardPage *self = DASHBOARD_PAGE(user_data);

  if (self->update_timer != 0) {
    g_source_remove(self->update_timer);
    self->update_timer = 0;
  }
}

/* --- Class/init --- */
static void
dashboard_page_class_init (DashboardPageClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  gtk_widget_class_set_template_from_resource (widget_class,
      "/org/gnome/Example/dashboard_page.ui");

  gtk_widget_class_bind_template_child (widget_class, DashboardPage, refresh_button);
  gtk_widget_class_bind_template_child (widget_class, DashboardPage, test_gauge);

  register_page_signals(G_TYPE_FROM_CLASS(klass));
}

static void
dashboard_page_init (DashboardPage *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->update_timer = 0;

  g_signal_connect (self, "activated",   G_CALLBACK (on_page_activated),   self);
  g_signal_connect (self, "deactivated", G_CALLBACK (on_page_deactivated), self);

  /* Example: connect signal to refresh_button */
  g_signal_connect (self->refresh_button, "clicked",
                    G_CALLBACK (gtk_widget_queue_draw), self);
}

