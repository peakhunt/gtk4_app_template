#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define DASHBOARD_TYPE_PAGE (dashboard_page_get_type())

G_DECLARE_FINAL_TYPE (DashboardPage, dashboard_page, DASHBOARD, PAGE, GtkBox)

G_END_DECLS
