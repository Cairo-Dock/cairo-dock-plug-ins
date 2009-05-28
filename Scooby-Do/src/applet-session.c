/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-command-finder.h"
#include "applet-session.h"


void cd_do_open_session (void)
{
	// on termine l'animation de fin de la precedente session.
	cd_do_exit_session ();
	
	if (myData.iLocateAvailable == 0)  // comme ca on peut re-verifier la presence de locate en re-activant le module.
		myData.iLocateAvailable = (cd_do_check_locate_is_available () ? 1 : -1);
	
	// on se met en attente de texte.
	myData.sCurrentText = g_string_sized_new (20);
	myConfig.labelDescription.iSize = myConfig.fFontSizeRatio * g_pMainDock->iMaxDockHeight;
	myData.iPromptAnimationCount = 0;
	if (! myData.bNavigationMode && myData.pPromptSurface == NULL)
	{
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
		double fTextXOffset, fTextYOffset;
		myData.pPromptSurface = cairo_dock_create_surface_from_text (D_("Enter your search"), pCairoContext, &myConfig.labelDescription, 1., &myData.iPromptWidth, &myData.iPromptHeight, &fTextXOffset, &fTextYOffset);
		cairo_destroy (pCairoContext);
		if (g_bUseOpenGL)
		{
			myData.iPromptTexture = cairo_dock_create_texture_from_surface (myData.pPromptSurface);
		}
	}
	else if (myData.bNavigationMode && myData.pArrowSurface == NULL)
	{
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
		double fTextXOffset, fTextYOffset;
		myData.pArrowSurface = cairo_dock_create_surface_for_icon (MY_APPLET_SHARE_DATA_DIR"/arrows.svg", pCairoContext, g_pMainDock->iMaxDockHeight, g_pMainDock->iMaxDockHeight);
		myData.iArrowWidth = g_pMainDock->iMaxDockHeight;
		myData.iArrowHeight = g_pMainDock->iMaxDockHeight;
		cairo_destroy (pCairoContext);
		if (g_bUseOpenGL)
		{
			myData.iArrowTexture = cairo_dock_create_texture_from_surface (myData.pArrowSurface);
		}
	}
	// on montre le main dock.
	myData.bIgnoreIconState = TRUE;
	cairo_dock_emit_enter_signal (g_pMainDock);
	myData.bIgnoreIconState = FALSE;
	
	// le main dock prend le focus.
	myData.iPreviouslyActiveWindow = cairo_dock_get_active_xwindow ();
	gtk_window_present (GTK_WINDOW (g_pMainDock->pWidget));
	cairo_dock_freeze_docks (TRUE);
	
	// On lance l'animation d'attente.
	cairo_dock_launch_animation (CAIRO_CONTAINER (g_pMainDock));
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
	
	// on chache le filtre.
	cd_do_hide_filter_dialog ();
	myData.iCurrentFilter = 0;
	myData.bMatchCase = FALSE;
	
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
	cairo_dock_freeze_docks (FALSE);
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
	if (myData.pMatchingIcons != NULL)
	{
		Icon *pIcon;
		GList *ic;
		for (ic = myData.pMatchingIcons; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->acDesktopFileName && strncmp (pIcon->acDesktopFileName, "/usr", 4) == 0 && pIcon->pIconBuffer != NULL)
			{
				cairo_surface_destroy (pIcon->pIconBuffer);
				pIcon->pIconBuffer = NULL;
				if (pIcon->iIconTexture != 0)
				{
					_cairo_dock_delete_texture (pIcon->iIconTexture);
					pIcon->iIconTexture = 0;
				}
			}
		}
		g_list_free (myData.pMatchingIcons);
		myData.pMatchingIcons = NULL;
		myData.pCurrentMatchingElement = NULL;
	}
	if (myData.pMatchingFiles != NULL)
	{
		g_strfreev (myData.pMatchingFiles);
		myData.pMatchingFiles = NULL;
		myData.iNbMatchingFiles = 0;
	}
	if (myData.pInfoSurface != NULL)
	{
		cairo_surface_destroy (myData.pInfoSurface);
		myData.pInfoSurface = NULL;
	}
	if (myData.iInfoTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iInfoTexture);
		myData.iInfoTexture = 0;
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


void cd_do_load_pending_caracters (void)
{
	cairo_surface_t *pSurface;
	GLuint iTexture;
	gboolean bLoadTexture = (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock));
	gchar c[2] = {'\0', '\0'};
	double fTextXOffset, fTextYOffset;
	CDChar *pChar;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	int iDeltaT = cairo_dock_get_animation_delta_t (g_pMainDock);
	int i, iOffsetX=0;
	for (i = myData.iNbValidCaracters-0; i < myData.sCurrentText->len; i++)
	{
		g_print (" on charge la lettre '%c' (%d) tex:%d\n", myData.sCurrentText->str[i], i, bLoadTexture);
		c[0] = myData.sCurrentText->str[i];
		
		pChar = g_new0 (CDChar, 1);
		pChar->c = c[0];
		pChar->iInitialX = myData.iTextWidth/2 + iOffsetX;  // il part du coin haut droit.
		pChar->iInitialY = g_pMainDock->iCurrentHeight/2;  // en bas.
		pChar->iCurrentX = pChar->iInitialX;
		pChar->iCurrentY = pChar->iInitialY;
		pChar->fRotationAngle = 10. * myConfig.iAppearanceDuration / iDeltaT;
		g_print (" on commence a x=%d\n", pChar->iInitialX);
		myData.pCharList = g_list_append (myData.pCharList, pChar);
		
		// on cree la surface.
		pSurface = cairo_dock_create_surface_from_text (c, pCairoContext, &myConfig.labelDescription, 1., &pChar->iWidth, &pChar->iHeight, &fTextXOffset, &fTextYOffset);
		if (g_pMainDock->bHorizontalDock)
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
	int i;
	for (i = myData.iNbValidCaracters; i < myData.sCurrentText->len && c != NULL; i ++)
	{
		g_print ("on efface '%c'\n", myData.sCurrentText->str[i]);
		c_prev = c->prev;
		pChar = c->data;
		
		myData.iTextWidth -= pChar->iWidth;
		cd_do_free_char (pChar);
		myData.pCharList = g_list_delete_link (myData.pCharList, c);  // detruit c.
		c = c_prev;
	}
	
	// on tronque la chaine de la meme maniere.
	g_string_truncate (myData.sCurrentText, myData.iNbValidCaracters);
	g_print (" -> '%s' (%d)\n", myData.sCurrentText->str, myData.iNbValidCaracters);
	
	// on remet a jour la hauteur du texte.
	myData.iTextHeight = 0;
	for (c = myData.pCharList; c != NULL; c = c->next)
	{
		pChar = c->data;
		myData.iTextHeight = MAX (myData.iTextHeight, pChar->iHeight);
	}
}
