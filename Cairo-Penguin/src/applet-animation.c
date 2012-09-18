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
	
	Icon *pFirstDrawnIcon = cairo_dock_get_first_icon (myDock->icons);
	int iXMin = (pFirstDrawnIcon != NULL ? pFirstDrawnIcon->fXAtRest : 0);
	iXMin = 0;
	int iXMax = iXMin + myDock->fFlatDockWidth;
	int iHeight = myDock->container.iHeight;
	
	penguin_calculate_new_position (myApplet, pAnimation, iXMin, iXMax, iHeight);
	
	penguin_advance_to_next_frame (myApplet, pAnimation);
	
	if (myDock->container.bIsHorizontal)
	{
		area.x = (int) ((myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + MIN (iPreviousPositionX, myData.iCurrentPositionX));
		area.y = myDock->container.iHeight - MAX (iPreviousPositionY, myData.iCurrentPositionY) - pAnimation->iFrameHeight;
		area.width = abs (iPreviousPositionX - myData.iCurrentPositionX) + pAnimation->iFrameWidth + 1;  // +1 car sinon on a des trainees parfois, pas compris pourquoi :-/
		area.height = abs (iPreviousPositionY - myData.iCurrentPositionY) + pAnimation->iFrameHeight;
	}
	else
	{
		if (myDock->container.bDirectionUp)
		{
			if (!g_bUseOpenGL)
			{
				area.y = (int) ((myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + MAX (iPreviousPositionX, myData.iCurrentPositionX));
				area.y = myDock->container.iWidth - area.y;
			}
			else
			{
				area.y = (int) ((myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + MAX (iPreviousPositionX, myData.iCurrentPositionX)) + pAnimation->iFrameWidth;
				area.y = myDock->container.iWidth - area.y;
			}
			area.x = myDock->container.iHeight - MAX (iPreviousPositionY, myData.iCurrentPositionY) - pAnimation->iFrameHeight;
		}
		else
		{
			area.y = (int) ((myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + MIN (iPreviousPositionX, myData.iCurrentPositionX));
			area.x = MAX (iPreviousPositionY, myData.iCurrentPositionY);
		}
		area.height = abs (iPreviousPositionX - myData.iCurrentPositionX) + pAnimation->iFrameWidth + 1;  // meme remarque sur le +1.
		area.width = abs (iPreviousPositionY - myData.iCurrentPositionY) + pAnimation->iFrameHeight;
	}
	cairo_dock_redraw_container_area (myContainer, &area);
}

static void _penguin_draw_texture (CairoDockModuleInstance *myApplet, PenguinAnimation *pAnimation, double fOffsetX, double fOffsetY, double fScale)
{
	g_return_if_fail (pAnimation->iTexture != 0);
	int iIconWidth, iIconHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iIconWidth, &iIconHeight);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	_cairo_dock_set_alpha (1.);
	
	glBindTexture (GL_TEXTURE_2D, pAnimation->iTexture);
	_cairo_dock_apply_current_texture_portion_at_size_with_offset ((double) myData.iCurrentFrame/pAnimation->iNbFrames, 
		.5*myData.iCurrentDirection,
		1./pAnimation->iNbFrames,
		1./pAnimation->iNbDirections,
		pAnimation->iFrameWidth*fScale,
		pAnimation->iFrameHeight*fScale,
		floor (fOffsetX + myData.iCurrentPositionX + .5*pAnimation->iFrameWidth) + .5,
		floor (fOffsetY + myData.iCurrentPositionY + .5*pAnimation->iFrameHeight*fScale) + .5);
	_cairo_dock_disable_texture ();
}
void penguin_draw_on_dock_opengl (CairoDockModuleInstance *myApplet, CairoContainer *pContainer)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL)
		return ;
	
	glPushMatrix ();
	///glLoadIdentity ();
	
	if (! myDock->container.bIsHorizontal)
	{
		glTranslatef (myDock->container.iHeight/2, myDock->container.iWidth/2, 0.);
		glRotatef (myDock->container.bDirectionUp ? 90. : -90., 0., 0., 1.);
		glTranslatef (- myDock->container.iWidth/2, - myDock->container.iHeight/2, 0.);
	}
	_penguin_draw_texture (myApplet, pAnimation, (myDock->container.iWidth - myDock->fFlatDockWidth) * .5, 0., 1.);
	
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
	
	if (myDock->container.bIsHorizontal)
	{
		cairo_translate (pCairoContext, floor ((myDock->container.iWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX), myDock->container.iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight);
		cairo_set_source_surface (pCairoContext, pSurface, 0.0, 0.0);
		cairo_paint (pCairoContext);
	}
	else
	{
		if (myDock->container.bDirectionUp)
			cairo_translate (pCairoContext,
				myDock->container.iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight,
				floor (myDock->container.iWidth - (.5*(myDock->container.iWidth - myDock->fFlatDockWidth) + myData.iCurrentPositionX)));
		else
			cairo_translate (pCairoContext,
				myData.iCurrentPositionY,
				floor (.5*(myDock->container.iWidth - myDock->fFlatDockWidth) + myData.iCurrentPositionX));
		cairo_dock_draw_surface (pCairoContext, pSurface, pAnimation->iFrameWidth, pAnimation->iFrameHeight, myDock->container.bDirectionUp, myDock->container.bIsHorizontal, 1.);
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
	int iWidth = myIcon->fWidth / myDock->container.fRatio * fScale;
	int iHeight = myIcon->fHeight / myDock->container.fRatio * fScale;
	int iXMin = - iWidth / 2;
	int iXMax = - iXMin;
	
	penguin_calculate_new_position (myApplet, pAnimation, iXMin, iXMax, iHeight);
	
	penguin_advance_to_next_frame (myApplet, pAnimation);
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
		
		int iIconWidth, iIconHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&iIconWidth, &iIconHeight);
		
		g_return_if_fail (pAnimation->iTexture != 0);
		double f = (1 + myIconsParam.fAmplitude) / fScale;
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
		
		CD_APPLET_FINISH_DRAWING_MY_ICON;
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
			cairo_scale (myDrawContext, (1 + myIconsParam.fAmplitude) / fScale, (1 + myIconsParam.fAmplitude) / fScale);
			cairo_set_source_surface (
				myDrawContext,
				pSurface,
				iXMax + myData.iCurrentPositionX,
				iHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight);
			cairo_paint (myDrawContext);
			cairo_restore (myDrawContext);
		}
		
		//\________________ les reflets.
		///CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	}
	
	CD_APPLET_REDRAW_MY_ICON;
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
					//cd_debug ("myData.iCurrentDirection <- %d", myData.iCurrentDirection);
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
	
	if (myData.iCurrentPositionY < (myConfig.bFree ? myDocksParam.iDockLineWidth + myConfig.iGroundOffset : 0))
	{
		myData.iCurrentPositionY = (myConfig.bFree ? myDocksParam.iDockLineWidth + myConfig.iGroundOffset : 0);
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
		
		if (pAnimation->bEnding)  // c'est une animation de fin, elle ne se joue qu'une seule fois.
		{
			myData.iSleepingTime = 0;
			if (! myConfig.bFree)
			{
				cairo_dock_erase_cairo_context (myDrawContext);  // CD_APPLET_SET_SURFACE_ON_MY_ICON (NULL)
				
				/**if (myIcon->pReflectionBuffer != NULL)
				{
					cairo_surface_destroy (myIcon->pReflectionBuffer);
					myIcon->pReflectionBuffer = NULL;
				}*/
				if (CAIRO_DOCK_CONTAINER_IS_OPENGL (myContainer))
					cairo_dock_update_icon_texture (myIcon);
			}
			else  // on reste sur la derniere image de l'animation de fin.
			{
				myData.iCurrentFrame = pAnimation->iNbFrames - 1;
			}
			
			penguin_start_animating_with_delay (myApplet);
		}
		else if (myData.iCount * myData.fFrameDelay * pAnimation->iNbFrames > myConfig.iDelayBetweenChanges)  // il est temps de changer d'animation.
		{
			int iNewAnimation = penguin_choose_next_animation (myApplet, pAnimation);
			penguin_set_new_animation (myApplet, iNewAnimation);
		}
	}
}



