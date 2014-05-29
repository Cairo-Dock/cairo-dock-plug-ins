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

#include "cairo-dock.h"
#include "interface-main-query.h"


  /////////////
 /// QUERY ///
/////////////

typedef struct {
	const gchar *cType;
	const gchar *cName;
	const gchar *cCommand;
	const gchar *cClass;
	const gchar *cContainerName;
	guint Xid;  // ID of a window, as returned by gldi_window_get_id(); when using X, it's a 'Window' type
	const gchar *cDesktopFile;
	const gchar *cModuleName;
	gint iPosition;
	GList *pMatchingIcons;
} CDQuery;

static gboolean _prepare_query (CDQuery *pQuery, const gchar *cKey, const gchar *cValue)
{
	// init query
	memset (pQuery, 0, sizeof (CDQuery));
	pQuery->iPosition = -1;
	
	g_return_val_if_fail (cKey != NULL, FALSE);
	if (cValue == NULL)  // use "none" keyword to look for "icons that don't have key".
		return FALSE;
	
	if (strcmp (cKey, "name") == 0 || strcmp (cKey, "label") == 0)
		pQuery->cName = cValue;
	else if (strcmp (cKey, "command") == 0)
		pQuery->cCommand = cValue;
	else if (strcmp (cKey, "class") == 0)
		pQuery->cClass = cValue;
	else if (strcmp (cKey, "container") == 0)
		pQuery->cContainerName = cValue;
	else if (strcmp (cKey, "Xid") == 0)
		pQuery->Xid = strtol(cValue, NULL, 0);  // can read hexa, decimal or octal.
	else if (strcmp (cKey, "config-file") == 0)
		pQuery->cDesktopFile = cValue;
	else if (strcmp (cKey, "module") == 0)
		pQuery->cModuleName = cValue;
	else if (strcmp (cKey, "position") == 0)
		pQuery->iPosition = atoi (cValue);
	else if (strcmp (cKey, "type") == 0)
		pQuery->cType = cValue;
	else
	{
		cd_warning ("wrong key (%s)", cKey);
		return FALSE;
	}
	return TRUE;
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

static inline gboolean _strings_match (const gchar *q, const gchar *p)  // query, parameter
{
	if (!p)
		return (strcmp (q, "none") == 0);
	int n = strlen(q);
	if (n != 0 && q[n-1] == '*')  // ok with UTF-8 too.
		return (strncmp (q, p, n-1) == 0);
	return (strcmp (q, p) == 0);
}

static inline gboolean _strings_match_case (const gchar *q, const gchar *p)  // query, parameter
{
	if (!p)
		return (strcmp (q, "none") == 0);
	int n = strlen(q);
	if (n != 0 && q[n-1] == '*')
		return (g_ascii_strncasecmp (q, p, n-1) == 0);
	return (g_ascii_strcasecmp (q, p) == 0);
}


  /////////////////////
 /// ICON MATCHING ///
/////////////////////

static gboolean _icon_is_matching (Icon *pIcon, CDQuery *pQuery)
{
	gboolean r;
	GldiContainer *pContainer = cairo_dock_get_icon_container (pIcon);
	if (CAIRO_DOCK_IS_DOCK (pContainer))
	{
		CairoDock *pDock = CAIRO_DOCK (pContainer);
		if (pDock->iRefCount > 0)
		{
			Icon *pPointedIcon = cairo_dock_search_icon_pointing_on_dock (pDock, NULL);
			if (CAIRO_DOCK_IS_APPLET (pPointedIcon))  // ignore icons belonging to applets, because they should be managed by the applet only.
				return FALSE;
		}
	}
	else if (CAIRO_DOCK_IS_DESKLET (pContainer))
	{
		Icon *pMainIcon = CAIRO_DESKLET (pContainer)->pIcon;
		if (pIcon != pMainIcon)  // same
			return FALSE;
	}
	
	if (pQuery->cType)
	{
		if (strcmp (pQuery->cType, CD_TYPE_ICON) == 0)
			return TRUE;
		const gchar *cType;
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
		r = (strcmp (pQuery->cType, cType) == 0);
		if (r) return TRUE;  // a query has only 1 key set.
	}
	if (pQuery->cName)
	{
		r = _strings_match (pQuery->cName, pIcon->cName);
		if (r) return TRUE;
	}
	if (pQuery->cCommand)
	{
		r = _strings_match (pQuery->cCommand, pIcon->cCommand);
		if (r) return TRUE;
	}
	if (pQuery->cClass)
	{
		r = _strings_match_case (pQuery->cClass, pIcon->cClass);
		if (r) return TRUE;
	}
	if (pQuery->cContainerName)
	{
		const gchar *cContainerName = NULL;
		if (CAIRO_DOCK_IS_DOCK (pContainer))
			cContainerName = gldi_dock_get_name (CAIRO_DOCK(pContainer));
		else if (CAIRO_DOCK_IS_DESKLET (pContainer))
		{
			Icon *pMainIcon = CAIRO_DESKLET (pContainer)->pIcon;
			if (CAIRO_DOCK_IS_APPLET (pMainIcon))
				cContainerName = pMainIcon->pModuleInstance->pModule->pVisitCard->cModuleName;
		}
		r = _strings_match (pQuery->cContainerName, cContainerName);
		if (r) return TRUE;
	}
	if (pQuery->Xid != 0)
	{
		r = (gldi_window_get_id(pIcon->pAppli) == pQuery->Xid);
		if (r) return TRUE;
	}
	if (pQuery->cDesktopFile)
	{
		if (*pQuery->cDesktopFile == '/')  // query the complete path.
		{
			gchar *cDesktopFilePath = g_strdup_printf ("%s/%s", g_cCurrentLaunchersPath, pIcon->cDesktopFileName);
			r = _strings_match (pQuery->cDesktopFile, cDesktopFilePath);
			g_free (cDesktopFilePath);
		}
		else  // query the file name only.
		{
			r = _strings_match (pQuery->cDesktopFile, pIcon->cDesktopFileName);
		}
		/*if (!r && CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->cConfFilePath)
		{
			if (*pQuery->cDesktopFile == '/')  // query the complete path.
			{
				r = _strings_match (pQuery->cDesktopFile, pIcon->pModuleInstance->cConfFilePath);
			}
			else  // query the file name only.
			{
				gchar *str = strrchr (pIcon->pModuleInstance->cConfFilePath, '/');
				if (str)
					r = _strings_match (pQuery->cDesktopFile, str+1);
			}
		}*/
		if (r) return TRUE;
	}
	if (pQuery->cModuleName)
	{
		r = (CAIRO_DOCK_IS_APPLET (pIcon) && _strings_match (pQuery->cModuleName, pIcon->pModuleInstance->pModule->pVisitCard->cModuleName));
		if (r) return TRUE;
	}
	
	return FALSE;
}

static void _check_icon_matching (Icon *pIcon, CDQuery *pQuery)
{
	if (_icon_is_matching (pIcon, pQuery))
	{
		cd_debug ("found icon %s", pIcon->cName);
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
	}
}

static void _get_icon_at_position_in_dock (const gchar *cDockName, CairoDock *pDock, CDQuery *pQuery)
{
	Icon *pIcon = g_list_nth_data (pDock->icons, pQuery->iPosition);
	if (pIcon != NULL)
	{
		cd_debug ("found icon %s", pIcon->cName);
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
	}
}

static gboolean _get_icon_at_position_in_desklet (CairoDesklet *pDesklet, CDQuery *pQuery)
{
	Icon *pIcon = g_list_nth_data (pDesklet->icons, pQuery->iPosition);
	if (pIcon != NULL)
	{
		cd_debug ("found icon %s", pIcon->cName);
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
	}
	return FALSE;  // don't stop.
}

static GList *_find_matching_icons_for_key (const gchar *cKey, const gchar *cValue)
{
	//g_print ("  %s (%s, %s)\n", __func__, cKey, cValue);
	CDQuery query;
	gboolean bValidQuery = _prepare_query (&query, cKey, cValue);
	g_return_val_if_fail (bValidQuery, NULL);
	
	if (query.iPosition >= 0)
	{
		gldi_docks_foreach ((GHFunc) _get_icon_at_position_in_dock, &query);
		gldi_desklets_foreach ((GldiDeskletForeachFunc) _get_icon_at_position_in_desklet, &query);
	}
	else
	{
		gldi_icons_foreach ((GldiIconFunc)_check_icon_matching, &query);
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

GList *cd_dbus_find_matching_icons (gchar *cQuery)
{
	g_return_val_if_fail (cQuery != NULL, NULL);
	
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


  //////////////////////////
 /// CONTAINER MATCHING ///
//////////////////////////

static gboolean _container_is_matching (GldiContainer *pContainer, const gchar *cContainerName, const gchar *cConfFile, CDQuery *pQuery)
{
	gboolean r;
	if (pQuery->cType)
	{
		if (strcmp (pQuery->cType, "Container") == 0)
			return TRUE;
		const gchar *cType = "";
		if (CAIRO_DOCK_IS_DOCK (pContainer))
			cType = "Dock";
		else if (CAIRO_DOCK_IS_DESKLET (pContainer))
			cType = "Desklet";
		r = (strcmp (pQuery->cType, cType) == 0);
		if (r) return TRUE;
	}
	if (pQuery->cName)
	{
		r = _strings_match (pQuery->cName, cContainerName);
		if (r) return TRUE;
	}
	if (pQuery->cDesktopFile)
	{
		if (*pQuery->cDesktopFile == '/')  // path
		{
			r = _strings_match (pQuery->cDesktopFile, cConfFile);
		}
		else
		{
			const gchar *str = strrchr (cConfFile, '/');
			r = _strings_match (pQuery->cDesktopFile, str+1);
		}
		if (r) return TRUE;
	}
	
	return FALSE	;
}

static void _check_dock_matching (const gchar *cDockName, CairoDock *pDock, CDQuery *pQuery)
{
	if (pDock->iRefCount > 0)  // ignore sub-docks, as they are managed by the icon-container that holds it.
		return;
	gchar *cConfFile = NULL;
	if (pDock->bIsMainDock)
		cConfFile = g_strdup (g_cConfFile);
	else
		cConfFile = g_strdup_printf ("%s/%s.conf", g_cCurrentThemePath, cDockName);
	
	if (_container_is_matching (CAIRO_CONTAINER (pDock), cDockName, cConfFile, pQuery))
	{
		cd_debug ("found dock %s", cDockName);
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pDock);
	}
	
	g_free (cConfFile);
}

static gboolean _check_desklet_matching (CairoDesklet *pDesklet, CDQuery *pQuery)
{
	Icon *pIcon = pDesklet->pIcon;
	g_return_val_if_fail (CAIRO_DOCK_ICON_TYPE_IS_APPLET (pIcon), FALSE);
	
	const gchar *cConfFile = pIcon->pModuleInstance->cConfFilePath;
	const gchar *cName = pIcon->pModuleInstance->pModule->pVisitCard->cModuleName;
	
	if (_container_is_matching (CAIRO_CONTAINER (pDesklet), cName, cConfFile, pQuery))
	{
		cd_debug ("found desklet %s", cName);
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pDesklet);
	}
	
	return FALSE;  // don't stop.
}

static GList *_find_matching_containers_for_key (const gchar *cKey, const gchar *cValue)
{
	//g_print ("  %s (%s, %s)\n", __func__, cKey, cValue);
	CDQuery query;
	gboolean bValidQuery = _prepare_query (&query, cKey, cValue);
	g_return_val_if_fail (bValidQuery, NULL);
	
	gldi_docks_foreach ((GHFunc) _check_dock_matching, &query);
	gldi_desklets_foreach ((GldiDeskletForeachFunc) _check_desklet_matching, &query);
	
	return query.pMatchingIcons;
}
static GList *_find_matching_containers_for_test (gchar *cTest)
{
	g_return_val_if_fail (cTest != NULL, NULL);
	//g_print (" %s (%s)\n", __func__, cTest);
	
	gchar *str = strchr (cTest, '=');
	g_return_val_if_fail (str != NULL, NULL);
	
	*str = '\0';
	gchar *cKey = g_strstrip ((gchar*)cTest);  // g_strstrip modifies the string in place (by moving the rest of the characters forward and cutting the trailing spaces)
	gchar *cValue = g_strstrip (str+1);
	
	return _find_matching_containers_for_key (cKey, cValue);
}

GList *cd_dbus_find_matching_containers (gchar *cQuery)
{
	g_return_val_if_fail (cQuery != NULL, NULL);
	//g_print ("%s (%s)\n", __func__, cQuery);
	
	gchar *str;
	str = strchr (cQuery, '|');  // a && b || c && d <=> (a && b) || (c && d)
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_containers (cQuery);
		GList *pList2 = cd_dbus_find_matching_containers (str+1);
		return _concat (pList1, pList2);
	}
	str = strchr (cQuery, '&');
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_containers (cQuery);
		GList *pList2 = cd_dbus_find_matching_containers (str+1);
		return _merge (pList1, pList2);
	}
	return _find_matching_containers_for_test (cQuery);
}



  ///////////////////////
 /// MODULE MATCHING ///
