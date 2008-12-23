/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-icon-renderer.h"

static float fCapsuleObjectPlaneS[4] = { 0.59f*2, 0., 0., 0. }; // pour un plaquages propre des textures
static float fCapsuleObjectPlaneT[4] = { 0., 0.59f*2, 0., 0. };  // le 2 c'est le 'c'.

void cd_animation_render_capsule (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground)
{
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	if (bInvisibleBackground)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend la capsule transparente.
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);  // la capsule "ecrase" le fond.
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



static float fObjectS[4] = { 0.59f*2, 0., 0., 0. }; // pour un plaquages propre des textures
static float fObjectT[4] = { 0., 0.59f*2, 0., 0. };  // le 2 c'est le 'c'.

void cd_animation_render_cube (Icon *pIcon, CairoDock *pDock, gboolean bInvisibleBackground)
{
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	if (bInvisibleBackground)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend la capsule transparente.
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);  // la capsule "ecrase" le fond.
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
	if (bInvisibleBackground)
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend la capsule transparente.
	else
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);  // la capsule "ecrase" le fond.
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
			cairo_dock_set_icon_scale (pIcon, pDock, fScaleFactor);
			cd_animation_render_square (pIcon, pDock, bInvisibleBackground);
		break;
		case CD_CUBE_MESH :
			glRotatef (fabs (pData->fRotationAngle/4), 1., 0., 0.);
			cairo_dock_set_icon_scale (pIcon, pDock, (1. + pData->fAdjustFactor * (sqrt (2) - 1)) * fScaleFactor);
			cd_animation_render_cube (pIcon, pDock, bInvisibleBackground);
		break;
		case CD_CAPSULE_MESH :
			cairo_dock_set_icon_scale (pIcon, pDock, fScaleFactor);
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
		pIcon->fAlpha = 1. - .3 * pData->fPulseAlpha;
		glColor4f (1., 1., 1., pIcon->fAlpha);
	}
	else
		glColor4fv(myConfig.pMeshColor);  // ici on peut donner une teinte aux reflets chrome.
	
	_draw_rotating_icon (pIcon, pDock, pData, 1.);
	
	if (pData->fPulseAlpha != 0 && myConfig.bPulseSameShape)
	{
		glColor4f (1., 1., 1., pData->fPulseAlpha);
		double fScaleFactor = (1 - myConfig.fPulseZoom) * pData->fPulseAlpha + myConfig.fPulseZoom;
		_draw_rotating_icon (pIcon, pDock, pData, fScaleFactor);
	}
	pIcon->fAlpha = fAlpha;
}

void cd_animations_draw_rotating_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	double fWidthFactor = pData->fWidthFactor;
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
	double fDamageWidthFactor = pData->fWidthFactor;
	pData->fWidthFactor = cos (pData->fRotationAngle/180.*G_PI);
	if (fabs (pData->fWidthFactor) < .01)
		pData->fWidthFactor = .01;
	
	if (! pDock->bIsShrinkingDown && ! pDock->bIsGrowingUp)
	{
		fDamageWidthFactor = MAX (fabs (fDamageWidthFactor), fabs (pData->fWidthFactor));
		pIcon->fWidthFactor *= fDamageWidthFactor;
		
		cairo_dock_redraw_my_icon (pIcon, CAIRO_CONTAINER (pDock));
		
		pIcon->fWidthFactor /= fDamageWidthFactor;
	}
}
