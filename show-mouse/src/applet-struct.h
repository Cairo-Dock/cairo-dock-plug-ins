
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum {
	CD_SHOW_MOUSE_ON_DOCK=1,
	CD_SHOW_MOUSE_ON_DESKLET=2,
	CD_SHOW_MOUSE_ON_BOTH=3
	} CDShowMouseType;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gdouble fRotationSpeed;
	gint iParticleLifeTime;
	gint iNbParticles;
	gint iParticleSize;
	gdouble pColor1[3];
	gdouble pColor2[3];
	gboolean bMysticalFire;
	gint iNbSources;
	gdouble fScattering;
	CDShowMouseType iContainerType;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iTexture;
	CDShowMouseType iContainerType;
	} ;

typedef struct _CDShowMouseData {
	CairoParticleSystem *pSystem;
	gdouble fRotationAngle;
	gdouble fAlpha;
	gdouble *pSourceCoords;
	gdouble fRadius;
	} CDShowMouseData;

#endif
