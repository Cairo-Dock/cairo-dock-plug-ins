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
	
	cairo_dock_set_quick_info (pIcon, pContainer, cQuickInfo && *cQuickInfo != '\0' ? cQuickInfo : NULL);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	cairo_dock_set_icon_name (cLabel, pIcon, pContainer);
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
	
	CairoEmblem *pEmblem = cairo_dock_make_emblem (cImage, pIcon, pContainer);
	pEmblem->iPosition = iPosition;
	cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, pContainer);
	cairo_dock_free_emblem (pEmblem);
	
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
	
	// On empeche l'accumulation de dialogues informatifs.
	cairo_dock_remove_dialog_if_any_full (pIcon, FALSE);  // hors dialogues interactifs.
	
	cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
	return TRUE;
}

static gboolean _applet_ask_question (dbusApplet *pDbusApplet, const gchar *cMessage, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	pDbusApplet->pDialog = cairo_dock_show_dialog_with_question (cMessage, pIcon, pContainer, "same icon", (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_question, pDbusApplet, NULL);
	return TRUE;
}

static gboolean _applet_ask_value (dbusApplet *pDbusApplet, const gchar *cMessage, gdouble fInitialValue, gdouble fMaxValue, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	pDbusApplet->pDialog = cairo_dock_show_dialog_with_value (cMessage, pIcon, pContainer, "same icon", fInitialValue, fMaxValue, (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_value, pDbusApplet, NULL);
	return TRUE;
}

static gboolean _applet_ask_text (dbusApplet *pDbusApplet, const gchar *cMessage, const gchar *cInitialText, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	pDbusApplet->pDialog = cairo_dock_show_dialog_with_entry (cMessage, pIcon, pContainer, "same icon", cInitialText, (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_text, pDbusApplet, NULL);
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

gboolean cd_dbus_sub_applet_show_dialog (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, gint iDuration, const gchar *cIconID, GError **error)
{
	return _applet_show_dialog (pDbusSubApplet->pApplet, cMessage, iDuration, cIconID, error);
}

gboolean cd_dbus_sub_applet_ask_question (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, const gchar *cIconID, GError **error)
{
	return _applet_ask_question (pDbusSubApplet->pApplet, cMessage, cIconID, error);
}

gboolean cd_dbus_sub_applet_ask_value (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, gdouble fInitialValue, gdouble fMaxValue, const gchar *cIconID, GError **error)
{
	return _applet_ask_value (pDbusSubApplet->pApplet, cMessage, fInitialValue, fMaxValue, cIconID, error);
}

gboolean cd_dbus_sub_applet_ask_text (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, const gchar *cInitialText, const gchar *cIconID, GError **error)
{
	return _applet_ask_text (pDbusSubApplet->pApplet, cMessage, cInitialText, cIconID, error);
}


gboolean cd_dbus_sub_applet_add_sub_icons (dbusSubApplet *pDbusSubApplet, const gchar **pIconFields, GError **error)
{
	//g_print ("%s ()\n", __func__);
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
		pOneIcon = cairo_dock_create_dummy_launcher (g_strdup (pIconFields[3*i]),
			g_strdup (pIconFields[3*i+1]),
			g_strdup (pIconFields[3*i+2]),
			NULL,
			i + n);
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
			if (pIcon->cName == NULL)
				cairo_dock_set_icon_name (pInstance->pModule->pVisitCard->cModuleName, pIcon, pContainer);
			if (cairo_dock_check_unique_subdock_name (pIcon))
				cairo_dock_set_icon_name (pIcon->cName, pIcon, pContainer);
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
				cairo_dock_load_icon_buffers (pOneIcon, CAIRO_CONTAINER (pIcon->pSubDock));
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
			cairo_dock_destroy_dock (pIcon->pSubDock, pIcon->cName);
			pIcon->pSubDock = NULL;
		}
		pInstance->pDesklet->icons = g_list_concat (pInstance->pDesklet->icons, pIconsList);
		gpointer data[2] = {GINT_TO_POINTER (TRUE), GINT_TO_POINTER (FALSE)};
		cairo_dock_set_desklet_renderer_by_name (pInstance->pDesklet, "Caroussel", (CairoDeskletRendererConfigPtr) data);
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
				cairo_dock_destroy_dock (pIcon->pSubDock, pIcon->cName);
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

gboolean cd_dbus_applet_demands_attention (dbusApplet *pDbusApplet, gboolean bStart, const gchar *cAnimation, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, NULL, &pIcon, &pContainer))
		return FALSE;
	
	if (bStart)
	{
		if (CAIRO_DOCK_IS_DOCK (pContainer))
		{
			cairo_dock_request_icon_attention (pIcon, CAIRO_DOCK (pContainer), cAnimation, 0);  // 0 <=> sans arret.
		}
	}
	else if (pIcon->bIsDemandingAttention)
	{
		cairo_dock_stop_icon_attention (pIcon, CAIRO_DOCK (pContainer));
	}
	return TRUE;
}

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_show_dialog (pDbusApplet, message, iDuration, NULL, error);
}

gboolean cd_dbus_applet_ask_question (dbusApplet *pDbusApplet, const gchar *message, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_ask_question (pDbusApplet, message, NULL, error);
}

gboolean cd_dbus_applet_ask_value (dbusApplet *pDbusApplet, const gchar *message, gdouble fInitialValue, gdouble fMaxValue, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_ask_value (pDbusApplet, message, fInitialValue, fMaxValue, NULL, error);
}

gboolean cd_dbus_applet_ask_text (dbusApplet *pDbusApplet, const gchar *message, const gchar *cInitialText, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_ask_text (pDbusApplet, message, cInitialText, NULL, error);
}


gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
	if (strcmp (cType, "gauge") == 0)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		///attr.cThemePath = cairo_dock_get_gauge_theme_path (cTheme, CAIRO_DOCK_ANY_THEME);
		attr.cThemePath = cairo_dock_get_data_renderer_theme_path ("gauge", cTheme, CAIRO_DOCK_ANY_THEME);
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
	if (pIcon->pDataRenderer == NULL)
		cairo_dock_add_new_data_renderer_on_icon (pIcon, pContainer, pRenderAttr);
	else
		cairo_dock_reload_data_renderer_on_icon (pIcon, pContainer, pRenderAttr);
	
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
	cairo_dock_render_new_data_on_icon (pIcon, pContainer, pDrawContext, (double *)pValues->data);
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

gboolean cd_dbus_applet_show_appli (dbusApplet *pDbusApplet, gboolean bShow, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL && pIcon->Xid != 0, FALSE);
	
	if (bShow)
		cairo_dock_show_xwindow (pIcon->Xid);
	else
		cairo_dock_minimize_xwindow (pIcon->Xid);
	
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

gboolean cd_dbus_applet_add_menu_items (dbusApplet *pDbusApplet, GPtrArray *pItems, GError **error)
{
	if (myData.pModuleMainMenu == NULL || myData.pModuleSubMenu == NULL || pDbusApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'AddMenuItems' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return FALSE;
	}
	
	// on recupere la position du sous-menu par defaut, afin d'inserer les items apres lui dans le menu principal.
	GList *pChildren = gtk_container_get_children (GTK_CONTAINER (myData.pModuleMainMenu));
	GList *c = g_list_find (pChildren, myData.pModuleSubMenu);
	GtkMenuItem *item;
	for (c = pChildren; c != NULL; c = c->next)
	{
		item = c->data;
		if (gtk_menu_item_get_submenu (item) == myData.pModuleSubMenu)
			break;
	}
	g_return_val_if_fail (c, FALSE);
	int iPosition = g_list_position (pChildren, c) + 1;
	g_list_free (pChildren);
	
	// table des menus et groupes de radio-boutons.
	GHashTable *pSubMenus = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	GHashTable *pGroups = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	
	// on parcours la liste des items.
	GHashTable *pItem;
	GtkWidget *pMenuItem, *pMenu;
	GSList *group = NULL;
	GValue *v;
	guint i;
	for (i = 0; i < pItems->len; i ++)
	{
		pItem = g_ptr_array_index (pItems, i);
		
		// on recupere ses proprietes.
		const gchar *cLabel = NULL, *cIcon = NULL, *cToolTip = NULL;
		int iType = 0, iMenuID = -1, id = i, iGroupID = 0;
		gboolean bState = FALSE;
		gpointer data;
		
		v = g_hash_table_lookup (pItem, "type");
		if (v && G_VALUE_HOLDS_INT (v))
			iType = g_value_get_int (v);
		
		v = g_hash_table_lookup (pItem, "label");
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabel = g_value_get_string (v);
		
		v = g_hash_table_lookup (pItem, "id");
		if (v && G_VALUE_HOLDS_INT (v))
			id = g_value_get_int (v);
		data = GINT_TO_POINTER (id);
		
		v = g_hash_table_lookup (pItem, "state");
		if (v && G_VALUE_HOLDS_BOOLEAN (v))
			bState = g_value_get_boolean (v);
		
		v = g_hash_table_lookup (pItem, "group");
		if (v && G_VALUE_HOLDS_INT (v))
		{
			iGroupID = g_value_get_int (v);
			group = g_hash_table_lookup (pGroups, &iGroupID);  // si NULL, ca fera un nouveau groupe.
		}
		else  // si on ne definit pas le groupe, c'est donc le groupe en cours qui est utilise, ou un nouveau groupe si encore aucun n'est en cours.
			iGroupID = id;  // utilise seulement si le groupe est nouvellement cree, pour l'enregistrer.
		
		// on cree l'item suivant son type.
		switch (iType)
		{
			case 0 :  // normal entry
				pMenuItem = gtk_image_menu_item_new_with_label (cLabel);
				g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (cd_dbus_emit_on_menu_select), data);
			break;
			case 1:  // sub-menu
				pMenuItem = gtk_image_menu_item_new_with_label (cLabel);
				GtkWidget *pSubMenu = gtk_menu_new ();
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
				int *pID = g_new (int, 1);
				*pID = id;
				g_hash_table_insert (pSubMenus, pID, pSubMenu);
			break;
			case 2:  // separator
				pMenuItem = gtk_separator_menu_item_new ();
			break;
			case 3:  // check-box
				pMenuItem = gtk_check_menu_item_new_with_label (cLabel);
				if (bState)
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(pMenuItem), bState);
				g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(cd_dbus_emit_on_menu_select), data);
			break;
			case 4:  // group-box
				pMenuItem = gtk_radio_menu_item_new_with_label (group, cLabel);
				if (group == NULL)  // le groupe ne change plus par la suite (g_list_append).
				{
					group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM(pMenuItem));
					int *pID = g_new (int, 1);
					*pID = iGroupID;
					g_hash_table_insert (pGroups, pID, group);
				}
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(pMenuItem), bState);
				g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(cd_dbus_emit_on_menu_select), data);
			break;
			default:
				continue;
		}
		
		// on lui rajoute son icone et son tooltip.
		if (iType == 0 || iType == 1)
		{
			v = g_hash_table_lookup (pItem, "icon");
			if (v && G_VALUE_HOLDS_STRING (v))
			{
				cIcon = g_value_get_string (v);
				if (cIcon)
				{
					GtkWidget *image = NULL;
					if (*cIcon == '/')
					{
						GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIcon, 16, 16, NULL);
						if (pixbuf)
						{
							image = gtk_image_new_from_pixbuf (pixbuf);
							g_object_unref (pixbuf);
						}
					}
					else
					{
						image = gtk_image_new_from_stock (cIcon, GTK_ICON_SIZE_MENU);
					}
					gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
				}
			}
		}
		
		v = g_hash_table_lookup (pItem, "tooltip");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cToolTip = g_value_get_string (v);
			gtk_widget_set_tooltip_text (pMenuItem, cToolTip);
		}
		
		// on l'insere dans son menu.
		v = g_hash_table_lookup (pItem, "menu");
		if (v && G_VALUE_HOLDS_INT (v))
			iMenuID = g_value_get_int (v);
		if (iMenuID == 0)
			pMenu = myData.pModuleMainMenu;
		else if (iMenuID == -1)
			pMenu = myData.pModuleSubMenu;
		else
		{
			pMenu = g_hash_table_lookup (pSubMenus, &iMenuID);
			if (pMenu == NULL)
				pMenu = myData.pModuleSubMenu;
		}
		
		if (pMenu == myData.pModuleMainMenu)
			gtk_menu_shell_insert (GTK_MENU_SHELL (pMenu), pMenuItem, iPosition++);
		else
			gtk_menu_shell_append (GTK_MENU_SHELL (pMenu), pMenuItem);
	}
	
	g_hash_table_destroy (pSubMenus);
	g_hash_table_destroy (pGroups);
	gtk_widget_show_all (myData.pModuleMainMenu);
	
	return TRUE;
}


