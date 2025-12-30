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

/******************************************************************************
exemples : 
----------

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetQuickInfo string:123 string:"class=firefox"

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.Animate string:default int32:2 string:"class=firefox"

******************************************************************************/

#include <unistd.h>
#include <glib.h>

#include <cairo-dock.h>
#include <cairo-dock-module-instance-manager.h>
#include "interface-main-query.h"
#include "interface-main-methods.h"


#define nullify_argument(string) do {\
	if (string != NULL && (*string == '\0' || strcmp (string, "any") == 0 || strcmp (string, "none") == 0))\
		string = NULL; } while (0)

static gboolean dbus_deskletVisible = FALSE;


  ///////////////////
 ///  UTILITIES  ///
///////////////////

static void _main_reboot (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableReboot)
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Reboot disabled in config");
	else
	{
		cairo_dock_load_current_theme ();
		g_dbus_method_invocation_return_value (pInv, NULL);
	}
}

static void _main_quit (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableQuit)
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Quit disabled in config");
	else
	{
		gtk_main_quit ();
		g_dbus_method_invocation_return_value (pInv, NULL);
	}
}

static void _show_hide_one_dock (const gchar *cDockName, CairoDock *pDock, gpointer data)
{
	if (pDock->iRefCount != 0)
		return ;
	gboolean bShow = GPOINTER_TO_INT (data);
	if (bShow)
	{
		///cairo_dock_pop_up (pDock);
		///if (pDock->bAutoHide)
			gldi_dock_enter_synthetic (pDock);
	}
	else
	{
		///cairo_dock_pop_down (pDock);  // ne fait rien s'il n'etait pas "popped".
		///if (pDock->bAutoHide)
			gldi_dock_leave_synthetic (pDock);
	}
}
static void _main_show_dock (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableShowDock)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "ShowDock disabled in config");
		return;
	}
	
	if (g_pMainDock == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL); // not an error
		return;
	}
	
	// pPar should be a tuple: (i)
	gint32 iVisibility;
	g_variant_get_child (pPar, 0, "i", &iVisibility);
	
	gboolean bShow;
	switch (iVisibility)
	{
		case 0:  // hide
			bShow = FALSE;
		break;
		case 1:  // show
			bShow = TRUE;
		break;
		case 2:  // toggle
		default:
			bShow = (g_pMainDock->bIsBelow || (g_pMainDock->bAutoHide && g_pMainDock->fHideOffset == 1));
		break;		
	}
	
	if (bShow)
		cairo_dock_stop_quick_hide ();
	
	gldi_docks_foreach ((GHFunc) _show_hide_one_dock, GINT_TO_POINTER (bShow));
	
	if (! bShow)
		cairo_dock_quick_hide_all_docks ();
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_show_desklet (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableDesklets)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "ShowDesklet disabled in config");
		return;
	}
	if (dbus_deskletVisible)
	{
		gldi_desklets_set_visibility_to_default ();
	}
	else
	{
		gboolean bWidgetLayer = FALSE;
		g_variant_get_child (pPar, 0, "b", &bWidgetLayer);
		gldi_desklets_set_visible (bWidgetLayer);
	}
	dbus_deskletVisible = !dbus_deskletVisible;
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}


  ///////////////////
 /// SET ON ICON ///
///////////////////

