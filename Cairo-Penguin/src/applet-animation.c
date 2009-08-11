/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-theme.h"
#include "applet-animation.h"


void penguin_move_in_dock (CairoDockModuleInstance *myApplet)
{
	static GdkRectangle area;
	if (! cairo_dock_animation_will_be_visible (myDock))
		return ;
	
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	g_return_if_fail (pAnimation != NULL);
	int iPreviousPositionX = myData.iCurrentPositionX, iPreviousPositionY = myData.iCurrentPositionY;
	
	Icon *pFirstDrawnIcon = NULL;
	if (myDock->pFirstDrawnElement != NULL)
		pFirstDrawnIcon = myDock->pFirstDrawnElement->data;
	if (pFirstDrawnIcon == NULL && myDock->icons != NULL)
		pFirstDrawnIcon = myDock->icons->data;
	int iXMin = (pFirstDrawnIcon != NULL ? pFirstDrawnIcon->fXAtRest : 0);
	int iXMax = iXMin + myDock->fFlatDockWidth;
	int iHeight = myDock->iCurrentHeight;
	
	penguin_calculate_new_position (myApplet, pAnimation, iXMin, iXMax, iHeight);
	
	if (myDock->bHorizontalDock)
	{
		area.x = (int) ((myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + MIN (iPreviousPositionX, myData.iCurrentPositionX));
		area.y = myDock->iCurrentHeight - MAX (iPreviousPositionY, myData.iCurrentPositionY) - pAnimation->iFrameHeight;
		area.width = abs (iPreviousPositionX - myData.iCurrentPositionX) + pAnimation->iFrameWidth;
		area.height = abs (iPreviousPositionY - myData.iCurrentPositionY) + pAnimation->iFrameHeight;
	}
	else
	{
		area.y = (int) ((myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + MIN (iPreviousPositionX, myData.iCurrentPositionX));
		if (myDock->bDirectionUp)
		{
			area.x = myDock->iCurrentHeight - MAX (iPreviousPositionY, myData.iCurrentPositionY) - pAnimation->iFrameHeight;
			area.y = myDock->iCurrentWidth - area.y;
		}
		else
		{
			area.x = MAX (iPreviousPositionY, myData.iCurrentPositionY);
		}
		area.height = abs (iPreviousPositionX - myData.iCurrentPositionX) + pAnimation->iFrameWidth;
		area.width = abs (iPreviousPositionY - myData.iCurrentPositionY) + pAnimation->iFrameHeight;
	}
	cairo_dock_redraw_container_area (myContainer, &area);
	
	penguin_advance_to_next_frame (myApplet, pAnimation);
}

static void _penguin_draw_texture (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation, double fOffsetX, double fOffsetY, double fScale)
{
	g_return_if_fail (pAnimation->iTexture != 0);
	int iIconWidth, iIconHeight;
	cairo_dock_get_icon_extent (myIcon, myContainer, &iIconWidth, &iIconHeight);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	glBindTexture (GL_TEXTURE_2D, pAnimation->iTexture);
	_cairo_dock_apply_current_texture_portion_at_size_with_offset (1.*myData.iCurrentFrame/pAnimation->iNbFrames, 
		.5*myData.iCurrentDirection, 1./pAnimation->iNbFrames, 1./pAnimation->iNbDirections,
		pAnimation->iFrameWidth*fScale, pAnimation->iFrameHeight*fScale,
		fOffsetX + myData.iCurrentPositionX, fOffsetY + myData.iCurrentPositionY + pAnimation->iFrameHeight*fScale/2);
	_cairo_dock_disable_texture ();
}
void penguin_draw_on_dock_opengl (CairoDockModuleInstance *myApplet, CairoContainer *pContainer)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL)
		return ;
	
	glPushMatrix ();
	glLoadIdentity ();
	
	if (! myDock->bHorizontalDock)
	{
		glTranslatef (myDock->iCurrentHeight/2, myDock->iCurrentWidth/2, 0.);
		glRotatef (myDock->bDirectionUp ? 90. : -90., 0., 0., 1.);
		glTranslatef (- myDock->iCurrentWidth/2, - myDock->iCurrentHeight/2, 0.);
	}
	_penguin_draw_texture (myApplet, pAnimation, (myDock->iCurrentWidth - myDock->fFlatDockWidth) * .5, 0., 1.);
	
	glPopMatrix ();
}

