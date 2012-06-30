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

// enumerations of the animations, which also corresponds to the list order in the conf file.
typedef enum {
	CD_ANIMATIONS_BOUNCE=0,
	CD_ANIMATIONS_ROTATE,
	CD_ANIMATIONS_BLINK,
	CD_ANIMATIONS_PULSE,
	CD_ANIMATIONS_WOBBLY,
	CD_ANIMATIONS_WAVE,
	CD_ANIMATIONS_SPOT,
	CD_ANIMATIONS_BUSY,
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
	gchar *cSpotImage;
	gchar *cSpotFrontImage;
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
	
	gint iBusyDuration;
	gboolean bContinueBusy;
	gchar *cBusyImage;
	gdouble fBusySize;
	
	CDAnimationsEffects iEffectsOnMouseOver[CD_ANIMATIONS_NB_EFFECTS];
	CDAnimationsEffects iEffectsOnClick[CAIRO_DOCK_NB_GROUPS][CD_ANIMATIONS_NB_EFFECTS];
	gint iNbRoundsOnClick[CAIRO_DOCK_NB_GROUPS];
	
	gboolean bContinue[CD_ANIMATIONS_NB_EFFECTS];
	} ;


typedef struct _CDAnimationGridNode {
	gdouble x, y;
	gdouble vx, vy;
	gdouble fx, fy;
	gdouble rk[5][4];
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
	gboolean bGrowingSpot;
	
	gboolean bIsWobblying;
	CDAnimationGridNode gridNodes[4][4];
	GLfloat pCtrlPts[4][4][3];
	gint iWobblyCount;
	gdouble fWobblyWidthFactor, fWobblyHeightFactor;
	
	gboolean bIsWaving;
	gdouble fWavePosition;
	gint iNumActiveNodes;
	GLfloat pVertices[4*(CD_WAVE_NB_POINTS+2)];
	GLfloat pCoords[4*(CD_WAVE_NB_POINTS+2)];
	
	gdouble fPulseSpeed;
	gdouble fPulseAlpha;
	
	gint iNumRound;
	
	gboolean bIsBouncing;
	gint iBounceCount;
	gdouble fElevation;
	gdouble fFlattenFactor;
	gdouble fResizeFactor;
	
	gboolean bIsBlinking;
	gint iBlinkCount;
	gdouble fBlinkAlpha;
	
	CairoDockImageBuffer *pBusyImage;
	
	gboolean bIsUnfolding;
	
	gint iReflectShadeCount;
	gboolean bHasBeenPulsed;
	
	GList *pUsedAnimations;  // animations currently running on the icon, ordered by their rendering order.
	} CDAnimationData;

// generic Animation interface
typedef struct _CDAnimation {
	// interface
	void (*init) (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL);
	gboolean (*update) (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double dt, gboolean bUseOpenGL, gboolean bRepeat);  // returns TRUE if the round is still playing, FALSE if it has finished.
	void (*render) (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext);  // pCairoContext is NULL in OpenGL
	void (*post_render) (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext);  // pCairoContext is NULL in OpenGL
	// properties
	const gchar *cName;
	const gchar *cDisplayedName;
	gboolean bDrawIcon;  // whether the animation draws the icon itself (or just alter the drawing context).
	gboolean bDrawReflect;  // TRUE if the animation can draw the reflect itself.
	CDAnimationsEffects id;  // ID of the animation, the same as in the conf file.
	// internal
	guint iRenderingOrder;  // order in which it will be called during the rendering.
	guint iRegisteredId;  // registration ID in the core
	} CDAnimation;

typedef struct _CDCurrentAnimation {
	CDAnimation *pAnimation;
	gboolean bIsPlaying;  // TRUE if currently playing, FALSE if it has already finished.
	} CDCurrentAnimation;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iChromeTexture;
	GLuint iCallList[CD_ANIMATIONS_NB_MESH];
	GLuint iSpotTexture;
	GLuint iHaloTexture;
	GLuint iSpotFrontTexture;
	GLuint iRaysTexture;
	///gint iAnimationID[CD_ANIMATIONS_NB_EFFECTS];
	CairoDockImageBuffer *pBusyImage;
	CDAnimation pAnimations[CD_ANIMATIONS_NB_EFFECTS];  // same order as in the conf file.
	} ;

#endif
