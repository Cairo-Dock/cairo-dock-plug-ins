
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	PENGUIN_DOWN = -1,
	PENGUIN_HORIZONTAL,
	PENGUIN_UP
	} PenguinDirectionType;


typedef struct {
	gchar *cFilePath;
	gint iNbDirections;
	gint iNbFrames;
	gint iSpeed;
	gint iAcceleration;
	gint iTerminalVelocity;
	gboolean bEnding;
	PenguinDirectionType iDirection;
	cairo_surface_t ***pSurfaces;
	int iFrameWidth, iFrameHeight;
	} PenguinAnimation;

typedef struct {
	gchar *cThemePath;
	gint iDelayBetweenChanges;
	gdouble fAlpha;
	gboolean bFree;
	} AppletConfig;


typedef struct {
	gint iCurrentAnimation;
	gint iCurrentPositionX, iCurrentPositionY;
	gint iCurrentSpeed;
	gint iCurrentDirection;
	gint iCurrentFrame;
	gint iCount;
	gdouble fFrameDelay;
	PenguinAnimation defaultAnimation;
	gint iNbAnimations;
	PenguinAnimation *pAnimations;
	gint iNbEndingAnimations;
	gint *pEndingAnimations;
	gint iNbBeginningAnimations;
	gint *pBeginningAnimations;
	gint iNbMovmentAnimations;
	gint *pMovmentAnimations;
	gint iNbGoUpAnimations;
	gint *pGoUpAnimations;
	gint iNbRestAnimations;
	gint *pRestAnimations;
	guint iSidAnimation;
	guint iSidRestartDelayed;
	CairoDockDialog *pDialog;
	} AppletData;

#endif