void penguin_draw_on_dock (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL)
		return ;
	
	g_return_if_fail (pAnimation->pSurfaces != NULL);
	cairo_surface_t *pSurface = pAnimation->pSurfaces[myData.iCurrentDirection][myData.iCurrentFrame];
	
	cairo_save (pCairoContext);
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	
	if (myDock->bHorizontalDock)
	{
		cairo_translate (pCairoContext, floor ((myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX), myDock->iCurrentHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight);
		cairo_set_source_surface (pCairoContext, pSurface, 0.0, 0.0);
		cairo_paint (pCairoContext);
	}
	else
	{
		if (myDock->bDirectionUp)
			cairo_translate (pCairoContext,
				myDock->iCurrentHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight,
				myDock->iCurrentWidth - (floor ((myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX)));
		else
			cairo_translate (pCairoContext,
				myData.iCurrentPositionY,
				floor ((myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX));
		cairo_dock_draw_surface (pCairoContext, pSurface, pAnimation->iFrameWidth, pAnimation->iFrameHeight, myDock->bDirectionUp, myDock->bHorizontalDock, 1.);
	}
	
	cairo_restore (pCairoContext);
}
gboolean penguin_render_on_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext)
{
	if (pContainer != myContainer)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	if (! cairo_dock_animation_will_be_visible (myDock))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pCairoContext != NULL)
		penguin_draw_on_dock (myApplet, pContainer, pCairoContext);
	else
		penguin_draw_on_dock_opengl (myApplet, pContainer);
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}



void penguin_move_in_icon (CairoDockModuleInstance *myApplet)
{
	if (! cairo_dock_animation_will_be_visible (myDock))
		return ;
	
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	g_return_if_fail (pAnimation != NULL);
	
	double fScale = (pAnimation->iNbFrames > 1 || pAnimation->iSpeed != 0 || pAnimation->iAcceleration != 0 ? myIcon->fScale : 1.);  // s'il est a l'arret on le met a la taille de l'icone au repos.
	int iWidth = myIcon->fWidth / myDock->fRatio * fScale;
	int iHeight = myIcon->fHeight / myDock->fRatio * fScale;
	int iXMin = - iWidth / 2;
	int iXMax = - iXMin;
	
	penguin_calculate_new_position (myApplet, pAnimation, iXMin, iXMax, iHeight);
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
			return ;
		
		int iIconWidth, iIconHeight;
		cairo_dock_get_icon_extent (myIcon, myContainer, &iIconWidth, &iIconHeight);
		
		g_return_if_fail (pAnimation->iTexture != 0);
		double f = (1 + g_fAmplitude) / fScale;
		double x, y;  // centre du pingouin, en coordonnées absolues.
		x = myData.iCurrentPositionX - iXMin - iIconWidth/2 + pAnimation->iFrameWidth/2*f;
		y = myData.iCurrentPositionY + pAnimation->iFrameHeight/2*f;
		
		_cairo_dock_enable_texture ();
		_cairo_dock_set_blend_alpha ();
		_cairo_dock_set_alpha (1.);
		
		glBindTexture (GL_TEXTURE_2D, pAnimation->iTexture);
		_cairo_dock_apply_current_texture_portion_at_size_with_offset (1.*myData.iCurrentFrame/pAnimation->iNbFrames, 
			.5*myData.iCurrentDirection, 1./pAnimation->iNbFrames, 1./pAnimation->iNbDirections,
			pAnimation->iFrameWidth*f, pAnimation->iFrameHeight*f,
			x, - iIconHeight/2 + y);
		_cairo_dock_disable_texture ();
		
		cairo_dock_end_draw_icon (myIcon, myContainer);
	}
	else
	{
		g_return_if_fail (pAnimation->pSurfaces != NULL);
		cairo_surface_t *pSurface = pAnimation->pSurfaces[myData.iCurrentDirection][myData.iCurrentFrame];
		g_return_if_fail (pSurface != NULL);
		
		//\________________ On efface l'ancienne image.
		cairo_set_source_rgba (myDrawContext, 0.0, 0.0, 0.0, 0.0);
		cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
		cairo_paint (myDrawContext);
		cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
		
		//\________________ On applique la nouvelle image.
		if (pSurface != NULL)
		{
			cairo_save (myDrawContext);
			cairo_scale (myDrawContext, (1 + g_fAmplitude) / fScale, (1 + g_fAmplitude) / fScale);
			cairo_set_source_surface (
				myDrawContext,
				pSurface,
				iXMax + myData.iCurrentPositionX,
				iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight);
			cairo_paint (myDrawContext);
			cairo_restore (myDrawContext);
		}
		
		//\________________ les reflets.
		CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	}
	
	CD_APPLET_REDRAW_MY_ICON;
	
	penguin_advance_to_next_frame (myApplet, pAnimation);
}



