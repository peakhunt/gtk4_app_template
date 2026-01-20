#pragma once

#include <adwaita.h>

G_BEGIN_DECLS

#define YOUR_APP_TYPE_APPLICATION (your_app_application_get_type())

G_DECLARE_FINAL_TYPE (YourAppApplication, your_app_application, YOUR_APP, APPLICATION, AdwApplication)

YourAppApplication *your_app_application_new (const char *application_id, GApplicationFlags flags);

G_END_DECLS
