#include "gauge_widget.h"
#include <math.h>
#include <graphene.h>
#include <pango/pangocairo.h>

/* Properties */
enum
{
  PROP_0,
  PROP_MIN,
  PROP_MAX,
  PROP_VALUE,
  PROP_SHOW_DIGITAL,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* Instance struct lives in the .c file */
struct _GaugeWidget
{
  GtkWidget parent_instance;

  double   min;
  double   max;
  double   value;
  gboolean show_digital;
};

G_DEFINE_TYPE(GaugeWidget, gauge_widget, GTK_TYPE_WIDGET)

/* --- Helpers --- */
static inline double
angle_from_value(double v, double min, double max)
{
  const double range = max - min;
  double frac = range > 0 ? (v - min) / range : 0.0;
  frac = CLAMP(frac, 0.0, 1.0);
  /* Map to -90°..+90° (left to right), screen coords (Y down) */
  return -M_PI_2 + frac * M_PI;
}

/* --- Snapshot --- */
static void
gauge_widget_snapshot(GtkWidget *widget, GtkSnapshot *snapshot)
{
  GaugeWidget *self = GAUGE_WIDGET(widget);
  const int w = gtk_widget_get_width(widget);
  const int h = gtk_widget_get_height(widget);

  graphene_rect_t bounds = GRAPHENE_RECT_INIT(0, 0, (float)w, (float)h);
  cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &bounds);

  /* Layout: center slightly lower to leave room for labels/readout */
  const double cx = w / 2.0;
  const double cy = h * 0.52;
  const double radius = MIN(w, h) * 0.42;

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

  /* Dial arc (semicircle) */
  cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
  cairo_set_line_width(cr, 2.0);
  cairo_arc(cr, cx, cy, radius, -M_PI_2, M_PI_2);
  cairo_stroke(cr);

  /* Ticks */
  const double major_inset = 12.0;
  const double minor_inset = 8.0;
  const double tick_outer  = radius - 2.0;

  /* Major ticks + labels every 20 units */
  for (int v = (int)self->min; v <= (int)self->max; v += 20) {
    const double a = angle_from_value(v, self->min, self->max);

    /* Major tick line */
    const double x1 = cx + cos(a) * (radius - major_inset);
    const double y1 = cy + sin(a) * (radius - major_inset);
    const double x2 = cx + cos(a) * (tick_outer);
    const double y2 = cy + sin(a) * (tick_outer);

    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);

    /* Label */
    char buf[8];
    g_snprintf(buf, sizeof(buf), "%d", v);

    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string("Sans 10");
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_text(layout, buf, -1);

    int tw = 0, th = 0;
    pango_layout_get_pixel_size(layout, &tw, &th);

    /* Place labels slightly inside the major tick */
    const double lx = cx + cos(a) * (radius - 28.0) - tw / 2.0;
    const double ly = cy + sin(a) * (radius - 28.0) - th / 2.0;

    cairo_save(cr);
    cairo_translate(cr, lx, ly);
    pango_cairo_show_layout(cr, layout);
    cairo_restore(cr);

    pango_font_description_free(desc);
    g_object_unref(layout);
  }

  /* Minor ticks at 10,30,50,70,90 */
  for (int v = (int)self->min; v <= (int)self->max; v += 10) {
    if (v % 20 == 0) continue;
    const double a = angle_from_value(v, self->min, self->max);

    const double x1 = cx + cos(a) * (radius - minor_inset);
    const double y1 = cy + sin(a) * (radius - minor_inset);
    const double x2 = cx + cos(a) * (tick_outer);
    const double y2 = cy + sin(a) * (tick_outer);

    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
  }

  /* Needle (red), mapped to the same arc */
  const double na = angle_from_value(self->value, self->min, self->max);
  const double nx = cx + cos(na) * (radius - 20.0);
  const double ny = cy + sin(na) * (radius - 20.0);

  cairo_set_source_rgb(cr, 0.90, 0.20, 0.20);
  cairo_set_line_width(cr, 3.0);
  cairo_move_to(cr, cx, cy);
  cairo_line_to(cr, nx, ny);
  cairo_stroke(cr);

  /* Hub */
  cairo_arc(cr, cx, cy, 4.5, 0, 2 * M_PI);
  cairo_fill(cr);

  /* Digital readout centered below */
  if (self->show_digital) {
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string("Monospace 14");
    pango_layout_set_font_description(layout, desc);

    char buf[32];
    g_snprintf(buf, sizeof(buf), "%.2f", self->value);
    pango_layout_set_text(layout, buf, -1);

    int tw = 0, th = 0;
    pango_layout_get_pixel_size(layout, &tw, &th);

    const double x = (w - tw) / 2.0;
    // const double y = cy + radius + 10.0; /* just below the arc */
    const double y = h * 0.6;


    cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
    cairo_save(cr);
    cairo_translate(cr, x, y);
    pango_cairo_show_layout(cr, layout);
    cairo_restore(cr);

    pango_font_description_free(desc);
    g_object_unref(layout);
  }

  cairo_destroy(cr);
}