static void _main_set_quick_info (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableSetQuickInfo)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "SetQuickInfo disabled in config");
		return;
	}
	
	// pPar should be (ss)
	const gchar *cQuickInfo;
	gchar *cIconQuery;
	// note: only cIconQuery is copied and need to be freed later
	g_variant_get (pPar, "(&ss)", &cQuickInfo, &cIconQuery);
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		// not an error?
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	nullify_argument (cQuickInfo);
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (pContainer == NULL)
			continue;
		
		gldi_icon_set_quick_info (pIcon, cQuickInfo);
		cairo_dock_redraw_icon (pIcon);
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_set_label (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableSetLabel)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "SetLabel disabled in config");
		return;
	}
	
	const gchar *cLabel;
	gchar *cIconQuery;
	// note: only cIconQuery is copied and need to be freed later
	g_variant_get (pPar, "(&ss)", &cLabel, &cIconQuery);
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	nullify_argument (cLabel);
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (pContainer == NULL)
			continue;
		
		gldi_icon_set_name (pIcon, cLabel);
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_set_icon (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableSetIcon)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "SetIcon disabled in config");
		return;
	}
	
	const gchar *cImage;
	gchar *cIconQuery;
	g_variant_get (pPar, "(&ss)", &cImage, &cIconQuery);
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->image.pSurface == NULL)
			continue;
		
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (pContainer == NULL)
			continue;
		
		cairo_t *pIconContext = cairo_create (pIcon->image.pSurface);
		cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
		cairo_destroy (pIconContext);
		cairo_dock_redraw_icon (pIcon);
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_set_emblem (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableSetIcon)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "SetEmblem disabled in config");
		return;
	}
	
	const gchar *cImage;
	gint32 iPosition;
	gchar *cIconQuery;
	g_variant_get (pPar, "(&sis)", &cImage, &iPosition, &cIconQuery);
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->image.pSurface == NULL)
			continue;
		
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (pContainer == NULL)
			continue;
		
		if (cImage == NULL || *cImage == '\0' || strcmp (cImage, "none") == 0)
		{
			cairo_dock_remove_overlay_at_position (pIcon, iPosition < CAIRO_OVERLAY_NB_POSITIONS ? iPosition : iPosition - CAIRO_OVERLAY_NB_POSITIONS, myApplet);  // for ease of use, handle both case similarily.
		}
		else
		{
			if (iPosition >= CAIRO_OVERLAY_NB_POSITIONS)  // [N; 2N-1] => print the overlay
				cairo_dock_print_overlay_on_icon_from_image (pIcon, cImage, iPosition - CAIRO_OVERLAY_NB_POSITIONS);
			else  // [0, N-1] => add it
				cairo_dock_add_overlay_from_image (pIcon, cImage, iPosition, myApplet);  // use 'myApplet' to identify the overlays set by the Dbus plug-in (since the plug-in can't be deactivated, 'myApplet' is constant).
		}
		
		cairo_dock_redraw_icon (pIcon);
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_animate (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableAnimateIcon)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Animate disabled in config");
		return;
	}
	
	const gchar *cAnimation;
	gint iNbRounds;
	gchar *cIconQuery;
	g_variant_get (pPar, "(&sis)", &cAnimation, &iNbRounds, &cIconQuery);
	if (! cAnimation) //!! TODO: is this possible? (we may just get an empty string)
	{
		g_free (cIconQuery);
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Animation parameter missing");
		return;
	}
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (! CAIRO_DOCK_IS_DOCK (pContainer))
			continue;
		gldi_icon_request_animation (pIcon, cAnimation, iNbRounds);
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_demands_attention (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableAnimateIcon)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "DemandsAttention disabled in config");
		return;
	}
	
	gboolean bStart;
	const gchar *cAnimation;
	gchar *cIconQuery;
	g_variant_get (pPar, "(b&ss)", &bStart, &cAnimation, &cIconQuery);
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (! CAIRO_DOCK_IS_DOCK (pContainer))
			continue;
		
		if (bStart)
		{
			gldi_icon_request_attention (pIcon, cAnimation, 0);  // 0 <=> non-stop.
		}
		else if (pIcon->bIsDemandingAttention)
		{
			gldi_icon_stop_attention (pIcon);
		}
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _main_show_dialog (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnablePopUp)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "ShowDialog disabled in config");
		return;
	}
	
	const gchar *cMessage;
	gint iDuration;
	gchar *cIconQuery;
	g_variant_get (pPar, "(&sis)", &cMessage, &iDuration, &cIconQuery);
	if (!cMessage) //!! TODO: is this possible? (we may just get an empty string)
	{
		g_free (cIconQuery);
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, "Message argument missing");
		return;
	}
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	
	Icon *pIcon;
	GldiContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_get_icon_container (pIcon);
		if (! CAIRO_DOCK_IS_DOCK (pContainer))
			continue;
		gldi_dialog_show_temporary_with_icon (cMessage, pIcon, pContainer, 1000 * iDuration, "same icon");
		break;  // only show 1 dialog.
	}
	
	if (ic == NULL)  // empty list, or didn't find a valid icon.
		gldi_dialog_show_general_message (cMessage, 1000 * iDuration);
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}


#ifdef DBUSMENU_GTK_FOUND
static void _on_menu_destroyed (GtkWidget *menu, CDIconData *pData)
{
	//g_print ("\n+++ %s ()\n\n", __func__);
	if (pData != NULL && pData->menu_items_list != NULL)
	{
		GList *mi;
		for (mi = pData->menu_items_list; mi != NULL; mi = mi->next)
		{
			DbusmenuMenuitem *pDbusMenuItem = mi->data;
			GtkMenuItem *pMenuItem = dbusmenu_gtkclient_menuitem_get (pData->client, pDbusMenuItem);
			if (gtk_widget_get_parent (GTK_WIDGET (pMenuItem)) != NULL)  // it might not be in the menu if it has been added after the menu was built.
				gtk_container_remove (GTK_CONTAINER (menu), GTK_WIDGET (pMenuItem));
		}
	}
}
static gboolean cd_dbus_main_emit_on_build_menu (gpointer data, Icon *pClickedIcon, GldiContainer *pClickedContainer, GtkWidget *pMenu)
{
	if (pClickedIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	CDIconData *pData = CD_APPLET_GET_MY_ICON_DATA (pClickedIcon);
	if (pData != NULL && pData->menu_items_list != NULL)
	{
		GList *mi;
		for (mi = pData->menu_items_list; mi != NULL; mi = mi->next)
		{
			DbusmenuMenuitem *pDbusMenuItem = mi->data;
			GtkMenuItem *pMenuItem = dbusmenu_gtkclient_menuitem_get (pData->client, pDbusMenuItem);
			gtk_menu_shell_append (GTK_MENU_SHELL(pMenu), GTK_WIDGET (pMenuItem));
			gtk_widget_show (GTK_WIDGET (pMenuItem));
		}
		
		// when the menu is destroyed, the menu-items are destroyed too (even if we set a reference on them, their content will be unvalidated); so we must remove them from the menu before that happens.
		g_signal_connect (G_OBJECT (pMenu),
			"destroy",
			G_CALLBACK (_on_menu_destroyed),
			pData);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}


static void root_child_added (DbusmenuMenuitem * root, DbusmenuMenuitem * child, guint position, CDIconData *pData)
{
	cd_debug ("%s (%d)", __func__, position);
	pData->menu_items_list = g_list_insert (pData->menu_items_list, child, position);  // simply add it to the list, the associated menu-item will appear in the menu the next time the user right-clicks (we don't bother to refresh the menu in real-time).
}

static void root_child_moved (DbusmenuMenuitem * root, DbusmenuMenuitem * child, guint newposition, guint oldposition, CDIconData *pData)
{
	cd_debug ("%s (%d -> %d)", __func__, oldposition, newposition);
	GList *mi = g_list_nth (pData->menu_items_list, oldposition);
	pData->menu_items_list = g_list_remove_link (pData->menu_items_list, mi);
	pData->menu_items_list = g_list_insert (pData->menu_items_list, child, newposition);  // same remark
}

static void root_child_delete (DbusmenuMenuitem * root, DbusmenuMenuitem * child, CDIconData *pData)
{
	cd_debug ("%s ()", __func__);
	pData->menu_items_list = g_list_remove (pData->menu_items_list, child);  // same remark
}

static void root_changed (DbusmenuGtkClient * client, DbusmenuMenuitem * newroot, CDIconData *pData)
{
	cd_debug ("%s (%p", __func__, newroot);
	if (newroot == NULL)
	{
		return;
	}
	
	// get the current childs
	GList * child = NULL;
	for (child = dbusmenu_menuitem_get_children(newroot); child != NULL; child = g_list_next(child))
	{
		pData->menu_items_list = g_list_append (pData->menu_items_list, child->data);
	}
	
	// watch for any new/updated/removed child
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_ADDED, G_CALLBACK(root_child_added), pData);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_MOVED, G_CALLBACK(root_child_moved), pData);
	g_signal_connect(G_OBJECT(newroot), DBUSMENU_MENUITEM_SIGNAL_CHILD_REMOVED, G_CALLBACK(root_child_delete), pData);
}

