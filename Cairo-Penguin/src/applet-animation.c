/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-notifications.h"
#include "applet-animation.h"

CD_APPLET_INCLUDE_MY_VARS


gboolean penguin_move_in_dock (gpointer data)
{
	static GdkRectangle area;
	//cd_message ("");
	if (! cairo_dock_animation_will_be_visible (myDock))
		return TRUE;
	
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	g_return_val_if_fail (pAnimation != NULL, TRUE);
	int iPreviousPositionX = myData.iCurrentPositionX, iPreviousPositionY = myData.iCurrentPositionY;
	
	int iXMin = ((Icon *)myDock->pFirstDrawnElement->data)->fXAtRest;
	int iXMax = iXMin + myDock->fFlatDockWidth;
	int iHeight = myDock->iCurrentHeight;
	
	penguin_calculate_new_position (pAnimation, iXMin, iXMax, iHeight);
	
	area.x = (myDock->iCurrentWidth - myDock->fFlatDockWidth) * .5 + MIN (iPreviousPositionX, myData.iCurrentPositionX);
	area.y = myDock->iCurrentHeight - MAX (iPreviousPositionY, myData.iCurrentPositionY) - pAnimation->iFrameHeight;
	area.width = abs (iPreviousPositionX - myData.iCurrentPositionX) + pAnimation->iFrameWidth;
	area.height = abs (iPreviousPositionY - myData.iCurrentPositionY) + pAnimation->iFrameHeight;
	
	//g_print (" -> (%d;%d) (%dx%d)\n", area.x, area.y, area.width, area.height);
	if (area.width > 0 && area.height > 0)
	{
#ifdef HAVE_GLITZ
		if (myContainer->pDrawFormat && myContainer->pDrawFormat->doublebuffer)
			gtk_widget_queue_draw (myContainer->pWidget);
		else
#endif
		gdk_window_invalidate_rect (myContainer->pWidget->window, &area, FALSE);
	}
	
	penguin_advance_to_next_frame (pAnimation);
	
	return TRUE;
}

gboolean penguin_draw_on_dock (GtkWidget *pWidget,
	GdkEventExpose *pExpose,
	gpointer data)
{
	//g_print ("%s (%d,%d ; %d;%d)\n", __func__, myData.iCurrentDirection, myData.iCurrentFrame, myData.iCurrentPositionX, myData.iCurrentPositionY);
	if (! cairo_dock_animation_will_be_visible (myDock))
		return FALSE;
	
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	if (pAnimation == NULL)
		return FALSE;
	
	g_return_val_if_fail (pAnimation->pSurfaces != NULL, FALSE);
	cairo_surface_t *pSurface = pAnimation->pSurfaces[myData.iCurrentDirection][myData.iCurrentFrame];
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
	g_return_val_if_fail (cairo_status (pCairoContext) == CAIRO_STATUS_SUCCESS, FALSE);
	
	if (pExpose->area.x + pExpose->area.y != 0)  // x et/ou y sont > 0.
	{
		cairo_rectangle (pCairoContext,
			pExpose->area.x,
			pExpose->area.y,
			pExpose->area.width,
			pExpose->area.height);
		cairo_clip (pCairoContext);
	}
	cairo_set_operator (pCairoContext, CAIRO_OPERATOR_OVER);
	cairo_translate (pCairoContext, (myDock->iCurrentWidth - myDock->fFlatDockWidth) / 2 + myData.iCurrentPositionX, myDock->iCurrentHeight - myData.iCurrentPositionY - pAnimation->iFrameHeight);
	cairo_set_source_surface (pCairoContext, pSurface, 0.0, 0.0);
	cairo_paint (pCairoContext);

	cairo_destroy (pCairoContext);
#ifdef HAVE_GLITZ
	if (myContainer->pDrawFormat && myContainer->pDrawFormat->doublebuffer)
		glitz_drawable_swap_buffers (myContainer->pGlitzDrawable);
#endif
	return FALSE;
}

