
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *cGPUName;
	int iVideoRam;
	gchar *cDriverVersion;
	int iGPUTemp;
} nVidiaData;

typedef enum {
	MY_APPLET_TEMP_NONE = 0,
	MY_APPLET_TEMP_ON_QUICKINFO,
	MY_APPLET_TEMP_ON_NAME,
	MY_APPLET_TEMP_ON_ICON,
	MY_APPLET_TEMP_NUMBER,
} nVidiaTemp;

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	nVidiaTemp iDrawTemp;
	gchar *cBrokenUserImage;
	gchar *cGThemePath;
	gchar *defaultTitle;
	int iCheckInterval;
	int iLowerLimit;
	int iUpperLimit;
	gboolean bCardName;
} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
	nVidiaData pGPUData;
	gboolean bAcquisitionOK;
	int iPreviousGPUTemp;
	CairoDockMeasure *pMeasureTimer;
	Gauge *pGauge;
} AppletData;


#endif
