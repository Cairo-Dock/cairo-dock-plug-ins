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

#include <gdk/gdkx.h>

#include "applet-struct.h"
#include "applet-search.h"
#include "applet-listing.h"
#include "applet-notifications.h"
#include "applet-session.h"


void cd_do_open_session (void)
{
	if (cd_do_session_is_running ())  // session already running
		return;
	
	// on termine la precedente session.
	cd_do_exit_session ();
	cd_do_stop_all_backends ();  // on le fait maintenant pour ne pas bloquer au exit.
	
	// register to draw on dock.
	if (cd_do_session_is_off ())
	{
		cairo_dock_register_notification_on_object (g_pMainDock,
			NOTIFICATION_UPDATE,
			(CairoDockNotificationFunc) cd_do_update_container,
			CAIRO_DOCK_RUN_AFTER, NULL);
		cairo_dock_register_notification_on_object (g_pMainDock,
			NOTIFICATION_RENDER,
			(CairoDockNotificationFunc) cd_do_render,
			CAIRO_DOCK_RUN_AFTER, NULL);
	}
	
	// wait for keyboard input.
	cairo_dock_register_notification_on_object (&myContainersMgr,
		NOTIFICATION_KEY_PRESSED,
		(CairoDockNotificationFunc) cd_do_key_pressed,
		CAIRO_DOCK_RUN_AFTER, NULL);
	cairo_dock_register_notification_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) cd_do_check_active_dock,
		CAIRO_DOCK_RUN_AFTER, NULL);
	
	myData.sCurrentText = g_string_sized_new (20);
	myConfig.labelDescription.iSize = myConfig.fFontSizeRatio * g_pMainDock->iMaxDockHeight;
	myData.iPromptAnimationCount = 0;
	if (myData.pPromptSurface == NULL)
	{
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
		myData.pPromptSurface = cairo_dock_create_surface_from_text (D_("Enter your search"), &myConfig.labelDescription, &myData.iPromptWidth, &myData.iPromptHeight);
		cairo_destroy (pCairoContext);
		if (g_bUseOpenGL)
		{
			myData.iPromptTexture = cairo_dock_create_texture_from_surface (myData.pPromptSurface);
		}
	}
	
	// on montre le main dock.
	cairo_dock_emit_enter_signal (CAIRO_CONTAINER (g_pMainDock));
	
	// le main dock prend le focus.
	myData.iPreviouslyActiveWindow = cairo_dock_get_active_xwindow ();
	//if (cairo_dock_get_desklet_by_Xid (myData.iPreviouslyActiveWindow))
	//	myData.iPreviouslyActiveWindow = 0;
	///gtk_window_present (GTK_WINDOW (g_pMainDock->container.pWidget));
	gtk_window_present_with_time (GTK_WINDOW (g_pMainDock->container.pWidget), gdk_x11_get_server_time (gldi_container_get_gdk_window(CAIRO_CONTAINER (g_pMainDock))));  // pour eviter la prevention du vol de focus.
	cairo_dock_freeze_docks (TRUE);
	
	// On lance l'animation d'attente.
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));
	
	myData.iSessionState = 2;
}

void cd_do_close_session (void)
{
	if (! cd_do_session_is_running ())  // session not running
		return;
	
	// no more keyboard input.
	cairo_dock_remove_notification_func_on_object (&myContainersMgr,
		NOTIFICATION_KEY_PRESSED,
		(CairoDockNotificationFunc) cd_do_key_pressed, NULL);
	cairo_dock_remove_notification_func_on_object (&myDesktopMgr,
		NOTIFICATION_WINDOW_ACTIVATED,
		(CairoDockNotificationFunc) cd_do_check_active_dock, NULL);
	
	g_string_free (myData.sCurrentText, TRUE);
	myData.sCurrentText = NULL;
	myData.iNbValidCaracters = 0;
	
	// on cache les resultats
	cd_do_hide_listing ();
	
	g_free (myData.cSearchText);
	myData.cSearchText = NULL;
	myData.iCurrentFilter = 0;
	
	cairo_dock_emit_leave_signal (CAIRO_CONTAINER (g_pMainDock));
	
	// on redonne le focus a l'ancienne fenetre.
	if (myData.iPreviouslyActiveWindow != 0)
	{
		/// ne le faire que si on a encore le focus, sinon c'est que l'utilisateur a change lui-meme de fenetre...
		//Window iActiveWindow = cairo_dock_get_active_xwindow ();
		
		//cairo_dock_show_xwindow (myData.iPreviouslyActiveWindow);
		myData.iPreviouslyActiveWindow = 0;
	}
	
	// on quitte dans une animation.
	myData.iCloseTime = myConfig.iCloseDuration;
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));
	cairo_dock_freeze_docks (FALSE);
	
	myData.iSessionState = 1;
}

void cd_do_exit_session (void)
{
	if (cd_do_session_is_off ())  // session already off
		return;
	
	
	cd_do_close_session ();
	
	myData.iCloseTime = 0;
	
	cairo_dock_remove_notification_func_on_object (g_pMainDock, NOTIFICATION_RENDER, (CairoDockNotificationFunc) cd_do_render, NULL);
	cairo_dock_remove_notification_func_on_object (g_pMainDock, NOTIFICATION_UPDATE, (CairoDockNotificationFunc) cd_do_update_container, NULL);
	
	/// arreter les backends...
	
	
	if (myData.pCharList != NULL)
	{
		cd_do_free_char_list (myData.pCharList);
		myData.pCharList = NULL;
		myData.iTextWidth = 0;
		myData.iTextHeight = 0;
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
	}
	if (myData.pMatchingIcons != NULL)
	{
		Icon *pIcon;
		GList *ic;
		for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->cDesktopFileName && strncmp (pIcon->cDesktopFileName, "/usr", 4) == 0 && pIcon->image.pSurface != NULL)
			{
				cairo_surface_destroy (pIcon->image.pSurface);
				pIcon->image.pSurface = NULL;
				if (pIcon->image.iTexture != 0)
				{
					_cairo_dock_delete_texture (pIcon->image.iTexture);
					pIcon->image.iTexture = 0;
				}
			}
		}
		g_list_free (myData.pMatchingIcons);
		myData.pMatchingIcons = NULL;
		myData.pCurrentMatchingElement = NULL;
		myData.pCurrentMatchingElement = NULL;
		myData.iMatchingGlideCount = 0;
		myData.iPreviousMatchingOffset = 0;
		myData.iCurrentMatchingOffset = 0;
	}
	
	myData.iSessionState = 0;
}



