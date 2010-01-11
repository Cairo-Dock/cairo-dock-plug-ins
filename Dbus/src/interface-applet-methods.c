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

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/my_applet org.cairodock.CairoDock.applet.SetLabel string:new_label

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/demo  org.cairodock.CairoDock.applet.AddDataRenderer string:gauge int32:2 string:Turbo-night-fuel

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/demo  org.cairodock.CairoDock.applet.RenderValues array:double:.7,.2

******************************************************************************/

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"

static inline CairoDockModuleInstance *_get_module_instance_from_dbus_applet (dbusApplet *pDbusApplet)
{
	CairoDockModule *pModule = cairo_dock_find_module_from_name (pDbusApplet->cModuleName);
	g_return_val_if_fail (pModule != NULL && pModule->pInstancesList != NULL, NULL);
	
	return pModule->pInstancesList->data;
}

static inline gboolean _get_icon_and_container_from_id (dbusApplet *pDbusApplet, const gchar *cIconID, Icon **pIcon, CairoContainer **pContainer)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	if (cIconID == NULL)
	{
		*pIcon = pInstance->pIcon;
		*pContainer = pInstance->pContainer;
	}
	else
	{
		GList *piconsList = (pInstance->pDock ? (pInstance->pIcon->pSubDock ? pInstance->pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		*pIcon = cairo_dock_get_icon_with_command (piconsList, cIconID);
		*pContainer = (pInstance->pDesklet ? CAIRO_CONTAINER (pInstance->pDesklet) : CAIRO_CONTAINER (pInstance->pIcon->pSubDock));
	}
	g_return_val_if_fail (pIcon != NULL && pContainer != NULL, FALSE);
	return TRUE;
}


static gboolean _applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_quick_info (pCairoContext, cQuickInfo, pIcon, fMaxScale);
	cairo_destroy (pCairoContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (pContainer);
	cairo_dock_set_icon_name (pCairoContext, cLabel, pIcon, pContainer);
	cairo_destroy (pCairoContext);
	return TRUE;
}

static gboolean _applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_set_emblem (dbusApplet *pDbusApplet, const gchar *cImage, gint iPosition, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	
	CairoEmblem *pEmblem = cairo_dock_make_emblem (cImage, pIcon, pContainer, pIconContext);
	pEmblem->iPosition = iPosition;
	cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, pContainer);
	cairo_dock_free_emblem (pEmblem);
	
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (CAIRO_DOCK_IS_DOCK (pContainer) && cAnimation != NULL)
	{
		cairo_dock_request_icon_animation (pIcon, CAIRO_DOCK (pContainer), cAnimation, iNbRounds);
		return TRUE;
	}
	return FALSE;
}

static gboolean _applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
	return TRUE;
}


  ///////////////////////////////////////////////////
 ////////// sub-applet interface methods ///////////
///////////////////////////////////////////////////

gboolean cd_dbus_sub_applet_set_quick_info (dbusSubApplet *pDbusSubApplet, const gchar *cQuickInfo, const gchar *cIconID, GError **error)
{
	return _applet_set_quick_info (pDbusSubApplet->pApplet, cQuickInfo, cIconID, error);
}

gboolean cd_dbus_sub_applet_set_label (dbusSubApplet *pDbusSubApplet, const gchar *cLabel, const gchar *cIconID, GError **error)
{
	return _applet_set_label (pDbusSubApplet->pApplet, cLabel, cIconID, error);
}

gboolean cd_dbus_sub_applet_set_icon (dbusSubApplet *pDbusSubApplet, const gchar *cImage, const gchar *cIconID, GError **error)
{
	return _applet_set_icon (pDbusSubApplet->pApplet, cImage, cIconID, error);
}

gboolean cd_dbus_sub_applet_set_emblem (dbusSubApplet *pDbusSubApplet, const gchar *cImage, gint iPosition, const gchar *cIconID, GError **error)
{
	return _applet_set_emblem (pDbusSubApplet->pApplet, cImage, iPosition, cIconID, error);
}

gboolean cd_dbus_sub_applet_animate (dbusSubApplet *pDbusSubApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID, GError **error)
{
	return _applet_animate (pDbusSubApplet->pApplet, cAnimation, iNbRounds, cIconID, error);
}

gboolean cd_dbus_sub_applet_show_dialog (dbusSubApplet *pDbusSubApplet, const gchar *message, gint iDuration, const gchar *cIconID, GError **error)
{
	return _applet_show_dialog (pDbusSubApplet->pApplet, message, iDuration, cIconID, error);
}

