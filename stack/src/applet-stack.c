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

#include <string.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-load-icons.h"
#include "applet-stack.h"
 

void cd_stack_check_local (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile) {
	
	if (myConfig.cStackDir == NULL)  // applet nouvellement instanciee.
	{
		GString *sDirPath = g_string_new ("");
		
		int i = 0;
		do
		{
			if (i == 0)
				g_string_printf (sDirPath, "%s/stack", g_cCairoDockDataDir);
			else
				g_string_printf (sDirPath, "%s/stack-%d", g_cCairoDockDataDir, i);
			i ++;
			g_print ("stack : test de %s\n", sDirPath->str);
		} while (g_file_test (sDirPath->str, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE));
		
		myConfig.cStackDir = sDirPath->str;
		g_string_free (sDirPath, FALSE);
		g_key_file_set_string (pKeyFile, "Configuration", "stack dir", myConfig.cStackDir);
		cairo_dock_write_keys_to_file (pKeyFile, myApplet->cConfFilePath);
	}
	cd_debug ("Stack : reperoire local : %s", myConfig.cStackDir);
	
	if (! g_file_test (myConfig.cStackDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
		g_mkdir_with_parents (myConfig.cStackDir, 7*8*8+7*8+5);
		cd_debug ("Stack local directory created (%s)", myConfig.cStackDir);
	}
}

void cd_stack_clear_stack (CairoDockModuleInstance *myApplet) {
	gchar *cCommand = g_strdup_printf("rm -rf \"%s\"/*", myConfig.cStackDir);
	cd_debug("Stack: will use '%s'", cCommand);
	int r = system (cCommand);
	g_free(cCommand);
	
	CD_APPLET_DELETE_MY_ICONS_LIST;
}


void cd_stack_remove_item (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	gchar *cFilePath = g_strdup_printf ("%s/%s", myConfig.cStackDir, pIcon->cDesktopFileName);
	cd_message ("removing %s...", cFilePath);
	g_remove (cFilePath);
	g_free (cFilePath);
	
	if (myDock)
	{
		cairo_dock_detach_icon_from_dock (pIcon, myIcon->pSubDock, FALSE);
		cairo_dock_update_dock_size (myIcon->pSubDock);
	}
	else
	{
		myDesklet->icons = g_list_remove (myDesklet->icons, pIcon);
		gtk_widget_queue_draw (myDesklet->container.pWidget);
	}
	cairo_dock_free_icon (pIcon);
}



Icon *cd_stack_create_item (CairoDockModuleInstance *myApplet, const gchar *cStackDirectory, const gchar *cContent)
{
	gchar *cName;
	double fOrder = 0;
	int iDate;
	
	if (cairo_dock_string_is_adress (cContent) || *cContent == '/')
	{
		if (strncmp (cContent, "http://", 7) == 0 || strncmp (cContent, "www", 3) == 0 || strncmp (cContent, "https://", 8) == 0)
		{
			gchar *buf = g_strdup (cContent);
			gchar *str = strchr (buf, '?');
			if (str != NULL)
			{
				*str = '\0';
				str ++;
			}
			else
				str = buf;
			if (str[strlen(str)-1] == '/')
				str[strlen(str)-1] = '\0';
			str = strrchr (buf, '/');
			if (str != NULL && *(str+1) != '\0')
			{
				cName = g_strdup (str+1);
				g_free (buf);
			}
			else
			{
				cName = buf;
			}
		}
		else
		{
			gchar *cFileName = (*cContent == '/' ? g_strdup (cContent) : g_filename_from_uri (cContent, NULL, NULL));  // virer l'extension ?
			cName = g_path_get_basename (cFileName);
			g_free (cFileName);
		}
	}
	else
	{
		cName = cairo_dock_cut_string (cContent, 15);  // 15 caracteres par defaut.
	}
	g_return_val_if_fail (cName != NULL, NULL);
	
	GList *pIconsList = (myDock ? (myIcon->pSubDock != NULL ? myIcon->pSubDock->icons : NULL) : myDesklet->icons);
	GList *ic;
	Icon *icon;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		fOrder = MAX (fOrder, icon->fOrder);
	}
	
	iDate = time (NULL);
	
	GKeyFile *pKeyFile = g_key_file_new();
	g_key_file_set_string (pKeyFile, "Desktop Entry", "Name", cName);
	g_key_file_set_integer (pKeyFile, "Desktop Entry", "Date", iDate);
	g_key_file_set_double (pKeyFile, "Desktop Entry", "Order", fOrder);
	if (*cContent == '/')
	{
		gchar *cURI = g_filename_to_uri (cContent, NULL, NULL);
		if (cURI == NULL)
		{
			g_key_file_free (pKeyFile);
			g_free (cURI);
			cd_warning ("stack : '%s' is not a valid adress", cContent);
			return NULL;
		}
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Content", cURI);
		g_free (cURI);
	}
	else
		g_key_file_set_string (pKeyFile, "Desktop Entry", "Content", cContent);
	
	GString *sConfFilePath = g_string_new ("");
	int i = 0;
	do
	{
		if (i == 0)
			g_string_printf (sConfFilePath, "%s/%s", cStackDirectory, cName);
		else
			g_string_printf (sConfFilePath, "%s/%s.%d", cStackDirectory, cName, i);
		i ++;
	} while (g_file_test (sConfFilePath->str, G_FILE_TEST_EXISTS));
	
	cairo_dock_write_keys_to_file (pKeyFile, sConfFilePath->str);
	
	Icon *pIcon = cd_stack_build_one_icon (myApplet, pKeyFile);
	if (pIcon != NULL)
		pIcon->cDesktopFileName = g_path_get_basename (sConfFilePath->str);
	
	g_key_file_free (pKeyFile);
	g_string_free (sConfFilePath, TRUE);
	return pIcon;
}

