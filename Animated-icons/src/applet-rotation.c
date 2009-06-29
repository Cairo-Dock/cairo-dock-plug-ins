/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-mesh-factory.h"
#include "applet-rotation.h"

static float fCapsuleObjectPlaneS[4] = { 0.59f*2, 0., 0., 0. }; // pour un plaquages propre des textures
static float fCapsuleObjectPlaneT[4] = { 0., 0.59f*2, 0., 0. };  // le 2 c'est le 'c'.

void cd_animations_init_rotation (CDAnimationData *pData, double dt, gboolean bUseOpenGL)
{
	if (bUseOpenGL)
	{
		if (myData.iChromeTexture == 0)
			myData.iChromeTexture = cd_animation_load_chrome_texture ();
		if (myData.iCallList[myConfig.iMeshType] == 0)
			myData.iCallList[myConfig.iMeshType] = cd_animations_load_mesh (myConfig.iMeshType);
	}
	else
		pData->fRotateWidthFactor = 1.;
	pData->fRotationSpeed = 360. / myConfig.iRotationDuration * dt;
	pData->fRotationBrake = 1.;
	pData->fAdjustFactor = 0.;
	pData->bRotationBeginning = TRUE;
}


void cd_animation_render_capsule (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground)
{
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	/*if (bInvisibleBackground)
		_cairo_dock_set_blend_alpha ();  // rend la capsule transparente.
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);  // la capsule "ecrase" le fond.*/
	glEnable(GL_TEXTURE);
	
	glActiveTexture(GL_TEXTURE0); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iChromeTexture);
	glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glActiveTexture(GL_TEXTURE1); // Go pour le texturing 2eme passe
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	//glColor4f(1., 1., 1., pIcon->fAlpha);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // la je veux un mapping tout ce qu'il y a de plus classique
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexGenfv(GL_S, GL_OBJECT_PLANE, fCapsuleObjectPlaneS); // Je decale un peu la texture
	glTexGenfv(GL_T, GL_OBJECT_PLANE, fCapsuleObjectPlaneT);
	glEnable(GL_TEXTURE_GEN_S);                                // generation texture en S
	glEnable(GL_TEXTURE_GEN_T);        // et en T
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Le mode de combinaison des textures
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);  /// ca sature ...
	//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	//glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.3);  // booster la combinaison.
	
	glPolygonMode (GL_FRONT, GL_FILL);
	glCallList (myData.iCallList[CD_CAPSULE_MESH]);
	
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
}


void cd_animation_render_cube (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground)
{
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	/*if (bInvisibleBackground)
		_cairo_dock_set_blend_alpha ();  // rend la capsule transparente.
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);  // la capsule "ecrase" le fond.
	_cairo_dock_set_blend_alpha ();*/
	glEnable(GL_TEXTURE);
	
	glActiveTexture(GL_TEXTURE0); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iChromeTexture);
	glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glActiveTexture(GL_TEXTURE1); // Go pour le texturing 2eme passe
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	glColor4f(1., 1., 1., pIcon->fAlpha);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Le mode de combinaison des textures
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);  /// ca sature ...
	//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	//glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.3);  // booster la combinaison.
	
	glPolygonMode (GL_FRONT, GL_FILL);
	glCallList (myData.iCallList[CD_CUBE_MESH]);
	
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
}


void cd_animation_render_square (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground)
{
	glEnable (GL_BLEND);
	/*if (bInvisibleBackground)
		_cairo_dock_set_blend_alpha ();  // rend la capsule transparente.
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);  // la capsule "ecrase" le fond.*/
	glEnable(GL_TEXTURE);
	
	glActiveTexture(GL_TEXTURE0); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iChromeTexture);
	glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glActiveTexture(GL_TEXTURE1); // Go pour le texturing 2eme passe
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	//glColor4f(1., 1., 1., pIcon->fAlpha);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Le mode de combinaison des textures
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	//glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.2);  // booster la combinaison.
	
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glCallList (myData.iCallList[CD_SQUARE_MESH]);
	
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable (GL_BLEND);
}


