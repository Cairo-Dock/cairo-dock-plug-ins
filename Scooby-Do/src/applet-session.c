/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-session.h"


void cd_do_open_session (void)
{
	// on termine l'animation de fin de la precedente session.
	cd_do_exit_session ();
	
	// on se met en attente de texte.
	myData.sCurrentText = g_string_sized_new (20);
	
	// on montre le main dock.
	myData.bIgnoreIconState = TRUE;
	cairo_dock_emit_enter_signal (g_pMainDock);
	myData.bIgnoreIconState = FALSE;
	
	// le main dock prend le focus.
	myData.iPreviouslyActiveWindow = cairo_dock_get_active_xwindow ();
	gtk_window_present (GTK_WINDOW (g_pMainDock->pWidget));
	
	myConfig.labelDescription.iSize = MIN (50, myConfig.fFontSizeRatio * g_pMainDock->iMaxDockHeight);
}

void cd_do_close_session (void)
{
	// on ne veut plus de texte.
	g_string_free (myData.sCurrentText, TRUE);
	myData.sCurrentText = NULL;
	myData.iNbValidCaracters = 0;
	
	// on remet a zero la session.
	if (myData.pCurrentIcon != NULL)
	{
		myData.bIgnoreIconState = TRUE;
		cairo_dock_stop_icon_animation (myData.pCurrentIcon);
		myData.bIgnoreIconState = FALSE;
		myData.pCurrentIcon = NULL;
	}
	
	if (myData.pCurrentDock != NULL)
	{
		//cairo_dock_leave_from_main_dock (myData.pCurrentDock);  /// voir avec un emit_leave_signal ...
		cairo_dock_emit_leave_signal (myData.pCurrentDock);
		myData.pCurrentDock = NULL;
	}
	if (myData.pCurrentDock != g_pMainDock)
	{
		cairo_dock_emit_leave_signal (g_pMainDock);
	}
	
	// on redonne le focus a l'ancienne fenetre.
	if (myData.iPreviouslyActiveWindow != 0)
	{
		/// ne le faire que si on a encore le focus, sinon c'est que l'utilisateur a change lui-meme de fenetre...
		Window iActiveWindow = cairo_dock_get_active_xwindow ();
		
		//cairo_dock_show_xwindow (myData.iPreviouslyActiveWindow);
		myData.iPreviouslyActiveWindow = 0;
	}
	
	// on quitte dans une animation.
	myData.iCloseTime = myConfig.iCloseDuration;
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));
}

void cd_do_exit_session (void)
{
	myData.iCloseTime = 0;
	if (myData.pCharList != NULL)
	{
		cd_do_free_char_list (myData.pCharList);
		myData.pCharList = NULL;
		myData.iTextWidth = 0;
		myData.iTextHeight = 0;
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
	}
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