void penguin_calculate_new_position (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation, int iXMin, int iXMax, int iHeight)
{
	//\________________ On calule la nouvelle vitesse.
	if (pAnimation->iAcceleration != 0 && myData.iCurrentSpeed != pAnimation->iTerminalVelocity)
	{
		myData.iCurrentSpeed += pAnimation->iAcceleration;
		if ( (pAnimation->iAcceleration > 0 && myData.iCurrentSpeed > pAnimation->iTerminalVelocity) || (pAnimation->iAcceleration < 0 && myData.iCurrentSpeed < pAnimation->iTerminalVelocity))
			myData.iCurrentSpeed = pAnimation->iTerminalVelocity;
	}
	
	//\________________ On calule la nouvelle position.
	int sens;
	if (pAnimation->iDirection == PENGUIN_HORIZONTAL)
	{
		sens = (myData.iCurrentDirection == 0 ? -1 : 1);
		myData.iCurrentPositionX += sens * myData.iCurrentSpeed;
	}
	else
	{
		sens = (pAnimation->iDirection == PENGUIN_UP ? 1 : -1);
		myData.iCurrentPositionY += sens * myData.iCurrentSpeed;
	}
	
	//\________________ On tient compte des contraintes.
	if (myData.iCurrentPositionX < iXMin || myData.iCurrentPositionX + pAnimation->iFrameWidth > iXMax)
	{
		if (myData.iCurrentPositionX < iXMin)
			myData.iCurrentPositionX = iXMin;
		else
			myData.iCurrentPositionX = iXMax - pAnimation->iFrameWidth;
		if (pAnimation->iDirection == PENGUIN_HORIZONTAL && myConfig.bFree)  // dans l'icone on continue l'animation.
		{
			if (pAnimation->iNbDirections == 2) // on peut repartir dans l'autre sens ou remonter.
			{
				int iRandom = g_random_int_range (0, 3);
				if (iRandom != 0)  // 2 chance sur 3.
				{
					myData.iCurrentDirection = 1 - myData.iCurrentDirection;
					cd_debug ("myData.iCurrentDirection <- %d", myData.iCurrentDirection);
				}
				else
				{
					int iNewAnimation = penguin_choose_go_up_animation (myApplet);
					penguin_set_new_animation (myApplet, iNewAnimation);
				}
			}
			else  // on remonte.
			{
				int iNewAnimation = penguin_choose_go_up_animation (myApplet);
				penguin_set_new_animation (myApplet, iNewAnimation);
			}
		}
	}
	
	if (myData.iCurrentPositionY < (myConfig.bFree ? myBackground.iDockLineWidth + myConfig.iGroundOffset : 0))
	{
		myData.iCurrentPositionY = (myConfig.bFree ? myBackground.iDockLineWidth + myConfig.iGroundOffset : 0);
	}
	else if (myData.iCurrentPositionY + pAnimation->iFrameHeight > iHeight)
	{
		myData.iCurrentPositionY = iHeight - pAnimation->iFrameHeight;
	}
}



void penguin_advance_to_next_frame (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation)
{
	myData.iCurrentFrame ++;
	if (myData.iCurrentFrame >= pAnimation->iNbFrames)
	{
		myData.iCurrentFrame = 0;
		myData.iCount ++;
		if (( myData.iCount * myData.fFrameDelay * pAnimation->iNbFrames > myConfig.iDelayBetweenChanges) || pAnimation->bEnding)  // il est temps de changer d'animation.
		{
			if (pAnimation->bEnding)
			{
				myData.iSleepingTime = 0;
				if (! myConfig.bFree)
				{
					cairo_save (myDrawContext);  // on n'utilise pas CD_APPLET_SET_SURFACE_ON_MY_ICON (NULL) car il nous cree le pFullIconBuffer qui apres ecrase notre dessin.
					cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
					cairo_set_source_rgba (
						myDrawContext,
						0, 0, 0, 0);
					cairo_paint (myDrawContext);
					cairo_restore (myDrawContext);
					
					if (myIcon->pReflectionBuffer != NULL)
					{
						cairo_surface_destroy (myIcon->pReflectionBuffer);
						myIcon->pReflectionBuffer = NULL;
					}
					 if (CAIRO_DOCK_CONTAINER_IS_OPENGL (myContainer))
						cairo_dock_update_icon_texture (myIcon);
					else
						CD_APPLET_REDRAW_MY_ICON;
				}
				else  // on reste sur la derniere image de l'animation de fin.
				{
					myData.iCurrentFrame = pAnimation->iNbFrames - 1;
				}
				
				penguin_start_animating_with_delay (myApplet);
			}
			else
			{
				int iNewAnimation = penguin_choose_next_animation (myApplet, pAnimation);
				penguin_set_new_animation (myApplet, iNewAnimation);
			}
		}
	}
}