///////////////////////
static gboolean _add_module (const gchar *cModuleName, GldiModule *pModule, CDQuery *pQuery)
{
	pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pModule);
	return FALSE;  // don't stop.
}
static void _add_manager (GldiManager *pManager, CDQuery *pQuery)
{
	pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pManager);
}
static GList *_find_matching_modules_for_key (const gchar *cKey, const gchar *cValue)
{
	//g_print ("  %s (%s, %s)\n", __func__, cKey, cValue);
	CDQuery query;
	gboolean bValidQuery = _prepare_query (&query, cKey, cValue);
	g_return_val_if_fail (bValidQuery, NULL);
	
	if (query.cType)
	{
		if (strcmp (query.cType, CD_TYPE_MODULE) == 0)
			gldi_module_foreach ((GHRFunc)_add_module, &query);
		else if (strcmp (query.cType, CD_TYPE_MANAGER) == 0)
			gldi_managers_foreach ((GFunc)_add_manager, &query);
	}
	
	if (query.cName)  // the only relevant way to identify a module/maanger is by name, so we just need to look for the given name.
	{
		GldiModule *pModule = gldi_module_get (query.cName);
		if (pModule != NULL)
		{
			cd_debug ("found module %s", pModule->pVisitCard->cModuleName);
			query.pMatchingIcons = g_list_prepend (query.pMatchingIcons, pModule);
		}
		else
		{
			GldiManager *pManager = gldi_manager_get (query.cName);
			if (pManager != NULL)
			{
				cd_debug ("found manager %s", pManager->cModuleName);
				query.pMatchingIcons = g_list_prepend (query.pMatchingIcons, pManager);
			}
		}
	}
	
	return query.pMatchingIcons;
}
static GList *_find_matching_modules_for_test (gchar *cTest)
{
	g_return_val_if_fail (cTest != NULL, NULL);
	//g_print (" %s (%s)\n", __func__, cTest);
	
	gchar *str = strchr (cTest, '=');
	g_return_val_if_fail (str != NULL, NULL);
	
	*str = '\0';
	gchar *cKey = g_strstrip ((gchar*)cTest);  // g_strstrip modifies the string in place (by moving the rest of the characters forward and cutting the trailing spaces)
	gchar *cValue = g_strstrip (str+1);
	
	return _find_matching_modules_for_key (cKey, cValue);
}