static void _main_set_menu (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableSetMenu)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "SetMenu disabled in config");
		return;
	}
	
	const gchar *cBusName;
	const gchar *cMenuPath;
	gchar *cIconQuery;
	g_variant_get (pPar, "(&s&ss)", &cBusName, &cMenuPath, &cIconQuery);
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	cd_debug ("%s (%s , %s)", __func__, cBusName, cMenuPath);
	static gboolean s_bInit = FALSE;
	if (! s_bInit)  // register for right-click events once.
	{
		s_bInit = TRUE;
		gldi_object_register_notification (&myContainerObjectMgr,
			NOTIFICATION_BUILD_ICON_MENU,
			(GldiNotificationFunc) cd_dbus_main_emit_on_build_menu,
			GLDI_RUN_FIRST,
			NULL);
	}
	
	if (cBusName && *cBusName == '\0')  // nullify empty object and path
		cBusName = NULL;
	if (cMenuPath && *cMenuPath == '\0')
		cMenuPath = NULL;
	
	Icon *pIcon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		CDIconData *pData = CD_APPLET_GET_MY_ICON_DATA (pIcon);
		if (pData == NULL)
		{
			pData = g_new0 (CDIconData, 1);
			CD_APPLET_SET_MY_ICON_DATA (pIcon, pData);
		}
		
		if (! cairo_dock_strings_differ (pData->cMenuPath, cMenuPath)
		&& ! cairo_dock_strings_differ (pData->cBusName, cBusName))
			continue;  // same menu -> nothing to do
		
		// remove any previous menu
		if (pData->cBusName)
		{
			cd_debug ("menu %s (%s) is removed", pData->cBusName, pData->cMenuPath);
			g_free (pData->cBusName);
			g_free (pData->cMenuPath);
			
			g_list_free (pData->menu_items_list);
			pData->menu_items_list = NULL;
			
			g_object_unref (pData->client);
			pData->client = NULL;
		}
		
		// remember the current menu
		pData->cBusName = g_strdup (cBusName);
		pData->cMenuPath = g_strdup (cMenuPath);
		
		// if a menu is set, build the client and wait for the root child to appear on our side of the bus.
		if (cBusName && cMenuPath && *cMenuPath != '\0')
		{
			cd_debug ("new menu %s (%s)", cBusName, cMenuPath);
			pData->client = dbusmenu_gtkclient_new(pData->cBusName, pData->cMenuPath);
			g_signal_connect(G_OBJECT(pData->client), DBUSMENU_GTKCLIENT_SIGNAL_ROOT_CHANGED, G_CALLBACK(root_changed), pData);
		}
	}
	
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}
#else
static void _main_set_menu (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
		"Cairo-Dock has not been compiled with DbusMenu support, so The 'SetMenu' method won't work.");
}
#endif


static void _main_set_progress (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableSetProgress)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "SetProgress disabled in config");
		return;
	}
	
	double fPercent;
	gchar *cIconQuery;
	g_variant_get (pPar, "(ds)", &fPercent, &cIconQuery);
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	g_free (cIconQuery);
	if (pList == NULL)
	{
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	Icon *pIcon;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		
		if (cairo_dock_get_icon_data_renderer (pIcon) == NULL)
		{
			CairoProgressBarAttribute attr;
			memset (&attr, 0, sizeof (CairoProgressBarAttribute));
			CairoDataRendererAttribute *pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
			pRenderAttr->cModelName = "progressbar";
			cairo_dock_add_new_data_renderer_on_icon (pIcon, pIcon->pContainer, pRenderAttr);
		}
		
		if (fPercent < 0)
			fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
		cairo_dock_render_new_data_on_icon (pIcon, pIcon->pContainer, NULL, &fPercent);
	}
	g_list_free (pList);
	g_dbus_method_invocation_return_value (pInv, NULL);
}



  ///////////
 /// ADD ///
///////////

