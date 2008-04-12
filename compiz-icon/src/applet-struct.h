
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

typedef enum {
  COMPIZ_FUSION = 0,
  METACITY,
  KWIN,
  XFCE,
} compizWM;

typedef enum {
  COMPIZ_DEFAULT = 0,
  COMPIZ_BROKEN,
  COMPIZ_OTHER,
  COMPIZ_SETTING,
  COMPIZ_EMERALD,
  COMPIZ_RELOAD,
  COMPIZ_NB_ITEMS,
} compizIcons;

typedef enum {
	COMPIZ_NO_ACTION = 0,
	COMPIZ_SWITCH_WM,
	COMPIZ_LAYER,
	COMPIZ_EXPO,
	COMPIZ_SHOW_DESKTOP,
	COMPIZ_NB_ACTIONS
} compizAction;

//\___________ structure containing the applet's configuration parameters.
typedef struct {
  gboolean lBinding;
  gboolean iRendering;
  gboolean selfDecorator;
  
  gboolean protectDecorator;
  gboolean forceConfig;
  gboolean fSwitch;
  compizWM iWM;
  gchar *cRenderer;
  gchar *sDecoratorCMD;
  gchar *cUserImage[COMPIZ_NB_ITEMS];
  compizAction iActionOnMiddleClick;
  //cairo_surface_t *cSurface[3];
} AppletConfig;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
typedef struct {
  gint iCompizIcon;
  gboolean isEmerald;
  gboolean isCompiz;
  gboolean bNeedRedraw;
  gboolean bAcquisitionOK;
  gint iTimer;
} AppletData;


#endif
