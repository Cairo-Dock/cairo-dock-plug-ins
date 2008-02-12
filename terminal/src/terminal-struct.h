
#ifndef __CD_TERMINAL_STRUCT__
#define  __CD_TERMINAL_STRUCT__

#include <cairo-dock.h>

typedef struct {
	guint16  transparency;
	GdkColor backcolor;
	GdkColor forecolor;
	gchar *shortcut;
	gint iNbRows;
	gint iNbColumns;
	int iPositionX, iPositionY;
	} AppletConfig;

typedef struct {
	CairoDockDesklet *desklet;
	CairoDockDialog *dialog;
	GtkWidget *tab;
	} AppletData;


#endif
