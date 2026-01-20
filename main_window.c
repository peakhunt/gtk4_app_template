#include "main_window.h"
#include "dashboard_page.h"
#include "preferences_page.h"

struct _MainWindow {
  AdwApplicationWindow parent_instance;

  GtkButton           *sidebar_toggle;
  AdwOverlaySplitView *split_view;
  AdwViewStack        *main_stack;
};

G_DEFINE_FINAL_TYPE (MainWindow, main_window, ADW_TYPE_APPLICATION_WINDOW)

/* Toggle sidebar visibility */
static void
toggle_sidebar (GtkButton *button, MainWindow *self)
{
  gboolean shown = adw_overlay_split_view_get_show_sidebar (self->split_view);
  adw_overlay_split_view_set_show_sidebar (self->split_view, !shown);
}

/* Switch to Dashboard page */
static void
show_dashboard (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  adw_view_stack_set_visible_child_name (self->main_stack, "dashboard");
}

/* Switch to Preferences page */
static void
show_preferences (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  MainWindow *self = MAIN_WINDOW (user_data);
  adw_view_stack_set_visible_child_name (self->main_stack, "preferences");
}

static const GActionEntry win_actions[] = {
  { "show-dashboard",   show_dashboard   },
  { "show-preferences", show_preferences },
};

static void
main_window_class_init (MainWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  /* Ensure custom page types are registered before template instantiation */
  g_type_ensure (DASHBOARD_TYPE_PAGE);
  g_type_ensure (PREFERENCES_TYPE_PAGE);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Example/main_window.ui");

  gtk_widget_class_bind_template_child (widget_class, MainWindow, sidebar_toggle);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, split_view);
  gtk_widget_class_bind_template_child (widget_class, MainWindow, main_stack);

  gtk_widget_class_bind_template_callback (widget_class, toggle_sidebar);
}

static void
main_window_init (MainWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  /* Register window-specific actions (win.*) */
  g_action_map_add_action_entries (G_ACTION_MAP (self),
                                   win_actions,
                                   G_N_ELEMENTS (win_actions),
                                   self);

  /* Start on the dashboard page */
  adw_view_stack_set_visible_child_name (self->main_stack, "dashboard");
}
