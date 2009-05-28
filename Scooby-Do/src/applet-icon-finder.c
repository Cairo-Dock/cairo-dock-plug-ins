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
#include "applet-icon-finder.h"

static inline gboolean _cd_do_icon_match (Icon *pIcon, const gchar *cCommandPrefix, int length)
{
	gboolean bFound = FALSE;
	if (pIcon->cBaseURI != NULL)
	{
		gchar *cFile = g_path_get_basename (pIcon->acCommand);
		bFound = (cFile && g_ascii_strncasecmp (cCommandPrefix, cFile, length) == 0);
		g_free (cFile);
	}
	else if (pIcon->acCommand)
	{
		bFound = (g_ascii_strncasecmp (cCommandPrefix, pIcon->acCommand, length) == 0);
	}
	return bFound;
}

static void _find_icon_in_dock_with_command (Icon *pIcon, CairoDock *pDock, gpointer *data)
{
	gchar *cCommandPrefix = data[0];
	int length = GPOINTER_TO_INT (data[1]);
	Icon *pAfterIcon = data[2];
	Icon **pFoundIcon = data[3];
	CairoDock **pFoundDock = data[4];
	Icon **pFirstIcon = data[5];
	CairoDock **pFirstParentDock = data[6];
	if (pDock == g_pMainDock || *pFoundIcon != NULL) // on a deja cherche dans le main dock, ou deja trouve ce qu'on cherchait.
		return ;
	
	gboolean bFound = _cd_do_icon_match (pIcon, cCommandPrefix, length);
	if (bFound)
	{
		if (pAfterIcon == NULL)
		{
			*pFoundIcon = pIcon;
			*pFoundDock = pDock;
		}
		else
		{
			if (*pFirstIcon == NULL)  // on garde une trace de la 1ere icone pour boucler dans la liste.
			{
				*pFirstIcon = pIcon;
				*pFirstParentDock = g_pMainDock;
			}
			if (pIcon == pAfterIcon)
			{
				data[2] = NULL;
			}
		}
	}
}
Icon *cd_do_search_icon_by_command (const gchar *cCommandPrefix, Icon *pAfterIcon, CairoDock **pDock)
{
	g_return_val_if_fail (cCommandPrefix != NULL, NULL);
	
	//\_________________ on cherche en premier dans le main dock, car il est deja visible.
	int length = strlen (cCommandPrefix);
	Icon *pIcon, *pFirstIcon = NULL;
	CairoDock *pParentDock, *pFirstParentDock = NULL;
	GList *ic;
	for (ic = g_pMainDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->acCommand && g_ascii_strncasecmp (cCommandPrefix, pIcon->acCommand, length) == 0)
		{
			if (pAfterIcon == NULL)
			{
				*pDock = g_pMainDock;
				return pIcon;
			}
			else
			{
				if (pFirstIcon == NULL)  // on garde une trace de la 1ere icone pour boucler dans la liste.
				{
					pFirstIcon = pIcon;
					pFirstParentDock = g_pMainDock;
				}
				if (pIcon == pAfterIcon)
				{
					pAfterIcon = NULL;
				}
			}
		}
	}
	
	//\_________________ si on a rien trouve on cherche dans tous les docks.
	pIcon = NULL;
	*pDock = NULL;
	gpointer data[7];
	data[0] = (gchar *)cCommandPrefix;
	data[1] = GINT_TO_POINTER (length);
	data[2] = pAfterIcon;
	data[3] = &pIcon;
	data[4] = pDock;
	data[5] = &pFirstIcon;
	data[6] = &pFirstParentDock;
	cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _find_icon_in_dock_with_command, data);
	
	if (pIcon == NULL)
	{
		pIcon = pFirstIcon;
		*pDock = pFirstParentDock;
	}
	return pIcon;
}