gboolean cd_dbus_applet_get (dbusApplet *pDbusApplet, const gchar *cProperty, GValue *v, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, cProperty);
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	// x, y, orientation, type, width, height
	if (strcmp (cProperty, "x") == 0)
	{
		int x;
		if (pContainer->bIsHorizontal)
		{
			x = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		}
		else
		{
			x = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
		}
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, x);
	}
	else if (strcmp (cProperty, "y") == 0)
	{
		int y;
		if (pContainer->bIsHorizontal)
		{
			y = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
		}
		else
		{
			y = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		}
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, y);
	}
	else if (strcmp (cProperty, "orientation") == 0)
	{
		CairoDockPositionType iScreenBorder = (g_pMainDock ? ((! g_pMainDock->container.bIsHorizontal) << 1) | (! g_pMainDock->container.bDirectionUp) : 0);
		g_value_init (v, G_TYPE_UINT);
		g_value_set_uint (v, iScreenBorder);
	}
	else if (strcmp (cProperty, "container") == 0)
	{
		g_value_init (v, G_TYPE_UINT);
		g_value_set_uint (v, pContainer->iType);
	}
	else if (strcmp (cProperty, "width") == 0)
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, pContainer, &iWidth, &iHeight);
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iWidth);
	}
	else if (strcmp (cProperty, "height") == 0)
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, pContainer, &iWidth, &iHeight);
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iHeight);
	}
	else if (strncmp (cProperty, "Xid", 3) == 0)
	{
		Window Xid = pIcon->Xid;
		g_value_init (v, G_TYPE_UINT64);
		g_value_set_uint64 (v, Xid);
	}
	else if (strcmp (cProperty, "has_focus") == 0)
	{
		gboolean bHasFocus = (pIcon->Xid != 0 && pIcon->Xid == cairo_dock_get_current_active_window ());
		g_value_init (v, G_TYPE_BOOLEAN);
		g_value_set_boolean (v, bHasFocus);
	}
	else
	{
		g_set_error (error, 1, 1, "the property %s doesn't exist", cProperty);
		return FALSE;
	}
	return TRUE;
}

