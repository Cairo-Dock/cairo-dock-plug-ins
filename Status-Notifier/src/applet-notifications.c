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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"

static void _get_x_y (Icon *pIcon, CairoContainer *pContainer, int *x, int *y)
{
	if (pContainer->bIsHorizontal)
	{
		*x = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		*y = pContainer->iWindowPositionY + (pContainer->bDirectionUp ? 0 : pContainer->iHeight);
	}
	else
	{
		*x = pContainer->iWindowPositionY + (pContainer->bDirectionUp ? 0 : pContainer->iHeight);
		*y = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
	}
	g_print ("menu position : %d;%d\n", *x, *y);
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
static inline gboolean _emit_click (Icon *pIcon, CairoContainer *pContainer, const gchar *cSignal)
{
	if (pContainer == CAIRO_CONTAINER (myIcon->pSubDock) && myIcon->pSubDock != NULL)
	{
		if (pIcon != NULL)
		{
			cairo_dock_remove_dialog_if_any (pIcon);
			
			CDStatusNotifierItem *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
			if (pItemData != NULL)
			{
				if (pItemData->iSidPopupTooltip != 0)
				{
					g_source_remove (pItemData->iSidPopupTooltip);
					pItemData->iSidPopupTooltip = 0;
				}	
				int x, y;
				_get_x_y (pIcon, pContainer, &x, &y);
				dbus_g_proxy_call (pItemData->pProxy, cSignal, NULL,
					G_TYPE_INT, x,
					G_TYPE_INT, y,
					G_TYPE_INVALID,
					G_TYPE_INVALID);
			}
		}
		return TRUE;
	}
	return FALSE;
}
CD_APPLET_ON_CLICK_BEGIN
	_emit_click (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER, "Activate");
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_emit_click (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER, "SecondaryActivate");
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN
	if (CD_APPLET_CLICKED_CONTAINER == CAIRO_CONTAINER (myIcon->pSubDock) && myIcon->pSubDock != NULL)
	{
		if (CD_APPLET_CLICKED_ICON != NULL)
		{
			CDStatusNotifierItem *pItemData = CD_APPLET_GET_MY_ICON_DATA (CD_APPLET_CLICKED_ICON);
			if (pItemData != NULL)
			{
				dbus_g_proxy_call (pItemData->pProxy, "Scroll", NULL,
					G_TYPE_INT, CD_APPLET_SCROLL_UP ? +1 : -1,
					G_TYPE_STRING, "vertical",
					G_TYPE_INVALID,
					G_TYPE_INVALID);
			}
		}
	}
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


gboolean cd_status_notifier_on_right_click (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu, gboolean *bDiscardMenu)
{
	if (pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock) && myIcon->pSubDock != NULL)
	{
		*bDiscardMenu = TRUE;
		if (pClickedIcon != NULL)
		{
			_emit_click (pClickedIcon, pClickedContainer, "ContextMenu");
		}
		return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}


static gboolean _popup_tooltip (Icon *pIcon)
{
	CDStatusNotifierItem *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pItemData != NULL && pItemData->pToolTip != NULL)
	{
		myDialogs.dialogTextDescription.bUseMarkup = TRUE;
		g_print ("pItemData->pToolTip->cMessage : %s\n", pItemData->pToolTip->cMessage);
		gchar *cText = g_strdup_printf ("<b>%s</b>\n%s", pItemData->pToolTip->cTitle, pItemData->pToolTip->cMessage);
		gchar *cIconPath = NULL;
		if (pItemData->pToolTip->cIconName)
		{
			cIconPath = cairo_dock_search_icon_s_path (pItemData->pToolTip->cIconName);
		}
		
		cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 3000, cIconPath ? cIconPath : "same icon");
		g_free (cText);
		myDialogs.dialogTextDescription.bUseMarkup = FALSE;
		pItemData->iSidPopupTooltip = 0;
	}
	return FALSE;
}
gboolean cd_status_notifier_on_enter_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pDock == myIcon->pSubDock && myIcon->pSubDock != NULL)
	{
		/*Icon *icon = NULL;
		GList *ic;
		for (ic = myData.pIcons; ic != NULL; ic = ic->next)
		{
			icon = ic->data;
			CDStatusNotifierItem *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
			if (pItemData && pItemData->iSidPopupTooltip != 0)
			{
				g_source_remove (pItemData->iSidPopupTooltip);
				pItemData->iSidPopupTooltip = 0;
			}
			cairo_dock_remove_dialog_if_any (icon);
		}
		
		if (pIcon)
		{
			CDStatusNotifierItem *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
			pItemData->iSidPopupTooltip = g_timeout_add (600, (GSourceFunc) _popup_tooltip, pIcon);
		}*/
	}
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