GList *cd_dbus_find_matching_modules (gchar *cQuery)
{
	g_return_val_if_fail (cQuery != NULL, NULL);
	//g_print ("%s (%s)\n", __func__, cQuery);
	
	gchar *str;
	str = strchr (cQuery, '|');  // a && b || c && d <=> (a && b) || (c && d)
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_modules (cQuery);
		GList *pList2 = cd_dbus_find_matching_modules (str+1);
		return _concat (pList1, pList2);
	}
	str = strchr (cQuery, '&');
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_modules (cQuery);
		GList *pList2 = cd_dbus_find_matching_modules (str+1);
		return _merge (pList1, pList2);
	}
	return _find_matching_modules_for_test (cQuery);
}


  ////////////////////////////////
 /// MODULE INSTANCE MATCHING ///
////////////////////////////////

static gboolean _module_instance_is_matching (GldiModuleInstance *pModuleInstance, CDQuery *pQuery)
{
	gboolean r;
	
	if (pQuery->cType && strcmp (pQuery->cType, CD_TYPE_MODULE_INSTANCE) == 0)
		return TRUE;
		
	if (pQuery->cModuleName)
	{
		r = _strings_match (pQuery->cModuleName, pModuleInstance->pModule->pVisitCard->cModuleName);
		if (r) return TRUE;
	}
	if (pQuery->cDesktopFile && pModuleInstance->cConfFilePath)
	{
		if (*pQuery->cDesktopFile == '/')  // path
		{
			r = _strings_match (pQuery->cDesktopFile, pModuleInstance->cConfFilePath);
		}
		else
		{
			const gchar *str = strrchr (pModuleInstance->cConfFilePath, '/');
			r = _strings_match (pQuery->cDesktopFile, str+1);
		}
		if (r) return TRUE;
	}
	
	return FALSE;
}

