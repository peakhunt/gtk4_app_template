#pragma once
#include <adwaita.h>

G_BEGIN_DECLS

#define PREFERENCES_TYPE_PAGE (preferences_page_get_type())

G_DECLARE_FINAL_TYPE (PreferencesPage, preferences_page, PREFERENCES, PAGE, GtkBox)

G_END_DECLS

