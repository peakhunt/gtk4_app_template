#pragma once
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GAUGE_TYPE_WIDGET (gauge_widget_get_type())

/* Declare a final type: GaugeWidget extends GtkWidget */
G_DECLARE_FINAL_TYPE(GaugeWidget, gauge_widget, GAUGE, WIDGET, GtkWidget)

/* Public API */
GtkWidget *gauge_widget_new(void);

void       gauge_widget_set_range(GaugeWidget *self, double min, double max);
void       gauge_widget_set_value(GaugeWidget *self, double value);
double     gauge_widget_get_value(GaugeWidget *self);
void       gauge_widget_set_show_digital(GaugeWidget *self, gboolean show);
gboolean   gauge_widget_get_show_digital(GaugeWidget *self);

G_END_DECLS
