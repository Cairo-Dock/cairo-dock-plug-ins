
#ifndef __CD_TERMINAL_STRUCT__
#define  __CD_TERMINAL_STRUCT__

#include <cairo-dock.h>

struct _AppletConfig {
	guint16  transparency;
	GdkColor backcolor;
	GdkColor forecolor;
	gchar *shortcut;
	gint iNbRows;
	gint iNbColumns;
	int iPositionX, iPositionY;
	} ;

struct _AppletData {
	CairoDialog *dialog;
	GtkWidget *tab;
	} ;


#endif
