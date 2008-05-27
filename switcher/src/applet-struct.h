
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>
#include <glib.h>

#define SWITCHER_DEFAULT_NAME "Switcher"

typedef struct {
	gboolean bCurrentView;
	gboolean bShowSubDock;
	gboolean bDisplayNumDesk;
	gint iCheckInterval;
	gchar *cDefaultIcon;
	gchar *cBrokenIcon;
	gchar *cShortcut;
	gboolean bUseSeparator;
	gboolean bDesklet3D;
	gchar *cThemePath;
	gchar *cRenderer;
	gchar *cRGBColor;
	double RGBLineColors[4];
	double RGBIndColors[4];
	double cLineSize;
	} AppletConfig;

typedef enum {
	MY_APPLET_NOTHING = 0,
	MY_APPLET_NUMERO_DESKTOP,
	MY_APPLET_NB_QUICK_INFO_TYPE
	} MyAppletQuickInfoType;

enum 
{
	IMAGE_CACHE_PIXBUF,
	IMAGE_CACHE_SURFACE
};


typedef struct
{
	gpointer	data;
	gint		width;
	gint		height;
	time_t		time_stamp;
	int			img_type;
}Image_cache_item;




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
	int MaxWidthIcon;
	int MaxHeightIcon;
	int MaxNbLigne;
	int NumDeskbyLigne;
	int	i;
	double MyLineSize;
	GdkPixbuf    *icon;
	GdkPixbuf *scale;
GdkPixbuf *iconedock;
}SwitcherApplet;


typedef struct {
	gint no_data;
	int acNum;
	SwitcherApplet switcher;
	gint iSidTimer;
	GList *pDeskletIconList;
	GList *pIconList;
	gint iMaxIconWidth;
	gint iNbIcons;
	gboolean bErrorRetrievingData;
	int LoadAfterCompiz;
	int *g_iNbDesktops;
	int iDesktopNumber;
	cairo_surface_t *pSurface;
	cairo_surface_t *pSurfaceNew;
	cairo_surface_t *pBrokenSurface;
	} AppletData;


#endif
