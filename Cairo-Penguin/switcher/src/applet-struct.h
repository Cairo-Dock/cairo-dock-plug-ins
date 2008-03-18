#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <glib.h>


#define SWITCHER_DEFAULT_NAME "Switcher"
typedef struct {
	gint iNbDesks;
	gint iNbCols;
	gint iNbRows;
	gboolean bUseSeparator;
	gboolean bDesklet3D;
	gchar *cRenderer;
	} AppletConfig;

enum
{
	CENTRE,
	NW,
	NE,
	SE,
	SW
};

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
{	//struct Switcher_Applet	*mySwitcher;
  	int 	*ScreenCurrentSize;
	int 	*ScreenCurrentNum;
	
//int 	*ScreenNbRows;
//	int 	*ScreenNbCols;
	int			i;
}SwitcherApplet;

typedef struct {
	gint *numdesks;
	gchar *cName;
	gchar *cDate;
	gchar *cTempMax;
	gchar *cTempMin;
	gchar *cSunRise;
	gchar *cSunSet;
	//DayPart part[2];
	} Desk;

typedef struct {
	gint no_data;
	int acNum;
	SwitcherApplet switcher;
	int *g_iNbDesktops;
	int iDesktopNumber;
	int loadaftercompiz;
	GList *pDeskletIconList;
	} AppletData;


#endif
