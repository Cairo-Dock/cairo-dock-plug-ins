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

#include "applet-struct.h"
#include "applet-item.h"
#include "applet-draw.h"
#include "applet-notifications.h"

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.

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
	//g_print ("click position : %d;%d\n", *x, *y);
}
static inline gboolean _emit_click (CDStatusNotifierItem *pItem, Icon *pIcon, CairoContainer *pContainer, const gchar *cSignal)
{
	int x, y;
	_get_x_y (pIcon, pContainer, &x, &y);
	
	GError *erreur = NULL;
	dbus_g_proxy_call (pItem->pProxy, cSignal, &erreur,
		G_TYPE_INT, x,
		G_TYPE_INT, y,
		G_TYPE_INVALID,
		G_TYPE_INVALID);
	if (erreur != NULL)
	{
		//g_print ("method %s failed (%s)\n", cSignal, erreur->message);
		g_error_free (erreur);
		return FALSE;
	}
	return TRUE;
}

static inline CDStatusNotifierItem *_get_item (Icon *pClickedIcon, CairoContainer *pClickedContainer)
{
	CDStatusNotifierItem *pItem = NULL;
	if (myConfig.bCompactMode)
	{
		if (pClickedIcon == myIcon)  // clic sur la bonne icone.
		{
			pItem = cd_satus_notifier_find_item_from_coord ();
		}
	}
	else
	{
		if ((myIcon->pSubDock != NULL && pClickedContainer == CAIRO_CONTAINER (myIcon->pSubDock)) ||
			(myDesklet && pClickedContainer == myContainer))  // clic sur le bon container.
		{
			pItem = cd_satus_notifier_get_item_from_icon (pClickedIcon);
		}
	}
	return pItem;
}


static inline gboolean _popup_menu (CDStatusNotifierItem *pItem, Icon *pIcon, CairoContainer *pContainer)
{
	gboolean r = FALSE;
	if (pItem->cMenuPath != NULL)
	{
		if (pItem->pMenu == NULL)
			pItem->pMenu = dbusmenu_gtkmenu_new ((gchar *)pItem->cService, (gchar *)pItem->cMenuPath);
		if (pItem->pMenu != NULL)
		{
			cairo_dock_popup_menu_on_icon (GTK_WIDGET (pItem->pMenu), pIcon, pContainer);
			r = TRUE;
		}
	}

	if (!r)
	{
		r = _emit_click (pItem, pIcon, pContainer, "ContextMenu");
	}

	if (!r)
	{
		r = _emit_click (pItem, pIcon, pContainer, "Activate");
	}
}

