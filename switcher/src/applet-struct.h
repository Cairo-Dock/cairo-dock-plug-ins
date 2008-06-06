
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#include <glib.h>

#define SWITCHER_DEFAULT_NAME "Switcher"

typedef struct {
	gboolean bCurrentView;
	gboolean bMapWallpaper;
	gboolean bDisplayNumDesk;
	gboolean bInvertIndicator;
	gint iCheckInterval;
	gchar *cDefaultIcon;
	gchar *cDefaultSDockIcon;
	gchar *cBrokenIcon;
	gchar *cShortcut;
	gboolean bUseSeparator;
	gboolean bDesklet3D;
	gchar *cThemePath;
	gchar *cRenderer;
	gchar *cRGBColor;
	double RGBInLineColors[4];
	double RGBLineColors[4];
	double RGBIndColors[4];
	double cInLineSize;
	double cLineSize;
	} AppletConfig;


typedef struct
{
  	int 	*ScreenCurrentSize;
	int 	*ScreenCurrentNum;
	int ScreenCurrentNums;
	int ScreenCurrentSizes;
	int iNbViewportX;
	int iNbViewportY;
	int iDesktopViewportX;
	int iDesktopViewportY;
	double MaxWidthIcon;
	double MaxHeightIcon;
	double MaxNbLigne;
	double NumDeskbyLigne;
	double	i;
	double MyLineSize;
}SwitcherApplet;


typedef struct {
	SwitcherApplet switcher;
	GList *pDeskletIconList;
	GList *pIconList;
	gint iMaxIconWidth;
	gint iNbIcons;
	gboolean bErrorRetrievingData;
	int *g_iNbDesktops;
	cairo_surface_t *pSurface;
	cairo_surface_t *pSurfaceSDock;
	cairo_surface_t *pBrokenSurface;
} AppletData;
#endif
