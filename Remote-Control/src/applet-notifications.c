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

#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include "applet-struct.h"
#include "applet-icon-finder.h"
#include "applet-session.h"
#include "applet-notifications.h"

#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION > 20)
#include <gdk/gdkkeysyms-compat.h>
#endif

#define _alpha_prompt(k,n) .6*cos (G_PI/2*fabs ((double) ((k % (2*n)) - n) / n))

const int s_iNbPromptAnimationSteps = 40;

static void cd_do_simulate_click (GldiContainer *pContainer, Icon *pIcon, int iModifierType)
{
	g_return_if_fail (pIcon != NULL);
	myData.bIgnoreClick = TRUE;  // -> ignore the "click" notification, which would close the session and remove the notification while in the loop. the caller of this method should ensure to close the session after.
	gldi_object_notify (pContainer, NOTIFICATION_CLICK_ICON, pIcon, pContainer, iModifierType);
	myData.bIgnoreClick = FALSE;
}

static inline int _orient_arrow (GldiContainer *pContainer, int iKeyVal)
{
	switch (iKeyVal)
	{
		case GDK_Up :
			if (pContainer->bIsHorizontal)
			{
				if (!pContainer->bDirectionUp)
					iKeyVal = GDK_Down;
			}
			else
			{
				iKeyVal = GDK_Left;
			}
		break;
		
		case GDK_Down :
			if (pContainer->bIsHorizontal)
			{
				if (!pContainer->bDirectionUp)
					iKeyVal = GDK_Up;
			}
			else
			{
				iKeyVal = GDK_Right;
			}
		break;
		
		case GDK_Left :
			if (!pContainer->bIsHorizontal)
			{
				if (pContainer->bDirectionUp)
					iKeyVal = GDK_Up;
				else
					iKeyVal = GDK_Down;
			}
		break;
		
		case GDK_Right :
			if (!pContainer->bIsHorizontal)
			{
				if (pContainer->bDirectionUp)
					iKeyVal = GDK_Down;
				else
					iKeyVal = GDK_Up;
			}
		break;
		default:
		break;

	}
	return iKeyVal;
}
static void _find_next_dock (CairoDock *pDock, gpointer *data)
{
	if (data[3] == NULL)  // first root dock in the list.
		data[3] = pDock;
	if (data[0] == pDock)  // this dock is the current one, we'll take the next one.
		data[2] = GINT_TO_POINTER (TRUE);
	else if (data[2])  // we take this one.
		data[1] = pDock;
}
static void _activate_nth_icon (guint iKeyVal, guint iModifierType)  // iKeyVal is already in the correct interval.
{
	cd_debug ("%s (%d)", __func__, iKeyVal);
	// get the index of the icon: we want "1" to be the first icon, because it's more natural and it follows the keyboard layout. So "0" will actually be the 10th icon.
	int iIndex;  // from 0
	if (iKeyVal >= GDK_0 && iKeyVal <= GDK_9)
	{
		if (iKeyVal == GDK_0)
			iIndex = 9;
		else
			iIndex = iKeyVal - GDK_1;
	}
	else
	{
		if (iKeyVal == GDK_KP_0)
			iIndex = 9;
		else
			iIndex = iKeyVal - GDK_KP_1;
	}
	cd_debug ("click on %d", iIndex);
	// retrieve the nth icon in the current dock.
	int n = 0;
	Icon *pNthIcon = NULL, *pIcon;
	GList *ic;
	for (ic = myData.pCurrentDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (! CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		{
			if (n == iIndex)
			{
				pNthIcon = pIcon;
				break;
			}
			n ++;
		}
	}

	// execute the icon directly.
	if (pNthIcon != NULL)
	{
		cd_debug ("click on %s", pNthIcon->cName);
		cd_do_simulate_click (CAIRO_CONTAINER (myData.pCurrentDock), pNthIcon, iModifierType);

		gldi_icon_start_animation (pNthIcon);
		myData.bIgnoreIconState = FALSE;
		if (pNthIcon == myData.pCurrentIcon)
			myData.pCurrentIcon = NULL;  // sinon on va interrompre l'animation en fermant la session.

		cd_do_close_session ();
	}
}

gboolean cd_do_key_pressed (gpointer pUserData, GldiContainer *pContainer, guint iKeyVal, guint iModifierType, const gchar *string, int iKeyCode)
{
	g_return_val_if_fail (cd_do_session_is_running (), GLDI_NOTIFICATION_LET_PASS);
	g_return_val_if_fail (myData.pCurrentDock != NULL, GLDI_NOTIFICATION_LET_PASS);
	
	const gchar *cKeyName = gdk_keyval_name (iKeyVal);
	guint32 iUnicodeChar = gdk_keyval_to_unicode (iKeyVal);
	cd_debug ("+ cKeyName : %s (%c, %s, %d)", cKeyName, iUnicodeChar, string, iKeyCode);
	
	if (myData.sCurrentText->len == 0)
	{
		GdkKeymapKey *keys = NULL;
		guint *keyvals = NULL;
		int i, n_entries = 0;
		int iKeyVal2;
		gdk_keymap_get_entries_for_keycode (gdk_keymap_get_default (),
			iKeyCode,
			&keys,
			&keyvals,
			&n_entries);
		for (i = 0; i < n_entries; i ++)
		{
			iKeyVal2 = keyvals[i];
			if ((iKeyVal2 >= GDK_0 && iKeyVal2 <= GDK_9) || (iKeyVal2 >= GDK_KP_0 && iKeyVal2 <= GDK_KP_9))
			{
				iKeyVal = iKeyVal2;
				break;
			}
		}
		g_free (keys);
		g_free (keyvals);
	}
	
	if (iKeyVal == GDK_Escape)  // on clot la session.
	{
		// give the focus back to the window that had it before the user opened this session.
		if (myData.pPreviouslyActiveWindow != NULL)
		{
			gldi_window_show (myData.pPreviouslyActiveWindow);
		}
		cd_do_close_session ();
	}
	else if (iKeyVal == GDK_space && myData.sCurrentText->len == 0)  // pas d'espace en debut de chaine.
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
			gldi_icon_stop_animation (myData.pCurrentIcon);  // car on va perdre le focus.
			myData.bIgnoreIconState = FALSE;
			
			GtkWidget *menu = cairo_dock_build_menu (myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));
			cairo_dock_popup_menu_on_icon (menu, myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));
		}
	}
	else if (iKeyVal == GDK_BackSpace)  // on efface la derniere lettre.
	{
		if (myData.sCurrentText->len > 0)
		{
			cd_debug ("we remove the last letter of %s (%d)", myData.sCurrentText->str, myData.sCurrentText->len);
			
			g_string_truncate (myData.sCurrentText, myData.sCurrentText->len-1);
			
			// on relance la recherche.
			if (myData.pCurrentIcon == NULL)  // sinon l'icone actuelle convient toujours.
				cd_do_search_current_icon (FALSE);
		}
	}
	else if (iKeyVal == GDK_Tab)  // jump to next icon.
	{
		if (myData.sCurrentText->len > 0)
		{
			//gboolean bPrevious = iModifierType & GDK_SHIFT_MASK;
			// on cherche l'icone suivante.
			cd_do_search_current_icon (TRUE);  // pCurrentIcon peut etre NULL si elle s'est faite detruire pendant la recherche, auquel cas on cherchera juste normalement.
		}
	}
	else if (iKeyVal == GDK_Return)
	{
		if (myData.pCurrentIcon != NULL)
		{
			cd_debug ("we click on the icon '%s' [%d, %d]", myData.pCurrentIcon->cName, iModifierType, GDK_SHIFT_MASK);
			
			myData.bIgnoreIconState = TRUE;
			if (iModifierType & GDK_MOD1_MASK)  // ALT
			{
				myData.bIgnoreIconState = TRUE;
				gldi_icon_stop_animation (myData.pCurrentIcon);  // car aucune animation ne va la remplacer.
				myData.bIgnoreIconState = FALSE;
				gldi_object_notify (CAIRO_CONTAINER (myData.pCurrentDock), NOTIFICATION_MIDDLE_CLICK_ICON, myData.pCurrentIcon, myData.pCurrentDock);
			}
			else if (iModifierType & GDK_CONTROL_MASK)  // CTRL
			{
				myData.bIgnoreIconState = TRUE;
				gldi_icon_stop_animation (myData.pCurrentIcon);  // car on va perdre le focus.
				myData.bIgnoreIconState = FALSE;
				
				GtkWidget *menu = cairo_dock_build_menu (myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));
				cairo_dock_popup_menu_on_icon (menu, myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));
			}
			else
			{
				cd_do_simulate_click (CAIRO_CONTAINER (myData.pCurrentDock), myData.pCurrentIcon, iModifierType);
			}
			gldi_icon_start_animation (myData.pCurrentIcon);
			myData.bIgnoreIconState = FALSE;
			myData.pCurrentIcon = NULL;  // sinon on va interrompre l'animation en fermant la session.
		}
		cd_do_close_session ();
	}
	else if (iKeyVal == GDK_Left || iKeyVal == GDK_Right || iKeyVal == GDK_Up || iKeyVal == GDK_Down)
	{
		iKeyVal = _orient_arrow (pContainer, iKeyVal);
		if (iKeyVal == GDK_Up)
		{
			if (myData.pCurrentIcon != NULL && myData.pCurrentIcon->pSubDock != NULL)
			{
				cd_debug ("on monte dans le sous-dock %s", myData.pCurrentIcon->cName);
				Icon *pIcon = cairo_dock_get_first_icon (myData.pCurrentIcon->pSubDock->icons);
				cd_do_change_current_icon (pIcon, myData.pCurrentIcon->pSubDock);
			}
		}
		else if (iKeyVal == GDK_Down)
		{
			if (myData.pCurrentDock->iRefCount > 0)
			{
				CairoDock *pParentDock = NULL;
				Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (myData.pCurrentDock, &pParentDock);
				if (pPointingIcon != NULL)
				{
					cd_debug ("on redescend dans le dock parent via %s", pPointingIcon->cName);
					cd_do_change_current_icon (pPointingIcon, pParentDock);
				}
			}
		}
		else if (iKeyVal == GDK_Left)
		{
			if (myData.pCurrentDock->icons != NULL)
			{
				Icon *pPrevIcon = cairo_dock_get_previous_icon (myData.pCurrentDock->icons, myData.pCurrentIcon);
				if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pPrevIcon))
					pPrevIcon = cairo_dock_get_previous_icon (myData.pCurrentDock->icons, pPrevIcon);
				if (pPrevIcon == NULL)  // pas trouve ou bien 1ere icone.
				{
					pPrevIcon = cairo_dock_get_last_icon (myData.pCurrentDock->icons);
				}
				
				cd_debug ("on se deplace a gauche sur %s", pPrevIcon ? pPrevIcon->cName : "none");
				cd_do_change_current_icon (pPrevIcon, myData.pCurrentDock);
			}
		}
		else  // Gdk_Right.
		{
			if (myData.pCurrentDock->icons != NULL)
			{
				Icon *pNextIcon = cairo_dock_get_next_icon (myData.pCurrentDock->icons, myData.pCurrentIcon);
				if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pNextIcon))
					pNextIcon = cairo_dock_get_next_icon (myData.pCurrentDock->icons, pNextIcon);
				if (pNextIcon == NULL)  // pas trouve ou bien 1ere icone.
				{
					pNextIcon = cairo_dock_get_first_icon (myData.pCurrentDock->icons);
				}
				
				cd_debug ("on se deplace a gauche sur %s", pNextIcon ? pNextIcon->cName : "none");
				cd_do_change_current_icon (pNextIcon, myData.pCurrentDock);
			}
		}
	}
	else if (iKeyVal == GDK_Page_Down || iKeyVal == GDK_Page_Up || iKeyVal == GDK_Home || iKeyVal == GDK_End)
	{
		if (iModifierType & GDK_CONTROL_MASK)  // changement de dock principal
		{
			gpointer data[4] = {myData.pCurrentDock, NULL, GINT_TO_POINTER (FALSE), NULL};
			gldi_docks_foreach_root ((GFunc) _find_next_dock, data);
			CairoDock *pNextDock = data[1];
			if (pNextDock == NULL)
				pNextDock = data[3];
			if (pNextDock != NULL)
			{
				Icon *pNextIcon = NULL;
				int n = g_list_length (pNextDock->icons);
				if (n > 0)
				{
					pNextIcon =  g_list_nth_data (pNextDock->icons, (n-1) / 2);
					if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pNextIcon) && n > 1)
						pNextIcon = g_list_nth_data (pNextDock->icons, (n+1) / 2);
				}
				cd_do_change_current_icon (pNextIcon, pNextDock);
			}
		}
		
		Icon *pIcon = (iKeyVal == GDK_Page_Up || iKeyVal == GDK_Home ? cairo_dock_get_first_icon (myData.pCurrentDock->icons) : cairo_dock_get_last_icon (myData.pCurrentDock->icons));
		cd_debug ("on se deplace a l'extremite sur %s", pIcon ? pIcon->cName : "none");
		cd_do_change_current_icon (pIcon, myData.pCurrentDock);
	}
	else if ( ((iKeyVal >= GDK_0 && iKeyVal <= GDK_9) || (iKeyVal >= GDK_KP_0 && iKeyVal <= GDK_KP_9))
	&& myData.sCurrentText->len == 0)
	{
		_activate_nth_icon (iKeyVal, iModifierType);
	}
	else if (string)  /// utiliser l'unichar ...
	{
		cd_debug ("string:'%s'", string);
		g_string_append_c (myData.sCurrentText, *string);
		
		cd_do_search_current_icon (FALSE);
	}
	
	return GLDI_NOTIFICATION_INTERCEPT;
}


