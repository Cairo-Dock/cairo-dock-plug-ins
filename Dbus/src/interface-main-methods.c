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

#include "cairo-dock.h"
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
gboolean cd_dbus_main_show_dock (dbusMainObject *pDbusCallback, gint iVisibiliy, GError **error)
{
	if (! myConfig.bEnableShowDock)
		return FALSE;
	
	if (g_pMainDock == NULL)
		return FALSE;
	
	gboolean bShow;
	switch (iVisibiliy)
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


  //////////////////
 /// ICON QUERY ///
//////////////////

typedef struct {
	const gchar *cType;
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
} CDIconQueryBuffer;
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
static gboolean _icon_is_matching (Icon *pIcon, CairoContainer *pContainer, CDIconQueryBuffer *pQuery)
{
	gboolean bOr = FALSE;
	gboolean bAnd = TRUE;  // at least 1 of the fields is not nul.
	gboolean r;
	if (pQuery->cType)
	{
		const gchar *cType;
		if (CAIRO_DOCK_ICON_TYPE_IS_LAUNCHER (pIcon))
			cType = "Launcher";
		else if (CAIRO_DOCK_ICON_TYPE_IS_APPLI (pIcon))
			cType = "Application";
		else if (CAIRO_DOCK_ICON_TYPE_IS_APPLET (pIcon))
			cType = "Applet";
		else if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			cType = "Separator";
		else if (CAIRO_DOCK_ICON_TYPE_IS_CONTAINER (pIcon))
			cType = "Container";
		else if (CAIRO_DOCK_ICON_TYPE_IS_CLASS_CONTAINER (pIcon))
			cType = "Class-Container";
		else
			cType = "Other";
		r = (strcmp (pQuery->cType, cType) == 0);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cName)
	{
		r = _strings_match (pQuery->cName, pIcon->cName);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cCommand)
	{
		r = _strings_match (pQuery->cCommand, pIcon->cCommand);
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cClass)
	{
		r = _strings_match_case (pQuery->cClass, pIcon->cClass);
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
		r = _strings_match (pQuery->cContainerName, cContainerName);
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
		r = _strings_match (pQuery->cDesktopFile, pIcon->cDesktopFileName);
		if (!r && CAIRO_DOCK_IS_APPLET (pIcon) && pIcon->pModuleInstance->cConfFilePath)
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
		}
		bOr |= r;
		bAnd &= r;
	}
	if (pQuery->cModuleName)
	{
		r = (CAIRO_DOCK_IS_APPLET (pIcon) && _strings_match (pQuery->cModuleName, pIcon->pModuleInstance->pModule->pVisitCard->cModuleName));
		bOr |= r;
		bAnd &= r;
	}
	
	return ((pQuery->bMatchAll && bAnd) || (!pQuery->bMatchAll && bOr));
}
static void _check_icon_matching (Icon *pIcon, CairoContainer *pContainer, CDIconQueryBuffer *pQuery)
{
	if (_icon_is_matching (pIcon, pContainer, pQuery))
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
}
static void _get_icon_at_position_in_dock (const gchar *cDockName, CairoDock *pDock, CDIconQueryBuffer *pQuery)
{
	Icon *pIcon = g_list_nth_data (pDock->icons, pQuery->iPosition);
	if (pIcon != NULL)
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
}
static gboolean _get_icon_at_position_in_desklet (CairoDesklet *pDesklet, CDIconQueryBuffer *pQuery)
{
	Icon *pIcon = g_list_nth_data (pDesklet->icons, pQuery->iPosition);
	if (pIcon != NULL)
		pQuery->pMatchingIcons = g_list_prepend (pQuery->pMatchingIcons, pIcon);
	return FALSE;  // don't stop.
}
static gboolean _prepare_query (CDIconQueryBuffer *pQuery, const gchar *cKey, const gchar *cValue)
{
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
static GList *_find_matching_icons_for_key (const gchar *cKey, const gchar *cValue)
{
	//g_print ("  %s (%s, %s)\n", __func__, cKey, cValue);
	CDIconQueryBuffer query;
	memset (&query, 0, sizeof (CDIconQueryBuffer));
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


gboolean cd_dbus_main_get_icon_properties (dbusMainObject *pDbusCallback, gchar *cIconQuery, GPtrArray **pIconAttributes, GError **error)
{
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);  // if NULL, will just return an empty array.
	
	GPtrArray *pTab = g_ptr_array_new ();
	*pIconAttributes = pTab;
	
	GHashTable *h;
	GValue *v;
	Icon *pIcon;
	CairoContainer *pContainer;
	int iPosition;
	const gchar *cType;
	const gchar *cContainerName;
	const gchar *cDesktopFile;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		
		h = g_hash_table_new_full (g_str_hash,
			g_str_equal,
			g_free,  /// can we use const char here instead of duplicating each string ?...
			g_free);
		g_ptr_array_add (pTab, h);
		
		if (CAIRO_DOCK_ICON_TYPE_IS_LAUNCHER (pIcon))
			cType = "Launcher";
		else if (CAIRO_DOCK_ICON_TYPE_IS_APPLI (pIcon))
			cType = "Application";
		else if (CAIRO_DOCK_ICON_TYPE_IS_APPLET (pIcon))
			cType = "Applet";
		else if (CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
			cType = "Separator";
		else if (CAIRO_DOCK_ICON_TYPE_IS_CONTAINER (pIcon))
			cType = "Container";
		else if (CAIRO_DOCK_ICON_TYPE_IS_CLASS_CONTAINER (pIcon))
			cType = "Class-Container";
		else
			cType = "Other";
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, cType);
		g_hash_table_insert (h, g_strdup ("type"), v);
		
		cDesktopFile = NULL;
		if (pIcon->cDesktopFileName != NULL)
			cDesktopFile = pIcon->cDesktopFileName;
		else if (CAIRO_DOCK_IS_APPLET (pIcon))
			cDesktopFile = pIcon->pModuleInstance->cConfFilePath;
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, cDesktopFile);
		g_hash_table_insert (h, g_strdup ("config-file"), v);
		
		if (CAIRO_DOCK_IS_APPLET (pIcon))
		{
			v = g_new0 (GValue, 1);
			g_value_init (v, G_TYPE_STRING);
			g_value_set_string (v, pIcon->pModuleInstance->pModule->pVisitCard->cModuleName);
			g_hash_table_insert (h, g_strdup ("module"), v);
		}
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, pIcon->cName);  /// g_value_set_static_string ?...
		g_hash_table_insert (h, g_strdup ("name"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, pIcon->cCommand);
		g_hash_table_insert (h, g_strdup ("command"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, pIcon->cClass);
		g_hash_table_insert (h, g_strdup ("class"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, pIcon->cFileName);
		g_hash_table_insert (h, g_strdup ("icon"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, pIcon->cQuickInfo);
		g_hash_table_insert (h, g_strdup ("quick-info"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_UINT);
		g_value_set_uint (v, pIcon->Xid);
		g_hash_table_insert (h, g_strdup ("Xid"), v);
		
		iPosition = -1;
		cContainerName = NULL;
		if (CAIRO_DOCK_IS_DOCK (pContainer))
		{
			CairoDock *pDock = CAIRO_DOCK (pContainer);
			iPosition = g_list_index (pDock->icons, pIcon);
			cContainerName = pIcon->cParentDockName;
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
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iPosition);
		g_hash_table_insert (h, g_strdup ("position"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_STRING);
		g_value_set_string (v, cContainerName);
		g_hash_table_insert (h, g_strdup ("container"), v);
		
		v = g_new0 (GValue, 1);
		g_value_init (v, G_TYPE_DOUBLE);
		g_value_set_double (v, pIcon->fOrder);
		g_hash_table_insert (h, g_strdup ("order"), v);
	}
	
	g_list_free (pList);
	return TRUE;
}

static void _set_container_props (CairoContainer *pContainer, GHashTable *h)
{
	GValue *v;
	int x, y, w, ht;
	if (pContainer->bIsHorizontal)
	{
		x = pContainer->iWindowPositionX;
		y = pContainer->iWindowPositionY;
		w = pContainer->iWidth;
		ht = pContainer->iHeight;
	}
	else
	{
		y = pContainer->iWindowPositionX;
		x = pContainer->iWindowPositionY;
		ht = pContainer->iWidth;
		w = pContainer->iHeight;
	}
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, pContainer->iWindowPositionX);
	g_hash_table_insert (h, g_strdup ("x"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, y);
	g_hash_table_insert (h, g_strdup ("y"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, w);
	g_hash_table_insert (h, g_strdup ("width"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, ht);
	g_hash_table_insert (h, g_strdup ("height"), v);
	
	CairoDockPositionType iScreenBorder = ((! pContainer->bIsHorizontal) << 1) | (! pContainer->bDirectionUp);
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	g_value_set_uint (v, iScreenBorder);
	g_hash_table_insert (h, g_strdup ("orientation"), v);
}
static void _insert_dock_props (const gchar *cDockName, CairoDock *pDock, GPtrArray *pTab)
{
	GHashTable *h = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,  /// can we use const char here instead of duplicating each string ?...
		g_free);
	g_ptr_array_add (pTab, h);
	
	GValue *v;
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, "Dock");
	g_hash_table_insert (h, g_strdup ("type"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, cDockName);
	g_hash_table_insert (h, g_strdup ("name"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_BOOLEAN);
	g_value_set_boolean (v, (pDock->iRefCount > 0));
	g_hash_table_insert (h, g_strdup ("is-sub-dock"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, g_list_length (pDock->icons));
	g_hash_table_insert (h, g_strdup ("nb-icons"), v);
	
	_set_container_props (CAIRO_CONTAINER (pDock), h);
}
static gboolean _insert_desklet_props (CairoDesklet *pDesklet, GPtrArray *pTab)
{
	GHashTable *h = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,  /// can we use const char here instead of duplicating each string ?...
		g_free);
	g_ptr_array_add (pTab, h);
	
	GValue *v;
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, "Desklet");
	g_hash_table_insert (h, g_strdup ("type"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, CAIRO_DOCK_IS_APPLET (pDesklet->pIcon) ? pDesklet->pIcon->pModuleInstance->pModule->pVisitCard->cModuleName : "");
	g_hash_table_insert (h, g_strdup ("name"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, 1 + g_list_length (pDesklet->icons));
	g_hash_table_insert (h, g_strdup ("nb-icons"), v);
	
	_set_container_props (CAIRO_CONTAINER (pDesklet), h);
	return FALSE;
}
static gboolean _check_desklet_name (CairoDesklet *pDesklet, const gchar *cName)
{
	if (CAIRO_DOCK_IS_APPLET (pDesklet->pIcon))
	{
		return (strcmp (cName, pDesklet->pIcon->pModuleInstance->pModule->pVisitCard->cModuleName) == 0);
	}
	return FALSE;
}
gboolean cd_dbus_main_get_container_properties (dbusMainObject *pDbusCallback, const gchar *cName, GPtrArray **pAttributes, GError **error)
{
	nullify_argument (cName);
	
	GPtrArray *pTab = g_ptr_array_new ();
	*pAttributes = pTab;
	
	if (cName == NULL)
	{
		cairo_dock_foreach_docks ((GHFunc)_insert_dock_props, pTab);
		cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _insert_desklet_props, pTab);
	}
	else
	{
		CairoDock *pDock = cairo_dock_search_dock_from_name (cName);
		if (pDock != NULL)
		{
			_insert_dock_props (cName, pDock, pTab);
		}
		else
		{
			CairoDesklet *pDesklet = cairo_dock_foreach_desklet ((CairoDockForeachDeskletFunc) _check_desklet_name, (gpointer)cName);
			if (pDesklet != NULL)
			{
				_insert_desklet_props (pDesklet, pTab);
			}
		}
	}
	
	return TRUE;
}

static gboolean _insert_module_props (CairoDockModule *pModule, GPtrArray *pTab)
{
	GHashTable *h = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,  /// can we use const char here instead of duplicating each string ?...
		g_free);
	g_ptr_array_add (pTab, h);
	
	GValue *v;
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, pModule->pVisitCard->cModuleName);
	g_hash_table_insert (h, g_strdup ("name"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	g_value_set_uint (v, pModule->pVisitCard->iContainerType);
	g_hash_table_insert (h, g_strdup ("type"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	g_value_set_uint (v, pModule->pVisitCard->iCategory);
	g_hash_table_insert (h, g_strdup ("category"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, pModule->pVisitCard->cTitle);
	g_hash_table_insert (h, g_strdup ("title"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, pModule->pVisitCard->cIconFilePath);
	g_hash_table_insert (h, g_strdup ("icon"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, pModule->pVisitCard->cPreviewFilePath);
	g_hash_table_insert (h, g_strdup ("preview"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, dgettext (pModule->pVisitCard->cGettextDomain, pModule->pVisitCard->cDescription));
	g_hash_table_insert (h, g_strdup ("description"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRING);
	g_value_set_string (v, pModule->pVisitCard->cAuthor);
	g_hash_table_insert (h, g_strdup ("author"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_BOOLEAN);
	g_value_set_boolean (v, pModule->pVisitCard->bMultiInstance);
	g_hash_table_insert (h, g_strdup ("is-multi-instance"), v);
	
	cd_debug ("list instances ...");
	gchar **pInstances = g_new0 (gchar*, g_list_length (pModule->pInstancesList)+1);
	CairoDockModuleInstance *pInstance;
	int i = 0;
	GList *mi;
	for (mi = pModule->pInstancesList; mi != NULL; mi = mi->next)
	{
		pInstance = mi->data;
		pInstances[i++] = g_strdup (pInstance->cConfFilePath);
	}
	cd_debug ("write instances ...");
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_STRV);
	g_value_set_boxed (v, pInstances);
	g_hash_table_insert (h, g_strdup ("instances"), v);
	cd_debug ("done.");
	return TRUE;  // continue
}
gboolean cd_dbus_main_get_module_properties (dbusMainObject *pDbusCallback, const gchar *cName, GPtrArray **pAttributes, GError **error)
{
	nullify_argument (cName);
	
	GPtrArray *pTab = g_ptr_array_new ();
	*pAttributes = pTab;
	
	if (cName == NULL)
	{
		cairo_dock_foreach_module_in_alphabetical_order ((GCompareFunc) _insert_module_props, pTab);
	}
	else
	{
		CairoDockModule *pModule = cairo_dock_find_module_from_name (cName);
		if (pModule != NULL)
		{
			_insert_module_props (pModule, pTab);
		}
	}
	return TRUE;
}

gboolean cd_dbus_main_add_launcher (dbusMainObject *pDbusCallback, const gchar *cDesktopFilePath, gdouble fOrder, const gchar *cDockName, gchar **cLauncherFile, GError **error)
{
	*cLauncherFile = NULL;
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	g_return_val_if_fail (cDesktopFilePath != NULL, FALSE);
	
	//\_______________ get the dock where to insert the icon.
	nullify_argument (cDockName);
	if (cDockName == NULL)
		cDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	
	CairoDock * pParentDock = cairo_dock_search_dock_from_name (cDockName);
	if (pParentDock == NULL)
	{
		cd_warning ("dock %s does not exist", cDockName);
		pParentDock = g_pMainDock;
	}
	
	//\_______________ add a new icon in the current theme.
	int iLauncherType = -1;
	if (strcmp (cDesktopFilePath, "separator.desktop") == 0)
		iLauncherType = CAIRO_DOCK_DESKTOP_FILE_FOR_SEPARATOR;
	else if (strcmp (cDesktopFilePath, "container.desktop") == 0)
		iLauncherType = CAIRO_DOCK_DESKTOP_FILE_FOR_CONTAINER;
	else if (strcmp (cDesktopFilePath, "launcher.desktop") == 0)
		iLauncherType = CAIRO_DOCK_DESKTOP_FILE_FOR_LAUNCHER;
	
	if (fOrder < 0)
		fOrder = CAIRO_DOCK_LAST_ORDER;
	Icon *pNewIcon;
	if (iLauncherType != -1)
		pNewIcon = cairo_dock_add_new_launcher_by_type (iLauncherType, pParentDock, fOrder);
	else
		pNewIcon = cairo_dock_add_new_launcher_by_uri (cDesktopFilePath, pParentDock, fOrder);
	if (pNewIcon != NULL)
	{
		*cLauncherFile = g_strdup (pNewIcon->cDesktopFileName);
		return TRUE;
	}
	else
		return FALSE;
}

gboolean cd_dbus_main_add_temporary_icon (dbusMainObject *pDbusCallback, GHashTable *hIconAttributes, GError **error)
{
	if (! myConfig.bEnableCreateLauncher)
		return FALSE;
	
	g_return_val_if_fail (hIconAttributes != NULL, FALSE);
	
	//\_______________ get the attributes.
	GValue *v;
	const gchar *cType = "Launcher";
	v = g_hash_table_lookup (hIconAttributes, "type");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cType = g_value_get_string (v);
	}
	
	const gchar *cIcon = NULL;
	v = g_hash_table_lookup (hIconAttributes, "icon");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cIcon = g_value_get_string (v);
	}
	
	const gchar *cName = NULL;
	v = g_hash_table_lookup (hIconAttributes, "name");
	if (!v)
		v = g_hash_table_lookup (hIconAttributes, "label");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cName = g_value_get_string (v);
	}
	
	const gchar *cParentDockName = CAIRO_DOCK_MAIN_DOCK_NAME;
	v = g_hash_table_lookup (hIconAttributes, "container");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cParentDockName = g_value_get_string (v);
	}
	
	const gchar *cQuickInfo = NULL;
	v = g_hash_table_lookup (hIconAttributes, "quick-info");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cQuickInfo = g_value_get_string (v);
	}
	
	double fOrder = CAIRO_DOCK_LAST_ORDER;
	v = g_hash_table_lookup (hIconAttributes, "order");
	if (v && G_VALUE_HOLDS_DOUBLE (v))
	{
		fOrder = g_value_get_double (v);
	}
	
	int iPosition = -1;
	v = g_hash_table_lookup (hIconAttributes, "position");
	if (v && G_VALUE_HOLDS_INT (v))
	{
		iPosition = g_value_get_int (v);
	}
	
	const gchar *cCommand = NULL;
	v = g_hash_table_lookup (hIconAttributes, "command");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cCommand = g_value_get_string (v);
	}
	
	const gchar *cClass = NULL;
	v = g_hash_table_lookup (hIconAttributes, "class");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cClass = g_value_get_string (v);  // "none" to skip taskbar
	}
	
	CairoDock *pParentDock = cairo_dock_search_dock_from_name (cParentDockName);
	if (pParentDock == NULL)
	{
		cd_warning ("dock %s does not exist", cParentDockName);
		pParentDock = g_pMainDock;
	}
	
	if (iPosition > -1)  // the position will overwrite the order if ever both are defined.
	{
		fOrder = 0;
		Icon *nth_icon;
		double nth_order = -1;
		int i;
		GList *ic;
		for (i = 0, ic = pParentDock->icons; i < iPosition && ic != NULL; i ++, ic = ic->next)
		{
			nth_icon = ic->data;
			nth_order = nth_icon->fOrder;
		}
		if (ic == NULL)  // not enough icons, place the new one just after the last one.
		{
			fOrder = nth_order + 1;
		}
		else
		{
			nth_icon = ic->data;
			fOrder = (nth_order + nth_icon->fOrder) / 2;
		}
	}
	
	//\_______________ create a corresponding icon.
	Icon *pIcon;
	if (cType == NULL || strcmp (cType, "Launcher") == 0)
	{
		pIcon = cairo_dock_create_dummy_launcher (g_strdup (cName),
			g_strdup (cIcon),
			g_strdup (cCommand),
			g_strdup (cQuickInfo),
			fOrder);
		pIcon->iTrueType = CAIRO_DOCK_ICON_TYPE_LAUNCHER;  // make it a launcher, since we have no control on it, so that the dock handles it as any launcher.
		if (cClass == NULL)
		{
			gchar *cGuessedClass = cairo_dock_guess_class (cCommand, NULL);
			pIcon->cClass = cairo_dock_register_class (cGuessedClass);
			g_free (cGuessedClass);
		}
		else if (strcmp (cClass, "none") != 0)
		{
			pIcon->cClass = cairo_dock_register_class (cClass);
			if (pIcon->cClass == NULL)  // if we couldn't find the class desktop file, set the class anyway, since it was explicitely specified; the method caller probably knows more than us what he's doing.
				pIcon->cClass = g_strdup (cClass);
		}
	}
	else if (strcmp (cType, "Container") == 0)
	{
		int iSubdockViewType = 0;
		if (!cIcon || strcmp (cIcon, "Box") == 0)
		{
			iSubdockViewType = 3;
			cIcon = NULL;
		}
		else if (strcmp (cIcon, "Stack") == 0)
		{
			iSubdockViewType = 2;
			cIcon = NULL;
		}
		else if (strcmp (cIcon, "Emblems") == 0)
		{
			iSubdockViewType = 1;
			cIcon = NULL;
		}
		gchar *cUniqueName = cairo_dock_get_unique_dock_name (cName);
		pIcon = cairo_dock_create_dummy_launcher (cUniqueName,
			g_strdup (cIcon),
			NULL,
			NULL,
			fOrder);
		pIcon->iTrueType = CAIRO_DOCK_ICON_TYPE_CONTAINER;
		cairo_dock_set_subdock_content_renderer (pIcon, iSubdockViewType);
		pIcon->pSubDock = cairo_dock_create_subdock (pIcon->cName, NULL, pParentDock, NULL);  // NULL <=> default sub-docks view.
	}
	else if (strcmp (cType, "Separator") == 0)
	{
		pIcon = cairo_dock_create_separator_icon (CAIRO_DOCK_LAUNCHER, NULL);  // NULL because we'll load the icon ourselves later.
	}
	else
	{
		cd_warning ("type '%s' invalid", cType);
		return FALSE;
	}
	pIcon->cParentDockName = g_strdup (cParentDockName);
	
	//\_______________ load it inside the dock.
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

gboolean cd_dbus_main_reload_icon (dbusMainObject *pDbusCallback, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableTweakingLauncher)
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
		
		if ((CAIRO_DOCK_ICON_TYPE_IS_LAUNCHER (pIcon)
			|| CAIRO_DOCK_ICON_TYPE_IS_CONTAINER (pIcon)
			|| CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon))
		&& pIcon->cDesktopFileName != NULL)  // user icon.
		{
			cairo_dock_reload_launcher (pIcon);
		}
		else if (CAIRO_DOCK_IS_APPLET (pIcon))
		{
			cairo_dock_reload_module_instance (pIcon->pModuleInstance, TRUE);  // TRUE <=> reload config.
		}
		else  // for appli icons, reload their image (custom image for instance).
		{
			pContainer = cairo_dock_search_container_from_icon (pIcon);
			if (pContainer == NULL)
				continue;
			
			cairo_dock_reload_icon_image (pIcon, pContainer);
			cairo_dock_redraw_icon (pIcon, pContainer);
		}
	}
	
	return TRUE;
}

static gboolean _on_icon_deleted (GList *ic, Icon *pIcon)
{
	ic->data = NULL;
}
gboolean cd_dbus_main_remove_icon (dbusMainObject *pDbusCallback, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableTweakingLauncher)
		return FALSE;
	
	GList *pList = cd_dbus_find_matching_icons (cIconQuery);
	if (pList == NULL)
		return FALSE;
	
	// first connect to the "delete" signal, to not destroy 2 times an icon (case of an icon in a sub-dock that is destroyed just before).
	Icon *pIcon;
	CairoContainer *pContainer;
	GList *ic;
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		cairo_dock_register_notification_on_object (pIcon,
			NOTIFICATION_STOP_ICON,
			(CairoDockNotificationFunc) _on_icon_deleted,
			CAIRO_DOCK_RUN_FIRST, ic);
	}
	
	// delete all the matching icons, including their sub-icons.
	for (ic = pList; ic != NULL; ic = ic->next)
	{
		pIcon = ic->data;
		if (pIcon == NULL)  // icon has been destroyed just before.
			continue;
		
		cairo_dock_remove_notification_func_on_object (pIcon,
			NOTIFICATION_STOP_ICON,
			(CairoDockNotificationFunc) _on_icon_deleted,
			ic);  // remove it now, since maybe this icon won't be deleted.
		
		pContainer = cairo_dock_search_container_from_icon (pIcon);
		if (pContainer == NULL)
			continue;
		
		if (CAIRO_DOCK_ICON_TYPE_IS_LAUNCHER (pIcon)
		|| CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (pIcon)
		|| CAIRO_DOCK_ICON_TYPE_IS_CONTAINER (pIcon))  // case of a launcher/separator/sub-dock inside a dock.
		{
			if (pIcon->pSubDock != NULL)  // on detruit le sous-dock et ce qu'il contient.
			{
				cairo_dock_destroy_dock (pIcon->pSubDock, (pIcon->cClass != NULL ? pIcon->cClass : pIcon->cName));
				pIcon->pSubDock = NULL;
			}

			cairo_dock_trigger_icon_removal_from_dock (pIcon);
		}
		else if (CAIRO_DOCK_IS_APPLET (pIcon))  // case of an applet inside a dock or a desklet.
		{
			cairo_dock_remove_module_instance (pIcon->pModuleInstance);
		}  // don't remove appli icons, as they would anyway be re-created automatically by the applications-manager.
	}
	
	g_list_free (pList);
	return TRUE;
}


  ///////////////////
 /// SET ON ICON ///
///////////////////

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
		
		if (cImage == NULL || *cImage == '\0' || strcmp (cImage, "none") == 0)
		{
			cairo_dock_remove_overlay_at_position (pIcon, iPosition);
		}
		else
		{
			if (iPosition < 0)  // [-N, -1] => print the overlay
				cairo_dock_print_overlay_on_icon_from_image (pIcon, pContainer, cImage, - iPosition - 1);
			else  // [0, N-1] => add it
				cairo_dock_add_overlay_from_image (pIcon, cImage, iPosition);
		}
		
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
		cairo_dock_request_icon_animation (pIcon, pContainer, cAnimation, iNbRounds);
	}
	
	g_list_free (pList);
	return TRUE;
}

gboolean cd_dbus_main_demands_attention (dbusMainObject *pDbusCallback, gboolean bStart, const gchar *cAnimation, gchar *cIconQuery, GError **error)
{
	if (! myConfig.bEnableAnimateIcon)
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
		
		if (bStart)
		{
			cairo_dock_request_icon_attention (pIcon, CAIRO_DOCK (pContainer), cAnimation, 0);  // 0 <=> non-stop.
		}
		else if (pIcon->bIsDemandingAttention)
		{
			cairo_dock_stop_icon_attention (pIcon, CAIRO_DOCK (pContainer));
		}
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
