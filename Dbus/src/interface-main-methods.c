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

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.CreateLauncherFromScratch string:gimp.png string:"fake gimp" string:gimp string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetLabel string:new_label string:icon_name string:any string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetQuickInfo string:123 string:none string:none string:dustbin

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.Animate string:default int32:2 string:any string:firefox string:none

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock org.cairodock.CairoDock.SetIcon string:firefox-3.0 string:any string:nautilus string:none

******************************************************************************/

#include <unistd.h>
#include <glib.h>

#include "interface-main-methods.h"

#define nullify_argument(string) do {\
	if (string != NULL && (*string == '\0' || strcmp (string, "any") == 0 || strcmp (string, "none") == 0))\
		string = NULL; } while (0)

static gboolean dbus_deskletVisible = FALSE;
static guint dbus_xLastActiveWindow;

gboolean cd_dbus_main_reboot(dbusMainObject *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableReboot)
		return FALSE;
	cairo_dock_load_current_theme ();
	return TRUE;
}

gboolean cd_dbus_main_quit (dbusMainObject *pDbusCallback, GError **error)
{
	if (! myConfig.bEnableQuit)
		return FALSE;
	gtk_main_quit ();
	return TRUE;
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
			cairo_dock_emit_enter_signal (CAIRO_CONTAINER (pDock));
	}
	else
	{
		///cairo_dock_pop_down (pDock);  // ne fait rien s'il n'etait pas "popped".
		///if (pDock->bAutoHide)
			cairo_dock_emit_leave_signal (CAIRO_CONTAINER (pDock));
	}
}
gboolean cd_dbus_main_show_dock (dbusMainObject *pDbusCallback, gboolean bShow, GError **error)
{
	if (! myConfig.bEnableShowDock)
		return FALSE;
	
	if (bShow)
		cairo_dock_stop_quick_hide ();
	
	cairo_dock_foreach_docks ((GHFunc) _show_hide_one_dock, GINT_TO_POINTER (bShow));
	
	if (! bShow)
		cairo_dock_quick_hide_all_docks ();
	
	return TRUE;
}

gboolean cd_dbus_main_show_desklet (dbusMainObject *pDbusCallback, gboolean *widgetLayer, GError **error)
{
	if (! myConfig.bEnableDesklets)
		return FALSE;
	if (dbus_deskletVisible)
	{
		cairo_dock_set_desklets_visibility_to_default ();
		cairo_dock_show_xwindow (dbus_xLastActiveWindow);
	}
	else
	{
		dbus_xLastActiveWindow = cairo_dock_get_current_active_window ();
		cairo_dock_set_all_desklets_visible (widgetLayer != NULL ? *widgetLayer : FALSE);
	}
	dbus_deskletVisible = !dbus_deskletVisible;
	return TRUE;
}


gboolean cd_dbus_main_reload_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, GError **error)
{
	if (! myConfig.bEnableReloadModule)
		return FALSE;
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (pModule != NULL)
	{
		cairo_dock_reload_module (pModule, TRUE);  // TRUE <=> reload module conf file.
	}
	else
	{
		GldiManager *pManager = gldi_get_manager (cModuleName);
		if (pManager != NULL)
		{
			gldi_reload_manager (pManager, g_cConfFile);
		}
		else
		{
			cd_warning ("no module named '%s'", cModuleName);
			return FALSE;
		}
	}
	return TRUE;
}

gboolean cd_dbus_main_activate_module (dbusMainObject *pDbusCallback, const gchar *cModuleName, gboolean bActivate, GError **error)
{
	if (! myConfig.bEnableActivateModule)
		return FALSE;
	
	CairoDockModule *pModule = cairo_dock_find_module_from_name (cModuleName);
	if (pModule == NULL)
	{
		if (gldi_get_manager (cModuleName) != NULL)
			cd_warning ("Internal modules can't be (de)activated.");
		else
			cd_warning ("no such module (%s)", cModuleName);
		return FALSE;
	}
	
	if (bActivate)
		cairo_dock_activate_module_and_load (cModuleName);
	else
		cairo_dock_deactivate_module_and_unload (cModuleName);
	return TRUE;
}


