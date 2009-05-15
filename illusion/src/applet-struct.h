
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef enum _CDIllusionEffect {
	CD_ILLUSION_EVAPORATE=0,
	CD_ILLUSION_FADE_OUT,
	CD_ILLUSION_EXPLODE,
	CD_ILLUSION_BREAK,
	CD_ILLUSION_BLACK_HOLE,
	CD_ILLUSION_NB_EFFECTS
	} CDIllusionEffect;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	CDIllusionEffect iDisappearanceEffect;
	CDIllusionEffect iAppearanceEffect;
	
	gint iEvaporateDuration;
	gint iEvaporateNbParticles;
	gdouble pEvaporateColor1[3];
	gdouble pEvaporateColor2[3];
	gboolean bMysticalEvaporate;
	gint iNbEvaporateParticles;
	gint iEvaporateParticleSize;
	gdouble fEvaporateParticleSpeed;
	gboolean bEvaporateFromBottom;
	
	gint iFadeOutDuration;
	
	gint iExplodeDuration;
	gint iExplodeNbPiecesX, iExplodeNbPiecesY;
	gdouble fExplosionRadius;
	gboolean bExplodeCube;
	
	gint iBreakDuration;
	gint iBreakNbBorderPoints;
	
	gint iBlackHoleDuration;
	gdouble fBlackHoleRotationSpeed;
	gint iAttraction;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iEvaporateTexture;
	} ;

typedef struct {
	gdouble fRotationSpeed;
	gdouble vx, vy, vz;
	} CDIllusionExplosion;

typedef struct {
	gdouble pCoords[4*2];
	gint iNbPts;
	gdouble fCrackAngle;
	gdouble fRotationAngle;
	gdouble yinf;
	} CDIllusionBreak;

typedef struct {
	gdouble u, v;
	gdouble fTheta0, r0;
	gdouble fTheta;
	gdouble x, y;
	} CDIllusionBlackHole;


typedef struct _CDIllusionData {
	CDIllusionEffect iCurrentEffect;
	gint iEffectDuration;
	gdouble fTimeLimitPercent;
	gdouble fDeltaT;
	gint sens;
	gdouble fTime;
	
	gdouble fEvaporatePercent;
	CairoParticleSystem *pEvaporateSystem;
	
	gdouble fFadeOutAlpha;
	
	gdouble fExplosionRadius;
	gdouble fExplosionRotation;
	gdouble fExplodeAlpha;
	CDIllusionExplosion *pExplosionPart;
	
	CDIllusionBreak *pBreakPart;
	gint iNbBreakParts;
	gdouble dh;
	
	CDIllusionBlackHole *pBlackHolePoints;
	GLfloat *pBlackHoleCoords, *pBlackHoleVertices;
	} CDIllusionData;

#endif
