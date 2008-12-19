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
	glColor4fv(myConfig.pMeshColor);  // ici on peut donner une teinte aux reflets chrome.
	glEnable(GL_TEXTURE);
	
	glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iChromeTexture);
	glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
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
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glActiveTextureARB(GL_TEXTURE0_ARB);
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
	glColor4fv(myConfig.pMeshColor);  // ici on peut donner une teinte aux reflets chrome.
	glEnable(GL_TEXTURE);
	
	glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iChromeTexture);
	glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Le mode de combinaison des textures
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);  /// ca sature ...
	//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	//glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.3);  // booster la combinaison.
	
	glPolygonMode (GL_FRONT, GL_FILL);
	glCallList (myData.iCallList[CD_CUBE_MESH]);
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glActiveTextureARB(GL_TEXTURE0_ARB);
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
	glColor4fv(myConfig.pMeshColor);  // ici on peut donner une teinte aux reflets chrome.
	glEnable(GL_TEXTURE);
	
	glActiveTextureARB(GL_TEXTURE0_ARB); // Go pour le multitexturing 1ere passe
	glEnable(GL_TEXTURE_2D); // On active le texturing sur cette passe
	glBindTexture(GL_TEXTURE_2D, myData.iChromeTexture);
	glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glActiveTextureARB(GL_TEXTURE1_ARB); // Go pour le texturing 2eme passe
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, pIcon->iIconTexture);
	
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT); // Le mode de combinaison des textures
	glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
	//glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_MODULATE);
	//glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.2);  // booster la combinaison.
	
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glCallList (myData.iCallList[CD_SQUARE_MESH]);
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable (GL_BLEND);
}


void cd_animations_draw_rotating_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData)
{
	gboolean bInvisibleBackground = TRUE;
	glPushMatrix ();
	glRotatef (pData->fRotationAngle, 0., 1., 0.);
	switch (myConfig.iMeshType)
	{
		case CD_SQUARE_MESH :
		default :
			cairo_dock_set_icon_scale (pIcon, pDock, 1.);
			cd_animation_render_square (pIcon, pDock, bInvisibleBackground);
		break;
		case CD_CUBE_MESH :
			glRotatef (fabs (pData->fRotationAngle/4), 1., 0., 0.);
			cairo_dock_set_icon_scale (pIcon, pDock, 1. + pData->fAdjustFactor * (sqrt (2) - 1));
			cd_animation_render_cube (pIcon, pDock, bInvisibleBackground);
		break;
		case CD_CAPSULE_MESH :
			cairo_dock_set_icon_scale (pIcon, pDock, 1.);
			cd_animation_render_capsule (pIcon, pDock, bInvisibleBackground);
		break;
	}
	glPopMatrix ();
}

void cd_animations_draw_rotating_cairo (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	cd_animations_draw_cairo_icon (pIcon, pDock, pData, pCairoContext);
}



void cd_animations_draw_cairo_icon (Icon *pIcon, CairoDock *pDock, CDAnimationData *pData, cairo_t *pCairoContext)
{
	if (pIcon->pIconBuffer == NULL)
		return ;
	double fRatio = pDock->fRatio;
	double fPreviousAlpha = pIcon->fAlpha;
	
	if (pDock->bUseReflect && pIcon->pReflectionBuffer != NULL)  // on dessine les reflets.
	{
		cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
		if (pIcon->fAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, pIcon->fAlpha);

		cairo_restore (pCairoContext);  // retour juste apres la translation (fDrawX, fDrawY).

		cairo_save (pCairoContext);
		if (pDock->bHorizontalDock)
		{
			cairo_translate (pCairoContext, 0, - pIcon->fDeltaYReflection + (pDock->bDirectionUp ? pIcon->fHeight * pIcon->fScale : - myIcons.fReflectSize * pIcon->fScale));
			cairo_scale (pCairoContext, fRatio * pData->fWidthFactor * pIcon->fScale / (1 + myIcons.fAmplitude), fRatio * pData->fHeightFactor * pIcon->fScale / (1 + myIcons.fAmplitude));
		}
		else
		{
			cairo_translate (pCairoContext, - pIcon->fDeltaYReflection + (pDock->bDirectionUp ? pIcon->fHeight * pIcon->fScale : - myIcons.fReflectSize * pIcon->fScale), 0);
			cairo_scale (pCairoContext, fRatio * pData->fHeightFactor * pIcon->fScale / (1 + myIcons.fAmplitude), fRatio * pData->fWidthFactor * pIcon->fScale / (1 + myIcons.fAmplitude));
		}
		
		cairo_set_source_surface (pCairoContext, pIcon->pReflectionBuffer, 0.0, 0.0);
		
		if (mySystem.bDynamicReflection && pIcon->fScale > 1)
		{
			cairo_pattern_t *pGradationPattern;
			if (pDock->bHorizontalDock)
			{
				pGradationPattern = cairo_pattern_create_linear (0.,
					(pDock->bDirectionUp ? 0. : myIcons.fReflectSize / fRatio * (1 + myIcons.fAmplitude)),
					0.,
					(pDock->bDirectionUp ? myIcons.fReflectSize / fRatio * (1 + myIcons.fAmplitude) / pIcon->fScale : myIcons.fReflectSize / fRatio * (1 + myIcons.fAmplitude) * (1. - 1./ pIcon->fScale)));  // de haut en bas.
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					1.);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					1 - (pIcon->fScale - 1) / myIcons.fAmplitude);  // astuce pour ne pas avoir a re-creer la surface de la reflection.
			}
			else
			{
				pGradationPattern = cairo_pattern_create_linear ((pDock->bDirectionUp ? 0. : myIcons.fReflectSize / fRatio * (1 + myIcons.fAmplitude)),
					0.,
					(pDock->bDirectionUp ? myIcons.fReflectSize / fRatio * (1 + myIcons.fAmplitude) / pIcon->fScale : myIcons.fReflectSize / fRatio * (1 + myIcons.fAmplitude) * (1. - 1./ pIcon->fScale)),
					0.);
				g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
				
				cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					0.,
					0.,
					0.,
					0.,
					1.);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					1.,
					0.,
					0.,
					0.,
					1. - (pIcon->fScale - 1) / myIcons.fAmplitude);  // astuce pour ne pas avoir a re-creer la surface de la reflection.
			}
			cairo_save (pCairoContext);
			cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
			cairo_translate (pCairoContext, 0, 0);
			cairo_mask (pCairoContext, pGradationPattern);
			cairo_restore (pCairoContext);

			cairo_pattern_destroy (pGradationPattern);
		}
		else
		{
			if (pIcon->fAlpha == 1)
				cairo_paint (pCairoContext);
			else
				cairo_paint_with_alpha (pCairoContext, pIcon->fAlpha);
		}
	}
	else  // on dessine l'icone tout simplement.
	{
		if (pIcon->pIconBuffer != NULL)
			cairo_set_source_surface (pCairoContext, pIcon->pIconBuffer, 0.0, 0.0);
		if (pIcon->fAlpha == 1)
			cairo_paint (pCairoContext);
		else
			cairo_paint_with_alpha (pCairoContext, pIcon->fAlpha);
	}
}
