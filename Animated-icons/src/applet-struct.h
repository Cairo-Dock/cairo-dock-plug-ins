
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

typedef enum {
	CD_HORIZONTAL_STRECTH=0,
	CD_VERTICAL_STRECTH,
	CD_CORNER_STRECTH,
	CD_ANIMATIONS_NB_STRECTH
	} CDAnimationsStretchType;

typedef enum {
	CD_ANIMATIONS_BOUNCE=0,
	CD_ANIMATIONS_ROTATE,
	CD_ANIMATIONS_BLINK,
	CD_ANIMATIONS_PULSE,
	CD_ANIMATIONS_WOBBLY,
	CD_ANIMATIONS_WAVE,
	CD_ANIMATIONS_SPOT,
	CD_ANIMATIONS_NB_EFFECTS
	} CDAnimationsEffects;

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
	
	gboolean bContinueWobbly;
	gint iNbGridNodes;
	CDAnimationsStretchType iInitialStrecth;
	gdouble fSpringConstant;
	gdouble fFriction;
	
	gint iWaveDuration;
	gboolean bContinueWave;
	gdouble fWaveWidth;
	gdouble fWaveAmplitude;
	
	gint iPulseDuration;
	gboolean bContinuePulse;
	gdouble fPulseZoom;
	gboolean bPulseSameShape;
	
	gint iBounceDuration;
	gboolean bContinueBounce;
	gdouble fBounceResize;
	gdouble fBounceFlatten;
	
	gint iBlinkDuration;
	gboolean bContinueBlink;
	
	CDAnimationsEffects iEffectsOnMouseOver[CD_ANIMATIONS_NB_EFFECTS];
	CDAnimationsEffects iEffectsOnClick[CAIRO_DOCK_NB_TYPES][CD_ANIMATIONS_NB_EFFECTS];
	gint iNbRoundsOnClick[CAIRO_DOCK_NB_TYPES];
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iChromeTexture;
	GLuint iCallList[CD_ANIMATIONS_NB_MESH];
	GLuint iSpotTexture;
	GLuint iHaloTexture;
	GLuint iSpotFrontTexture;
	GLuint iRaysTexture;
	gint iAnimationID[CD_ANIMATIONS_NB_EFFECTS];
	} ;

typedef struct _CDAnimationGridNode {
	gdouble x, y;
	gdouble vx, vy;
	gdouble fx, fy;
	} CDAnimationGridNode;

#define CD_WAVE_NB_POINTS 9

typedef struct _CDAnimationData {
	gdouble fRotationSpeed;
	gdouble fRotationAngle;
	gdouble fRotationBrake;
	gdouble fAdjustFactor;
	gboolean bRotationBeginning;
	gdouble fRotateWidthFactor;
	
	gdouble fIconOffsetY;
	gdouble fRadiusFactor;
	gdouble fHaloRotationAngle;
	CairoParticleSystem *pRaysSystem;
	
	gboolean bIsWobblying;
	CDAnimationGridNode gridNodes[4][4];
	GLfloat pCtrlPts[4][4][3];
	
	gboolean bIsWaving;
	gdouble fWavePosition;
	gint iNumActiveNodes;
	GLfloat pVertices[3*(2*CD_WAVE_NB_POINTS+2)];
	GLfloat pCoords[2*(2*CD_WAVE_NB_POINTS+2)];
	GLfloat pColors[4*(2*CD_WAVE_NB_POINTS+2)];
	
	gdouble fPulseSpeed;
	gdouble fPulseAlpha;
	
	gint iNumRound;
	
	gint iWobblyCount;
	gdouble fWobblyWidthFactor, fWobblyHeightFactor;
	
	gboolean bIsBouncing;
	gint iBounceCount;
	gdouble fElevation;
	gdouble fFlattenFactor;
	gdouble fResizeFactor;
	
	gboolean bIsBlinking;
	gint iBlinkCount;
	gdouble fBlinkAlpha;
	
	} CDAnimationData;

#endif
