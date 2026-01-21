#include "gauge_widget.h"
#include <math.h>
#include <graphene.h>
#include <pango/pangocairo.h>

/* Properties */
enum {
  PROP_0,
  PROP_MIN,
  PROP_MAX,
  PROP_VALUE,
  PROP_SHOW_DIGITAL,
  PROP_DURATION_MS,   /* new property */
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

/* Instance struct */
struct _GaugeWidget {
  GtkWidget parent_instance;

  double   min;
  double   max;
  double   value;
  gboolean show_digital;

  cairo_surface_t *static_surface;
  int cached_w, cached_h;

  double target_value;      /* where the needle should end up */
  double anim_value;        /* current animated value */
  guint  anim_tick_id;      /* tick callback ID */
  gint64 anim_start_time;   /* microseconds from frame clock */
  gint64 anim_duration;     /* total duration in microseconds */
  double anim_start_value;  /* where needle started */
  double anim_target_value; /* where needle should end */

  double duration_ms;       /* base animation duration in ms (scales with delta) */
};

G_DEFINE_TYPE(GaugeWidget, gauge_widget, GTK_TYPE_WIDGET)

/* --- Helpers --- */

/* Map value to angle for top-facing semicircle: π..2π (180°..360°) */
static inline double
angle_from_value(double v, double min, double max)
{
  const double range = max - min;
  if (range <= 0.0)
    return M_PI;

  double frac = (v - min) / range;
  if (frac < 0.0) frac = 0.0;
  if (frac > 1.0) frac = 1.0;

  return M_PI + frac * M_PI; /* sweep left (π) → right (2π) */
}

static inline void
invalidate_static_cache(GaugeWidget *self)
{
  if (self->static_surface) {
    cairo_surface_destroy(self->static_surface);
    self->static_surface = NULL;
  }
  self->cached_w = 0;
  self->cached_h = 0;
}

/* --- Static dial rebuild --- */
static void
gauge_widget_rebuild_static(GaugeWidget *self, int w, int h)
{
  if (w <= 0 || h <= 0)
    return;

  invalidate_static_cache(self);

  self->static_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  self->cached_w = w;
  self->cached_h = h;

  cairo_t *cr = cairo_create(self->static_surface);

  const double cx = w / 2.0;
  const double cy = h * 0.55;
  const double radius = MIN(w, h) * 0.42;

  cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);

  /* Dial arc: top-facing semicircle (left→top→right) */
  cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
  cairo_set_line_width(cr, 2.0);
  cairo_arc(cr, cx, cy, radius, M_PI, 2 * M_PI);
  cairo_stroke(cr);

  /* Major ticks + labels every 20 units */
  for (int v = (int)self->min; v <= (int)self->max; v += 20) {
    double a = angle_from_value(v, self->min, self->max);

    double x1 = cx + cos(a) * (radius - 12.0);
    double y1 = cy + sin(a) * (radius - 12.0);
    double x2 = cx + cos(a) * (radius - 2.0);
    double y2 = cy + sin(a) * (radius - 2.0);

    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);

    char buf[8];
    g_snprintf(buf, sizeof(buf), "%d", v);

    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string("Sans 10");
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_text(layout, buf, -1);

    int tw = 0, th = 0;
    pango_layout_get_pixel_size(layout, &tw, &th);

    double lx = cx + cos(a) * (radius - 28.0) - tw / 2.0;
    double ly = cy + sin(a) * (radius - 28.0) - th / 2.0;

    cairo_save(cr);
    cairo_translate(cr, lx, ly);
    pango_cairo_show_layout(cr, layout);
    cairo_restore(cr);

    pango_font_description_free(desc);
    g_object_unref(layout);
  }

  /* Minor ticks every 10 units */
  for (int v = (int)self->min; v <= (int)self->max; v += 10) {
    if (v % 20 == 0) continue;
    double a = angle_from_value(v, self->min, self->max);

    double x1 = cx + cos(a) * (radius - 8.0);
    double y1 = cy + sin(a) * (radius - 8.0);
    double x2 = cx + cos(a) * (radius - 2.0);
    double y2 = cy + sin(a) * (radius - 2.0);

    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
  }

  cairo_destroy(cr);
}

