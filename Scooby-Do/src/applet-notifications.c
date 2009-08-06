/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-draw.h"
#include "applet-icon-finder.h"
#include "applet-command-finder.h"
#include "applet-session.h"
#include "applet-listing.h"
#include "applet-notifications.h"


gboolean cd_do_render (gpointer pUserData, CairoContainer *pContainer, cairo_t *pCairoContext)
{
	if (pContainer != CAIRO_CONTAINER (g_pMainDock) || ! cd_do_session_is_running ())
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (pCairoContext)
	{
		cd_do_render_cairo (g_pMainDock, pCairoContext);
	}
	else
	{
		cd_do_render_opengl (g_pMainDock);
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


gboolean cd_do_update_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	if (! cd_do_session_is_running ())
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (myData.iMotionCount != 0)
	{
		myData.iMotionCount --;
		double f = (double) myData.iMotionCount / 10;
		cairo_dock_emit_motion_signal (CAIRO_DOCK (pContainer),
			f * myData.iPrevMouseX + (1-f) * myData.iMouseX,
			f * myData.iPrevMouseY + (1-f) * myData.iMouseY);
		*bContinueAnimation = TRUE;
	}
	
	if (pContainer != CAIRO_CONTAINER (g_pMainDock))
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
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
	else if (cd_do_session_is_waiting_for_input ())
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
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


/*gboolean cd_do_enter_container (gpointer pUserData, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	if (myData.sCurrentText == NULL || myData.bIgnoreIconState)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	cd_do_close_session ();
	
	*bStartAnimation = TRUE;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}*/


gboolean cd_do_check_icon_stopped (gpointer pUserData, Icon *pIcon)
{
	if (pIcon == myData.pCurrentIcon && ! myData.bIgnoreIconState)
	{
		g_print ("notre icone vient de se faire detruire\n");
		myData.pCurrentIcon = NULL;
		myData.pCurrentDock = NULL;
		
		// eventuellement emuler un TAB pour trouver la suivante ...
	}
	if (myData.pMatchingIcons != NULL)
	{
		myData.pMatchingIcons = g_list_remove (myData.pMatchingIcons, pIcon);
		if (myData.pCurrentMatchingElement && myData.pCurrentMatchingElement->data == pIcon)
			myData.pCurrentMatchingElement = NULL;
		if (myData.pCurrentApplicationToLoad && myData.pCurrentApplicationToLoad->data == pIcon)
		{
			myData.pCurrentApplicationToLoad = myData.pCurrentApplicationToLoad->next;
		}
	}
	
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static void _check_is_dock (gchar *cDockName, CairoDock *pDock, gpointer *data)
{
	Window xActiveWindow = GPOINTER_TO_INT (data[0]);
	if (GDK_WINDOW_XID (pDock->pWidget->window) == xActiveWindow)
		data[1] = GINT_TO_POINTER (1);
}
gboolean cd_do_check_active_dock (gpointer pUserData, Window *XActiveWindow)
{
	if (myData.sCurrentText == NULL || XActiveWindow == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	gpointer data[2] = {GINT_TO_POINTER (*XActiveWindow), 0};
	cairo_dock_foreach_docks ((GHFunc) _check_is_dock, data);
	
	if (data[1] == 0)
		gtk_window_present (GTK_WINDOW (g_pMainDock->pWidget));
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static void _place_menu (GtkMenu *menu,
	gint *x,
	gint *y,
	gboolean *push_in,
	gpointer user_data)
{
	/// gerer les docks verticaux ...
	*x = myData.pCurrentDock->iWindowPositionX + myData.pCurrentDock->iMouseX;
	*y = myData.pCurrentDock->iWindowPositionY;
	*push_in = TRUE;
}

gboolean cd_do_key_pressed (gpointer pUserData, CairoContainer *pContainer, guint iKeyVal, guint iModifierType, const gchar *string)
{
	if (myData.sCurrentText == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	const gchar *cKeyName = gdk_keyval_name (iKeyVal);  // gdk_keyval_to_unicode
	guint32 iUnicodeChar = gdk_keyval_to_unicode (iKeyVal);
	g_print ("+ cKeyName : %s (%c, %s)\n", cKeyName, iUnicodeChar, string);
	
	if (iKeyVal == GDK_Escape)  // on clot la session.
	{
		cairo_dock_show_xwindow (myData.iPreviouslyActiveWindow);
		cd_do_close_session ();
	}
	else if (iKeyVal == GDK_space && myData.iNbValidCaracters == 0)  // pas d'espace en debut de chaine.
	{
		// on rejette.
	}
	else if (iKeyVal >= GDK_Shift_L && iKeyVal <= GDK_Hyper_R)  // on n'ecrit pas les modificateurs.
	{
		// on rejette.
	}
	else if (iKeyVal == GDK_Menu)  // emulation du clic droit.
	{
		if (myData.bNavigationMode)
		{
			if (myData.pCurrentIcon != NULL)
			{
				myData.bIgnoreIconState = TRUE;
				cairo_dock_stop_icon_animation (myData.pCurrentIcon);  // car on va perdre le focus.
				myData.bIgnoreIconState = FALSE;
			}
			if (myData.pCurrentDock == NULL)
				myData.pCurrentDock = g_pMainDock;
			myData.pCurrentDock->bMenuVisible = TRUE;
			GtkWidget *menu = cairo_dock_build_menu (myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));
			gtk_widget_show_all (menu);
			gtk_menu_popup (GTK_MENU (menu),
				NULL,
				NULL,
				(GtkMenuPositionFunc) _place_menu,  // pour positionner le menu sur le dock plutot que sur la souris.
				NULL,
				1,
				gtk_get_current_event_time ());
		}
	}
	else if (iKeyVal == GDK_BackSpace)  // on efface la derniere lettre.
	{
		if (myData.iNbValidCaracters > 0)
		{
			g_print ("%d/%d (%s)\n", myData.iNbValidCaracters, myData.sCurrentText->len, myData.sCurrentText->str);
			if (myData.iNbValidCaracters == myData.sCurrentText->len)  // pas de completion en cours => on efface la derniere lettre tapee.
				myData.iNbValidCaracters --;
			
			// on efface les lettres precedentes jusqu'a la derniere position validee.
			cd_do_delete_invalid_caracters ();
			
			// on relance la recherche.
			if (myData.bNavigationMode)
			{
				if (myData.pCurrentIcon == NULL)  // sinon l'icone actuelle convient toujours.
					cd_do_search_current_icon (FALSE);
			}
			else
			{
				g_list_free (myData.pMatchingIcons);
				myData.pMatchingIcons = NULL;
				cd_do_search_matching_icons ();
				if (myData.pMatchingIcons == NULL && myData.sCurrentText->len > 0)  // on n'a trouve aucun programme, on cherche un fichier.
				{
					cd_do_find_matching_files ();
				}
				else  // on a trouve au moins un programme, on cache le listing des fichiers.
				{
					cd_do_hide_listing ();
				}
			}
			
			// on repositionne les caracteres et on anime tout ca.
			cd_do_launch_appearance_animation ();
		}
	}
	else if (iKeyVal == GDK_Tab)  // completion.
	{
		if (myData.iNbValidCaracters > 0)
		{
			gboolean bPrevious = iModifierType & GDK_SHIFT_MASK;
			if (myData.bNavigationMode)
			{
				// on cherche l'icone suivante.
				cd_do_search_current_icon (TRUE);  // pCurrentIcon peut etre NULL si elle s'est faite detruire pendant la recherche, auquel cas on cherchera juste normalement.
			}
			else 
			{
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
	}
	else if (iKeyVal == GDK_Return)
	{
		if (myData.bNavigationMode)
		{
			if (myData.pCurrentIcon != NULL && myData.pCurrentDock != NULL)
			{
				g_print ("on clique sur l'icone '%s' [%d, %d]\n", myData.pCurrentIcon->acName, iModifierType, GDK_SHIFT_MASK);
				
				myData.bIgnoreIconState = TRUE;
				if (iModifierType & GDK_MOD1_MASK)  // ALT
				{
					myData.bIgnoreIconState = TRUE;
					cairo_dock_stop_icon_animation (myData.pCurrentIcon);  // car aucune animation ne va la remplacer.
					myData.bIgnoreIconState = FALSE;
					cairo_dock_notify (CAIRO_DOCK_MIDDLE_CLICK_ICON, myData.pCurrentIcon, myData.pCurrentDock);
				}
				else if (iModifierType & GDK_CONTROL_MASK)  // CTRL
				{
					myData.bIgnoreIconState = TRUE;
					cairo_dock_stop_icon_animation (myData.pCurrentIcon);  // car on va perdre le focus.
					myData.bIgnoreIconState = FALSE;
					
					myData.pCurrentDock->bMenuVisible = TRUE;
					GtkWidget *menu = cairo_dock_build_menu (myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));
					gtk_widget_show_all (menu);
					gtk_menu_popup (GTK_MENU (menu),
						NULL,
						NULL,
						NULL,
						NULL,
						1,
						gtk_get_current_event_time ());
				}
				else if (myData.pCurrentIcon != NULL)
					cairo_dock_notify (CAIRO_DOCK_CLICK_ICON, myData.pCurrentIcon, myData.pCurrentDock, iModifierType);
				if (myData.pCurrentIcon != NULL)
					cairo_dock_start_icon_animation (myData.pCurrentIcon, myData.pCurrentDock);
				myData.bIgnoreIconState = FALSE;
				myData.pCurrentIcon = NULL;  // sinon on va interrompre l'animation en fermant la session.
			}
		}
		else  // mode recherche.
		{
			if (myData.pMatchingIcons != NULL)  // on a une appli a lancer.
			{
				Icon *pIcon = (myData.pCurrentMatchingElement ? myData.pCurrentMatchingElement->data : myData.pMatchingIcons->data);
				cairo_dock_launch_command (pIcon->acCommand);
			}
			else if (myData.pListing && myData.pListing->pEntries)  // pas d'appli mais une entree => on l'execute.
			{
				CDEntry *pEntry = &myData.pListing->pEntries[myData.pListing->iCurrentEntry];
				g_print ("on valide l'entree '%s'\n", pEntry->cPath);
				if (pEntry->execute)
					pEntry->execute (pEntry);
			}
			else if (myData.iNbValidCaracters > 0)  // pas de fichier mais du texte => on l'execute tel quel.
			{
				gchar *cCommand = g_strdup_printf ("%s/calc.sh '%s'", MY_APPLET_SHARE_DATA_DIR, myData.sCurrentText->str);
				gchar *cResult = cairo_dock_launch_command_sync (cCommand);
				g_free (cCommand);
				if (cResult != NULL && strcmp (cResult, "0") != 0)
				{
					g_print ("result : '%s'\n", cResult);
					GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
					gtk_clipboard_set_text (pClipBoard, cResult, -1);
					Icon *pIcon = cairo_dock_get_dialogless_icon ();
					cairo_dock_show_temporary_dialog_with_icon (D_("The value %s has been copied into the clipboard."),
						pIcon,
						CAIRO_CONTAINER (g_pMainDock),
						3000,
						MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
						cResult);
					double fResult = atof (cResult);
				}
				else  // le calcul n'a rien donne, on execute sans chercher.
				{
					g_print ("on execute '%s'\n", myData.sCurrentText->str);
					cairo_dock_launch_command (myData.sCurrentText->str);
				}
				g_free (cResult);
			}
		}
		cd_do_close_session ();
	}
	else if (iKeyVal == GDK_Left || iKeyVal == GDK_Right || iKeyVal == GDK_Up || iKeyVal == GDK_Down)
	{
		if (myData.bNavigationMode)
		{
			if (iKeyVal == GDK_Up)
			{
				if (myData.pCurrentIcon != NULL && myData.pCurrentIcon->pSubDock != NULL)
				{
					g_print ("on monte dans le sous-dock %s\n", myData.pCurrentIcon->acName);
					Icon *pIcon = cairo_dock_get_first_icon (myData.pCurrentIcon->pSubDock->icons);
					cd_do_change_current_icon (pIcon, myData.pCurrentIcon->pSubDock);
				}
			}
			else if (iKeyVal == GDK_Down)
			{
				if (myData.pCurrentDock != NULL && myData.pCurrentDock->iRefCount > 0)
				{
					CairoDock *pParentDock = NULL;
					Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (myData.pCurrentDock, &pParentDock);
					if (pPointingIcon != NULL)
					{
						g_print ("on redescend dans le dock parent via %s\n", pPointingIcon->acName);
						cd_do_change_current_icon (pPointingIcon, pParentDock);
					}
				}
			}
			else if (iKeyVal == GDK_Left)
			{
				if (myData.pCurrentDock == NULL)  // on initialise le deplacement.
				{
					myData.pCurrentDock = g_pMainDock;
					int n = g_list_length (g_pMainDock->icons);
					if (n > 0)
					{
						myData.pCurrentIcon =  g_list_nth_data (g_pMainDock->icons, (n-1) / 2);
						if (CAIRO_DOCK_IS_SEPARATOR (myData.pCurrentIcon) && n > 1)
							myData.pCurrentIcon = g_list_nth_data (g_pMainDock->icons, (n+1) / 2);
					}
				}
				if (myData.pCurrentDock->icons != NULL)
				{
					Icon *pPrevIcon = cairo_dock_get_previous_icon (myData.pCurrentDock->icons, myData.pCurrentIcon);
					if (CAIRO_DOCK_IS_SEPARATOR (pPrevIcon))
						pPrevIcon = cairo_dock_get_previous_icon (myData.pCurrentDock->icons, pPrevIcon);
					if (pPrevIcon == NULL)  // pas trouve ou bien 1ere icone.
					{
						pPrevIcon = cairo_dock_get_last_icon (myData.pCurrentDock->icons);
					}
					
					g_print ("on se deplace a gauche sur %s\n", pPrevIcon ? pPrevIcon->acName : "none");
					cd_do_change_current_icon (pPrevIcon, myData.pCurrentDock);
				}
			}
			else  // Gdk_Right.
			{
				if (myData.pCurrentDock == NULL)  // on initialise le deplacement.
				{
					myData.pCurrentDock = g_pMainDock;
					int n = g_list_length (g_pMainDock->icons);
					if (n > 0)
					{
						myData.pCurrentIcon =  g_list_nth_data (g_pMainDock->icons, (n-1) / 2);
						if (CAIRO_DOCK_IS_SEPARATOR (myData.pCurrentIcon) && n > 1)
							myData.pCurrentIcon = g_list_nth_data (g_pMainDock->icons, (n+1) / 2);
					}
				}
				if (myData.pCurrentDock->icons != NULL)
				{
					Icon *pNextIcon = cairo_dock_get_next_icon (myData.pCurrentDock->icons, myData.pCurrentIcon);
					if (CAIRO_DOCK_IS_SEPARATOR (pNextIcon))
						pNextIcon = cairo_dock_get_next_icon (myData.pCurrentDock->icons, pNextIcon);
					if (pNextIcon == NULL)  // pas trouve ou bien 1ere icone.
					{
						pNextIcon = cairo_dock_get_first_icon (myData.pCurrentDock->icons);
					}
					
					g_print ("on se deplace a gauche sur %s\n", pNextIcon ? pNextIcon->acName : "none");
					cd_do_change_current_icon (pNextIcon, myData.pCurrentDock);
				}
			}
		}
		else if (myData.pMatchingIcons != NULL)
		{
			cd_do_select_previous_next_matching_icon (iKeyVal == GDK_Right || iKeyVal == GDK_Down);
		}
		else if (myData.pListing != NULL && myData.pListing->pEntries != NULL)
		{
			if (iKeyVal == GDK_Down)
			{
				cd_do_select_prev_next_entry_in_listing (TRUE);  // next
			}
			else if (iKeyVal == GDK_Up)
			{
				cd_do_select_prev_next_entry_in_listing (FALSE);  // previous
			}
			else if (iKeyVal == GDK_Right)
			{
				cd_do_show_current_sub_listing ();
			}
			else if (iKeyVal == GDK_Left)
			{
				cd_do_show_previous_listing ();
			}
		}
	}
	else if (iKeyVal == GDK_Page_Down || iKeyVal == GDK_Page_Up || iKeyVal == GDK_Home || iKeyVal == GDK_End)
	{
		if (myData.bNavigationMode)
		{
			if (myData.pCurrentDock == NULL)  // on initialise le deplacement.
				myData.pCurrentDock = g_pMainDock;
			Icon *pIcon = (iKeyVal == GDK_Page_Up || iKeyVal == GDK_Home ? cairo_dock_get_first_icon (myData.pCurrentDock->icons) : cairo_dock_get_last_icon (myData.pCurrentDock->icons));
			g_print ("on se deplace a l'extremite sur %s\n", pIcon ? pIcon->acName : "none");
			cd_do_change_current_icon (pIcon, myData.pCurrentDock);
		}
		else if (myData.pListing != NULL)
		{
			if (iKeyVal == GDK_Page_Down || iKeyVal == GDK_Page_Up)
				cd_do_select_prev_next_page_in_listing (iKeyVal == GDK_Page_Down);  // TRUE <=> next page
			else
				cd_do_select_last_first_entry_in_listing (iKeyVal == GDK_End);  // TRUE <=> last entry.
		}
	}
	else if (iKeyVal >= GDK_F1 && iKeyVal <= GDK_F9)
	{
		if (! myData.bNavigationMode && myData.pListing != NULL && GTK_WIDGET_VISIBLE (myData.pListing->container.pWidget))
		{
			g_print ("modification du filtre : option n°%d", iKeyVal - GDK_F1);
			cd_do_activate_filter_option (iKeyVal - GDK_F1);
			cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pListing));
		}
	}
	else if (string)  /// utiliser l'unichar ...
	{
		g_print ("string:'%s'\n", string);
		if ((iModifierType & GDK_CONTROL_MASK) && iUnicodeChar == 'v')  // CTRL+v
		{
			g_print ("CTRL+v\n");
			GtkClipboard *pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
			gchar *cText = gtk_clipboard_wait_for_text (pClipBoard);  // la main loop s'execute pendant ce temps.
			if (cText != NULL)
			{
				g_print ("clipboard : '%s'\n", cText);
				gchar *str = strchr (cText, '\r');
				if (str)
					*str = '\0';
				str = strchr (cText, '\n');
				if (str)
					*str = '\0';
				g_string_append (myData.sCurrentText, cText);
				cd_do_load_pending_caracters ();
				cd_do_launch_appearance_animation ();
				myData.iNbValidCaracters = myData.sCurrentText->len;  // cela valide le texte colle.
			}
			return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		}
		
		// on rajoute la lettre au mot
		g_string_append_c (myData.sCurrentText, *string);
		myData.iNbValidCaracters = myData.sCurrentText->len;  // l'utilisateur valide la nouvelle lettre ainsi que celles precedemment ajoutee par completion.
		
		if (myData.bNavigationMode)  // on cherche un lanceur correspondant.
		{
			cd_do_search_current_icon (FALSE);
		}
		else  // on cherche la liste des icones qui correspondent.
		{
			if (! (myData.bFoundNothing || (myData.pListing && myData.pListing->pEntries)))  // on n'est pas deja dans une recherche de fichiers
			{
				cd_do_search_matching_icons ();
			}
			
			// si on n'a trouve aucun lanceur, on lance la recherche de fichiers.
			if (myData.pMatchingIcons == NULL && ! myData.bFoundNothing)
			{
				// on cherche un fichier correspondant.
				cd_do_find_matching_files ();
			}
		}
		
		// on rajoute une surface/texture pour la/les nouvelle(s) lettre(s).
		myData.iNbValidCaracters --;  // le nouveau caractere n'est pas encore charge.
		cd_do_load_pending_caracters ();
		myData.iNbValidCaracters ++;
		
		// on repositionne les caracteres et on anime tout ca.
		cd_do_launch_appearance_animation ();
	}
	
	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
}


#define _cd_do_on_shortkey(bNewModeNav) \
	if (myData.sCurrentText == NULL) { \
		myData.bNavigationMode = bNewModeNav; \
		cd_do_open_session (); } \
	else { \
		cairo_dock_show_xwindow (myData.iPreviouslyActiveWindow);\
		cd_do_close_session (); \
		if (myData.bNavigationMode != bNewModeNav) { \
			cd_do_open_session (); \
			myData.bNavigationMode = bNewModeNav; } }
	
void cd_do_on_shortkey_nav (const char *keystring, gpointer data)
{
	_cd_do_on_shortkey (TRUE);
}

void cd_do_on_shortkey_search (const char *keystring, gpointer data)
{
	_cd_do_on_shortkey (FALSE);
}