static void _main_add (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableAddRemove)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Adding and removing items disabled in config");
		return;
	}
	
	GVariant *pProperties;
	g_variant_get (pPar, "(@a{sv})", &pProperties);
	
	gchar *cConfigFile = NULL; // result
	
	
	const gchar *cType = "";
	g_variant_lookup (pProperties, "type", "&s", &cType);
	CDMainType iType = cd_dbus_get_main_type (cType, -1);
	
	switch (iType)
	{
		case CD_MAIN_TYPE_ICON:
		{
			// get the dock
			const gchar *cDockName = NULL;
			if (! g_variant_lookup (pProperties, "container", "&s", &cDockName) || ! cDockName)
				cDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
			CairoDock *pParentDock = gldi_dock_get (cDockName);
			if (pParentDock == NULL)
			{
				cd_warning ("dock %s does not exist", cDockName);
				pParentDock = g_pMainDock;
			}
			
			// get the order
			double fOrder = 0;
			GVariant *v = g_variant_lookup_value (pProperties, "order", NULL);
			gboolean bValid = FALSE;
			if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("i")))
				{ fOrder = g_variant_get_int32 (v); bValid = TRUE; }
			else if (v && g_variant_is_of_type (v, G_VARIANT_TYPE ("d")))
				{ fOrder = g_variant_get_double (v); bValid = TRUE; }
			if (bValid)
			{
				if (fOrder < 0) fOrder = CAIRO_DOCK_LAST_ORDER;
			}
			else  // no order defined, look for a position
			{
				gint32 iPosition;
				if (g_variant_lookup (pProperties, "position", "i", &iPosition))
				{
					if (iPosition >= 0)
					{
						gint32 i;
						GList *ic;
						Icon *icon;
						for (ic = pParentDock->icons, i = 0; ic != NULL && i < iPosition; ic = ic->next)
						{
							icon = ic->data;
							if (GLDI_OBJECT_IS_AUTO_SEPARATOR_ICON (icon))
								continue;
							i ++;
						}
						if (ic != NULL)
						{
							Icon *pPrevIcon = (ic->prev ? ic->prev->data : NULL);
							Icon *pNextIcon = ic->data;
							if (pPrevIcon == NULL)
								fOrder = pNextIcon->fOrder - 1;
							else if (cairo_dock_get_icon_order (pNextIcon) != cairo_dock_get_icon_order (pPrevIcon))
								fOrder = pPrevIcon->fOrder + 1;
							else
								fOrder = (pNextIcon->fOrder + pPrevIcon->fOrder) / 2;
						}
						else  // at the end
							fOrder = CAIRO_DOCK_LAST_ORDER;
					}
				}
			}
			
			Icon *pNewIcon = NULL;
			if (strcmp (cType, CD_TYPE_LAUNCHER) == 0)
			{
				const gchar *cDesktopFile = NULL;
				g_variant_lookup (pProperties, "config-file", "&s", &cDesktopFile);
				
				if (cDesktopFile != NULL)
				{
					pNewIcon = gldi_launcher_add_new (cDesktopFile, pParentDock, fOrder);
				}
				else
				{
					pNewIcon = gldi_launcher_add_new (NULL, pParentDock, fOrder);
					
					// get additional properties
					const gchar *cName = NULL;
					g_variant_lookup (pProperties, "name", "&s", &cName);
					const gchar *cIcon = NULL;
					g_variant_lookup (pProperties, "icon", "&s", &cIcon);
					const gchar *cCommand = NULL;
					g_variant_lookup (pProperties, "command", "&s", &cCommand);
					const gchar *cClass = NULL;
					g_variant_lookup (pProperties, "class", "&s", &cClass);
					
					// open the conf-file and set the fields.
					gchar *cConfFilePath = (*pNewIcon->cDesktopFileName == '/' ? g_strdup (pNewIcon->cDesktopFileName) : g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, pNewIcon->cDesktopFileName));
					GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
					
					if (cName)
						g_key_file_set_string (pKeyFile, "Desktop Entry", "Name", cName);
					
					if (cIcon)
						g_key_file_set_string (pKeyFile, "Desktop Entry", "Icon", cIcon);
					
					if (cCommand)
						g_key_file_set_string (pKeyFile, "Desktop Entry", "Exec", cCommand);
					
					if (cClass)
						g_key_file_set_string (pKeyFile, "Desktop Entry", "StartupWMClass", cClass);
					
					cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
					
					g_key_file_free (pKeyFile);
					g_free (cConfFilePath);
					gldi_object_reload (GLDI_OBJECT(pNewIcon), TRUE);
				}
			}
			else if (strcmp (cType, CD_TYPE_SEPARATOR) == 0)
			{
				pNewIcon = gldi_separator_icon_add_new (pParentDock, fOrder);
			}
			else if (strcmp (cType, CD_TYPE_STACK_ICON) == 0)
			{
				pNewIcon = gldi_stack_icon_add_new (pParentDock, fOrder);
				
				// get additional properties
				const gchar *cName = NULL;
				g_variant_lookup (pProperties, "name", "&s", &cName);
				const gchar *cIcon = NULL;
				g_variant_lookup (pProperties, "icon", "&s", &cIcon);
				
				// open the conf-file and set the fields.
				gchar *cConfFilePath = (*pNewIcon->cDesktopFileName == '/' ? g_strdup (pNewIcon->cDesktopFileName) : g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, pNewIcon->cDesktopFileName));
				GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
				
				if (cName)
					g_key_file_set_string (pKeyFile, "Desktop Entry", "Name", cName);
				
				if (cIcon)
					g_key_file_set_string (pKeyFile, "Desktop Entry", "Icon", cIcon);
				
				cairo_dock_write_keys_to_file (pKeyFile, cConfFilePath);
				
				g_key_file_free (pKeyFile);
				g_free (cConfFilePath);
				gldi_object_reload (GLDI_OBJECT(pNewIcon), TRUE);
			}
			else
			{
				g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
					"can't add an icon of type '%s'", cType);
				g_variant_unref (pProperties);
				return;
			}
			if (pNewIcon != NULL && pNewIcon->cDesktopFileName != NULL)
				cConfigFile = (*pNewIcon->cDesktopFileName == '/' ? g_strdup (pNewIcon->cDesktopFileName) : g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, pNewIcon->cDesktopFileName));
		}
		break;
		
		case CD_MAIN_TYPE_CONTAINER:
		{
			if (strcmp (cType, CD_TYPE_DOCK) == 0)
			{
				gchar *cDockName = gldi_dock_add_conf_file ();
				CairoDock *pDock = gldi_dock_new (cDockName);
				if (!pDock)
				{
					g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_FAILED,
						"Cannot create new dock");
					g_variant_unref (pProperties);
					return;
				}
				cConfigFile = g_strdup_printf ("%s/%s.conf", g_cCurrentThemePath, cDockName);
				g_free (cDockName);
			}
			else
			{
				g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
					"can't add a desklet, add a module-instance instead");
				g_variant_unref (pProperties);
				return;
			}
		}
		break;
		case CD_MAIN_TYPE_MODULE:
		{
			const gchar *cModuleName = NULL;
			g_variant_lookup (pProperties, "module", "&s", &cModuleName);
			
			GldiModule *pModule = cModuleName ? gldi_module_get (cModuleName) : NULL;
			if (pModule == NULL)
			{
				if (cModuleName) g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
					G_DBUS_ERROR_FILE_NOT_FOUND, "no such module (%s)", cModuleName);
				else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
					"can't add a desklet, missing module name");
				g_variant_unref (pProperties);
				return;
			}
			if (pModule->pInstancesList == NULL)
				gldi_module_activate (pModule);
		}
		break;
		case CD_MAIN_TYPE_MODULE_INSTANCE:
		{
			const gchar *cModuleName = NULL;
			g_variant_lookup (pProperties, "module", "&s", &cModuleName);
			
			GldiModule *pModule = cModuleName ? gldi_module_get (cModuleName) : NULL;
			if (pModule == NULL)
			{
				if (cModuleName) g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
					G_DBUS_ERROR_FILE_NOT_FOUND, "no such module (%s)", cModuleName);
				else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS,
					"can't add a desklet, missing module name");
				g_variant_unref (pProperties);
				return;
			}
			
			if (pModule->pInstancesList == NULL)
			{
				gldi_module_activate (pModule);
			}
			else if (pModule->pVisitCard->bMultiInstance)
			{
				gldi_module_add_instance (pModule);
			}
			if (pModule->pInstancesList)
			{
				GldiModuleInstance *pModuleInstance = pModule->pInstancesList->data;  // prepend
				cConfigFile = g_strdup (pModuleInstance->cConfFilePath);
			}
		}
		break;
		default:
			g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
				G_DBUS_ERROR_INVALID_ARGS, "Unknown type (%s)", cType);
			g_variant_unref (pProperties);
			return;
		break;
	}
	
	g_variant_unref (pProperties);
	
	GVariant *res = NULL;
	if (cConfigFile)
	{
		GVariant *tmp = g_variant_new_take_string (cConfigFile);
		res = g_variant_new_tuple (&tmp, 1);
	}
	else res = g_variant_new ("(s)", "");
	
	g_dbus_method_invocation_return_value (pInv, res);
}


  //////////////
 /// RELOAD ///
