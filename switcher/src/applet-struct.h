
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define SWITCHER_DEFAULT_NAME "Switcher"


typedef enum {
	SWICTHER_DRAW_FRAME,
	SWICTHER_FILL,
	SWICTHER_FILL_INVERTED,
	SWICTHER_NB_MODES,
	} SwitcherDrawCurrentDesktopMode;

struct _AppletConfig {
	gboolean bCompactView;
	gboolean bMapWallpaper;
	gboolean bDrawWindows;
	gboolean bDisplayNumDesk;
	gchar *cDefaultIcon;
	gboolean bDesklet3D;
	gchar *cThemePath;
	gchar *cRenderer;
	gdouble RGBInLineColors[4];
	gdouble RGBLineColors[4];
	gdouble RGBWLineColors[4];
	gdouble RGBIndColors[4];
	gint iInLineSize;
	gint iLineSize;
	gint iWLineSize;
	gboolean bPreserveScreenRatio;
	SwitcherDrawCurrentDesktopMode iDrawCurrentDesktopMode;
	} ;

typedef struct
{
	gint iCurrentDesktop, iCurrentViewportX, iCurrentViewportY;
	gint iNbViewportTotal;
	gint iNbLines, iNbColumns;
	gint iCurrentLine, iCurrentColumn;
	double fOneViewportWidth;
	double fOneViewportHeight;
} SwitcherApplet;


struct _AppletData {
	SwitcherApplet switcher;
	cairo_surface_t *pDefaultMapSurface;
} ;
#endif