static gboolean _check_module_instance_matching (const gchar *cModuleName, GldiModule *pModule, CDQuery *pQuery)
{
	GldiModuleInstance *pInstance;
	GList *mi;
	for (mi = pModule->pInstancesList; mi != NULL; mi = mi->next)
	{
		pInstance = mi->data;
		if (_module_instance_is_matching (pInstance, pQuery))
		{
			cd_debug ("found module instance %s", pInstance->pModule->pVisitCard->cModuleName);
			pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pInstance);
		}
	}
	return FALSE;  // don't stop.
}

static GList *_find_matching_module_instances_for_key (const gchar *cKey, const gchar *cValue)
{
	//g_print ("  %s (%s, %s)\n", __func__, cKey, cValue);
	CDQuery query;
	gboolean bValidQuery = _prepare_query (&query, cKey, cValue);
	g_return_val_if_fail (bValidQuery, NULL);
	
	gldi_module_foreach ((GHRFunc)_check_module_instance_matching, &query);
	
	return query.pMatchingIcons;
}
static GList *_find_matching_module_instances_for_test (gchar *cTest)
{
	g_return_val_if_fail (cTest != NULL, NULL);
	//g_print (" %s (%s)\n", __func__, cTest);
	
	gchar *str = strchr (cTest, '=');
	g_return_val_if_fail (str != NULL, NULL);
	
	*str = '\0';
	gchar *cKey = g_strstrip ((gchar*)cTest);  // g_strstrip modifies the string in place (by moving the rest of the characters forward and cutting the trailing spaces)
	gchar *cValue = g_strstrip (str+1);
	
	return _find_matching_module_instances_for_key (cKey, cValue);
}