//////////////

static void _main_reload (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableAddRemove)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Adding and removing items disabled in config");
		return;
	}
	
	gchar *cQuery = NULL;
	g_variant_get_child (pPar, 0, "s", &cQuery);
	
	GList *pObjects = cQuery ? cd_dbus_find_matching_objects (cQuery) : NULL;
	GList *o;
	GldiObject *obj;
	for (o = pObjects; o != NULL; o = o->next)
	{
		obj = o->data;
		gldi_object_reload (obj, TRUE);
	}
	g_list_free (pObjects);
	g_dbus_method_invocation_return_value (pInv, NULL);
}


  //////////////
 /// REMOVE ///
//////////////

static gboolean _on_object_deleted (GList *o, G_GNUC_UNUSED GldiObject *obj)
{
	o->data = NULL;
	return GLDI_NOTIFICATION_LET_PASS;
}
static void _main_remove (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableAddRemove)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Adding and removing items disabled in config");
		return;
	}
	
	gchar *cQuery = NULL;
	g_variant_get_child (pPar, 0, "s", &cQuery);
	GList *pObjects = cQuery ? cd_dbus_find_matching_objects (cQuery): NULL;
	
	// first connect to the "delete" signal, to not destroy 2 times an icon (case of an icon in a sub-dock that is destroyed just before).
	GldiObject *obj;
	GList *o;
	for (o = pObjects; o != NULL; o = o->next)
	{
		obj = o->data;
		gldi_object_register_notification (obj,
			NOTIFICATION_DESTROY,
			(GldiNotificationFunc) _on_object_deleted,
			GLDI_RUN_FIRST, o);
	}
	
	for (o = pObjects; o != NULL; o = o->next)
	{
		obj = o->data;
		if (! obj)  // has been deleted by a previous object destruction
			continue;
		gldi_object_delete (obj);
	}
	g_list_free (pObjects);
	g_dbus_method_invocation_return_value (pInv, NULL);
}


  //////////////////
 /// PROPERTIES ///
//////////////////

static inline GVariant *_variant_string_nonnull (const gchar *str)
{
	return g_variant_new_string (str ? str : "");
}

