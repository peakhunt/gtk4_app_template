#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define ABOUT_TYPE_PAGE (about_page_get_type())

G_DECLARE_FINAL_TYPE (AboutPage, about_page, ABOUT, PAGE, GtkBox)

G_END_DECLS