/* --- Snapshot --- */
static void
gauge_widget_snapshot(GtkWidget *widget, GtkSnapshot *snapshot)
{
  GaugeWidget *self = GAUGE_WIDGET(widget);
  int w = gtk_widget_get_width(widget);
  int h = gtk_widget_get_height(widget);

  if (!self->static_surface || w != self->cached_w || h != self->cached_h)
    gauge_widget_rebuild_static(self, w, h);

  graphene_rect_t bounds = GRAPHENE_RECT_INIT(0, 0, (float)w, (float)h);
  cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &bounds);

  if (self->static_surface) {
    cairo_set_source_surface(cr, self->static_surface, 0, 0);
    cairo_paint(cr);
  }

  /* Needle */
  const double cx = w / 2.0;
  const double cy = h * 0.55;
  const double radius = MIN(w, h) * 0.42;

  double na = angle_from_value(self->anim_value, self->min, self->max);
  double nx = cx + cos(na) * (radius - 20.0);
  double ny = cy + sin(na) * (radius - 20.0);

  cairo_set_source_rgb(cr, 0.90, 0.20, 0.20);
  cairo_set_line_width(cr, 3.0);
  cairo_move_to(cr, cx, cy);
  cairo_line_to(cr, nx, ny);
  cairo_stroke(cr);

  cairo_arc(cr, cx, cy, 4.5, 0, 2 * M_PI);
  cairo_fill(cr);

  /* Digital readout below arc */
  if (self->show_digital)
  {
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_from_string("Monospace 14");
    pango_layout_set_font_description(layout, desc);

    char buf[32];
    g_snprintf(buf, sizeof(buf), "%.2f", self->anim_value);
    pango_layout_set_text(layout, buf, -1);

    int tw = 0, th = 0;
    pango_layout_get_pixel_size(layout, &tw, &th);

    double x = (w - tw) / 2.0;
    double y = cy + radius * 0.05;

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
  const int base = 240;
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
    invalidate_static_cache(self);
    break;
  case PROP_MAX:
    self->max = g_value_get_double(value);
    invalidate_static_cache(self);
    break;
  case PROP_VALUE:
    gauge_widget_set_value(self, g_value_get_double(value));
    break;
  case PROP_SHOW_DIGITAL:
    self->show_digital = g_value_get_boolean(value);
    break;
  case PROP_DURATION_MS:
    self->duration_ms = g_value_get_double(value);
    if (self->duration_ms < 1.0) self->duration_ms = 1.0;
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
  case PROP_DURATION_MS:
    g_value_set_double(value, self->duration_ms);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

/* --- Dispose --- */
static void
gauge_widget_dispose(GObject *object)
{
  GaugeWidget *self = GAUGE_WIDGET(object);
  invalidate_static_cache(self);
  G_OBJECT_CLASS(gauge_widget_parent_class)->dispose(object);
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
  object_class->dispose      = gauge_widget_dispose;

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
  obj_properties[PROP_DURATION_MS] = g_param_spec_double("duration-ms", "Duration (ms)",
                                                         "Base animation duration in milliseconds (scaled by delta/50)",
                                                         1.0, G_MAXDOUBLE, 2000.0,
                                                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
  gtk_widget_class_set_css_name(widget_class, "gaugewidget");
}

static void
gauge_widget_on_unmap(GtkWidget *widget, gpointer user_data)
{
  GaugeWidget *self = GAUGE_WIDGET(widget);

  /* Stop animation */
  if (self->anim_tick_id != 0)
  {
    gtk_widget_remove_tick_callback(widget, self->anim_tick_id);
    self->anim_tick_id = 0;
  }

  /* Force needle to final value */
  if (self->anim_value != self->value)
  {
    self->anim_value = self->value;
    // gtk_widget_queue_draw(widget);
  }
}

static void
gauge_widget_on_map(GtkWidget *widget, gpointer user_data)
{
  GaugeWidget *self = GAUGE_WIDGET(widget);

  /* Ensure anim_value is synced to value */
  self->anim_value = self->value;
  gtk_widget_queue_draw(widget);
}

static void
gauge_widget_init(GaugeWidget *self)
{
  self->min = 0.0;
  self->max = 100.0;
  self->value = 0.0;
  self->show_digital = TRUE;

  self->static_surface = NULL;
  self->cached_w = 0;
  self->cached_h = 0;

  self->target_value  = 0;
  self->anim_value    = 0;
  self->anim_tick_id  = 0;

  self->anim_start_time   = 0;
  self->anim_duration     = 0;
  self->anim_start_value  = 0;
  self->anim_target_value = 0;

  self->duration_ms = 2000.0; /* default base duration */

  g_signal_connect(self, "unmap", G_CALLBACK(gauge_widget_on_unmap), NULL);
  g_signal_connect(self, "map",   G_CALLBACK(gauge_widget_on_map),   NULL);
}

/* --- Public API --- */
GtkWidget *
gauge_widget_new(void)
{
  return g_object_new(GAUGE_TYPE_WIDGET, NULL);
}

void
gauge_widget_set_range(GaugeWidget *self, double min, double max)
{
  g_object_set(self, "min", min, "max", max, NULL);
}

static gboolean
gauge_widget_tick_cb(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer user_data)
{
  GaugeWidget *self = GAUGE_WIDGET(widget);

  gint64 now = gdk_frame_clock_get_frame_time(frame_clock); /* µs */
  double t = 0.0;

  if (self->anim_duration > 0)
  {
    t = (double)(now - self->anim_start_time) / (double)self->anim_duration;
  }

  if (t >= 1.0)
  {
    self->anim_value = self->anim_target_value;

    gtk_widget_remove_tick_callback(widget, self->anim_tick_id);
    self->anim_tick_id = 0;

    /* Optional: emit "animation-finished" here */
  } else {
    /* Ease-out cubic interpolation */
    double u = 1.0 - pow(1.0 - t, 3);
    self->anim_value = self->anim_start_value +
      (self->anim_target_value - self->anim_start_value) * u;
  }

  gtk_widget_queue_draw(widget);
  return G_SOURCE_CONTINUE;
}

void
gauge_widget_set_value(GaugeWidget *self, double value)
{
  /* Clamp to range */
  if (value < self->min) value = self->min;
  if (value > self->max) value = self->max;

  /* Update canonical property immediately */
  self->value = value;

  /* Duration scales with delta relative to 50 units */
  double delta = fabs(value - self->anim_value);
  double duration_ms = (delta / 50.0) * self->duration_ms;
  if (duration_ms < 1.0) duration_ms = 1.0;

  /* Retarget mid-flight: start from current interpolated anim_value */
  self->anim_start_value  = self->anim_value;
  self->anim_target_value = value;
  self->anim_start_time   = g_get_monotonic_time(); /* µs */
  self->anim_duration     = (gint64)(duration_ms * 1000.0);

  if (self->anim_tick_id == 0) {
    self->anim_tick_id = gtk_widget_add_tick_callback(GTK_WIDGET(self),
        gauge_widget_tick_cb,
        NULL, NULL);
  }

  gtk_widget_queue_draw(GTK_WIDGET(self));
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
