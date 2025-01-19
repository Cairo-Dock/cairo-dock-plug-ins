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
#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-icon-finder.h"
#include "applet-search.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-notifications.h"

gboolean cd_do_render (gpointer pUserData, GldiContainer *pContainer, cairo_t *pCairoContext)
{
	g_return_val_if_fail (!cd_do_session_is_off (), GLDI_NOTIFICATION_LET_PASS);
	
	if (pCairoContext != NULL)
	{
		cd_do_render_cairo (g_pMainDock, pCairoContext);
	}
	else
	{
		cd_do_render_opengl (g_pMainDock);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_do_update_container (gpointer pUserData, GldiContainer *pContainer, gboolean *bContinueAnimation)
{
	g_return_val_if_fail (!cd_do_session_is_off (), GLDI_NOTIFICATION_LET_PASS);
	
	int iDeltaT = cairo_dock_get_animation_delta_t (pContainer);
	if (cd_do_session_is_closing ())
	{
		//\___________________ animation de fermeture de la session (disparition des lettres ou du prompt).
		myData.iCloseTime -= iDeltaT;
		if (myData.iCloseTime <= 0)
			cd_do_exit_session ();
		else
			*bContinueAnimation = TRUE;
		cairo_dock_redraw_container (pContainer);
	}
	else if (cd_do_session_is_running ())
	{
		if (myData.pCharList == NULL)
		{
			//\___________________ animation du prompt.
			if (myData.iPromptAnimationCount > -1)
			{
				myData.iPromptAnimationCount ++;
				*bContinueAnimation = TRUE;
			}
		}
		else
		{
			//\___________________ animation des caracteres : deplacement vers la gauche/droite et apparition.
			myData.iAppearanceTime -= iDeltaT;
			if (myData.iAppearanceTime < 0)
				myData.iAppearanceTime = 0;
			else
				*bContinueAnimation = TRUE;
			
			double f = (double) myData.iAppearanceTime / myConfig.iAppearanceDuration;
			CDChar *pChar;
			GList *c;
			for (c = myData.pCharList; c != NULL; c = c->next)
			{
				pChar = c->data;
				pChar->iCurrentX = f * pChar->iInitialX + (1-f) * pChar->iFinalX;
				pChar->iCurrentY = f * pChar->iInitialY + (1-f) * pChar->iFinalY;
				
				if (pChar->fRotationAngle != 0)
				{
					pChar->fRotationAngle -= 10.;  // 360. * iDeltaT / myConfig.iAppearanceDuration;
					if (pChar->fRotationAngle < 0)
						pChar->fRotationAngle = 0;
				}
			}
		}
		
		//\___________________ animation du decalage des icones d'appli correspondantes.
		if (myData.iMatchingGlideCount != 0)
		{
			myData.iMatchingGlideCount --;
			double f = (double) myData.iMatchingGlideCount / 10;
			myData.iCurrentMatchingOffset = myData.iPreviousMatchingOffset * f + myData.iMatchingAimPoint * (1 - f);
		}
		
		cairo_dock_redraw_container (pContainer);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


/*gboolean cd_do_enter_container (gpointer pUserData, GldiContainer *pContainer, gboolean *bStartAnimation)
{
	if (myData.sCurrentText == NULL || myData.bIgnoreIconState)
		return GLDI_NOTIFICATION_LET_PASS;
	
	cd_do_close_session ();
	
	*bStartAnimation = TRUE;
	return GLDI_NOTIFICATION_LET_PASS;
}*/


static void _check_dock_is_active (gchar *cDockName, CairoDock *pDock, gboolean *data)
{
	if (gldi_container_is_active (CAIRO_CONTAINER (pDock)))
		*data = TRUE;
}
gboolean cd_do_check_active_dock (gpointer pUserData, GldiWindowActor *actor)
{
	g_return_val_if_fail (cd_do_session_is_running (), GLDI_NOTIFICATION_LET_PASS);
	
	if (myData.sCurrentText == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	gboolean bDockIsActive = FALSE;
	gldi_docks_foreach ((GHFunc) _check_dock_is_active, &bDockIsActive);
	
	if (! bDockIsActive)
	{
		gtk_window_present (GTK_WINDOW (g_pMainDock->container.pWidget));
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_do_key_pressed (gpointer pUserData, GldiContainer *pContainer, guint iKeyVal, guint iModifierType, const gchar *string)
{
	g_return_val_if_fail (cd_do_session_is_running (), GLDI_NOTIFICATION_LET_PASS);

	if (myData.sCurrentText == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	const gchar *cKeyName = gdk_keyval_name (iKeyVal);
	guint32 iUnicodeChar = gdk_keyval_to_unicode (iKeyVal);
	cd_debug ("+ cKeyName : %s (%c, %s)", cKeyName, iUnicodeChar, string);

	if (iKeyVal == GDK_KEY_Escape)  // on clot la session.
	{
		cd_do_close_session ();
	}
	else if (iKeyVal == GDK_KEY_space && myData.iNbValidCaracters == 0)  // pas d'espace en debut de chaine.
	{
		// on rejette.
	}
	else if (iKeyVal >= GDK_KEY_Shift_L && iKeyVal <= GDK_KEY_Hyper_R)  // on n'ecrit pas les modificateurs.
	{
		// on rejette.
	}
	else if (iKeyVal == GDK_KEY_BackSpace)  // on efface la derniere lettre.
	{
		if (myData.iNbValidCaracters > 0)
		{
			cd_debug ("on efface la derniere lettre de %s %d/%d", myData.sCurrentText->str, myData.iNbValidCaracters, myData.sCurrentText->len);
			if (myData.iNbValidCaracters == myData.sCurrentText->len)  // pas de completion en cours => on efface la derniere lettre tapee.
				myData.iNbValidCaracters --;
			
			// on efface les lettres precedentes jusqu'a la derniere position validee.
			cd_do_delete_invalid_caracters ();
			
			// on relance la recherche.
			if (myData.pListingHistory == NULL)  // recherche principale.
			{
				g_list_free (myData.pMatchingIcons);
				myData.pMatchingIcons = NULL;
				cd_do_search_matching_icons ();
				if (myData.pMatchingIcons == NULL && myData.sCurrentText->len > 0)  // on n'a trouve aucun programme, on cherche des entrees.
				{
					if (myData.iSidLoadExternAppliIdle != 0)
					{
						g_source_remove (myData.iSidLoadExternAppliIdle);
						myData.iSidLoadExternAppliIdle = 0;
					}
					cd_do_launch_all_backends ();
				}
				else  // on a trouve au moins un programme, on cache le listing des fichiers.
				{
					
					cd_do_hide_listing ();
				}
			}
			else  // sous-recherche => on filtre.
			{
				cd_do_filter_current_listing ();
			}
			
			
			// on repositionne les caracteres et on anime tout ca.
			cd_do_launch_appearance_animation ();
		}
	}
	else if (iKeyVal == GDK_KEY_Tab)  // completion.
	{
		if (myData.iNbValidCaracters > 0)
		{
			gboolean bPrevious = iModifierType & GDK_SHIFT_MASK;
			if (myData.pMatchingIcons != NULL)
			{
				cd_do_select_previous_next_matching_icon (!bPrevious);
			}
			else
			{
				// faire un truc ?...
			}
		}
	}
	else if (iKeyVal == GDK_KEY_Return)
	{
		cd_debug ("Enter (%s)", myData.cSearchText);
		if (myData.pMatchingIcons != NULL)  // on a une appli a lancer.
		{
			Icon *pIcon = (myData.pCurrentMatchingElement ? myData.pCurrentMatchingElement->data : myData.pMatchingIcons->data);
			cairo_dock_launch_command_full (pIcon->cCommand, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
		}
		else if (myData.pListing && myData.pListing->pCurrentEntry)  // pas d'appli mais une entree => on l'execute.
		{
			CDEntry *pEntry = myData.pListing->pCurrentEntry->data;
			cd_debug ("on valide l'entree '%s ; %s'", pEntry->cName, pEntry->cPath);
			if (pEntry->execute)
				pEntry->execute (pEntry);
			else
				return GLDI_NOTIFICATION_INTERCEPT;
		}
		else if (myData.iNbValidCaracters > 0)  // pas d'entree mais du texte => on l'execute tel quel.
		{
			cd_debug ("on execute '%s'", myData.sCurrentText->str);
			cairo_dock_launch_command_full (myData.sCurrentText->str, NULL, GLDI_LAUNCH_GUI | GLDI_LAUNCH_SLICE);
		}
		
		if (!(iModifierType & GDK_CONTROL_MASK) && !(iModifierType & GDK_MOD1_MASK) && !(iModifierType & GDK_SHIFT_MASK))
			cd_do_close_session ();
	}
	else if (iKeyVal == GDK_KEY_Left || iKeyVal == GDK_KEY_Right || iKeyVal == GDK_KEY_Up || iKeyVal == GDK_KEY_Down)
	{
		if (myData.pMatchingIcons != NULL)
		{
			cd_do_select_previous_next_matching_icon (iKeyVal == GDK_KEY_Right || iKeyVal == GDK_KEY_Down);
		}
		else if (myData.pListing != NULL && myData.pListing->pEntries != NULL)
		{
			if (iKeyVal == GDK_KEY_Down)
			{
				cd_do_select_prev_next_entry_in_listing (TRUE);  // next
			}
			else if (iKeyVal == GDK_KEY_Up)
			{
				cd_do_select_prev_next_entry_in_listing (FALSE);  // previous
			}
			else if (iKeyVal == GDK_KEY_Right)
			{
				cd_do_show_current_sub_listing ();
			}
			else if (iKeyVal == GDK_KEY_Left)
			{
				cd_do_show_previous_listing ();
			}
		}
	}
	else if (iKeyVal == GDK_KEY_Page_Down || iKeyVal == GDK_KEY_Page_Up || iKeyVal == GDK_KEY_Home || iKeyVal == GDK_KEY_End)
	{
		if (myData.pListing != NULL)
		{
			if (iKeyVal == GDK_KEY_Page_Down || iKeyVal == GDK_KEY_Page_Up)
				cd_do_select_prev_next_page_in_listing (iKeyVal == GDK_KEY_Page_Down);  // TRUE <=> next page
			else
				cd_do_select_last_first_entry_in_listing (iKeyVal == GDK_KEY_End);  // TRUE <=> last entry.
		}
	}
	else if (iKeyVal >= GDK_KEY_F1 && iKeyVal <= GDK_KEY_F9)
	{
		if (myData.pListing != NULL && gldi_container_is_visible (CAIRO_CONTAINER (myData.pListing)))
		{
			cd_debug ("modification du filtre : option n°%d", iKeyVal - GDK_KEY_F1);
			cd_do_activate_filter_option (iKeyVal - GDK_KEY_F1);
			cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
		}
	}
	else if (string)  /// utiliser l'unichar ...
	{
		cd_debug ("string:'%s'", string);
		guint iNbNewChar = 0;
		if ((iModifierType & GDK_CONTROL_MASK) && iUnicodeChar == 'v')  // CTRL+v
		{
			cd_debug ("CTRL+v\n");
			GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
			gchar *cText = gtk_clipboard_wait_for_text (pClipBoard);  // la main loop s'execute pendant ce temps.
			if (cText != NULL)
			{
				cd_debug ("clipboard : '%s'", cText);
				iNbNewChar = strlen (cText);  /// penser a l'UTF-8 ...
				gchar *str = strchr (cText, '\r');
				if (str)
					*str = '\0';
				str = strchr (cText, '\n');
				if (str)
					*str = '\0';
				g_string_append (myData.sCurrentText, cText);
				cd_do_load_pending_caracters ();
				cd_do_launch_appearance_animation ();
				myData.iNbValidCaracters = myData.sCurrentText->len;  // cela valide le texte colle ainsi que les lettres precedemment ajoutee par completion.
			}
		}
		else  // on rajoute la lettre au mot
		{
			iNbNewChar = 1;
			g_string_append_c (myData.sCurrentText, *string);
			myData.iNbValidCaracters = myData.sCurrentText->len;  // l'utilisateur valide la nouvelle lettre ainsi que celles precedemment ajoutee par completion.
		}
		

		// on cherche la liste des icones qui correspondent.
		if (myData.pListingHistory == NULL)
		{
			//if (! (myData.bFoundNothing || (myData.pListing && myData.pListing->pEntries)))  // on n'est pas deja dans une recherche de fichiers
			if (myData.iNbValidCaracters == iNbNewChar || myData.pMatchingIcons != NULL)  // 1er ajout de lettre ou precedente recherche d'icones fructueuse => on remet ca.
			{
				cd_do_search_matching_icons ();
			}
			
			// si on n'a trouve aucun lanceur, on lance la recherche dans les backends.
			if (myData.pMatchingIcons == NULL)
			{
				cd_do_launch_all_backends ();
			}
		}
		else
		{
			cd_do_filter_current_listing ();
		}
		
		// on rajoute une surface/texture pour la/les nouvelle(s) lettre(s).
		myData.iNbValidCaracters -= iNbNewChar;  // le nouveau caractere n'est pas encore charge.
		cd_do_load_pending_caracters ();
		myData.iNbValidCaracters += iNbNewChar;
		
		// on repositionne les caracteres et on anime tout ca.
		cd_do_launch_appearance_animation ();
	}
	
	return GLDI_NOTIFICATION_INTERCEPT;
}


void cd_do_on_shortkey_search (const char *keystring, gpointer data)
{
	if (! cd_do_session_is_running ())
	{
		cd_do_open_session ();
	}
	else
	{
		cd_do_close_session ();
	}
}
