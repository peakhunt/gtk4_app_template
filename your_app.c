#include "config.h"

#include <glib/gi18n.h>

#include "your_app.h"
#include "main_window.h"

struct _YourAppApplication
{
  AdwApplication parent_instance;
};

G_DEFINE_FINAL_TYPE (YourAppApplication, your_app_application, ADW_TYPE_APPLICATION)

YourAppApplication*
your_app_application_new (const char *application_id, GApplicationFlags flags)
{
  g_return_val_if_fail (application_id != NULL, NULL);

  return g_object_new (YOUR_APP_TYPE_APPLICATION,
      "application-id", application_id,
      "flags", flags,
      "resource-base-path", RESOURCE_BASE_PATH,
      NULL);
}

static void
your_app_application_activate(GApplication *app)
{
#if 1
  GtkWindow *window;

  g_assert (YOUR_APP_IS_APPLICATION (app));

  window = gtk_application_get_active_window (GTK_APPLICATION (app));

  if (window == NULL)
    window = g_object_new (MAIN_TYPE_WINDOW, "application", app, NULL);

  // Register resource path with the icon theme
  GtkIconTheme *theme = gtk_icon_theme_get_for_display(gdk_display_get_default());
  gtk_icon_theme_add_resource_path(theme, "/org/gnome/Example/images");

  gtk_window_present (window);
#else
  GtkWidget *window;

  window = gtk_application_window_new (GTK_APPLICATION(app));
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);
  gtk_window_present (GTK_WINDOW (window));
#endif
}

static void
your_app_application_class_init(YourAppApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = your_app_application_activate;
}

static void
your_app_application_about_action (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  static const char *developers[] = {"your name", NULL};
  YourAppApplication *self = user_data;
  GtkWindow *window = NULL;

  g_assert (YOUR_APP_IS_APPLICATION (self));

  window = gtk_application_get_active_window (GTK_APPLICATION (self));

  adw_show_about_dialog (GTK_WIDGET (window),
      "application-name", "your app",
      "application-icon", "icon",
      "developer-name", "your name",
      "translator-credits", _("translator-credits"),
      "version", "0.1.0",
      "developers", developers,
      "copyright", "© 2026 your name",
      NULL);
}

static void
your_app_application_quit_action (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
	YourAppApplication *self = user_data;

	g_assert (YOUR_APP_IS_APPLICATION (self));

	g_application_quit (G_APPLICATION (self));
}

static const GActionEntry app_actions[] = {
	{ "quit", your_app_application_quit_action },
	{ "about", your_app_application_about_action },
};

static void
your_app_application_init (YourAppApplication *self)
{
  g_action_map_add_action_entries (G_ACTION_MAP (self),
      app_actions,
      G_N_ELEMENTS (app_actions),
      self);
  gtk_application_set_accels_for_action (GTK_APPLICATION (self),
      "app.quit",
      (const char *[]) { "<control>q", NULL });
}
