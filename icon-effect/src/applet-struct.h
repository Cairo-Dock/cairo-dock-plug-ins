/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


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
	CD_ICON_EFFECT_FIREWORK,
	CD_ICON_EFFECT_NB_EFFECTS
	} CDIconEffects;

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
	gboolean bFireLuminance;
	
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
	
	gint iFireworkDuration;
	gboolean bContinueFirework;
	gdouble pFireworkColor[3];
	gboolean bFireworkRandomColors;
	gint iNbFireworkParticles;
	gint iFireworkParticleSize;
	gboolean bFireworkLuminance;
	gint iNbFireworks;
	gboolean bFireworkShoot;
	gdouble fFireworkFriction;
	gdouble fFireworkRadius;
	
	gboolean bBackGround;
	CDIconEffects iEffectsUsed[CD_ICON_EFFECT_NB_EFFECTS];
	CDIconEffects iEffectsOnClick[CAIRO_DOCK_NB_TYPES][CD_ICON_EFFECT_NB_EFFECTS];
	gboolean bRotateEffects;
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iFireTexture;
	GLuint iStarTexture;
	GLuint iSnowTexture;
	GLuint iRainTexture;
	gint iAnimationID[CD_ICON_EFFECT_NB_EFFECTS];
	} ;

typedef struct _CDFirework {
	gdouble x_expl, y_expl;
	gdouble r_expl;
	gdouble v_expl;
	gdouble t_expl;
	gdouble x_sol;
	gdouble vx_decol;
	gdouble vy_decol;
	gdouble xf, yf;
	gdouble r;
	gdouble t;
	CairoParticleSystem *pParticleSystem;
	} CDFirework;
	
typedef struct _CDAnimationData {
	CairoParticleSystem *pFireSystem;
	CairoParticleSystem *pStarSystem;
	CairoParticleSystem *pSnowSystem;
	CairoParticleSystem *pRainSystem;
	CairoParticleSystem *pStormSystem;
	CDFirework *pFireworks;
	gint iNbFireworks;
	gint iNumRound;
	gint iRequestTime;
	} CDIconEffectData;

#endif