gboolean cd_dbus_sub_applet_add_sub_icons (dbusSubApplet *pDbusSubApplet, const gchar **pIconFields, GError **error)
{
	g_print ("%s ()\n", __func__);
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusSubApplet->pApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	GList *pCurrentIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
	Icon *pLastIcon = cairo_dock_get_last_icon (pCurrentIconsList);
	int n = (pLastIcon ? pLastIcon->fOrder + 1 : 0);
	
	GList *pIconsList = NULL;
	Icon *pOneIcon;
	int i;
	for (i = 0; pIconFields[3*i] && pIconFields[3*i+1] && pIconFields[3*i+2]; i ++)
	{
		pOneIcon = g_new0 (Icon, 1);
		pOneIcon->cName = g_strdup (pIconFields[3*i]);
		pOneIcon->cFileName = g_strdup (pIconFields[3*i+1]);
		pOneIcon->fOrder = i + n;
		pOneIcon->fScale = 1.;
		pOneIcon->fAlpha = 1.;
		pOneIcon->fWidthFactor = 1.;
		pOneIcon->fHeightFactor = 1.;
		pOneIcon->cCommand = g_strdup (pIconFields[3*i+2]);
		pIconsList = g_list_append (pIconsList, pOneIcon);
	}
	if (pIconFields[3*i] != NULL)
	{
		cd_warning ("the number of argument is incorrect\nThis may result in an incorrect number of loaded icons.");
	}
	
	if (pInstance->pDock)
	{
		if (pIcon->pSubDock == NULL)
		{
			cairo_t *pDrawContext = cairo_dock_create_context_from_container (pContainer);
			if (pIcon->cName == NULL)
				cairo_dock_set_icon_name (pDrawContext, pInstance->pModule->pVisitCard->cModuleName, pIcon, pContainer);
			if (cairo_dock_check_unique_subdock_name (pIcon))
				cairo_dock_set_icon_name (pDrawContext, pIcon->cName, pIcon, pContainer);
			cairo_destroy (pDrawContext);
			pIcon->pSubDock = cairo_dock_create_subdock_from_scratch (pIconsList, pIcon->cName, pInstance->pDock);
			//cairo_dock_set_renderer (pIcon->pSubDock, cRenderer);
			cairo_dock_update_dock_size (pIcon->pSubDock);
		}
		else
		{
			GList *ic;
			for (ic = pIconsList; ic != NULL; ic = ic->next)
			{
				pOneIcon = ic->data;
				cairo_dock_load_one_icon_from_scratch (pOneIcon, CAIRO_CONTAINER (pIcon->pSubDock));
				cairo_dock_insert_icon_in_dock (pOneIcon, pIcon->pSubDock, ! CAIRO_DOCK_UPDATE_DOCK_SIZE, ! CAIRO_DOCK_ANIMATE_ICON);
			}
			cairo_dock_update_dock_size (pIcon->pSubDock);
			g_list_free (pIconsList);
		}
	}
	else
	{
		if (pIcon->pSubDock != NULL)  // precaution.
		{
			cairo_dock_destroy_dock (pIcon->pSubDock, pIcon->cName, NULL, NULL);
			pIcon->pSubDock = NULL;
		}
		pInstance->pDesklet->icons = g_list_concat (pInstance->pDesklet->icons, pIconsList);
		gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
		cairo_dock_set_desklet_renderer_by_name (pInstance->pDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, (CairoDeskletRendererConfigPtr) data);
	}
	
	return TRUE;
}

gboolean cd_dbus_sub_applet_remove_sub_icon (dbusSubApplet *pDbusSubApplet, const gchar *cIconID, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusSubApplet->pApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	if (cIconID == NULL || strcmp (cIconID, "any") == 0)  // remove all
	{
		if (pInstance->pDesklet && pInstance->pDesklet->icons != NULL)
		{
			g_list_foreach (pInstance->pDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (pInstance->pDesklet->icons);
			pInstance->pDesklet->icons = NULL;
		}
		if (pIcon->pSubDock != NULL)
		{
			if (pInstance->pDesklet)
			{
				cairo_dock_destroy_dock (pIcon->pSubDock, pIcon->cName, NULL, NULL);
				pIcon->pSubDock = NULL;
			}
			else
			{
				g_list_foreach (pIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
				g_list_free (pIcon->pSubDock->icons);
				pIcon->pSubDock->icons = NULL;
				pIcon->pSubDock->pFirstDrawnElement = NULL;
			}
		}
	}
	else
	{
		GList *pIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		Icon *pOneIcon = cairo_dock_get_icon_with_command (pIconsList, cIconID);
		if (pInstance->pDock)
		{
			cairo_dock_detach_icon_from_dock (pOneIcon, pIcon->pSubDock, FALSE);
			cairo_dock_update_dock_size (pIcon->pSubDock);
		}
		else
		{
			pInstance->pDesklet->icons = g_list_remove (pInstance->pDesklet->icons, pOneIcon);
			gtk_widget_queue_draw (pInstance->pDesklet->container.pWidget);
		}
		cairo_dock_free_icon (pOneIcon);
	}
	
	return TRUE;
}


  ///////////////////////////////////////////////
 ////////// applet interface methods ///////////
///////////////////////////////////////////////

gboolean cd_dbus_applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, GError **error)
{
	return _applet_set_quick_info (pDbusApplet, cQuickInfo, NULL, error);
}

gboolean cd_dbus_applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, GError **error)
{
	return _applet_set_label (pDbusApplet, cLabel, NULL, error);
}

gboolean cd_dbus_applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, GError **error)
{
	return _applet_set_icon (pDbusApplet, cImage, NULL, error);
}