typedef struct {
	const gchar *cName;
	const gchar *cCommand;
	const gchar *cClass;
	const gchar *cContainerName;
	Window Xid;
	const gchar *cDesktopFile;
	const gchar *cModuleName;
	gint iPosition;
	gboolean bMatchAll;
	GList *pMatchingIcons;
} CDQueryIconBuffer;
static gboolean _icon_is_matching (Icon *pIcon, CairoContainer *pContainer, CDQueryIconBuffer *pQuery)
{
	gboolean bOr = FALSE;
	gboolean bAnd = TRUE;  // at least 1 of the fields is not nul.
	gboolean r;
	if (pQuery->cName)
	{
		r = (pIcon->cName && strcmp (pQuery->cName, pIcon->cName) == 0);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cCommand)
	{
		r = (pIcon->cCommand && strcmp (pQuery->cCommand, pIcon->cCommand) == 0);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cClass)
	{
		r = (pIcon->cClass && g_ascii_strncasecmp (pQuery->cClass, pIcon->cClass, strlen (pIcon->cClass)) == 0);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cContainerName)
	{
		const gchar *cContainerName = NULL;
		if (CAIRO_DOCK_IS_DOCK (pContainer))
			cContainerName = pIcon->cParentDockName;
		else if (CAIRO_DOCK_IS_DESKLET (pContainer))
		{
			Icon *pMainIcon = CAIRO_DESKLET (pContainer)->pIcon;
			if (CAIRO_DOCK_IS_APPLET (pMainIcon))
				cContainerName = pMainIcon->pModuleInstance->pModule->pVisitCard->cModuleName;
		}
		r = (cContainerName && strcmp (pQuery->cContainerName, cContainerName) == 0);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->Xid != 0)
	{
		r = (pIcon->Xid == pQuery->Xid);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cDesktopFile)
	{
		r = (pIcon->cDesktopFileName && strcmp (pQuery->cDesktopFile, pIcon->cDesktopFileName) == 0);
		if (!r && CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->cConfFilePath)
		{
			gchar *str = strrchr (pIcon->pModuleInstance->cConfFilePath, '/');
			r = (str && strcmp (str, pQuery->cDesktopFile));
		}
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cModuleName)
	{
		r = (CAIRO_DOCK_IS_APPLET (pIcon) && strcmp (pQuery->cModuleName, pIcon->pModuleInstance->pModule->pVisitCard->cModuleName) == 0);
		bOr |= r;
		bAnd &= r;
	}
	
	return ((pQuery->bMatchAll && bAnd) || (!pQuery->bMatchAll && bOr));
}
static void _check_icon_matching (Icon *pIcon, CairoContainer *pContainer, CDQueryIconBuffer *pQuery)
{
	if (_icon_is_matching (pIcon, pContainer, pQuery))
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
}
static void _get_icon_at_position_in_dock (const gchar *cDockName, CairoDock *pDock, CDQueryIconBuffer *pQuery)
{
	Icon *pIcon = g_list_nth_data (pDock->icons, pQuery->iPosition);
	if (pIcon != NULL)
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
}
static gboolean _get_icon_at_position_in_desklet (CairoDesklet *pDesklet, CDQueryIconBuffer *pQuery)
{
	Icon *pIcon = g_list_nth_data (pDesklet->icons, pQuery->iPosition);
	if (pIcon != NULL)
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
	return FALSE;  // don't stop.
}
static gboolean _prepare_query (CDQueryIconBuffer *pQuery, const gchar *cKey, const gchar *cValue)
{
	g_return_val_if_fail (cKey != NULL, FALSE);
	nullify_argument (cValue);
	if (cValue == NULL)  // so we can't search "icons that don't have key"
		return FALSE;
	
	if (strcmp (cKey, "name") == 0)
		pQuery->cName = cValue;
	else if (strcmp (cKey, "command") == 0)
		pQuery->cCommand = cValue;
	else if (strcmp (cKey, "class") == 0)
		pQuery->cClass = cValue;
	else if (strcmp (cKey, "container") == 0)
		pQuery->cContainerName = cValue;
	else if (strcmp (cKey, "Xid") == 0)
		pQuery->Xid = atoi (cValue);
	else if (strcmp (cKey, "config-file") == 0)
		pQuery->cDesktopFile = cValue;
	else if (strcmp (cKey, "module") == 0)
		pQuery->cModuleName = cValue;
	else if (strcmp (cKey, "position") == 0)
		pQuery->iPosition = atoi (cValue);
	else
	{
		cd_warning ("wrong key (%s)", cKey);
		return FALSE;
	}
	return TRUE;
}
static GList *_find_matching_icons_for_key (const gchar *cKey, const gchar *cValue)
{
	//g_print ("  %s (%s, %s)\n", __func__, cKey, cValue);
	CDQueryIconBuffer query;
	memset (&query, 0, sizeof (CDQueryIconBuffer));
	query.iPosition = -1;
	query.bMatchAll = TRUE;
	
	gboolean bValidQuery = _prepare_query (&query, cKey, cValue);
	g_return_val_if_fail (bValidQuery, NULL);
	
	if (query.iPosition >= 0)
	{
		cairo_dock_foreach_docks ((GHFunc) _get_icon_at_position_in_dock, &query);
		cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _get_icon_at_position_in_desklet, &query);
	}
	else
	{
		cairo_dock_foreach_icons ((CairoDockForeachIconFunc) _check_icon_matching, &query);
	}
	return query.pMatchingIcons;
}
static GList *_find_matching_icons_for_test (gchar *cTest)
{
	g_return_val_if_fail (cTest != NULL, NULL);
	//g_print (" %s (%s)\n", __func__, cTest);
	
	gchar *str = strchr (cTest, '=');
	g_return_val_if_fail (str != NULL, NULL);
	
	*str = '\0';
	gchar *cKey = g_strstrip ((gchar*)cTest);  // g_strstrip modifies the string in place (by moving the rest of the characters forward and cutting the trailing spaces)
	gchar *cValue = g_strstrip (str+1);
	
	return _find_matching_icons_for_key (cKey, cValue);
}
static GList *_merge (GList *pList1, GList *pList2)
{
	//g_print ("%s ()\n", __func__);
	GList *pList = NULL;
	GList *ic;
	Icon *pIcon;
	for (ic = pList1; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (g_list_find (pList2, pIcon) != NULL)
			pList = g_list_prepend (pList, pIcon);
	}
	g_list_free (pList1);
	g_list_free (pList2);
	return pList;
}
static GList *_concat (GList *pList1, GList *pList2)
{
	//g_print ("%s ()\n", __func__);
	GList *pList = g_list_copy (pList2);
	GList *ic;
	Icon *pIcon;
	for (ic = pList1; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (!g_list_find (pList2, pIcon))
			pList = g_list_prepend (pList, pIcon);
	}
	g_list_free (pList1);
	g_list_free (pList2);
	return pList;
}
GList *cd_dbus_find_matching_icons (gchar *cQuery)
{
	g_return_val_if_fail (cQuery != NULL, NULL);
	//g_print ("%s (%s)\n", __func__, cQuery);
	
	gchar *str;
	str = strchr (cQuery, '|');  // a && b || c && d <=> (a && b) || (c && d)
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_icons (cQuery);
		GList *pList2 = cd_dbus_find_matching_icons (str+1);
		return _concat (pList1, pList2);
	}
	str = strchr (cQuery, '&');
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_icons (cQuery);
		GList *pList2 = cd_dbus_find_matching_icons (str+1);
		return _merge (pList1, pList2);
	}
	return _find_matching_icons_for_test (cQuery);
}
gboolean cd_dbus_main_create_launcher_from_scratch (dbusMainObject *pDbusCallback, const gchar *cIconFile, const gchar *cLabel, const gchar *cCommand, const gchar *cParentDockName, GError **error)
{
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	
	nullify_argument (cParentDockName);
	if (cParentDockName == NULL)
		cParentDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (cParentDockName);
	if (pParentDock == NULL)
	{
		cd_message ("le dock parent (%s) n'existe pas, on le cree", cParentDockName);
		pParentDock = cairo_dock_create_dock (cParentDockName, NULL);
	}
	
	Icon *pIcon = cairo_dock_create_dummy_launcher (g_strdup (cLabel),
		g_strdup (cIconFile),
		g_strdup (cCommand),
		NULL,
		CAIRO_DOCK_LAST_ORDER);
	pIcon->iTrueType = CAIRO_DOCK_ICON_TYPE_LAUNCHER;  // make it a launcher, since we have no control on it, so that the dock handles it as any launcher.
	pIcon->cParentDockName = g_strdup (cParentDockName);
	
	gchar *cGuessedClass = cairo_dock_guess_class (cCommand, NULL);
	pIcon->cClass = cairo_dock_register_class (cGuessedClass);
	g_free (cGuessedClass);
	
	cairo_dock_load_icon_buffers (pIcon, CAIRO_CONTAINER (pParentDock));
	
	cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
	cairo_dock_start_icon_animation (pIcon, pParentDock);
	
	if (pIcon->cClass != NULL)
	{
		cairo_dock_inhibite_class (pIcon->cClass, pIcon);
	}
	
	return TRUE;
}

