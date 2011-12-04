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

#include "applet-struct.h"
#include "applet-session.h"
#include "applet-notifications.h"
#include "applet-icon-finder.h"

static inline gboolean _cd_do_icon_match (Icon *pIcon, const gchar *cCommandPrefix, int length)
{
	gboolean bMatch = FALSE;
	if (pIcon->cBaseURI != NULL)
	{
		gchar *cFile = g_path_get_basename (pIcon->cCommand);
		bMatch = (cFile && g_ascii_strncasecmp (cCommandPrefix, cFile, length) == 0);
		g_free (cFile);
	}
	else if (pIcon->cCommand)
	{
		bMatch = (g_ascii_strncasecmp (cCommandPrefix, pIcon->cCommand, length) == 0);
		if (!bMatch)
		{
			gchar *str = strchr (pIcon->cCommand, '-');  // on se limite au 1er tiret.
			if (str && *(str-1) != ' ')  // on verifie qu'il n'est pas un tiret d'option
			{
				str ++;
				bMatch = (g_strncasecmp (str, cCommandPrefix, length) == 0);
			}
			if (!bMatch && pIcon->cName)
				bMatch = (g_ascii_strncasecmp (cCommandPrefix, pIcon->cName, length) == 0);
		}
	}
	return bMatch;
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
	if (pDock == myData.pCurrentDock || *pFoundIcon != NULL) // on a deja cherche dans le main dock, ou deja trouve ce qu'on cherchait.
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
				*pFirstParentDock = pDock;
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
	
	//\_________________ on cherche en premier dans le dock courant, car il est deja visible.
	int length = strlen (cCommandPrefix);
	Icon *pIcon, *pFirstIcon = NULL;
	CairoDock *pParentDock, *pFirstParentDock = NULL;
	GList *ic;
	for (ic = myData.pCurrentDock->icons; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->cCommand && g_ascii_strncasecmp (cCommandPrefix, pIcon->cCommand, length) == 0)
		{
			if (pAfterIcon == NULL)
			{
				*pDock = myData.pCurrentDock;
				return pIcon;
			}
			else
			{
				if (pFirstIcon == NULL)  // on garde une trace de la 1ere icone pour boucler dans la liste.
				{
					pFirstIcon = pIcon;
					pFirstParentDock = myData.pCurrentDock;
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
	if (myData.pCurrentDock != NULL && pDock != myData.pCurrentDock)  // on remet au repos le dock precedemment anime.
	{
		g_print ("leave this dock\n");
		cairo_dock_emit_leave_signal (CAIRO_CONTAINER (myData.pCurrentDock));
		cairo_dock_remove_notification_func_on_object (myData.pCurrentDock, NOTIFICATION_RENDER_DOCK, (CairoDockNotificationFunc) cd_do_render, NULL);
		cairo_dock_remove_notification_func_on_object (myData.pCurrentDock, NOTIFICATION_UPDATE_DOCK, (CairoDockNotificationFunc) cd_do_update_container, NULL);
		cairo_dock_remove_notification_func_on_object (myData.pCurrentDock, NOTIFICATION_CLICK_ICON, (CairoDockNotificationFunc) cd_do_on_click, NULL);
		cairo_dock_remove_notification_func_on_object (myData.pCurrentDock, NOTIFICATION_MIDDLE_CLICK_ICON, (CairoDockNotificationFunc) cd_do_on_click, NULL);
	}
	if (pDock != NULL && pDock != myData.pCurrentDock)  // on montre le nouveau dock
	{
		g_print (" dock %p <- %p\n", myData.pCurrentDock, pDock);
		if (pDock != NULL)
		{
			if (pDock->iRefCount > 0)
			{
				CairoDock *pParentDock = NULL;
				Icon *pPointingIcon = cairo_dock_search_icon_pointing_on_dock (pDock, &pParentDock);
				if (pPointingIcon != NULL)
				{
					cairo_dock_show_subdock (pPointingIcon, pParentDock);  // utile pour le montrage des sous-docks au clic.
				}
			}
			else
			{
				/// utile de faire ca si on entre dedans ?...
				g_print ("enter this dock\n");
				if (pDock->bAutoHide)
					cairo_dock_start_showing (pDock);
				if (pDock->iVisibility == CAIRO_DOCK_VISI_KEEP_BELOW)
					cairo_dock_pop_up (pDock);
			}
			cairo_dock_emit_enter_signal (CAIRO_CONTAINER (pDock));
		}
		
		cairo_dock_register_notification_on_object (pDock,
			NOTIFICATION_UPDATE_DOCK,
			(CairoDockNotificationFunc) cd_do_update_container,
			CAIRO_DOCK_RUN_AFTER, NULL);
		cairo_dock_register_notification_on_object (pDock,
			NOTIFICATION_RENDER_DOCK,
			(CairoDockNotificationFunc) cd_do_render,
			CAIRO_DOCK_RUN_AFTER, NULL);
		cairo_dock_register_notification_on_object (pDock,
			NOTIFICATION_CLICK_ICON,
			(CairoDockNotificationFunc) cd_do_on_click,
			CAIRO_DOCK_RUN_AFTER, NULL);  // we don't disable the clicks, rather we will close the session.
		cairo_dock_register_notification_on_object (pDock,
			NOTIFICATION_MIDDLE_CLICK_ICON,
			(CairoDockNotificationFunc) cd_do_on_click,
			CAIRO_DOCK_RUN_AFTER, NULL);
	}
	if (pDock != NULL)
	{
		gtk_window_present (GTK_WINDOW (pDock->container.pWidget));
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
				pDock->container.bIsHorizontal ? x : y,
				pDock->container.bIsHorizontal ? y : x);
		}
		else
		{
			myData.iPrevMouseX = myData.iMouseX;
			myData.iPrevMouseY = myData.iMouseY;
			myData.iMotionCount = 10;
		}
		myData.iMouseX = x;
		myData.iMouseY = y;
		cairo_dock_request_icon_animation (pIcon, CAIRO_CONTAINER (pDock), myConfig.cIconAnimation, 1e6);  // interrompt l'animation de "mouse over".
		cairo_dock_launch_animation (CAIRO_CONTAINER (pDock));
	}
	
	myData.pCurrentDock = pDock;
	myData.pCurrentIcon = pIcon;
	g_print ("myData.pCurrentDock <- %p\n", myData.pCurrentDock);
}


void cd_do_search_current_icon (gboolean bLoopSearch)
{
	//\_________________ on cherche un lanceur correspondant.
	CairoDock *pDock;
	Icon *pIcon = cd_do_search_icon_by_command (myData.sCurrentText->str, (bLoopSearch ? myData.pCurrentIcon : NULL), &pDock);
	cd_debug ("found icon : %s\n", pIcon ? pIcon->cName : "none");
	
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
	motion.x_root = pDock->container.iWindowPositionX + pDock->container.iMouseX;
	motion.y_root = pDock->container.iWindowPositionY + pDock->container.iMouseY;
	motion.time = 0;
	motion.window = gldi_container_get_gdk_window (CAIRO_CONTAINER (pDock));
	#if (GTK_MAJOR_VERSION < 3)
	motion.device = gdk_device_get_core_pointer ();
	#else
	GdkDeviceManager *manager = gdk_display_get_device_manager (gdk_window_get_display (motion.window));
	motion.device = gdk_device_manager_get_client_pointer(manager);
	#endif
	g_signal_emit_by_name (pDock->container.pWidget, "motion-notify-event", &motion, &bReturn);
	return FALSE;
}