void cd_do_on_shortkey_nav (const char *keystring, gpointer data)
{
	if (! cd_do_session_is_running ())
	{
		cd_do_open_session ();
	}
	else
	{
		// give the focus back to the window that had it before the user opened this session.
		if (myData.pPreviouslyActiveWindow != NULL)
		{
			gldi_window_show (myData.pPreviouslyActiveWindow);
		}
		cd_do_close_session ();
	}
}


gboolean cd_do_update_container (gpointer pUserData, GldiContainer *pContainer, gboolean *bContinueAnimation)
{
	g_return_val_if_fail (!cd_do_session_is_off (), GLDI_NOTIFICATION_LET_PASS);
	
	if (myData.iMotionCount != 0)
	{
		myData.iMotionCount --;
		double f = (double) myData.iMotionCount / 10;
		cairo_dock_emit_motion_signal (CAIRO_DOCK (pContainer),
			f * myData.iPrevMouseX + (1-f) * myData.iMouseX,
			f * myData.iPrevMouseY + (1-f) * myData.iMouseY);
		*bContinueAnimation = TRUE;
	}
	
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
		//\___________________ animation du prompt.
		myData.iPromptAnimationCount ++;
		*bContinueAnimation = TRUE;
		
		cairo_dock_redraw_container (pContainer);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


gboolean cd_do_check_icon_destroyed (gpointer pUserData, Icon *pIcon)
{
	if (pIcon == myData.pCurrentIcon && ! myData.bIgnoreIconState)
	{
		cd_debug ("notre icone vient de se faire detruire");
		Icon *pNextIcon = NULL;
		if (myData.pCurrentDock != NULL)
		{
			pNextIcon = cairo_dock_get_next_icon (myData.pCurrentDock->icons, pIcon);
			if (! pNextIcon || CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pNextIcon))
			{
				pNextIcon = cairo_dock_get_previous_icon (myData.pCurrentDock->icons, pIcon);
				if (! pNextIcon || CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pNextIcon))
					pNextIcon = cairo_dock_get_first_icon (myData.pCurrentDock->icons);
			}
		}
		if (pNextIcon != NULL)
			cd_do_change_current_icon (pNextIcon, myData.pCurrentDock);
		else
			cd_do_exit_session ();
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}


