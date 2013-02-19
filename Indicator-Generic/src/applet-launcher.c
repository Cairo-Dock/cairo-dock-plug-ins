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

void cd_indicator_generic_load_all_indicators (CairoDockModuleInstance *myApplet)
{
	GError *error = NULL;
	GDir *pDir = g_dir_open (cd_indicator3_get_directory_path (), 0, &error); // all indicators are on the same dir
	if (error != NULL)
	{
		cd_warning ("Failed to load indicator3 dir: %s", cd_indicator3_get_directory_path ());
		return;
	}

	
	// for each indicator file, instanciate a new plugin with it (useful to place it where we want, all icons are not regrouped into one big icon)
	const gchar *cFileName;
	gchar *cInstanceFilePath;
	while ((cFileName = g_dir_read_name (pDir)) != NULL)
	{
		if (*cFileName == '\0' || ! g_str_has_suffix (cFileName, ".so")
			|| _is_an_exception (cFileName, myConfig.cExceptionsList))
			continue;

		gchar *cUserDataDirPath = cairo_dock_check_module_conf_dir (myApplet->pModule);
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
		cairo_dock_instanciate_module (myApplet->pModule, cInstanceFilePath);  // we don't have to free cInstanceFilePath
		g_free (cUserDataDirPath);
	}
	g_dir_close (pDir);
}