static void _add_icon_properties (Icon *pIcon, GVariantBuilder *pTab)
{
	GldiContainer *pContainer = cairo_dock_get_icon_container (pIcon);
	
	g_variant_builder_open (pTab, G_VARIANT_TYPE ("a{sv}"));
	
	int iPosition;
	const gchar *cType;
	const gchar *cContainerName;
	const gchar *cDesktopFile;
	
	if (CAIRO_DOCK_ICON_TYPE_IS_LAUNCHER (pIcon))
		cType = CD_TYPE_LAUNCHER;
	else if (CAIRO_DOCK_ICON_TYPE_IS_APPLI (pIcon))
		cType = CD_TYPE_APPLICATION;
	else if (CAIRO_DOCK_ICON_TYPE_IS_APPLET (pIcon))
		cType = CD_TYPE_APPLET;
	else if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		cType = CD_TYPE_SEPARATOR;
	else if (CAIRO_DOCK_ICON_TYPE_IS_CONTAINER (pIcon))
		cType = CD_TYPE_STACK_ICON;
	else if (CAIRO_DOCK_ICON_TYPE_IS_CLASS_CONTAINER (pIcon))
		cType = CD_TYPE_CLASS_ICON;
	else
		cType = CD_TYPE_ICON_OTHER;
	g_variant_builder_add (pTab, "{sv}", "type", g_variant_new_string (cType));
	
	cDesktopFile = "";
	if (pIcon->cDesktopFileName != NULL)
		cDesktopFile = pIcon->cDesktopFileName;
	else if (CAIRO_DOCK_IS_APPLET (pIcon))
		cDesktopFile = pIcon->pModuleInstance->cConfFilePath;
	g_variant_builder_add (pTab, "{sv}", "config-file", g_variant_new_string (cDesktopFile));
	
	if (CAIRO_DOCK_IS_APPLET (pIcon))
		g_variant_builder_add (pTab, "{sv}", "module", _variant_string_nonnull (
			pIcon->pModuleInstance->pModule->pVisitCard->cModuleName));
	
	g_variant_builder_add (pTab, "{sv}", "name", _variant_string_nonnull (pIcon->cName));
	g_variant_builder_add (pTab, "{sv}", "command", _variant_string_nonnull (pIcon->cCommand));
	g_variant_builder_add (pTab, "{sv}", "class", _variant_string_nonnull (pIcon->cClass));
	g_variant_builder_add (pTab, "{sv}", "icon", _variant_string_nonnull (pIcon->cFileName));
	g_variant_builder_add (pTab, "{sv}", "quick-info", _variant_string_nonnull (pIcon->cQuickInfo));
	g_variant_builder_add (pTab, "{sv}", "Xid", g_variant_new_int32 (gldi_window_get_id (pIcon->pAppli)));
	
	iPosition = -1;
	cContainerName = "";
	if (CAIRO_DOCK_IS_DOCK (pContainer))
	{
		CairoDock *pDock = CAIRO_DOCK (pContainer);
		iPosition = g_list_index (pDock->icons, pIcon);
		cContainerName = gldi_dock_get_name (pDock);
	}
	else if (CAIRO_DOCK_IS_DESKLET (pContainer))
	{
		CairoDesklet *pDesklet = CAIRO_DESKLET (pContainer);
		if (pDesklet->pIcon == pIcon)
			iPosition = 0;
		else
			iPosition = g_list_index (pDesklet->icons, pIcon);
		if (CAIRO_DOCK_IS_APPLET (pDesklet->pIcon))
			cContainerName = pDesklet->pIcon->pModuleInstance->pModule->pVisitCard->cModuleName;
	}
	
	g_variant_builder_add (pTab, "{sv}", "position", g_variant_new_int32 (iPosition));
	g_variant_builder_add (pTab, "{sv}", "container", _variant_string_nonnull (cContainerName));
	g_variant_builder_add (pTab, "{sv}", "order", g_variant_new_double (pIcon->fOrder));
	
	g_variant_builder_close (pTab);
}

#if GLIB_CHECK_VERSION (2, 84, 0)
#define _init_variant_builder g_variant_builder_init_static
#else
#define _init_variant_builder g_variant_builder_init
#endif

static void _add_module_properties (GldiModule *pModule, GVariantBuilder *pTab)
{
	g_variant_builder_open (pTab, G_VARIANT_TYPE ("a{sv}"));
	
	g_variant_builder_add (pTab, "{sv}", "type", g_variant_new_string (CD_TYPE_MODULE));
	g_variant_builder_add (pTab, "{sv}", "name", _variant_string_nonnull (pModule->pVisitCard->cModuleName));
	g_variant_builder_add (pTab, "{sv}", "module-type", g_variant_new_int32 (pModule->pVisitCard->iContainerType));
	g_variant_builder_add (pTab, "{sv}", "category", g_variant_new_int32 (pModule->pVisitCard->iCategory));
	g_variant_builder_add (pTab, "{sv}", "title", _variant_string_nonnull (pModule->pVisitCard->cTitle));
	g_variant_builder_add (pTab, "{sv}", "icon", _variant_string_nonnull (pModule->pVisitCard->cIconFilePath));
	g_variant_builder_add (pTab, "{sv}", "preview", _variant_string_nonnull (pModule->pVisitCard->cPreviewFilePath));
	g_variant_builder_add (pTab, "{sv}", "description", _variant_string_nonnull (pModule->pVisitCard->cDescription));
	g_variant_builder_add (pTab, "{sv}", "author", _variant_string_nonnull (pModule->pVisitCard->cAuthor));
	g_variant_builder_add (pTab, "{sv}", "is-multi-instance", g_variant_new_boolean (pModule->pVisitCard->bMultiInstance));
	
	cd_debug ("list instances ...");
	GVariantBuilder instances;
	_init_variant_builder (&instances, G_VARIANT_TYPE ("as"));
	
	GldiModuleInstance *pInstance;
	GList *mi;
	for (mi = pModule->pInstancesList; mi != NULL; mi = mi->next)
	{
		pInstance = mi->data;
		if (pInstance->cConfFilePath)
			g_variant_builder_add (&instances, "s", pInstance->cConfFilePath);
	}
	g_variant_builder_add (pTab, "{sv}", "instances", g_variant_new ("as", &instances));
	g_variant_builder_close (pTab);
}