static void _draw_rotating_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, double fScaleFactor)
{
	gboolean bInvisibleBackground = TRUE;
	glPushMatrix ();
	glRotatef (pData->fRotationAngle, 0., 1., 0.);
	switch (myConfig.iMeshType)
	{
		case CD_SQUARE_MESH :
		default :
			cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), fScaleFactor);
			cd_animation_render_square (pIcon, pDock, bInvisibleBackground);
		break;
		case CD_CUBE_MESH :
			glRotatef (fabs (pData->fRotationAngle/4), 1., 0., 0.);
			cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), (1. + pData->fAdjustFactor * (sqrt (2) - 1)) * fScaleFactor);
			cd_animation_render_cube (pIcon, pDock, bInvisibleBackground);
		break;
		case CD_CAPSULE_MESH :
			cairo_dock_set_icon_scale (pIcon, CAIRO_CONTAINER (pDock), fScaleFactor);
			cd_animation_render_capsule (pIcon, pDock, bInvisibleBackground);
		break;
	}
	glPopMatrix ();
}
void cd_animations_draw_rotating_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	double fAlpha = pIcon->fAlpha;
	if (pData->fPulseAlpha != 0 && myConfig.bPulseSameShape)
	{
		_cairo_dock_set_alpha (pIcon->fAlpha * (1. - .5 * pData->fPulseAlpha));
		///pIcon->fAlpha *= 1. - .5 * pData->fPulseAlpha;
	}
	else
		glColor4f(myConfig.pMeshColor[0], myConfig.pMeshColor[1], myConfig.pMeshColor[2], pIcon->fAlpha);  // ici on peut donner une teinte aux reflets chrome.
	if (myConfig.pMeshColor[3] == 1)
		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	else
		//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_cairo_dock_set_blend_alpha ();
	_draw_rotating_icon (pIcon, pDock, pData, 1.);
	
	if (pData->fPulseAlpha != 0 && myConfig.bPulseSameShape)
	{
		_cairo_dock_set_alpha (pData->fPulseAlpha);
		double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
		glTranslatef (0., 0., -fScaleFactor * pIcon->fHeight * pIcon->fScale/2);
		_cairo_dock_set_blend_alpha ();
		_draw_rotating_icon (pIcon, pDock, pData, fScaleFactor);
		glTranslatef (0., 0., fScaleFactor * pIcon->fHeight * pIcon->fScale/2);
	}
	
	if (pDock->bUseReflect)
	{
		glPushMatrix ();
		_cairo_dock_set_alpha (myIcons.fAlbedo * sqrt (myIcons.fAlbedo) * pIcon->fAlpha);  // transparence du reflet, arrange pour essayer de cacher l'absence de degrade :p
		double fOffsetY = pIcon->fHeight * pIcon->fScale + (0 + pIcon->fDeltaYReflection) * pDock->fRatio;
		if (pDock->bHorizontalDock)
		{
			if (pDock->bDirectionUp)
			{
				fOffsetY = pIcon->fHeight * pIcon->fScale + pIcon->fDeltaYReflection;
				glTranslatef (0., - fOffsetY, 0.);
				//glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, - pIcon->fHeight * pIcon->fScale, 1.);  // taille du reflet et on se retourne.
			}
			else
			{
				glTranslatef (0., fOffsetY, 0.);
				//glScalef (pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, myIcons.fReflectSize * pDock->fRatio, 1.);
			}
			glScalef (1., -1., 1.);
		}
		else
		{
			if (pDock->bDirectionUp)
			{
				glTranslatef (fOffsetY, 0., 0.);
				//glScalef (- myIcons.fReflectSize * pDock->fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
			}
			else
			{
				glTranslatef (- fOffsetY, 0., 0.);
				//glScalef (myIcons.fReflectSize * pDock->fRatio, pIcon->fWidth * pIcon->fWidthFactor * pIcon->fScale, 1.);
			}
			glScalef (-1., 1., 1.);
		}
		
		_cairo_dock_set_blend_alpha ();
		_draw_rotating_icon (pIcon, pDock, pData, 1.);
		glPopMatrix ();
	}
	pIcon->fAlpha = fAlpha;
}

void cd_animations_draw_rotating_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	double fWidthFactor = pData->fRotateWidthFactor;
	pIcon->fWidthFactor *= fWidthFactor;
	cairo_save (pCairoContext);
	
	if (pDock->bHorizontalDock)
		cairo_translate (pCairoContext,
			pIcon->fWidth * pIcon->fScale * (1 - fWidthFactor) / 2,
			1.);
	else
		cairo_translate (pCairoContext,
			1.,
			pIcon->fWidth * pIcon->fScale * (1 - fWidthFactor) / 2);
	
	cairo_dock_draw_icon_cairo (pIcon, pDock, pCairoContext);
	
	cairo_restore (pCairoContext);
	
	pIcon->fWidthFactor /= fWidthFactor;
}


void cd_animations_update_rotating_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	double fDamageWidthFactor = pData->fRotateWidthFactor;
	pData->fRotateWidthFactor = cos (pData->fRotationAngle/180.*G_PI);
	if (fabs (pData->fRotateWidthFactor) < .01)
		pData->fRotateWidthFactor = .01;
	
	if (! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
	{
		fDamageWidthFactor = MAX (fabs (fDamageWidthFactor), fabs (pData->fRotateWidthFactor));
		pIcon->fWidthFactor *= fDamageWidthFactor;
		
		cairo_dock_redraw_my_icon (pIcon, CAIRO_CONTAINER (pDock));
		
		pIcon->fWidthFactor /= fDamageWidthFactor;
	}
}