static void _find_launcher_in_dock (Icon *pIcon, CairoDock *pDock, gpointer *data)
{
	gchar *cDesktopFile = data[0];
	Icon **pFoundIcon = data[1];
	if ((pIcon->cDesktopFileName && g_ascii_strncasecmp (cDesktopFile, pIcon->cDesktopFileName, strlen (cDesktopFile)) == 0)
	|| (pIcon->cCommand && g_ascii_strncasecmp (cDesktopFile, pIcon->cCommand, strlen (cDesktopFile)) == 0))
	{
		*pFoundIcon = pIcon;
	}
}
static Icon *cd_dbus_find_launcher (const gchar *cDesktopFile)
{
	Icon *pIcon = NULL;
	gpointer data[2];
	data[0] = (gpointer) cDesktopFile;
	data[1] = &pIcon;
	cairo_dock_foreach_icons_in_docks ((CairoDockForeachIconFunc) _find_launcher_in_dock, data);
	return pIcon;
}

gboolean cd_dbus_main_load_launcher_from_file (dbusMainObject *pDbusCallback, const gchar *cDesktopFile, GError **error)  // pris de cairo_dock_add_new_launcher_by_uri().
{
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFile != NULL, FALSE);
	
	Icon *pIcon = cd_dbus_find_launcher (cDesktopFile);
	if (pIcon != NULL)
	{
		cd_warning ("file '%s' has already been loaded", cDesktopFile);
		return FALSE;
	}
	
	pIcon = cairo_dock_create_icon_from_desktop_file (cDesktopFile);
	if (pIcon == NULL)
	{
		cd_warning ("the icon couldn't be created, check that the file '%s' exists in the current theme, and is a valid and readable .desktop file\n", cDesktopFile);
		return FALSE;
	}
	
	CairoDock * pParentDock = cairo_dock_search_dock_from_name (pIcon->cParentDockName);
	if (pParentDock != NULL)  // a priori toujours vrai puisqu'il est cree au besoin. En fait c'est probablement le main dock pour un .desktop de base.
	{
		cairo_dock_insert_icon_in_dock (pIcon, pParentDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON);
		cairo_dock_start_icon_animation (pIcon, pParentDock);
	}
	cd_debug (" => cDesktopFileName : %s\n", pIcon->cDesktopFileName);
	
	return TRUE;
}