gboolean cd_dbus_applet_set_emblem (dbusApplet *pDbusApplet, const gchar *cImage, gint iPosition, GError **error)
{
	return _applet_set_emblem (pDbusApplet, cImage, iPosition, NULL, error);
}

gboolean cd_dbus_applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, GError **error)
{
	return _applet_animate (pDbusApplet, cAnimation, iNbRounds, NULL, error);
}

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error)
{
	g_print ("%s (%s)\n", __func__, message);
	return _applet_show_dialog (pDbusApplet, message, iDuration, NULL, error);
}

gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	CairoDataRendererAttribute *pRenderAttr;  // les attributs du data-renderer global.
	if (strcmp (cType, "gauge") == 0)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		attr.cThemePath = cairo_dock_get_gauge_theme_path (cTheme, CAIRO_DOCK_ANY_THEME);
	}
	else if (strcmp (cType, "gauge") == 0)
	{
		CairoGraphAttribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		pRenderAttr->iMemorySize = (pIcon->fWidth > 1 ? pIcon->fWidth : 32);  // fWidht peut etre <= 1 en mode desklet au chargement.
		// Line;Plain;Bar;Circle;Plain Circle
		if (cTheme == NULL || strcmp (cTheme, "Line") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_LINE;
		else if (strcmp (cTheme, "Plain") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_PLAIN;
		else if (strcmp (cTheme, "Bar") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_BAR;
		else if (strcmp (cTheme, "Circle") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_CIRCLE;
		else if (strcmp (cTheme, "Plain Circle") == 0)
			attr.iType = CAIRO_DOCK_GRAPH2_CIRCLE_PLAIN;
		attr.iRadius = 10;
		attr.bMixGraphs = FALSE;
		double *fHighColor = g_new (double, iNbValues*3);
		double *fLowColor = g_new (double, iNbValues*3);
		int i;
		for (i = 0; i < iNbValues; i ++)
		{
			fHighColor[3*i] = 1;
			fHighColor[3*i+1] = 0;
			fHighColor[3*i+2] = 0;
			fLowColor[3*i] = 0;
			fLowColor[3*i+1] = 1;
			fLowColor[3*i+2] = 1;
		}
		attr.fHighColor = fHighColor;
		attr.fLowColor = fLowColor;
		attr.fBackGroundColor[0] = 0;
		attr.fBackGroundColor[0] = 0;
		attr.fBackGroundColor[0] = 1;
		attr.fBackGroundColor[0] = .4;
	}
	else if (strcmp (cType, "bar") == 0)
	{
		/// A FAIRE...
	}
	
	if (pRenderAttr == NULL)
		return FALSE;
	
	pRenderAttr->iLatencyTime = 500;  // 1/2s
	pRenderAttr->iNbValues = iNbValues;
	//pRenderAttr->bUpdateMinMax = TRUE;
	//pRenderAttr->bWriteValues = TRUE;
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
	if (pIcon->pDataRenderer == NULL)
		cairo_dock_add_new_data_renderer_on_icon (pIcon, pContainer, pDrawContext, pRenderAttr);
	else
		cairo_dock_reload_data_renderer_on_icon (pIcon, pContainer, pDrawContext, pRenderAttr);
	cairo_destroy (pDrawContext);
	
	return TRUE;
}

gboolean cd_dbus_applet_render_values (dbusApplet *pDbusApplet, GArray *pValues, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_render_new_data_on_icon (pIcon, pContainer, myDrawContext, (double *)pValues->data);
	cairo_destroy (pDrawContext);
	
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_applet_control_appli (dbusApplet *pDbusApplet, const gchar *cApplicationClass, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	gchar *cClass = (cApplicationClass ? g_ascii_strdown (cApplicationClass, -1) : NULL);
	if (cairo_dock_strings_differ (pIcon->cClass, cClass))
	{
		if (pIcon->cClass != NULL)
			cairo_dock_deinhibate_class (pIcon->cClass, pIcon);
		if (cClass != NULL)
			cairo_dock_inhibate_class (cClass, pIcon);
		if (! cairo_dock_is_loading ())
		{
			CairoContainer *pContainer = pInstance->pContainer;
			if (pContainer != NULL)
				cairo_dock_redraw_icon (pIcon, pContainer);
		}
	}
	
	g_free (cClass);
	return TRUE;
}

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error)
{
	if (myData.pModuleSubMenu == NULL || pDbusApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'PopulateMenu' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return FALSE;
	}
	
	int i;
	for (i = 0; pLabels[i] != NULL; i ++)
	{
		if (*pLabels[i] == '\0')
		{
			gtk_menu_shell_append (GTK_MENU_SHELL (myData.pModuleSubMenu), gtk_separator_menu_item_new ());
		}
		else
		{
			cairo_dock_add_in_menu_with_stock_and_data (pLabels[i],
				NULL,
				(GFunc) cd_dbus_emit_on_menu_select,
				myData.pModuleSubMenu,
				GINT_TO_POINTER (i));
		}
	}
	gtk_widget_show_all (myData.pModuleSubMenu);
	
	return TRUE;
}