void cd_do_free_char (CDChar *pChar)
{
	if (pChar == NULL)
		return ;
	if (pChar->pSurface != NULL)
	{
		cairo_surface_destroy (pChar->pSurface);
	}
	if (pChar->iTexture != 0)
	{
		_cairo_dock_delete_texture (pChar->iTexture);
	}
	g_free (pChar);
}

void cd_do_free_char_list (GList *pCharList)
{
	g_list_foreach (pCharList, (GFunc) cd_do_free_char, NULL);
	g_list_free (pCharList);
}


void cd_do_load_pending_caracters (void)
{
	cairo_surface_t *pSurface;
	gboolean bLoadTexture = (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock));
	gchar c[2] = {'\0', '\0'};
	CDChar *pChar;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	int iDeltaT = cairo_dock_get_animation_delta_t (g_pMainDock);
	guint i;
	int iOffsetX=0;
	for (i = myData.iNbValidCaracters-0; i < myData.sCurrentText->len; i++)
	{
		//g_print (" on charge la lettre '%c' (%d) tex:%d\n", myData.sCurrentText->str[i], i, bLoadTexture);
		c[0] = myData.sCurrentText->str[i];
		
		pChar = g_new0 (CDChar, 1);
		pChar->c = c[0];
		pChar->iInitialX = myData.iTextWidth/2 + iOffsetX;  // il part du coin haut droit.
		pChar->iInitialY = g_pMainDock->container.iHeight/2;  // en bas.
		pChar->iCurrentX = pChar->iInitialX;
		pChar->iCurrentY = pChar->iInitialY;
		pChar->fRotationAngle = 10. * myConfig.iAppearanceDuration / iDeltaT;
		//g_print (" on commence a x=%d\n", pChar->iInitialX);
		myData.pCharList = g_list_append (myData.pCharList, pChar);
		
		// on cree la surface.
		pSurface = cairo_dock_create_surface_from_text (c, &myConfig.labelDescription, &pChar->iWidth, &pChar->iHeight);
		if (g_pMainDock->container.bIsHorizontal)
		{
			myData.iTextWidth += pChar->iWidth;
			iOffsetX += pChar->iWidth;
			pChar->iInitialY = g_pMainDock->iMaxDockHeight - pChar->iHeight;
			myData.iTextHeight = MAX (myData.iTextHeight, pChar->iHeight);
		}
		else
		{
			myData.iTextHeight += pChar->iHeight;
			iOffsetX += pChar->iHeight;
			pChar->iInitialY = g_pMainDock->iMaxDockHeight - pChar->iWidth;
			myData.iTextWidth = MAX (myData.iTextWidth, pChar->iWidth);
		}
		
		// on cree la texture.
		if (bLoadTexture)
		{
			pChar->iTexture = cairo_dock_create_texture_from_surface (pSurface);
			cairo_surface_destroy (pSurface);
		}
		else
		{
			pChar->pSurface = pSurface;
		}
	}
	cairo_destroy (pCairoContext);
}


void cd_do_compute_final_coords (void)
{
	int x = - myData.iTextWidth / 2;  // par rapport au milieu du dock.
	CDChar *pChar;
	GList *c;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		
		pChar->iFinalX = x;
		pChar->iFinalY = 0;
		x += pChar->iWidth;
		
		pChar->iInitialX = pChar->iCurrentX;
		pChar->iInitialY = pChar->iCurrentY;
	}
}


void cd_do_launch_appearance_animation (void)
{
	cd_do_compute_final_coords ();
	myData.iAppearanceTime = myConfig.iAppearanceDuration;
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));  // animation de disparition.	
}


void cd_do_delete_invalid_caracters (void)
{
	if (myData.sCurrentText->len == 0)
		return;
	
	// on efface les lettres precedentes jusqu'a la derniere position validee.
	CDChar *pChar;
	GList *c = g_list_last (myData.pCharList), *c_prev;
	guint i;
	for (i = myData.iNbValidCaracters; i < myData.sCurrentText->len && c != NULL; i ++)
	{
		//g_print ("on efface '%c'\n", myData.sCurrentText->str[i]);
		c_prev = c->prev;
		pChar = c->data;
		
		myData.iTextWidth -= pChar->iWidth;
		cd_do_free_char (pChar);
		myData.pCharList = g_list_delete_link (myData.pCharList, c);  // detruit c.
		c = c_prev;
	}
	
	// on tronque la chaine de la meme maniere.
	g_string_truncate (myData.sCurrentText, myData.iNbValidCaracters);
	cd_debug (" -> '%s' (%d)", myData.sCurrentText->str, myData.iNbValidCaracters);
	
	// on remet a jour la hauteur du texte.
	myData.iTextHeight = 0;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		myData.iTextHeight = MAX (myData.iTextHeight, pChar->iHeight);
	}
}