/* --- Measure --- */
static void
gauge_widget_measure(GtkWidget *widget,
                     GtkOrientation orientation,
                     int for_size,
                     int *minimum,
                     int *natural,
                     int *minimum_baseline,
                     int *natural_baseline)
{
  const int base = 240; /* a bit larger to fit labels/readout comfortably */
  if (minimum) *minimum = base;
  if (natural) *natural = base;
  if (minimum_baseline) *minimum_baseline = -1;
  if (natural_baseline) *natural_baseline = -1;
}

/* --- Properties --- */
static void
gauge_widget_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GaugeWidget *self = GAUGE_WIDGET(object);
  switch (prop_id) {
  case PROP_MIN:
    self->min = g_value_get_double(value);
    break;

  case PROP_MAX:
    self->max = g_value_get_double(value);
    break;

  case PROP_VALUE:
    self->value = g_value_get_double(value);
    break;

  case PROP_SHOW_DIGITAL:
    self->show_digital = g_value_get_boolean(value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    return;
  }
  gtk_widget_queue_draw(GTK_WIDGET(object));
}

static void
gauge_widget_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GaugeWidget *self = GAUGE_WIDGET(object);
  switch (prop_id) {
  case PROP_MIN:
    g_value_set_double(value, self->min);
    break;

  case PROP_MAX:
    g_value_set_double(value, self->max);
    break;

  case PROP_VALUE:
    g_value_set_double(value, self->value);
    break;

  case PROP_SHOW_DIGITAL:
    g_value_set_boolean(value, self->show_digital);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}

/* --- Class/init --- */
static void
gauge_widget_class_init(GaugeWidgetClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  widget_class->snapshot = gauge_widget_snapshot;
  widget_class->measure  = gauge_widget_measure;

  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = gauge_widget_set_property;
  object_class->get_property = gauge_widget_get_property;

  obj_properties[PROP_MIN] = g_param_spec_double("min", "Minimum", "Minimum value",
                                                 -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_MAX] = g_param_spec_double("max", "Maximum", "Maximum value",
                                                 -G_MAXDOUBLE, G_MAXDOUBLE, 100.0,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_VALUE] = g_param_spec_double("value", "Value", "Current value",
                                                 -G_MAXDOUBLE, G_MAXDOUBLE, 0.0,
                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  obj_properties[PROP_SHOW_DIGITAL] = g_param_spec_boolean("show-digital", "Show digital", "Show digital readout",
                                                           TRUE,
                                                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
  gtk_widget_class_set_css_name(widget_class, "gaugewidget");
}

static void
gauge_widget_init(GaugeWidget *self)
{
  self->min = 0.0;
  self->max = 100.0;
  self->value = 0.0;
  self->show_digital = TRUE;
}

/* --- Public API --- */
GtkWidget *gauge_widget_new(void)
{
  return g_object_new(GAUGE_TYPE_WIDGET, NULL);
}

void
gauge_widget_set_range(GaugeWidget *self, double min, double max)
{
  g_object_set(self, "min", min, "max", max, NULL);
}

void
gauge_widget_set_value(GaugeWidget *self, double value)
{
  g_object_set(self, "value", value, NULL);
}

double
gauge_widget_get_value(GaugeWidget *self)
{
  double v = 0.0;
  g_object_get(self, "value", &v, NULL);
  return v;
}

void
gauge_widget_set_show_digital(GaugeWidget *self, gboolean show)
{
  g_object_set(self, "show-digital", show, NULL);
}

gboolean
gauge_widget_get_show_digital(GaugeWidget *self)
{
  gboolean s = FALSE;
  g_object_get(self, "show-digital", &s, NULL);
  return s;
}