GList *cd_dbus_find_matching_module_instances (gchar *cQuery)
{
	g_return_val_if_fail (cQuery != NULL, NULL);
	//g_print ("%s (%s)\n", __func__, cQuery);
	
	gchar *str;
	str = strchr (cQuery, '|');  // a && b || c && d <=> (a && b) || (c && d)
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_module_instances (cQuery);
		GList *pList2 = cd_dbus_find_matching_module_instances (str+1);
		return _concat (pList1, pList2);
	}
	str = strchr (cQuery, '&');
	if (str)
	{
		*str = '\0';
		GList *pList1 = cd_dbus_find_matching_module_instances (cQuery);
		GList *pList2 = cd_dbus_find_matching_module_instances (str+1);
		return _merge (pList1, pList2);
	}
	return _find_matching_module_instances_for_test (cQuery);
}


CDMainType cd_dbus_get_main_type (const gchar *cType, int n)
{
	if (!cType)
		return CD_MAIN_TYPE_UNKNOWN;
	if (n <= 0)
		n = strlen (cType);
	
	if (strncmp (cType, CD_TYPE_ICON, n) == 0
	|| strncmp (cType, CD_TYPE_LAUNCHER, n) == 0
	|| strncmp (cType, CD_TYPE_APPLICATION, n) == 0
	|| strncmp (cType, CD_TYPE_APPLET, n) == 0
	|| strncmp (cType, CD_TYPE_SEPARATOR, n) == 0
	|| strncmp (cType, CD_TYPE_STACK_ICON, n) == 0
	|| strncmp (cType, CD_TYPE_CLASS_ICON, n) == 0
	|| strncmp (cType, CD_TYPE_ICON_OTHER, n) == 0)
	{
		return CD_MAIN_TYPE_ICON;
	}
	else if (strncmp (cType, CD_TYPE_CONTAINER, n) == 0
	|| strncmp (cType, CD_TYPE_DOCK, n) == 0
	|| strncmp (cType, CD_TYPE_DESKLET, n) == 0)
	{
		return CD_MAIN_TYPE_CONTAINER;
	}
	else if (strncmp (cType, CD_TYPE_MODULE, n) == 0
	|| strncmp (cType, CD_TYPE_MANAGER, n) == 0)
	{
		return CD_MAIN_TYPE_MODULE;
	}
	else if (strncmp (cType, CD_TYPE_MODULE_INSTANCE, n) == 0)
	{
		return CD_MAIN_TYPE_MODULE_INSTANCE;
	}
	else  // wrong type
	{
		return CD_MAIN_TYPE_UNKNOWN;
	}
}

