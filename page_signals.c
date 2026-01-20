#include "page_signals.h"

void
register_page_signals (GType type)
{
  g_signal_new ("activated", type, G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL,
                G_TYPE_NONE, 0);

  g_signal_new ("deactivated", type, G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL,
                G_TYPE_NONE, 0);
}
