
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

typedef struct {
	gint iDuration;
	gboolean bContinue;
	gdouble pColor1[3];
	gdouble pColor2[3];
	gboolean bMystical;
	gint iNbParticles;
	gint iParticleSize;
	gdouble fParticleSpeed;
} CDIconEffectParticleSystemParameters;

typedef enum {
	CD_ICON_EFFECT_FIRE=0,
	CD_ICON_EFFECT_STARS,
	CD_ICON_EFFECT_RAIN,
	CD_ICON_EFFECT_SNOW,
	CD_ICON_EFFECT_SAND,
	CD_ICON_EFFECT_NB_EFFECTS
	} CDIconEffectType;

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gint iFireDuration;
	gboolean bContinueFire;
	gdouble pFireColor1[3];
	gdouble pFireColor2[3];
	gboolean bMysticalFire;
	gint iNbFireParticles;
	gint iFireParticleSize;
	gdouble fFireParticleSpeed;
	
	gint iStarDuration;
	gboolean bContinueStar;
	gdouble pStarColor1[3];
	gdouble pStarColor2[3];
	gboolean bMysticalStars;
	gint iNbStarParticles;
	gint iStarParticleSize;
	
	gint iSnowDuration;
	gboolean bContinueSnow;
	gdouble pSnowColor1[3];
	gdouble pSnowColor2[3];
	gint iNbSnowParticles;
	gint iSnowParticleSize;
	gdouble fSnowParticleSpeed;
	
	gint iRainDuration;
	gboolean bContinueRain;
	gdouble pRainColor1[3];
	gdouble pRainColor2[3];
	gint iNbRainParticles;
	gint iRainParticleSize;
	gdouble fRainParticleSpeed;
	
	gint iStormDuration;
	gboolean bContinueStorm;
	gdouble pStormColor1[3];
	gdouble pStormColor2[3];
	gint iNbStormParticles;
	gint iStormParticleSize;
	
	gboolean bBackGround;
	gint iEffectsUsed[CD_ICON_EFFECT_NB_EFFECTS];
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iFireTexture;
	GLuint iStarTexture;
	GLuint iSnowTexture;
	GLuint iRainTexture;
	} ;

typedef struct _CDAnimationData {
	CairoParticleSystem *pFireSystem;
	CairoParticleSystem *pStarSystem;
	CairoParticleSystem *pSnowSystem;
	CairoParticleSystem *pRainSystem;
	CairoParticleSystem *pStormSystem;
	} CDIconEffectData;

#endif