gboolean cd_dbus_applet_get_all (dbusApplet *pDbusApplet, GHashTable **hProperties, GError **error)
{
	cd_debug ("%s ()\n", __func__);
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	int x, y;
	if (pContainer->bIsHorizontal)
	{
		x = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		y = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
	}
	else
	{
		y = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		x = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
	}
	CairoDockPositionType iScreenBorder = ((! pContainer->bIsHorizontal) << 1) | (! pContainer->bDirectionUp);
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pIcon, pContainer, &iWidth, &iHeight);
	
	Window Xid = pIcon->Xid;
	gboolean bHasFocus = (pIcon->Xid != 0 && pIcon->Xid == cairo_dock_get_current_active_window ());
	
	GHashTable *h = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		g_free);
	*hProperties = h;
	GValue *v;
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, x);
	g_hash_table_insert (h, g_strdup ("x"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, y);
	g_hash_table_insert (h, g_strdup ("y"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	g_value_set_uint (v, iScreenBorder);
	g_hash_table_insert (h, g_strdup ("orientation"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	g_value_set_uint (v, pContainer->iType);
	g_hash_table_insert (h, g_strdup ("container"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, iWidth);
	g_hash_table_insert (h, g_strdup ("width"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, iHeight);
	g_hash_table_insert (h, g_strdup ("height"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT64);
	g_value_set_uint64(v, Xid);
	g_hash_table_insert (h, g_strdup ("Xid"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_BOOLEAN);
	g_value_set_boolean (v, bHasFocus);
	g_hash_table_insert (h, g_strdup ("has_focus"), v);
	
	return TRUE;
}