int penguin_choose_movement_animation (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	if (myData.iNbMovmentAnimations == 0)
		return 0;
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbMovmentAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbMovmentAnimations, myData.pMovmentAnimations[iRandom]);
		return myData.pMovmentAnimations[iRandom];
	}
}

int penguin_choose_go_up_animation (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	if (myData.iNbGoUpAnimations == 0)
		return penguin_choose_movement_animation (myApplet);
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbGoUpAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbGoUpAnimations, myData.pGoUpAnimations[iRandom]);
		return myData.pGoUpAnimations[iRandom];
	}
}

int penguin_choose_beginning_animation (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	if (myData.iNbBeginningAnimations == 0)
		return penguin_choose_movement_animation (myApplet);
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbBeginningAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbBeginningAnimations, myData.pBeginningAnimations[iRandom]);
		return myData.pBeginningAnimations[iRandom];
	}
}

int penguin_choose_ending_animation (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	if (myData.iNbEndingAnimations == 0)
		return penguin_choose_go_up_animation (myApplet);
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbEndingAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbEndingAnimations, myData.pEndingAnimations[iRandom]);
		return myData.pEndingAnimations[iRandom];
	}
}

int penguin_choose_resting_animation (CairoDockModuleInstance *myApplet)
{
	cd_debug ("");
	if (myData.iNbRestAnimations == 0)
		return penguin_choose_go_up_animation (myApplet);
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbRestAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbRestAnimations, myData.pRestAnimations[iRandom]);
		return myData.pRestAnimations[iRandom];
	}
}

int penguin_choose_next_animation (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation)
{
	cd_debug ("");
	int iNewAnimation;
	if (pAnimation == NULL || pAnimation->bEnding)  // le pingouin est en fin d'animation, on le relance.
	{
		iNewAnimation = penguin_choose_beginning_animation (myApplet);
	}
	else if (pAnimation->iDirection == PENGUIN_HORIZONTAL)  // le pingouin se deplace.
	{
		if (myConfig.bFree)
			iNewAnimation = penguin_choose_movement_animation (myApplet);
		else  // dans l'icone on ne repart pas en haut sur les bords.
		{
			int iRandom = g_random_int_range (0, 3);
			if (iRandom == 0)
				iNewAnimation = penguin_choose_go_up_animation (myApplet);
			else
				iNewAnimation = penguin_choose_movement_animation (myApplet);
		}
	}
	else  // le pingouin monte ou descend.
	{
		if (pAnimation->iDirection == PENGUIN_UP)  // il monte, on le refait descendre.
			iNewAnimation = penguin_choose_beginning_animation (myApplet);
		else  // il descend, on le fait se deplacer.
			iNewAnimation = penguin_choose_movement_animation (myApplet);
	}
	return iNewAnimation;
}