gboolean penguin_move_in_icon (gpointer data)
{
	//g_print ("%s (%d,%d) ; (%d,%d)\n", __func__, myData.iCurrentDirection, myData.iCurrentFrame, myData.iCurrentPositionX, myData.iCurrentPositionY);
	if (! cairo_dock_animation_will_be_visible (myDock))
		return TRUE;
	
	PenguinAnimation *pAnimation = penguin_get_current_animation ();
	g_return_val_if_fail (pAnimation != NULL && pAnimation->pSurfaces != NULL, TRUE);
	cairo_surface_t *pSurface = pAnimation->pSurfaces[myData.iCurrentDirection][myData.iCurrentFrame];
	g_return_val_if_fail (pSurface != NULL, TRUE);
	
	double fScale = (pAnimation->iNbFrames > 1 || pAnimation->iSpeed != 0 || pAnimation->iAcceleration != 0 ? myIcon->fScale : 1.);  // s'il est a l'arret on le met a la taille de l'icone au repos.
	int iXMin = - myIcon->fWidth / myDock->fRatio * fScale / 2;
	int iXMax = - iXMin;
	int iHeight = myIcon->fHeight / myDock->fRatio * fScale;
	
	penguin_calculate_new_position (pAnimation, iXMin, iXMax, iHeight);
	
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
	
	if (myDock != NULL && myDock->bUseReflect)
	{
		cairo_surface_t *pReflet = myIcon->pReflectionBuffer;
		myIcon->pReflectionBuffer = NULL;
		cairo_surface_destroy (pReflet);
		
		myIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (myIcon->pIconBuffer,
			myDrawContext,
			(myDock->bHorizontalDock ? myIcon->fWidth : myIcon->fHeight) * (1 + g_fAmplitude) / myDock->fRatio,
			(myDock->bHorizontalDock ? myIcon->fHeight : myIcon->fWidth) * (1 + g_fAmplitude) / myDock->fRatio,
			myDock->bHorizontalDock,
			1 + g_fAmplitude,
			myDock->bDirectionUp);
	}
	
	CD_APPLET_REDRAW_MY_ICON
	
	penguin_advance_to_next_frame (pAnimation);
	return TRUE;
}



void penguin_calculate_new_position (PenguinAnimation *pAnimation, int iXMin, int iXMax, int iHeight)
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
				if (iRandom == 0)  // 1 chance sur 3.
				{
					myData.iCurrentDirection = 1 - myData.iCurrentDirection;
					cd_debug ("myData.iCurrentDirection <- %d", myData.iCurrentDirection);
				}
				else
				{
					int iNewAnimation = penguin_choose_go_up_animation ();
					penguin_set_new_animation (iNewAnimation);
				}
			}
			else  // on remonte.
			{
				int iNewAnimation = penguin_choose_go_up_animation ();
				penguin_set_new_animation (iNewAnimation);
			}
		}
	}
	
	if (myData.iCurrentPositionY < (myConfig.bFree ? g_iDockLineWidth : 0))
	{
		myData.iCurrentPositionY = (myConfig.bFree ? g_iDockLineWidth : 0);
	}
	else if (myData.iCurrentPositionY + pAnimation->iFrameHeight > iHeight)
	{
		myData.iCurrentPositionY = iHeight - pAnimation->iFrameHeight;
	}
}



void penguin_advance_to_next_frame (PenguinAnimation *pAnimation)
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
				g_source_remove (myData.iSidAnimation);
				myData.iSidAnimation = 0;
				
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
					CD_APPLET_REDRAW_MY_ICON
				}
				else  // on reste sur la derniere image de l'animation de fin.
				{
					myData.iCurrentFrame = pAnimation->iNbFrames - 1;
				}
				
				penguin_start_animating_with_delay (FALSE);
			}
			else
			{
				int iNewAnimation = penguin_choose_next_animation (pAnimation);
				penguin_set_new_animation (iNewAnimation);
			}
		}
	}
}



int penguin_choose_movement_animation (void)
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

int penguin_choose_go_up_animation (void)
{
	cd_debug ("");
	if (myData.iNbGoUpAnimations == 0)
		return penguin_choose_movement_animation ();
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbGoUpAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbGoUpAnimations, myData.pGoUpAnimations[iRandom]);
		return myData.pGoUpAnimations[iRandom];
	}
}

int penguin_choose_beginning_animation (void)
{
	cd_debug ("");
	if (myData.iNbBeginningAnimations == 0)
		return penguin_choose_movement_animation ();
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbBeginningAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbBeginningAnimations, myData.pBeginningAnimations[iRandom]);
		return myData.pBeginningAnimations[iRandom];
	}
}

int penguin_choose_ending_animation (void)
{
	cd_debug ("");
	if (myData.iNbEndingAnimations == 0)
		return penguin_choose_go_up_animation ();
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbEndingAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbEndingAnimations, myData.pEndingAnimations[iRandom]);
		return myData.pEndingAnimations[iRandom];
	}
}

int penguin_choose_resting_animation (void)
{
	cd_debug ("");
	if (myData.iNbRestAnimations == 0)
		return penguin_choose_go_up_animation ();
	else
	{
		int iRandom = g_random_int_range (0, myData.iNbRestAnimations);  // [a;b[
		//g_print (  "0<%d<%d => %d\n", iRandom, myData.iNbRestAnimations, myData.pRestAnimations[iRandom]);
		return myData.pRestAnimations[iRandom];
	}
}

