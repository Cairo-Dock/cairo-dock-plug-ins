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


#include "applet-launcher.h"
#include "applet-struct.h"

// if it's not in the exception list
static gboolean _is_an_exception (const gchar *cIndicatorName, gchar **cExceptionsList)
{
	if (cExceptionsList != NULL)
	{
		for (int i = 0; cExceptionsList[i] != NULL; i++)
		{
			if (g_strcmp0 (cIndicatorName, cExceptionsList[i]) == 0)
				return TRUE;
		}
	}
	return FALSE;
}

static void _on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, GldiModuleInstance *myApplet)
{
	g_return_if_fail (cURI != NULL);
	CD_APPLET_ENTER;

	/* No need to reload all indicators if one is modified:
	 *  these files are modified but the corresponding daemons are not reloaded
	 */
	if (iEventType != CAIRO_DOCK_FILE_MODIFIED) // created/removed
	{
		cd_debug ("File event: Reload all indicators");
		cd_indicator_generic_reload_all_indicators (myApplet);
	}

	CD_APPLET_LEAVE();
}

void cd_indicator_generic_add_monitor_dir (GldiModuleInstance *myApplet)
{
	cairo_dock_fm_add_monitor_full (cd_indicator3_get_directory_path (), TRUE,
		NULL, (CairoDockFMMonitorCallback) _on_file_event, myApplet);
	#ifdef IS_INDICATOR_NG
	cairo_dock_fm_add_monitor_full (INDICATOR_SERVICE_DIR, TRUE,
		NULL, (CairoDockFMMonitorCallback) _on_file_event, myApplet);
	#endif
}

void cd_indicator_generic_remove_monitor_dir (void)
{
	cairo_dock_fm_remove_monitor_full (cd_indicator3_get_directory_path (),
		TRUE, NULL);
	#ifdef IS_INDICATOR_NG
	cairo_dock_fm_remove_monitor_full (INDICATOR_SERVICE_DIR, TRUE, NULL);
	#endif
}

GDir * cd_indicator_generic_open_dir_modules (GldiModuleInstance *myApplet)
{
	GError *error = NULL;
	GDir *pDir = g_dir_open (cd_indicator3_get_directory_path (), 0, &error); // all indicators are on the same dir
	if (error != NULL)
	{
		cd_warning ("Failed to load indicator3 modules dir: %s", cd_indicator3_get_directory_path ());
		return NULL;
	}
	return pDir;
}

GDir * cd_indicator_generic_open_dir_sevices (GldiModuleInstance *myApplet)
{
	#ifdef IS_INDICATOR_NG
	GError *error = NULL;
	GDir *pDir = g_dir_open (INDICATOR_SERVICE_DIR, 0, &error); // all indicators are on the same dir
	if (error != NULL)
	{
		cd_warning ("Failed to load indicator3 services dir: %s", INDICATOR_SERVICE_DIR);
		return NULL;
	}
	return pDir;
	#else
	return NULL;
	#endif
}

static gint _load_all_indicators_in_dir (GldiModuleInstance *myApplet, GDir *pDir, gboolean bIsModule)
{
	// for each indicator file, instanciate a new plugin with it
	// (useful to place it where we want, all icons are not regrouped into one big icon)
	const gchar *cFileName;
	gchar *cInstanceFilePath;
	GldiModuleInstance *pModuleInstance;
	gint iNbFiles = 0;
	while ((cFileName = g_dir_read_name (pDir)) != NULL)
	{
		if (*cFileName == '\0' || (bIsModule && ! g_str_has_suffix (cFileName, G_MODULE_SUFFIX))
			|| _is_an_exception (cFileName, myConfig.cExceptionsList))
			continue;

		gchar *cUserDataDirPath = gldi_module_get_config_dir (myApplet->pModule);
		// config file: indicator.so.conf ; e.g. libprintersmenu.so.conf
		cInstanceFilePath = g_strdup_printf ("%s/%s.conf", cUserDataDirPath, cFileName);
		if (! g_file_test (cInstanceFilePath, G_FILE_TEST_EXISTS))
		{
			// new indicator: create the .conf file
			gchar *cConfFileOriginalPath = g_strdup_printf ("%s/%s",
				cUserDataDirPath, myApplet->pModule->pVisitCard->cConfFileName);

			GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFileOriginalPath);
			g_free (cConfFileOriginalPath);
			if (pKeyFile != NULL)
			{
				// added the indicator in the .conf file
				g_key_file_set_string (pKeyFile, "Configuration", "indicator", cFileName);
				// remove exceptions' lists
				cairo_dock_remove_group_key_from_conf_file (pKeyFile, "Configuration", "except-edit");
				g_key_file_remove_key (pKeyFile, "Configuration", "exceptions", NULL);
				// write
				cairo_dock_write_keys_to_file (pKeyFile, cInstanceFilePath);
				// free
				g_key_file_free (pKeyFile);
			}
		}
		// create the new icon
		pModuleInstance = gldi_module_instance_new (myApplet->pModule, cInstanceFilePath);  // we don't have to free cInstanceFilePath
		myData.pIndicatorsList = g_list_prepend (myData.pIndicatorsList, pModuleInstance);
		g_free (cUserDataDirPath);
		iNbFiles++;
	}
	g_dir_close (pDir);
	return iNbFiles;
}

gint cd_indicator_generic_load_all_indicators (GldiModuleInstance *myApplet, GDir *pDirModules, GDir *pDirServices)
{
	gint iNbFiles = 0;
	if (pDirModules != NULL)
		iNbFiles = _load_all_indicators_in_dir (myApplet, pDirModules, TRUE);
	if (pDirServices != NULL)
		iNbFiles += _load_all_indicators_in_dir (myApplet, pDirServices, FALSE);
	return iNbFiles;
}

void cd_indicator_generic_reload_all_indicators (GldiModuleInstance *myApplet)
{
	cd_debug ("Reload all indicators");
	g_list_foreach (myData.pIndicatorsList, (GFunc)gldi_object_unref, NULL);

	g_list_free (myData.pIndicatorsList);
	myData.pIndicatorsList = NULL;

	GDir *pDirModules = cd_indicator_generic_open_dir_modules (myApplet);
	GDir *pDirServices = cd_indicator_generic_open_dir_sevices (myApplet);
	if (pDirModules == NULL && pDirServices == NULL)
	{
		myApplet->pModule->pVisitCard->iContainerType = CAIRO_DOCK_MODULE_IS_PLUGIN; // dir is empty...
		return;
	}

	myApplet->pModule->pVisitCard->iContainerType = CAIRO_DOCK_MODULE_CAN_DOCK | CAIRO_DOCK_MODULE_CAN_DESKLET;

	if (cd_indicator_generic_load_all_indicators (myApplet, pDirModules, pDirServices) == 0)
		myApplet->pModule->pVisitCard->iContainerType = CAIRO_DOCK_MODULE_IS_PLUGIN; // dir is empty...
}
