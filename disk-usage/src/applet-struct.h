
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


typedef enum {
	CD_AVAIL_SPACE,
	CD_USED_SPACE
} cdPercentDisplay;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cDevice;
	gchar *cWatermarkImagePath;
	gchar *defaultTitle;
	gchar *cDefaultName;
	
	const gchar *cGThemePath;
	
	gdouble fAlpha;
	
	gint iCheckInterval;
	
	CairoDockLabelDescription *pTopTextDescription;
	
	CairoDockInfoDisplay iInfoDisplay;
	
	cdPercentDisplay iPercentDisplay;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	Gauge *pGauge;
	
	long long llFree;
	long long llTotal;
	long long llUsed;
	long long llAvail;
	gdouble fPourcent;
	
	gchar *cType;
	
	GTimer *pClock;
	
	gboolean bAcquisitionOK;
	
	CairoDockTask *pTask;
} ;

#endif