gboolean cd_dbus_main_add_launcher (dbusMainObject *pDbusCallback, const gchar *cDesktopFilePath, gdouble fOrder, const gchar *cDockName, gchar **cLauncherFile, GError **error)
{
	*cLauncherFile = NULL;
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFilePath != NULL, FALSE);
	
	nullify_argument (cDockName);
	if (cDockName == NULL)
		cDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	
	CairoDock * pParentDock = cairo_dock_search_dock_from_name (cDockName);
		
	Icon *pNewIcon = cairo_dock_add_new_launcher_by_uri (cDesktopFilePath, pParentDock, fOrder >= 0 ? fOrder : CAIRO_DOCK_LAST_ORDER);
	if (pNewIcon != NULL)
	{
		*cLauncherFile = g_strdup (pNewIcon->cDesktopFileName);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cd_dbus_main_reload_launcher (dbusMainObject *pDbusCallback, const gchar *cLauncherFile, GError **error)
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	
	nullify_argument (cLauncherFile);
	g_return_val_if_fail (cLauncherFile != NULL, FALSE);
	
	Icon *pIcon = cd_dbus_find_launcher (cLauncherFile);
	if (pIcon == NULL)
		return FALSE;
	
	cairo_dock_reload_launcher (pIcon);
	
	return TRUE;
}