void penguin_set_new_animation (CairoDockModuleInstance *myApplet, int iNewAnimation)
{
	cd_message ("%s (%d)", __func__, iNewAnimation);
	PenguinAnimation *pPreviousAnimation = penguin_get_current_animation ();
	int iPreviousWidth = (pPreviousAnimation != NULL ? pPreviousAnimation->iFrameWidth : 0);
	int iPreviousHeight = (pPreviousAnimation != NULL ? pPreviousAnimation->iFrameHeight : 0);
	int iPreviousDirection = (pPreviousAnimation != NULL ? pPreviousAnimation->iDirection : 0);
	
	myData.iCurrentAnimation = iNewAnimation;
	myData.iCurrentFrame = 0;
	myData.iCount = 0;
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL)
		return ;
	myData.iCurrentSpeed = pAnimation->iSpeed;
	
	if (pAnimation->pSurfaces == NULL && pAnimation->iTexture == 0)
	{
		penguin_load_animation_buffer (pAnimation, myDrawContext, myConfig.fAlpha, CAIRO_DOCK_CONTAINER_IS_OPENGL (myContainer));
	}
	
	if (pAnimation->iDirection == PENGUIN_HORIZONTAL)
	{
		if (pAnimation->iNbDirections == 2)
			myData.iCurrentDirection = g_random_int_range (0, 2);  // [a;b[
		else
			myData.iCurrentDirection = 0;
		myData.iCurrentPositionY = (myConfig.bFree ? myBackground.iDockLineWidth + myConfig.iGroundOffset : 0);
	}
	else  // la direction reste la meme.
	{
		myData.iCurrentDirection = MIN (myData.iCurrentDirection, pAnimation->iNbDirections - 1);
		if (myData.iCurrentDirection == 1)  // on plaque a droite.
			myData.iCurrentPositionX += iPreviousWidth - pAnimation->iFrameWidth;
		if (pAnimation->iDirection == PENGUIN_DOWN)
		{
			if (myConfig.bFree)
				myData.iCurrentPositionY = myContainer->iHeight;
			else
				myData.iCurrentPositionY = myIcon->fHeight / myDock->fRatio * myIcon->fScale;
		}
	}
}


gboolean penguin_update_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	if (pContainer != myContainer)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	penguin_move_in_dock (myApplet);
	*bContinueAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean penguin_update_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	if (pIcon != myIcon)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	penguin_move_in_icon (myApplet);
	*bContinueAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


void penguin_start_animating (CairoDockModuleInstance *myApplet)
{
	int iNewAnimation = penguin_choose_beginning_animation (myApplet);
	penguin_set_new_animation (myApplet, iNewAnimation);
	
	penguin_remove_notfications();
	if (myConfig.bFree)
	{
		cairo_dock_register_notification_on_container (CAIRO_CONTAINER (g_pMainDock), CAIRO_DOCK_UPDATE_DOCK_SLOW, (CairoDockNotificationFunc) penguin_update_container, CAIRO_DOCK_RUN_AFTER, myApplet);
		cairo_dock_register_notification_on_container (CAIRO_CONTAINER (g_pMainDock), CAIRO_DOCK_RENDER_DOCK, (CairoDockNotificationFunc) penguin_render_on_container, CAIRO_DOCK_RUN_AFTER, myApplet);
	}
	else
	{
		cairo_dock_register_notification_on_icon (myIcon, CAIRO_DOCK_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) penguin_update_icon, CAIRO_DOCK_RUN_AFTER, myApplet);
	}
}

static gboolean _penguin_restart_delayed (CairoDockModuleInstance *myApplet)
{
	myData.iSidRestartDelayed = 0;
	penguin_start_animating (myApplet);
	
	if (! myData.bHasBeenStarted)
	{
		myData.bHasBeenStarted = TRUE;
		cd_message ("le pingouin demarre pour la 1ere fois");
		
		if (myConfig.bFree)  // attention : c'est un hack moyen; il faudrait pouvoir indiquer a cairo-dock de ne pas inserer notre icone...
		{
			cairo_dock_detach_icon_from_dock (myIcon, myDock, myIcons.bUseSeparator);
			cairo_dock_update_dock_size (myDock);
		}
		else
		{
			cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
		}
	}
	
	return FALSE;
}
void penguin_start_animating_with_delay (CairoDockModuleInstance *myApplet)
{
	if (myData.iSidRestartDelayed != 0)
		return ;
	if (cairo_dock_is_loading ())
	{
		myData.iSidRestartDelayed = g_timeout_add_seconds (2, (GSourceFunc) _penguin_restart_delayed, (gpointer) myApplet);  // priorite au chargement du dock, on demarrera plus tard.
	}
	else
	{
		myData.iSidRestartDelayed = g_timeout_add_seconds (1, (GSourceFunc) _penguin_restart_delayed, (gpointer) myApplet);  // on est oblige de faire ca, pour detacher l'icone apres que le dock l'ait inseree.
		//myData.iSidRestartDelayed = g_idle_add ((GSourceFunc) _penguin_restart_delayed, (gpointer) myApplet);  // on est oblige de faire ca, pour detacher l'icone apres que le dock l'ait inseree.
	}
}
