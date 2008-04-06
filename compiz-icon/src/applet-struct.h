
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

typedef enum {
  COMPIZ_FUSION = 0,
  METACITY,
  KWIN,
  XFCE,
} compizWM;

//\___________ structure containing the applet's configuration parameters.
typedef struct {
	gboolean lBinding;
  gboolean iRendering;
  gboolean selfDecorator;
  gboolean protectDecorator;
  gboolean fSwitch;
  compizWM iWM;
  gchar *cRenderer;
  gchar *sDecoratorCMD;
  //cairo_surface_t *cSurface[3];
} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
  gint iCompizIcon;
  gboolean isCompiz;
  gboolean bNeedRedraw;
  gboolean bAcquisitionOK;
  gint iTimer;
} AppletData;


#endif