GList *cd_dbus_find_matching_objects (gchar *cQuery)
{
	CDMainType iType = CD_MAIN_TYPE_UNKNOWN;
	GList *pObjects = NULL;
	gchar *str = strstr (cQuery, "type");
	if (str)
	{
		str = strchr (str+4, '=');  // jump to '='
		if (str)
		{
			str ++;  // skip it
			while (*str == ' ') str ++; // skip spaces
			gchar *end = str+1;
			while (*end != '\0' && *end != ' ' && *end != '&' && *end != '|') end ++;  // go to the end of the value
			int n = end - str;
			iType = cd_dbus_get_main_type (str, n);
		}
	}
	switch (iType)
	{
		case CD_MAIN_TYPE_ICON:
			pObjects = cd_dbus_find_matching_icons (cQuery);
		break;
	
		case CD_MAIN_TYPE_CONTAINER:
			pObjects = cd_dbus_find_matching_containers (cQuery);
		break;
		case CD_MAIN_TYPE_MODULE:
			pObjects = cd_dbus_find_matching_modules (cQuery);
		break;
		case CD_MAIN_TYPE_MODULE_INSTANCE:
			pObjects = cd_dbus_find_matching_module_instances (cQuery);
		break;
		default:
		{
			gchar *query = g_strdup (cQuery);
			GList *o = cd_dbus_find_matching_icons (query);
			pObjects = o;
			
			memcpy (query, cQuery, strlen (cQuery));
			o = cd_dbus_find_matching_containers (query);
			pObjects = g_list_concat (pObjects, o);
			
			memcpy (query, cQuery, strlen (cQuery));
			o = cd_dbus_find_matching_modules (query);
			pObjects = g_list_concat (pObjects, o);
			
			memcpy (query, cQuery, strlen (cQuery));
			o = cd_dbus_find_matching_module_instances (query);
			pObjects = g_list_concat (pObjects, o);
			g_free (query);
		}
		break;
	}
	return pObjects;
}
