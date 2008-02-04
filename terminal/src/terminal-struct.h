
#ifndef __CD_TERMINAL_STRUCT__
#define  __CD_TERMINAL_STRUCT__

#include <cairo-dock.h>

#include "cairo-dock-desklet.h"


typedef struct {
	guint16  transparency;
	gboolean always_on_top;
	GdkColor backcolor;
	GdkColor forecolor;
	gchar *shortcut;
	gint iNbRows;
	gint iNbColumns;
	gboolean bIsInitiallyDetached;
	int iPositionX, iPositionY;
	} AppletConfig;

typedef struct {
	CairoDockDesklet *desklet;
	CairoDockDialog *dialog;
	GtkWidget *tab;
	} AppletData;


#endif