int penguin_choose_movement_animation (CairoDockModuleInstance *myApplet)
{
	//cd_debug ("");
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
	//cd_debug ("");
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
	//cd_debug ("");
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
	//cd_debug ("");
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
	//cd_debug ("");
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
	//cd_debug ("");
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
	//cd_message ("%s (%d)", __func__, iNewAnimation);
	PenguinAnimation *pPreviousAnimation = penguin_get_current_animation ();
	int iPreviousWidth = (pPreviousAnimation != NULL ? pPreviousAnimation->iFrameWidth : 0);
	
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
		myData.iCurrentPositionY = (myConfig.bFree ? myDocksParam.iDockLineWidth + myConfig.iGroundOffset : 0);
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
				myData.iCurrentPositionY = myIcon->fHeight / myDock->container.fRatio * myIcon->fScale;
		}
	}
}


gboolean penguin_update_container (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL || (pAnimation->bEnding && myData.iCount > 0))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	penguin_move_in_dock (myApplet);
	*bContinueAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

gboolean penguin_update_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL || (pAnimation->bEnding && myData.iCount > 0))
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
		cairo_dock_register_notification_on_object (myContainer, NOTIFICATION_UPDATE_SLOW, (CairoDockNotificationFunc) penguin_update_container, CAIRO_DOCK_RUN_AFTER, myApplet);
		cairo_dock_register_notification_on_object (myContainer, NOTIFICATION_RENDER, (CairoDockNotificationFunc) penguin_render_on_container, CAIRO_DOCK_RUN_AFTER, myApplet);
	}
	else
	{
		cairo_dock_register_notification_on_object (myIcon, NOTIFICATION_UPDATE_ICON_SLOW, (CairoDockNotificationFunc) penguin_update_icon, CAIRO_DOCK_RUN_AFTER, myApplet);
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
			cairo_dock_detach_icon_from_dock (myIcon, myDock);
		}
		else
		{
			cairo_dock_insert_icon_in_dock (myIcon, myDock, ! CAIRO_DOCK_ANIMATE_ICON);
		}
		cairo_dock_launch_animation (myContainer);
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