static void _add_manager_properties (GldiManager *pManager, GVariantBuilder *pTab)
{
	g_variant_builder_open (pTab, G_VARIANT_TYPE ("a{sv}"));
	
	g_variant_builder_add (pTab, "{sv}", "type", g_variant_new_string (CD_TYPE_MANAGER));
	g_variant_builder_add (pTab, "{sv}", "name", _variant_string_nonnull (pManager->cModuleName));
	g_variant_builder_add (pTab, "{sv}", "config-file", _variant_string_nonnull (g_cConfFile));
	
	g_variant_builder_close (pTab);
}

static void _set_container_properties (GldiContainer *pContainer, GVariantBuilder *pTab)
{
	int x, y, w, h;
	if (pContainer->bIsHorizontal)
	{
		x = pContainer->iWindowPositionX;
		y = pContainer->iWindowPositionY;
		w = pContainer->iWidth;
		h = pContainer->iHeight;
	}
	else
	{
		y = pContainer->iWindowPositionX;
		x = pContainer->iWindowPositionY;
		h = pContainer->iWidth;
		w = pContainer->iHeight;
	}
	
	g_variant_builder_add (pTab, "{sv}", "x", g_variant_new_int32 (x));
	g_variant_builder_add (pTab, "{sv}", "y", g_variant_new_int32 (y));
	g_variant_builder_add (pTab, "{sv}", "width", g_variant_new_int32 (w));
	g_variant_builder_add (pTab, "{sv}", "height", g_variant_new_int32 (h));
	
	CairoDockPositionType iScreenBorder = ((! pContainer->bIsHorizontal) << 1) | (! pContainer->bDirectionUp);
	g_variant_builder_add (pTab, "{sv}", "orientation", g_variant_new_int32 (iScreenBorder));
}

static void _add_dock_properties (CairoDock *pDock, GVariantBuilder *pTab)
{
	g_variant_builder_open (pTab, G_VARIANT_TYPE ("a{sv}"));
	
	const gchar *cDockName = gldi_dock_get_name (pDock);
	g_variant_builder_add (pTab, "{sv}", "type", g_variant_new_string (CD_TYPE_DOCK));
	g_variant_builder_add (pTab, "{sv}", "name", _variant_string_nonnull (cDockName));
	g_variant_builder_add (pTab, "{sv}", "is-sub-dock", g_variant_new_boolean (pDock->iRefCount > 0));
	g_variant_builder_add (pTab, "{sv}", "nb-icons", g_variant_new_int32 (g_list_length (pDock->icons)));
	
	if (pDock->iRefCount == 0 && ! pDock->bIsMainDock)
	{
		gchar *cConfFilePath = g_strdup_printf ("%s/%s.conf", g_cCurrentThemePath, cDockName);
		g_variant_builder_add (pTab, "{sv}", "config-file", g_variant_new_take_string (cConfFilePath));
	}
	
	_set_container_properties (CAIRO_CONTAINER (pDock), pTab);
	
	g_variant_builder_close (pTab);
}

static void _add_desklet_properties (CairoDesklet *pDesklet, GVariantBuilder *pTab)
{
	g_variant_builder_open (pTab, G_VARIANT_TYPE ("a{sv}"));
	
	g_variant_builder_add (pTab, "{sv}", "type", g_variant_new_string (CD_TYPE_DESKLET));
	g_variant_builder_add (pTab, "{sv}", "name", _variant_string_nonnull (
		CAIRO_DOCK_IS_APPLET (pDesklet->pIcon) ? pDesklet->pIcon->pModuleInstance->pModule->pVisitCard->cModuleName : ""
	));
	g_variant_builder_add (pTab, "{sv}", "nb-icons", g_variant_new_int32 (1 + g_list_length (pDesklet->icons)));
	
	_set_container_properties (CAIRO_CONTAINER (pDesklet), pTab);
	
	g_variant_builder_close (pTab);
}

static void _add_module_instance_properties (GldiModuleInstance *pModuleInstance, GVariantBuilder *pTab)
{
	g_variant_builder_open (pTab, G_VARIANT_TYPE ("a{sv}"));
	
	g_variant_builder_add (pTab, "{sv}", "type", g_variant_new_string (CD_TYPE_MODULE_INSTANCE));
	g_variant_builder_add (pTab, "{sv}", "name", _variant_string_nonnull (pModuleInstance->pModule->pVisitCard->cModuleName));
	g_variant_builder_add (pTab, "{sv}", "config-file", _variant_string_nonnull (pModuleInstance->cConfFilePath));
	
	g_variant_builder_close (pTab);
}

static void _main_get_properties (GVariant *pPar, GDBusMethodInvocation *pInv)
{
	if (! myConfig.bEnableGetProps)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_ACCESS_DENIED, "Getting information about items is disabled in config");
		return;
	}
	
	gchar *cQuery = NULL;
	g_variant_get_child (pPar, 0, "s", &cQuery);
	GList *pObjects = cQuery ? cd_dbus_find_matching_objects (cQuery): NULL;
	
	GVariantBuilder res_builder;
	_init_variant_builder (&res_builder, G_VARIANT_TYPE ("(aa{sv})"));
	g_variant_builder_open (&res_builder, G_VARIANT_TYPE ("aa{sv}"));
	
	GList *o;
	GldiObject *obj;
	for (o = pObjects; o != NULL; o = o->next)
	{
		obj = o->data;
		if (CAIRO_DOCK_IS_ICON (obj))
		{
			Icon *pIcon = (Icon*)obj;
			_add_icon_properties (pIcon, &res_builder);
		}
		else if (CAIRO_DOCK_IS_CONTAINER (obj))
		{
			if (CAIRO_DOCK_IS_DOCK (obj))
			{
				CairoDock *pDock = CAIRO_DOCK (obj);
				_add_dock_properties (pDock, &res_builder);
			}
			else if (CAIRO_DOCK_IS_DESKLET (obj))
			{
				CairoDesklet *pDesklet = CAIRO_DESKLET (obj);
				_add_desklet_properties (pDesklet, &res_builder);
			}
		}
		else if (GLDI_OBJECT_IS_MODULE (obj))
		{
			GldiModule *pModule = (GldiModule *)obj;
			_add_module_properties (pModule, &res_builder);
		}
		else if (GLDI_OBJECT_IS_MANAGER (obj))
		{
			GldiManager *pManager = (GldiManager *)obj;
			_add_manager_properties (pManager, &res_builder);
		}
		else if (GLDI_OBJECT_IS_MODULE_INSTANCE (obj))
		{
			GldiModuleInstance *pModuleInstance = (GldiModuleInstance *)obj;
			_add_module_instance_properties (pModuleInstance, &res_builder);
		}
	}
	g_list_free (pObjects);
	
	g_variant_builder_close (&res_builder);
	
	g_dbus_method_invocation_return_value (pInv, g_variant_builder_end (&res_builder));
}


