#include "config.h"

#include "main_window.h"

struct _MainWindow
{
	AdwApplicationWindow  parent_instance;

	/* Template widgets */
	GtkLabel            *label;
};

G_DEFINE_FINAL_TYPE (MainWindow, main_window, ADW_TYPE_APPLICATION_WINDOW)

static void
main_window_class_init (MainWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Example/main_window.ui");
	gtk_widget_class_bind_template_child (widget_class, MainWindow, label);
}

static void
main_window_init (MainWindow *self)
{
	gtk_widget_init_template (GTK_WIDGET (self));
}