void cd_do_change_current_icon (Icon *pIcon, CairoDock *pDock)
{
	//\_________________ on gere le cachage et le montrage du dock precedent et actuel.
	if (myData.pCurrentDock != NULL && pDock != myData.pCurrentDock && myData.pCurrentDock != g_pMainDock)  // on remet au repos dock precedemment anime.
	{
		cairo_dock_emit_leave_signal (myData.pCurrentDock);
	}
	if (pDock != NULL && pDock != g_pMainDock && pDock != myData.pCurrentDock)  // on montre le nouveau dock
	{
		if (pDock != NULL)
		{
			if (pDock->iRefCount > 0)
			{
				CairoDock *pParentDock = NULL;
				Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (pDock, &pParentDock);
				if (pPointingIcon != NULL)
				{
					cairo_dock_show_subdock (pPointingIcon, FALSE, pParentDock);  // utile pour le montrage des sous-docks au clic.
				}
			}
			else
			{
				cairo_dock_pop_up (pDock);
			}
			cairo_dock_emit_enter_signal (pDock);
		}
	}
	if (pDock != NULL)
	{
		
		gtk_window_present (GTK_WINDOW (pDock->pWidget));
	}
	
	//\_________________ on gere l'allumage et l'eteignage de l'icone precedente et actuelle.
	if (myData.pCurrentIcon != NULL && pIcon != myData.pCurrentIcon)  // on remet au repos l'icone precedemment anime.
	{
		myData.bIgnoreIconState = TRUE;
		cairo_dock_stop_icon_animation (myData.pCurrentIcon);
		myData.bIgnoreIconState = FALSE;
		cairo_dock_redraw_icon (myData.pCurrentIcon, CAIRO_CONTAINER (myData.pCurrentDock));  /// utile ?...
	}
	if (pIcon != NULL && myData.pCurrentIcon != pIcon)  // on anime la nouvelle icone.
	{
		int x = pIcon->fXAtRest + pIcon->fWidth/2 + (- pDock->fFlatDockWidth + pDock->iMaxDockWidth)/2;
		int y = pIcon->fDrawY + pIcon->fHeight/2 * pIcon->fScale;
		if (1||myData.pCurrentDock != pDock)
		{
			cairo_dock_emit_motion_signal (pDock,
				x,
				y);
		}
		else
		{
			myData.iPrevMouseX = myData.iMouseX;
			myData.iPrevMouseY = myData.iMouseY;
			myData.iMotionCount = 10;
		}
		myData.iMouseX = x;
		myData.iMouseY = y;
		cairo_dock_request_icon_animation (pIcon, pDock, myConfig.cIconAnimation, 1e6);  // interrompt l'animation de "mouse over".
		cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));
		//if (myAccessibility.bShowSubDockOnClick)
		//	cairo_dock_show_subdock (pIcon, FALSE, pDock);
	}
	
	myData.pCurrentDock = pDock;
	myData.pCurrentIcon = pIcon;
	if (myData.pCurrentDock == NULL)
		gtk_window_present (GTK_WINDOW (g_pMainDock->pWidget));
}


void cd_do_search_current_icon (gboolean bLoopSearch)
{
	//\_________________ on cherche un lanceur correspondant.
	CairoDock *pDock;
	Icon *pIcon = cd_do_search_icon_by_command (myData.sCurrentText->str, (bLoopSearch ? myData.pCurrentIcon : NULL), &pDock);
	g_print ("found icon : %s\n", pIcon ? pIcon->acName : "none");
	
	//\_________________ on gere le changement d'icone/dock.
	cd_do_change_current_icon (pIcon, pDock);
}


gboolean cairo_dock_emit_motion_signal (CairoDock *pDock, int iMouseX, int iMouseY)
{
	static gboolean bReturn;
	static GdkEventMotion motion;
	motion.state = 0;
	motion.x = iMouseX;
	motion.y = iMouseY;
	motion.x_root = pDock->iWindowPositionX + pDock->iMouseX;
	motion.y_root = pDock->iWindowPositionY + pDock->iMouseY;
	motion.time = 0;
	motion.window = pDock->pWidget->window;
	motion.device = gdk_device_get_core_pointer ();
	g_signal_emit_by_name (pDock->pWidget, "motion-notify-event", &motion, &bReturn);
	return FALSE;
}