void cd_stack_create_and_load_item (CairoDockModuleInstance *myApplet, const gchar *cContent)
{
	//\_______________________ On cree l'item.
	Icon *pIcon = cd_stack_create_item (myApplet, myConfig.cStackDir, cContent);
	if (pIcon == NULL)  // peut arriver si l'icone est filtree.
		return ;
	
	//\_______________________ On le charge et on le rajoute au container.
	if (myDock)
	{
		if (myIcon->pSubDock == NULL)
		{
			GList *pStacksIconList = NULL;
			pStacksIconList = g_list_prepend (pStacksIconList, pIcon);
			CD_APPLET_CREATE_MY_SUBDOCK (pStacksIconList, myConfig.cRenderer);
		}
		else
		{
			cairo_dock_load_one_icon_from_scratch (pIcon, CAIRO_CONTAINER (myIcon->pSubDock));
			GCompareFunc pCompareFunc = NULL;
			switch (myConfig.iSortType)
			{
				case CD_STACK_SORT_BY_DATE :
				case CD_STACK_SORT_MANUALLY :
					pCompareFunc = (GCompareFunc) cairo_dock_compare_icons_order;
				break;
				case CD_STACK_SORT_BY_NAME :
					pCompareFunc = (GCompareFunc) cairo_dock_compare_icons_name;
				break;
				case CD_STACK_SORT_BY_TYPE :
				default :
					pCompareFunc = (GCompareFunc) cairo_dock_compare_icons_extension;
				break;
			}
			cairo_dock_insert_icon_in_dock_full (pIcon, myIcon->pSubDock, CAIRO_DOCK_UPDATE_DOCK_SIZE, CAIRO_DOCK_ANIMATE_ICON, ! CAIRO_DOCK_INSERT_SEPARATOR, pCompareFunc);
		}
	}
	else
	{
		GList *pStacksIconList = myDesklet->icons;
		pStacksIconList = cd_stack_insert_icon_in_list (myApplet, pStacksIconList, pIcon);
		
		//cairo_dock_load_one_icon_from_scratch (pIcon, CAIRO_CONTAINER (myDesklet));
		
		myDesklet->icons = pStacksIconList;
		
		const gchar *cDeskletRendererName = NULL;
		switch (myConfig.iDeskletRendererType)
		{
			case CD_DESKLET_SLIDE :
			default :
				cDeskletRendererName = "Slide";
			break ;
			
			case CD_DESKLET_TREE :
				cDeskletRendererName = "Tree";
			break ;
		}
		CD_APPLET_SET_DESKLET_RENDERER_WITH_DATA (cDeskletRendererName, NULL);
		CAIRO_DOCK_REDRAW_MY_CONTAINER;
	}
}

void cd_stack_set_item_name (const gchar *cDesktopFilePath, const gchar *cName)
{
	cairo_dock_update_conf_file (cDesktopFilePath,
		G_TYPE_STRING, "Desktop Entry", "Name", cName,
		G_TYPE_INVALID);
}

void cd_stack_set_item_order (const gchar *cDesktopFilePath, double fOrder)
{
	cairo_dock_update_conf_file (cDesktopFilePath,
		G_TYPE_DOUBLE, "Desktop Entry", "Order", fOrder,
		G_TYPE_INVALID);
}