void cd_dbus_main_method_call (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cSender,
	G_GNUC_UNUSED const gchar *cObj, // object path -- will always be org.cairodock.CairoDock
	G_GNUC_UNUSED const gchar *cInterface, // interface -- will always be org.cairodock.CairoDock
	const gchar *cMethod, GVariant *pPar, GDBusMethodInvocation* pInv, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_ENTER;
	if (! myConfigPtr || ! g_pMainDock)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "Cairo-Dock is reloading, DBus interface is unavailable");
		CD_APPLET_LEAVE ();
	}
	
	     if (!strcmp (cMethod, "Reboot")) _main_reboot (pPar, pInv);
	else if (!strcmp (cMethod, "Quit")) _main_quit (pPar, pInv);
	else if (!strcmp (cMethod, "ShowDock")) _main_show_dock (pPar, pInv);
	else if (!strcmp (cMethod, "ShowDesklet")) _main_show_desklet (pPar, pInv);
	else if (!strcmp (cMethod, "SetQuickInfo")) _main_set_quick_info (pPar, pInv);
	else if (!strcmp (cMethod, "SetLabel")) _main_set_label (pPar, pInv);
	else if (!strcmp (cMethod, "SetIcon")) _main_set_icon (pPar, pInv);
	else if (!strcmp (cMethod, "SetEmblem")) _main_set_emblem (pPar, pInv);
	else if (!strcmp (cMethod, "Animate")) _main_animate (pPar, pInv);
	else if (!strcmp (cMethod, "DemandsAttention")) _main_demands_attention (pPar, pInv);
	else if (!strcmp (cMethod, "ShowDialog")) _main_show_dialog (pPar, pInv);
	else if (!strcmp (cMethod, "SetMenu")) _main_set_menu (pPar, pInv);
	else if (!strcmp (cMethod, "SetProgress")) _main_set_progress (pPar, pInv);
	else if (!strcmp (cMethod, "Add")) _main_add (pPar, pInv);
	else if (!strcmp (cMethod, "Reload")) _main_reload (pPar, pInv);
	else if (!strcmp (cMethod, "Remove")) _main_remove (pPar, pInv);
	else if (!strcmp (cMethod, "GetProperties")) _main_get_properties (pPar, pInv);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD, "Unknown method: '%s'", cMethod);
	
	CD_APPLET_LEAVE ();
}

GVariant *cd_dbus_main_get_property (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cSender,
	G_GNUC_UNUSED const gchar *cObj, G_GNUC_UNUSED const gchar *cInterface, const gchar* cProp,
	GError** error, G_GNUC_UNUSED gpointer data)
{
	// we don't have any properties
	g_set_error (error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY, "Unknown property (%s)", cProp);
	return NULL;
}

const gchar *s_cMainXml = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\""
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">"
"<node name=\"/org/cairodock/CairoDock\">"
"	<interface name=\"org.cairodock.CairoDock\">"
"		<method name=\"Reboot\">"
"		</method>"
"		<method name=\"Quit\">"
"		</method>"
"		<method name=\"ShowDock\">"
"			<arg name=\"iVisibility\" type=\"i\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ShowDesklet\">"
"			<arg name=\"widgetLayer\" type=\"b\" direction=\"in\"/>"
"		</method>"
"		"
"		<method name=\"Add\">"
"			<arg name=\"pProperties\" direction=\"in\" type=\"a{sv}\"/>"
"			<arg name=\"cConfigFile\" type=\"s\" direction=\"out\"/>"
"		</method>"
"		<method name=\"Reload\">"
"			<arg name=\"cQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"Remove\">"
"			<arg name=\"cQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"GetProperties\">"
"			<arg name=\"cQuery\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"pProperties\" direction=\"out\" type=\"aa{sv}\"/>"
"		</method>"
"		"
"		<method name=\"SetQuickInfo\">"
"			<arg name=\"cQuickInfo\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetLabel\">"
"			<arg name=\"cLabel\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetIcon\">"
"			<arg name=\"cImage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetEmblem\">"
"			<arg name=\"cImage\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iPosition\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"Animate\">"
"			<arg name=\"cAnimation\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iNbRounds\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"DemandsAttention\">"
"			<arg name=\"bStart\" type=\"b\" direction=\"in\"/>"
"			<arg name=\"cAnimation\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"ShowDialog\">"
"			<arg name=\"message\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"iDuration\" type=\"i\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetMenu\">"
"			<arg name=\"cBusName\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cMenuPath\" type=\"s\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"		<method name=\"SetProgress\">"
"			<arg name=\"fPercent\" type=\"d\" direction=\"in\"/>"
"			<arg name=\"cIconQuery\" type=\"s\" direction=\"in\"/>"
"		</method>"
"	</interface>"
"</node>";