static inline void _cd_do_search_matching_icons_in_dock (CairoDock *pDock)
{
	Icon *pIcon;
	GList *ic;
	for (ic = pDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (_cd_do_icon_match (pIcon, myData.sCurrentText->str, myData.sCurrentText->len))
		{
			myData.pMatchingIcons = g_list_prepend (myData.pMatchingIcons, pIcon);
		}
	}
}
static void _cd_do_search_in_one_dock (Icon *pIcon, CairoDock *pDock, gpointer data)
{
	if (_cd_do_icon_match (pIcon, myData.sCurrentText->str, myData.sCurrentText->len))
	{
		myData.pMatchingIcons = g_list_prepend (myData.pMatchingIcons, pIcon);
	}
}
void cd_do_search_matching_icons (void)
{
	gchar *str = strchr (myData.sCurrentText->str, ' ');  // on ne compte pas les arguments d'une eventuelle commande deja tapee.
	int length = myData.sCurrentText->len;
	if (str != NULL)
	{
		g_string_set_size (myData.sCurrentText, str - myData.sCurrentText->str + 1);
		g_print ("str <- '%s' %d\n", myData.sCurrentText->str, myData.sCurrentText->len);
	}
		
	if (myData.pMatchingIcons == NULL)
	{
		if (myData.bSessionStartedAutomatically)  // on cherche dans le dock courant.
		{
			_cd_do_search_matching_icons_in_dock (myData.pCurrentDock);
			myData.pMatchingIcons = g_list_reverse (myData.pMatchingIcons);
		}
		else
		{
			// on parcours tous les docks.
			cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _cd_do_search_in_one_dock, NULL);
			myData.pMatchingIcons = g_list_reverse (myData.pMatchingIcons);
			
			// on rajoute les icones ne venant pas du dock.
			cd_do_find_matching_applications ();
		}
	}
	else  // optimisation : on peut se contenter de chercher parmi les icones deja trouvees.
	{
		GList *ic, *next_ic;
		Icon *pIcon;
		ic = myData.pMatchingIcons;
		while (ic != NULL)
		{
			pIcon = ic->data;
			next_ic = ic->next;
			if (! _cd_do_icon_match (pIcon, myData.sCurrentText->str, myData.sCurrentText->len))
				myData.pMatchingIcons = g_list_delete_link (myData.pMatchingIcons, ic);
			ic = next_ic;
		}
	}
	myData.pCurrentMatchingElement = NULL;
	cairo_dock_redraw_container (CAIRO_CONTAINER (myData.pCurrentDock));
	g_print ("%d / %d\n", length , myData.sCurrentText->len);
	if (length != myData.sCurrentText->len)
		g_string_set_size (myData.sCurrentText, length);
}


void cd_do_select_previous_next_matching_icon (gboolean bNext)
{
	GList *pMatchingElement = myData.pCurrentMatchingElement;
	if (!bNext)
		myData.pCurrentMatchingElement = cairo_dock_get_previous_element (myData.pCurrentMatchingElement, myData.pMatchingIcons);
	else
		myData.pCurrentMatchingElement = cairo_dock_get_next_element (myData.pCurrentMatchingElement, myData.pMatchingIcons);
	if (myData.pCurrentMatchingElement != pMatchingElement)  // on complete le texte et on redessine.
	{
		Icon *pIcon = myData.pCurrentMatchingElement->data;
		cd_do_delete_invalid_caracters ();
		
		if (pIcon->cBaseURI != NULL)
		{
			gchar *cFile = g_path_get_basename (pIcon->acCommand);
			g_string_assign (myData.sCurrentText, cFile);
			g_free (cFile);
		}
		else
			g_string_assign (myData.sCurrentText, pIcon->acCommand);
		
		
		cd_do_load_pending_caracters ();
		
		// on repositionne les caracteres et on anime tout ca.
		cd_do_launch_appearance_animation ();
		cairo_dock_redraw_container (g_pMainDock);
	}
}
