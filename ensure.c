#include "ensure.h"

void
ensure_types(void)
{
  g_type_ensure(GAUGE_TYPE_WIDGET);
  g_type_ensure (DASHBOARD_TYPE_PAGE);
  g_type_ensure (PREFERENCES_TYPE_PAGE);
}