static void _check_dock_is_active (gchar *cDockName, CairoDock *pDock, gboolean *data)
{
	if (gldi_container_is_active (CAIRO_CONTAINER (pDock)))
		*data = TRUE;
}
gboolean cd_do_check_active_dock (gpointer pUserData, GldiWindowActor *actor)
{
	if (! cd_do_session_is_running ())
		return GLDI_NOTIFICATION_LET_PASS;
	
	// check if a dock has the focus (the user has switched to either another dock, or another window)
	gboolean bDockIsActive = FALSE;
	gldi_docks_foreach ((GHFunc) _check_dock_is_active, &bDockIsActive);
	
	if (! bDockIsActive)  // the user has switched to another window -> close the session
	{
		cd_do_close_session ();
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


static void _render_cairo (GldiContainer *pContainer, cairo_t *pCairoContext)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	if (myData.pArrowImage->pSurface != NULL)
	{
		double fFrameWidth, fFrameHeight;
		double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
		if (pContainer->bIsHorizontal)
		{
			fFrameWidth = MIN (myData.pArrowImage->iWidth, pContainer->iWidth);
			fFrameHeight = MIN (myData.pArrowImage->iHeight, pContainer->iHeight);
			
			fDockOffsetX = (pContainer->iWidth - fFrameWidth) / 2;
			fDockOffsetY = (pContainer->iHeight - fFrameHeight) / 2;
		}
		else
		{
			fFrameWidth = MIN (myData.pArrowImage->iWidth, pContainer->iHeight);
			fFrameHeight = MIN (myData.pArrowImage->iHeight, pContainer->iWidth);
			
			fDockOffsetY = (pContainer->iWidth - fFrameWidth) / 2;
			fDockOffsetX = (pContainer->iHeight - fFrameHeight) / 2;
		}
		
		fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
		
		if (fAlpha != 0)
		{
			cairo_translate (pCairoContext, fDockOffsetX, fDockOffsetY);
			cairo_scale (pCairoContext, fFrameWidth / myData.pArrowImage->iWidth, fFrameHeight / myData.pArrowImage->iHeight);
			cairo_dock_draw_surface (pCairoContext, myData.pArrowImage->pSurface, myData.pArrowImage->iWidth, myData.pArrowImage->iHeight, pContainer->bDirectionUp, pContainer->bIsHorizontal, fAlpha);
		}
	}
}

static void _render_opengl (GldiContainer *pContainer)
{
	double fAlpha;
	if (myData.iCloseTime != 0) // animation de fin
		fAlpha = (double) myData.iCloseTime / myConfig.iCloseDuration;
	else
		fAlpha = 1.;
	
	if (myData.pArrowImage->iTexture != 0)
	{
		double fFrameWidth = myData.pArrowImage->iWidth;
		double fFrameHeight = myData.pArrowImage->iHeight;
		
		fFrameWidth = MIN (myData.pArrowImage->iWidth, pContainer->iWidth);
		fFrameHeight = MIN (myData.pArrowImage->iHeight, pContainer->iHeight);
		
		/*double fDockOffsetX, fDockOffsetY;  // Offset du coin haut gauche du prompt.
		fDockOffsetX = (pContainer->iWidth - fFrameWidth) / 2;
		fDockOffsetY = (pContainer->iHeight - fFrameHeight) / 2;*/
		
		fAlpha *= _alpha_prompt (myData.iPromptAnimationCount, s_iNbPromptAnimationSteps);
		
		if (fAlpha != 0)
		{
			glPushMatrix ();
			
			cairo_dock_set_container_orientation_opengl (pContainer);
			
			glTranslatef (pContainer->iWidth/2, pContainer->iHeight/2, 0.);
			
			_cairo_dock_enable_texture ();
			_cairo_dock_set_blend_alpha ();
			
			_cairo_dock_apply_texture_at_size_with_alpha (myData.pArrowImage->iTexture, fFrameWidth, fFrameHeight, fAlpha);
			
			_cairo_dock_disable_texture ();
			
			glPopMatrix();
		}
	}
}

gboolean cd_do_render (gpointer pUserData, GldiContainer *pContainer, cairo_t *pCairoContext)
{
	g_return_val_if_fail (!cd_do_session_is_off (), GLDI_NOTIFICATION_LET_PASS);
	
	if (pCairoContext != NULL)
	{
		_render_cairo (pContainer, pCairoContext);
	}
	else
	{
		_render_opengl (pContainer);
	}
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_do_on_click (gpointer pUserData, Icon *icon, GldiContainer *pContainer)
{
	g_return_val_if_fail (!cd_do_session_is_off (), GLDI_NOTIFICATION_LET_PASS);
	
	if (! myData.bIgnoreClick)
		cd_do_close_session ();
	
	return GLDI_NOTIFICATION_LET_PASS;
}

