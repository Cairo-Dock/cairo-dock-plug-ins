
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gchar *cGPUName;
	gint iVideoRam;
	gchar *cDriverVersion;
	gint iGPUTemp;
} nVidiaData;

typedef enum {
	MY_APPLET_TEMP_NONE = 0,
	MY_APPLET_TEMP_ON_QUICKINFO,
	MY_APPLET_TEMP_ON_NAME,
	MY_APPLET_TEMP_ON_ICON,
	MY_APPLET_TEMP_NUMBER,
} nVidiaTemp;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	nVidiaTemp iDrawTemp;
	gchar *cBrokenUserImage;
	const gchar *cGThemePath;
	gchar *cFilligranImagePath;
	gdouble fAlpha;
	gchar *defaultTitle;
	gchar *cSoundPath;
	gint iCheckInterval;
	gint iLowerLimit;
	gint iUpperLimit;
	gint iAlertLimit;
	gboolean bCardName;
	gboolean bAlert;
	gboolean bAlertSound;
} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	nVidiaData pGPUData;
	gboolean bAcquisitionOK;
	gboolean bAlerted;
	gboolean bSettingsTooOld;
	gint iPreviousGPUTemp;
	CairoDockMeasure *pMeasureTimer;
	CairoDockMeasure *pConfigMeasureTimer;
	Gauge *pGauge;
} ;


#endif