int penguin_choose_next_animation (PenguinAnimation *pAnimation)
{
	cd_debug ("");
	int iNewAnimation;
	if (pAnimation == NULL || pAnimation->bEnding)  // le pingouin est en fin d'animation, on le relance.
	{
		iNewAnimation = penguin_choose_beginning_animation ();
	}
	else if (pAnimation->iDirection == PENGUIN_HORIZONTAL)  // le pingouin se deplace.
	{
		if (myConfig.bFree)
			iNewAnimation = penguin_choose_movement_animation ();
		else  // dans l'icone on ne repart pas en haut sur les bords.
		{
			int iRandom = g_random_int_range (0, 3);
			if (iRandom == 0)
				iNewAnimation = penguin_choose_go_up_animation ();
			else
				iNewAnimation = penguin_choose_movement_animation ();
		}
	}
	else  // le pingouin monte ou descend.
	{
		if (pAnimation->iDirection == PENGUIN_UP)  // il monte, on le refait descendre.
			iNewAnimation = penguin_choose_beginning_animation ();
		else  // il descend, on le fait se deplacer.
			iNewAnimation = penguin_choose_movement_animation ();
	}
	return iNewAnimation;
}


void penguin_set_new_animation (int iNewAnimation)
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
	
	if (pAnimation->pSurfaces == NULL)
	{
		penguin_load_animation_buffer (pAnimation, myDrawContext);
	}
	
	if (pAnimation->iDirection == PENGUIN_HORIZONTAL)
	{
		if (pAnimation->iNbDirections == 2)
			myData.iCurrentDirection = g_random_int_range (0, 2);  // [a;b[
		else
			myData.iCurrentDirection = 0;
		myData.iCurrentPositionY = (myConfig.bFree ? g_iDockLineWidth : 0);
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


void penguin_start_animating (void)
{
	g_return_if_fail (myData.iSidAnimation == 0);
	int iNewAnimation = penguin_choose_beginning_animation ();
	penguin_set_new_animation (iNewAnimation);
	
	gulong iOnExposeCallbackID = g_signal_handler_find (G_OBJECT (myContainer->pWidget),
		G_SIGNAL_MATCH_FUNC,
		0,
		0,
		NULL,
		penguin_draw_on_dock,
		NULL);
	
	if (myConfig.bFree)
	{
		if (iOnExposeCallbackID <= 0)
			g_signal_connect_after (G_OBJECT (myContainer->pWidget),
				"expose-event",
				G_CALLBACK (penguin_draw_on_dock),
				myContainer);
		myData.iSidAnimation = g_timeout_add (1000 * myData.fFrameDelay, (GSourceFunc) penguin_move_in_dock, (gpointer) NULL);
	}
	else
	{
		if (iOnExposeCallbackID > 0)
			g_signal_handler_disconnect (G_OBJECT (myContainer->pWidget), iOnExposeCallbackID);
		myData.iSidAnimation = g_timeout_add (1000 * myData.fFrameDelay, (GSourceFunc) penguin_move_in_icon, (gpointer) NULL);
	}
}

static gboolean _penguin_restart_delayed (gpointer data)
{
	myData.iSidRestartDelayed = 0;
	penguin_start_animating ();
	
	gboolean bInit = GPOINTER_TO_INT (data);
	if (bInit)
	{
		cd_message ("le pingouin demarre pour la 1ere fois");
		cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK, CAIRO_DOCK_RUN_FIRST);
		cairo_dock_register_notification (CAIRO_DOCK_BUILD_MENU, (CairoDockNotificationFunc) CD_APPLET_ON_BUILD_MENU, CAIRO_DOCK_RUN_FIRST);
		cairo_dock_register_notification (CAIRO_DOCK_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_MIDDLE_CLICK, CAIRO_DOCK_RUN_FIRST);
		
		if (myConfig.bFree)  // attention : c'est un hack moyen; il faudrait pouvoir indiquer a cairo-dock de ne pas inserer notre icone...
		{
			cairo_dock_detach_icon_from_dock (myIcon, myDock, g_bUseSeparator);
			cairo_dock_update_dock_size (myDock);
		}
		else
		{
			cairo_dock_insert_icon_in_dock (myIcon, myDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON, CAIRO_DOCK_APPLY_RATIO, g_bUseSeparator);
		}
	}
	
	return FALSE;
}
void penguin_start_animating_with_delay (gboolean bInit)
{
	if (myData.iSidRestartDelayed == 0)
		myData.iSidRestartDelayed = g_timeout_add_seconds (1., (GSourceFunc) _penguin_restart_delayed, (gpointer) GINT_TO_POINTER (bInit));
}