gboolean cd_dbus_main_remove_launcher (dbusMainObject *pDbusCallback, const gchar *cLauncherFile, GError **error)  // cLauncherFile can be the command, for launchers that are created from scratch.
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	
	nullify_argument (cLauncherFile);
	g_return_val_if_fail (cLauncherFile != NULL, FALSE);
	
	Icon *pIcon = cd_dbus_find_launcher (cLauncherFile);
	if (pIcon == NULL)
		return FALSE;
	
	if (pIcon->pSubDock != NULL)  // on detruit le sous-dock et ce qu'il contient.
	{
		cairo_dock_destroy_dock (pIcon->pSubDock, (pIcon->cClass != NULL ? pIcon->cClass : pIcon->cName));
		pIcon->pSubDock = NULL;
	}
	
	cairo_dock_trigger_icon_removal_from_dock (pIcon);
	
	return TRUE;
}


gboolean cd_dbus_main_set_quick_info (dbusMainObject *pDbusCallback, const gchar *cQuickInfo, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableSetQuickInfo)
		return FALSE;
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	if (pList == NULL)
		return FALSE;
	
	nullify_argument (cQuickInfo);
	
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (pContainer == NULL)
			continue;
		
		cairo_dock_set_quick_info (pIcon, pContainer, cQuickInfo);
		cairo_dock_redraw_icon (pIcon, pContainer);
	}
	
	g_list_free (pList);
	return TRUE;
}

gboolean cd_dbus_main_set_label (dbusMainObject *pDbusCallback, const gchar *cLabel, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableSetLabel)
		return FALSE;
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	if (pList == NULL)
		return FALSE;
	
	nullify_argument (cLabel);
	
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (pContainer == NULL)
			continue;
		
		cairo_dock_set_icon_name (cLabel, pIcon, pContainer);
	}
	
	g_list_free (pList);
	return TRUE;
}

gboolean cd_dbus_main_set_icon (dbusMainObject *pDbusCallback, const gchar *cImage, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableSetIcon)
		return FALSE;
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	if (pList == NULL)
		return FALSE;
	
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer == NULL)
			continue;
		
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (pContainer == NULL)
			continue;
		
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
		cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
		cairo_destroy (pIconContext);
		cairo_dock_redraw_icon (pIcon, pContainer);
	}
	
	g_list_free (pList);
	return TRUE;
}

gboolean cd_dbus_main_set_emblem (dbusMainObject *pDbusCallback, const gchar *cImage, gint iPosition, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableSetIcon)
		return FALSE;
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	if (pList == NULL)
		return FALSE;
	
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon->pIconBuffer == NULL)
			continue;
		
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (pContainer == NULL)
			continue;
		
		cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	
		CairoEmblem *pEmblem = cairo_dock_make_emblem (cImage, pIcon, pContainer);
		pEmblem->iPosition = iPosition;
		cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, pContainer);
		cairo_dock_free_emblem (pEmblem);

		cairo_destroy (pIconContext);
		cairo_dock_redraw_icon (pIcon, pContainer);
	}
	
	g_list_free (pList);
	return TRUE;
}

gboolean cd_dbus_main_animate (dbusMainObject *pDbusCallback, const gchar *cAnimation, gint iNbRounds, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableAnimateIcon || cAnimation == NULL)
		return FALSE;
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	if (pList == NULL)
		return FALSE;
	
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (! CAIRO_DOCK_IS_DOCK (pContainer))
			continue;
		cairo_dock_request_icon_animation (pIcon, CAIRO_DOCK (pContainer), cAnimation, iNbRounds);
	}
	
	g_list_free (pList);
	return TRUE;
}

gboolean cd_dbus_main_show_dialog (dbusMainObject *pDbusCallback, const gchar *message, gint iDuration, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnablePopUp)
		return FALSE;
	g_return_val_if_fail (message != NULL, FALSE);
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (! CAIRO_DOCK_IS_DOCK (pContainer))
			continue;
		cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
		break;  // only show 1 dialog.
	}
	
	if (ic == NULL)  // empty list, or didn't find a valid icon.
		cairo_dock_show_general_message (message, 1000 * iDuration);
	
	g_list_free (pList);
	return TRUE;
}
