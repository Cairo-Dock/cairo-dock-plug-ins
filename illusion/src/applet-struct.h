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

typedef enum _CDIllusionEffect {
	CD_ILLUSION_EVAPORATE=0,
	CD_ILLUSION_FADE_OUT,
	CD_ILLUSION_EXPLODE,
	CD_ILLUSION_BREAK,
	CD_ILLUSION_BLACK_HOLE,
	CD_ILLUSION_NB_EFFECTS,
	CD_ILLUSION_LIGHTNING
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
	
	gint iLightningDuration;
	gint iLightningNbSources;
	gint iLightningNbCtrlPts;
	gdouble fLightningColor1[4];
	gdouble fLightningColor2[4];
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	GLuint iEvaporateTexture;
	GLuint iLightningTexture;
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

typedef struct {
	GLfloat *pVertexTab;
	gint iNbCurrentVertex;
	} CDIllusionLightning;


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
	
	CDIllusionLightning *pLightnings;
	gint iNbVertex;
	gint iNbSources;
	gdouble fLightningAlpha;
	} CDIllusionData;

#endif
