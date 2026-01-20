#include "config.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <adwaita.h>

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

/* Load global CSS once at application startup */
static void
your_app_application_startup (GApplication *app)
{
  /* Chain up to parent startup */
  G_APPLICATION_CLASS (your_app_application_parent_class)->startup (app);

  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (provider, "/org/gnome/Example/gtk.css");

  gtk_style_context_add_provider_for_display (
      gdk_display_get_default (),
      GTK_STYLE_PROVIDER (provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
your_app_application_activate (GApplication *app)
{
  GtkWindow *window;

  g_assert (YOUR_APP_IS_APPLICATION (app));

  window = gtk_application_get_active_window (GTK_APPLICATION (app));

  if (window == NULL)
    window = g_object_new (MAIN_TYPE_WINDOW, "application", app, NULL);

  /* Register resource path with the icon theme */
  GtkIconTheme *theme = gtk_icon_theme_get_for_display (gdk_display_get_default ());
  gtk_icon_theme_add_resource_path (theme, "/org/gnome/Example/images");

  gtk_window_present (window);
}

static void
your_app_application_class_init (YourAppApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->startup = your_app_application_startup;
  app_class->activate = your_app_application_activate;
}

static void
your_app_application_init (YourAppApplication *self)
{
  /* nothing needed here */
}
