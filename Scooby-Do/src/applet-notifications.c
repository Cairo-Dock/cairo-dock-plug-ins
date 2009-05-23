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
		cairo_dock_redraw_container (pContainer);  // definir une aire plus precisement (pour cairo) ...
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
		
		cairo_dock_redraw_container (pContainer);  // definir une aire plus precisement (pour cairo) ...
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
	else if (iKeyVal == GDK_BackSpace)  // on efface la derniere lettre.
	{
		if (myData.iNbValidCaracters > 0)
		{
			g_print ("%d/%d\n", myData.iNbValidCaracters, myData.sCurrentText->len);
			if (myData.iNbValidCaracters == myData.sCurrentText->len)  // pas de completion en cours => on efface la derniere lettre tapee.
				myData.iNbValidCaracters --;
			
			// on efface les lettres precedentes jusqu'a la derniere position validee.
			cd_do_delete_invalid_caracters ();
			
			// on cherche l'icone courante si aucune.
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
			}
			
			// on repositionne les caracteres et on anime tout ca.
			cd_do_launch_appearance_animation ();
			if (myData.iNbValidCaracters == 0)  // plus de caracteres => plus d'animation immediatement, donc on force une fois le redessin.
				cairo_dock_redraw_container (pContainer);
		}
	}
	else if (iKeyVal == GDK_Tab)  // completion.
	{
		if (myData.iNbValidCaracters > 0)  // pCurrentIcon peut etre NULL si elle s'est faite detruire pendant la recherche, auquel cas on cherchera juste normalement.
		{
			gboolean bPrevious = iModifierType & GDK_SHIFT_MASK;
			
			// on cherche l'icone suivante.
			cd_do_search_current_icon (TRUE);
			
			//if (myData.pCurrentIcon == NULL)
			{
				if (myData.completion)
				{
					gchar *tmp = g_new0 (gchar, myData.iNbValidCaracters+1);
					strncpy (tmp, myData.sCurrentText->str, myData.iNbValidCaracters);
					gchar *new_prefix = NULL;
					GList *pPossibleItems = g_completion_complete (myData.completion,
						tmp,
						&new_prefix);
					if (new_prefix != NULL)
					{
						myData.iNbValidCaracters = strlen (new_prefix);
						g_print ("myData.iNbValidCaracters <- %d\n", myData.iNbValidCaracters);
					}
					if (pPossibleItems != NULL)
					{
						gchar *item;
						GList *it;
						g_print ("on cherche %s => on a le choix entre :\n", tmp);
						for (it = pPossibleItems; it != NULL; it = it->next)
						{
							item = it->data;
							g_print (" - %s\n", item);
							if (g_ascii_strncasecmp (myData.sCurrentText->str, item, myData.sCurrentText->len) == 0)
							{
								g_print ("  on pointe actuellement sur %s\n", item);
								break ;
							}
						}
						if (it == NULL || it->next == NULL)  // pas trouve ou dernier.
							item = pPossibleItems->data;
						else
							item = it->next->data;
						g_print ("  --> on complete maintenant avec %s\n", item);
						
						// on effacer les anciens caracteres automatiques, et on charge les nouveaux.
						cd_do_delete_invalid_caracters ();
						
						g_string_assign (myData.sCurrentText, item);
						
						cd_do_load_pending_caracters ();
						
						// on repositionne les caracteres et on anime tout ca.
						cd_do_launch_appearance_animation ();
					}
					g_free (new_prefix);
					g_free (tmp);
				}
			}
		}
	}
	else if (iKeyVal == GDK_Return)
	{
		// on lance soit l'icone soit la commande
		if (myData.pCurrentDock == NULL)
			myData.pCurrentDock = g_pMainDock;
		if (myData.pCurrentIcon != NULL || myData.iNbValidCaracters == 0)
		{
			if (myData.pCurrentIcon != NULL)
				g_print ("on valide '%s' (icone %s) [%d, %d]\n", myData.pCurrentIcon->acCommand, myData.pCurrentIcon->acName, iModifierType, GDK_SHIFT_MASK);
			
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
		else if (myData.iNbValidCaracters > 0)
		{
			g_print ("on valide '%s'\n", myData.sCurrentText->str);
			gchar *cFile = g_strdup_printf ("%s/%s", g_getenv ("HOME"), myData.sCurrentText->str);
			if (g_file_test (cFile, G_FILE_TEST_EXISTS))
				cairo_dock_fm_launch_uri (cFile);
			else
				cairo_dock_launch_command (myData.sCurrentText->str);
			g_free (cFile);
		}
		cd_do_close_session ();
	}
	else if (iKeyVal == GDK_Left || iKeyVal == GDK_Right || iKeyVal == GDK_Up || iKeyVal == GDK_Down)
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
		else
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
	else if (iKeyVal == GDK_Page_Down || iKeyVal == GDK_Page_Up)
	{
		if (myData.pCurrentDock == NULL)  // on initialise le deplacement.
			myData.pCurrentDock = g_pMainDock;
		Icon *pIcon = (iKeyVal == GDK_Page_Up ? cairo_dock_get_first_icon (myData.pCurrentDock->icons) : cairo_dock_get_last_icon (myData.pCurrentDock->icons));
		g_print ("on se deplace a l'extremite sur %s\n", pIcon ? pIcon->acName : "none");
		cd_do_change_current_icon (pIcon, myData.pCurrentDock);
	}
	else if (string)  /// utiliser l'unichar ...
	{
		// on rajoute la lettre au mot
		g_string_append_c (myData.sCurrentText, *string);
		myData.iNbValidCaracters = myData.sCurrentText->len;  // l'utilisateur valide la nouvelle lettre ainsi que celles precedemment ajoutee par completion.
		
		if (myData.bNavigationMode)  // on cherche un lanceur correspondant.
		{
			cd_do_search_current_icon (FALSE);
		}
		else  // on cherche la liste des icones qui correspondent.
		{
			cd_do_search_matching_icons ();
		}
		
		// si on a trouve aucun lanceur, on essaye l'autocompletion.
		if (myData.pCurrentIcon == NULL)
		{
			// on cherche un programme correspondant, cf GMenu.
			cd_do_update_completion (myData.sCurrentText->str);
			
			if (myData.completion)
			{
				gchar *cCompletedString = NULL;
				g_completion_complete_utf8 (myData.completion,
					myData.sCurrentText->str,
					&cCompletedString);
				if (cCompletedString != NULL)
				{
					g_print ("on a pu completer la chaine => %s\n", cCompletedString);
					g_string_assign (myData.sCurrentText, cCompletedString);
				}
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


#define _cd_do_on_shortkey(...) \
	if (myData.sCurrentText == NULL) \
		cd_do_open_session (); \
	else \
		cd_do_close_session ();

void cd_do_on_shortkey_nav (const char *keystring, gpointer data)
{
	myData.bNavigationMode = TRUE;
	_cd_do_on_shortkey ();
}

void cd_do_on_shortkey_search (const char *keystring, gpointer data)
{
	myData.bNavigationMode = FALSE;
	_cd_do_on_shortkey ();
}
