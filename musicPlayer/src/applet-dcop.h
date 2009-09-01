#ifndef __CD_DCOP__
#define  __CD_DCOP__

#include <cairo-dock.h>
#include <gtk/gtk.h>
G_BEGIN_DECLS

gchar *cd_dcop_get_string (const char *cCommand);

gint cd_dcop_get_int (const char *cCommand);

gboolean cd_dcop_get_boolean (const char *cCommand);

G_END_DECLS

#endif