CD_APPLET_ON_CLICK_BEGIN
	CDStatusNotifierItem *pItem = _get_item (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER);
	//g_print ("click on item '%s'\n", pItem?pItem->cService:"none");
	if (pItem != NULL)
	{
		if (myConfig.bMenuOnLeftClick)
		{
			_popup_menu (pItem, CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER);
		}
		else
		{
			gboolean r = _emit_click (pItem, CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER, "Activate");
			if (!r)
			{
				if (pItem->cId != NULL)
				{
					/// TODO: try to get the icon in the taskbar, because launch the command doesn't raise the window if it was already visible (but it does pop up it if it was hidden, usually).
					cairo_dock_launch_command (pItem->cId);  // lancer une nouvelle fois l'appli montre sa fenetre (enfin, generalement).
				}
			}
		}
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	CDStatusNotifierItem *pItem = _get_item (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER);
	if (pItem != NULL)
	{
		_emit_click (pItem, CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER, "SecondaryActivate");
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_SCROLL_BEGIN
	CDStatusNotifierItem *pItem = _get_item (CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER);
	if (pItem != NULL)
	{
		GError *erreur = NULL;
		dbus_g_proxy_call (pItem->pProxy, "Scroll", &erreur,
			G_TYPE_INT, CD_APPLET_SCROLL_UP ? +1 : -1,
			G_TYPE_STRING, "vertical",
			G_TYPE_INVALID,
			G_TYPE_INVALID);
		if (erreur != NULL)
		{
			//g_print ("method %s failed (%s)\n", "Scroll", erreur->message);
			g_error_free (erreur);
		}
	}
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN

CD_APPLET_ON_BUILD_MENU_END


gboolean cd_status_notifier_on_right_click (CairoDockModuleInstance *myApplet, Icon *pClickedIcon, CairoContainer *pClickedContainer, GtkWidget *pAppletMenu, gboolean *bDiscardMenu)
{
	if (pClickedIcon == NULL || myConfig.bMenuOnLeftClick)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	CD_APPLET_ENTER;
	CDStatusNotifierItem *pItem = _get_item (pClickedIcon, pClickedContainer);
	if (pItem != NULL)
	{
		_popup_menu (pItem, pClickedIcon, pClickedContainer);

		*bDiscardMenu = TRUE;
		CD_APPLET_LEAVE (CAIRO_DOCK_INTERCEPT_NOTIFICATION);
	}
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}


static gboolean _popup_tooltip (Icon *pIcon)
{
	CDStatusNotifierItem *pItemData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
	if (pItemData != NULL && pItemData->pToolTip != NULL)
	{
		myDialogsParam.dialogTextDescription.bUseMarkup = TRUE;
		//g_print ("pItemData->pToolTip->cMessage : %s\n", pItemData->pToolTip->cMessage);
		gchar *cText = g_strdup_printf ("<b>%s</b>\n%s", pItemData->pToolTip->cTitle, pItemData->pToolTip->cMessage);
		gchar *cIconPath = NULL;
		if (pItemData->pToolTip->cIconName)
		{
			cIconPath = cairo_dock_search_icon_s_path (pItemData->pToolTip->cIconName);
		}
		
		cairo_dock_show_temporary_dialog_with_icon (cText, pIcon, CAIRO_CONTAINER (myIcon->pSubDock), 3000, cIconPath ? cIconPath : "same icon");
		g_free (cText);
		myDialogsParam.dialogTextDescription.bUseMarkup = FALSE;
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



gboolean on_mouse_moved (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
	
	CDStatusNotifierItem *pItem = cd_satus_notifier_find_item_from_coord ();
	
	if (pItem != myData.pCurrentlyHoveredItem)
	{
		myData.pCurrentlyHoveredItem = pItem;
		myData.fDesktopNameAlpha = 0.;
		if (pItem == NULL)
			CD_APPLET_SET_NAME_FOR_MY_ICON (NULL);
		else
			CD_APPLET_SET_NAME_FOR_MY_ICON (
				pItem->cTitle && *pItem->cTitle != '\0' ? pItem->cTitle :
				pItem->cLabel && *pItem->cLabel != '\0' ? pItem->cLabel :
				pItem->cId && *pItem->cId != '\0' ? pItem->cId :
				NULL);
		
		if (myDock)
			CAIRO_DOCK_REDRAW_MY_CONTAINER;
		else
			*bStartAnimation = TRUE;
	}
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_update_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	CD_APPLET_ENTER;
	if (! myIcon->bPointed || ! pContainer->bInside)
	{
		myData.fDesktopNameAlpha -= .07;
		if (myData.fDesktopNameAlpha < .01)
			myData.fDesktopNameAlpha = 0;
		if (myData.fDesktopNameAlpha != 0)
			*bContinueAnimation = TRUE;
	}
	else
	{
		myData.fDesktopNameAlpha += .07;
		if (myData.fDesktopNameAlpha > .99)
			myData.fDesktopNameAlpha = 1;
		if (myData.fDesktopNameAlpha != 1)
			*bContinueAnimation = TRUE;
	}
	CAIRO_DOCK_REDRAW_MY_CONTAINER;
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_render_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, cairo_t *pCairoContext)
{
	CD_APPLET_ENTER;
	int x, y;  // centre du texte.
	x = myIcon->fDrawX + myIcon->fWidth * myIcon->fScale / 2;
	y = myIcon->fDrawY + myIcon->fHeight * myIcon->fScale / 2;
	if (x - myIcon->iTextWidth/2 < 0)
	{
		x -= myIcon->iTextWidth/2;
	}
	if (pCairoContext != NULL)
	{
		if (myIcon->pTextBuffer == NULL)
			CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
		cairo_save (pCairoContext);
		cairo_translate (pCairoContext, x, y);
		cairo_set_source_surface (pCairoContext, myIcon->pTextBuffer, - myIcon->iTextWidth/2, - myIcon->iTextHeight/2);
		cairo_paint_with_alpha (pCairoContext, myData.fDesktopNameAlpha);
		cairo_restore (pCairoContext);
	}
	else
	{
		if (myIcon->iLabelTexture == 0)
			CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
		glPushMatrix ();
		if (myDesklet)
			glTranslatef (-myDesklet->container.iWidth/2, -myDesklet->container.iHeight/2, -myDesklet->container.iHeight*(sqrt(3)/2));
		glTranslatef (x - ((myIcon->iTextWidth & 1) ? 0.5 : 0.),
			y - ((myIcon->iTextHeight & 1) ? 0.5 : 0.),
			0);
		cairo_dock_draw_texture_with_alpha (myIcon->iLabelTexture, myIcon->iTextWidth, myIcon->iTextHeight, myData.fDesktopNameAlpha);
		glPopMatrix ();
	}
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
}

gboolean on_leave_desklet (CairoDockModuleInstance *myApplet, CairoContainer *pContainer, gboolean *bStartAnimation)
{
	*bStartAnimation = TRUE;
	myData.pCurrentlyHoveredItem = NULL;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
