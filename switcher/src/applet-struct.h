
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#include <glib.h>

#define SWITCHER_DEFAULT_NAME "Switcher"

typedef struct {
	gboolean bCompactView;
	gboolean bMapWallpaper;
	gboolean bDisplayNumDesk;
	gchar *cDefaultIcon;
	gboolean bDesklet3D;
	gchar *cThemePath;
	gchar *cRenderer;
	gdouble RGBInLineColors[4];
	gdouble RGBLineColors[4];
	gdouble RGBIndColors[4];
	gint iInLineSize;
	gint iLineSize;
	gboolean bPreserveScreenRatio;
	} AppletConfig;


typedef struct
{
	gint iCurrentDesktop, iCurrentViewportX, iCurrentViewportY;
	gint iNbViewportTotal;
	gint iNbLines, iNbColumns;
	gint iCurrentLine, iCurrentColumn;
	double fOneViewportWidth;
	double fOneViewportHeight;
}SwitcherApplet;


typedef struct {
	SwitcherApplet switcher;
	cairo_surface_t *pDefaultMapSurface;
} AppletData;
#endif
