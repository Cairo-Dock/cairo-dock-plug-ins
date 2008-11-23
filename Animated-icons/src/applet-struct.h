
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#define CD_ANIMATIONS_SPOT_HEIGHT 12

typedef enum {
	CD_SQUARE_MESH=0,
	CD_CUBE_MESH,
	CD_CAPSULE_MESH,
	CD_ANIMATIONS_NB_MESH
	} CDAnimationsMeshType;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gint iRotationDuration;
	gboolean bContinueRotation;
	CDAnimationsMeshType iMeshType;
	GLfloat pMeshColor[4];
	
	gint iSpotDuration;
	gboolean bContinueSpot;
	GLfloat pSpotColor[3];
	GLfloat pHaloColor[4];
	gdouble pRaysColor1[3];
	gdouble pRaysColor2[3];
	gboolean bMysticalRays;
	gint iNbRaysParticles;
	gint iRaysParticleSize;
	gdouble fRaysParticleSpeed;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iChromeTexture;
	GLuint iCallList[CD_ANIMATIONS_NB_MESH];
	GLuint iSpotTexture;
	GLuint iHaloTexture;
	GLuint iSpotFrontTexture;
	GLuint iRaysTexture;
	} ;

typedef struct _CDAnimationData {
	gdouble fRotationSpeed;
	gdouble fRotationAngle;
	gdouble fRotationBrake;
	gdouble fAdjustFactor;
	
	gdouble fIconOffsetY;
	gdouble fRadiusFactor;
	gdouble fHaloRotationAngle;
	CairoParticleSystem *pRaysSystem;
	} CDAnimationData;

#endif
