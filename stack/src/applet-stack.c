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
 

void cd_stack_check_local (CairoDockModuleInstance *myApplet, GKeyFile *pKeyFile)
{
	// be sure to not use the stack dir of another instance (it can happen when the applet is multi-instanciated, since new instances are initialized with the conf file of the first instance when they are created).
	GList *mi;
	CairoDockModuleInstance *applet;
	AppletConfig *cfg;
	for (mi = myApplet->pModule->pInstancesList; mi!= NULL; mi = mi->next)
	{
		applet = mi->data;
		if (applet == myApplet)
			continue;
		cfg = (AppletConfig*)applet->pConfig;
		if (cfg->cStackDir && strcmp (cfg->cStackDir, myConfig.cStackDir) == 0)
		{
			g_free (myConfig.cStackDir);
			myConfig.cStackDir = NULL;
		}
	}
	
	if (! g_file_test (myConfig.cStackDir, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE))
	{
		g_free (myConfig.cStackDir);
		myConfig.cStackDir = NULL;
	}
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
			cd_debug ("stack : test de %s\n", sDirPath->str);
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

void cd_stack_clear_stack (CairoDockModuleInstance *myApplet)
{
	gchar *cCommand = g_strdup_printf("rm -rf \"%s\"/*", myConfig.cStackDir);
	cd_debug("Stack: will use '%s'", cCommand);
	int r = system (cCommand);
	g_free(cCommand);
	
	CD_APPLET_DELETE_MY_ICONS_LIST;
	if (myDock)  // on ne veut pas d'un sous-dock vide, meme si on va probablement y rajouter des items aussitot.
	{
		cairo_dock_destroy_dock (myIcon->pSubDock, myIcon->cName);
		myIcon->pSubDock = NULL;
	}
}


void cd_stack_remove_item (CairoDockModuleInstance *myApplet, Icon *pIcon)
{
	gchar *cFilePath = g_strdup_printf ("%s/%s", myConfig.cStackDir, pIcon->cDesktopFileName);
	cd_message ("removing %s...", cFilePath);
	g_remove (cFilePath);
	g_free (cFilePath);
	
	CD_APPLET_REMOVE_ICON_FROM_MY_ICONS_LIST (pIcon);  // detruit l'icone.
}


static void _get_html_page (CDHtmlLink *pHtmlLink)
{
	CairoDockModuleInstance *myApplet = pHtmlLink->pApplet;
	// get the HTML page content
	gchar *cPageContent = cairo_dock_get_url_data (pHtmlLink->cURL, NULL);
	if (cPageContent == NULL)
	{
		cd_warning ("Stack: couldn't get the html page '%s'\n -> can't get the title and favicon", pHtmlLink->cURL);
		return;  // we could still draw the emblem in case it's already local, but not the title; so it wouldn't be really better.
	}
	
	// get the title
	gchar *str = strstr (cPageContent, "<title>");
	if (!str)
		str = strstr (cPageContent, "<TITLE>");
	if (str)
	{
		str += 7;
		gchar *str2 = strstr (str, "</title>");
		if (!str2)
			str2 = strstr (str, "</TITLE>");
		if (str2)
		{
			pHtmlLink->cTitle = g_strndup (str, str2 - str);
			// remove carriage returns.
			gchar *str = pHtmlLink->cTitle;
			while ((str = strchr (str, '\n')))
			{
				*str = ' ';  // replace the carriage returns with a space.
				str ++;  // begining of the new line.
				str2 = str;
				while (*str2 == ' ')
					str2 ++;
				if (str2 != str)  // remove spaces at the begining of the new line.
					strcpy (str, str2);
			}
		}
		cd_debug ("cTitle: '%s'", pHtmlLink->cTitle);
	}
	
	// get the domain name.
	gchar *cDomainName = NULL;
	str = strstr (pHtmlLink->cURL, "://");
	if (str)
	{
		str += 3;
		gchar *str2 = strchr (str, '/');
		if (str2)
			cDomainName = g_strndup (str, str2 - str);
	}
	cd_debug ("cDomainName: %s", cDomainName);
	
	// get the favicon or use the existing one.
	gchar *cLocalFaviconPath = NULL;
	if (cDomainName != NULL)
	{
		// check the favicons folder.
		gchar *cFaviconDir = g_strdup_printf ("%s/favicons", g_cCairoDockDataDir);
		if (! g_file_test (cFaviconDir, G_FILE_TEST_EXISTS))
		{
			g_mkdir (cFaviconDir, 7*8*8+7*8+5);
		}
		cLocalFaviconPath = g_strdup_printf ("%s/%s", cFaviconDir, cDomainName);
		g_free (cFaviconDir);
		
		// if favicon is not already on hard-disk, download it.
		if (! g_file_test (cLocalFaviconPath, G_FILE_TEST_EXISTS))
		{
			gboolean bGotFavIcon = FALSE;
			
			// try to get the favicon path specified in the html page.
			gchar *cRelPath = NULL;
			str = strstr (cPageContent, "<link rel=\"shortcut icon\"");
			if (!str)
				str = strstr (cPageContent, "<link rel=\"SHORTCUT ICON\"");
			if (str)  // found.
			{
				str += 25;
				// get its remote relative path.
				gchar *str2 = strstr (str, "href=\"");
				if (str2)
				{
					str2 += 6;
					gchar *str3 = strchr (str2, '"');
					if (str3)
					{
						cRelPath = g_strndup (str2, str3 - str2);
						cd_debug ("favicon: '%s'", cRelPath);
					}
				}
			}
			else  // no standard favicon, get the default one in the remote root dir.
			{
				cd_debug ("no favicon defined, looking for a default favicon.ico...");
				cRelPath = g_strdup ("favicon.ico");
			}
			
			// now download it.
			if (cRelPath != NULL)
			{
				gchar *cFaviconURL = NULL;
				if (*cRelPath == '/' && *(cRelPath+1) == '/')
				{
					cFaviconURL = g_strdup_printf ("http:%s", cRelPath);
				}
				else if (strstr (cRelPath, "://") != NULL)
				{
					cFaviconURL = cRelPath;
					cRelPath = NULL;
				}
				else
				{
					cFaviconURL = g_strdup_printf ("%s/%s", cDomainName, cRelPath);
				}
				cd_debug ("cFaviconURL: %s", cFaviconURL);
				
				bGotFavIcon = cairo_dock_download_file (cFaviconURL, cLocalFaviconPath);
				
				g_free (cFaviconURL);
				g_free (cRelPath);
			}
			
			if (! bGotFavIcon)  // couldn't get the favicon -> no favicon defined.
			{
				g_free (cLocalFaviconPath);
				cLocalFaviconPath = NULL;
			}
		}
	}
	pHtmlLink->cFaviconPath = cLocalFaviconPath;
}

static gboolean _update_html_link (CDHtmlLink *pHtmlLink)
{
	CairoDockModuleInstance *myApplet = pHtmlLink->pApplet;
	CD_APPLET_ENTER;
	
	// store in the conf file.
	cairo_dock_update_conf_file (pHtmlLink->cConfFilePath,
		G_TYPE_STRING, "Desktop Entry", "Favicon", pHtmlLink->cFaviconPath,
		G_TYPE_STRING, "Desktop Entry", "Name", pHtmlLink->cTitle,
		G_TYPE_INVALID);	
	
	// update the icon.
	gchar *cDesktopFileName = g_path_get_basename (pHtmlLink->cConfFilePath);
	if (cDesktopFileName)
	{
		Icon *pIcon = NULL;
		GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
		GList *ic;
		for (ic = pIconsList; ic != NULL; ic = ic->next)
		{
			pIcon = ic->data;
			if (pIcon->cDesktopFileName && strcmp (pIcon->cDesktopFileName, cDesktopFileName) == 0)
			{
				CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
				
				cairo_dock_set_icon_name (pHtmlLink->cTitle, pIcon, pContainer);
				
				cd_debug ("draw emblem on %s", pIcon->cName);
				cairo_dock_print_overlay_on_icon_from_image (pIcon, pContainer, pHtmlLink->cFaviconPath, CAIRO_OVERLAY_LOWER_RIGHT);
				cairo_dock_redraw_icon (pIcon, pContainer);
				break;
			}
		}
		g_free (cDesktopFileName);
	}
	
	cairo_dock_discard_task (pHtmlLink->pTask);
	myData.pGetPageTaskList = g_list_remove (myData.pGetPageTaskList, pHtmlLink->pTask);
	
	CD_APPLET_LEAVE (TRUE);
}

static void _free_html_link (CDHtmlLink *pHtmlLink)
{
	g_free (pHtmlLink->cURL);
	g_free (pHtmlLink->cTitle);
	g_free (pHtmlLink->cFaviconPath);
	g_free (pHtmlLink->cConfFilePath);
	g_free (pHtmlLink);
}

static void _set_icon_order (Icon *pIcon, CairoDockModuleInstance *myApplet, GCompareFunc comp)
{
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	Icon *icon;
	GList *ic;
	for (ic = pIconsList; ic != NULL; ic = ic->next)
	{
		icon = ic->data;
		if (comp (icon, pIcon) < 0)
		{
			if (ic->next != NULL)
			{
				Icon *next_icon = ic->next->data;
				pIcon->fOrder = (icon->fOrder + next_icon->fOrder) / 2;
			}
			else
			{
				pIcon->fOrder = icon->fOrder + 1;
			}
		}
	}
}
static Icon *_cd_stack_create_new_item (CairoDockModuleInstance *myApplet, const gchar *cContent)
{
	gchar *cName;
	double fOrder = 0;
	int iDate;
	CDHtmlLink *pHtmlLink = NULL;
	
	cd_debug ("Stack: got '%s'", cContent);
	//\_______________________ On lui trouve un petit nom.
	if (cairo_dock_string_is_adress (cContent) || *cContent == '/')
	{
		if (strncmp (cContent, "http://", 7) == 0 || strncmp (cContent, "www", 3) == 0 || strncmp (cContent, "https://", 8) == 0)
		{
			cd_debug (" -> html page");
			pHtmlLink = g_new0 (CDHtmlLink, 1);
			pHtmlLink->pApplet = myApplet;
			pHtmlLink->cURL = g_strdup (cContent);
			pHtmlLink->pTask = cairo_dock_new_task_full (0,
				(CairoDockGetDataAsyncFunc)_get_html_page,
				(CairoDockUpdateSyncFunc)_update_html_link,
				(GFreeFunc)_free_html_link,
				pHtmlLink);
			myData.pGetPageTaskList = g_list_prepend (myData.pGetPageTaskList, pHtmlLink->pTask);
			cairo_dock_launch_task (pHtmlLink->pTask);
			
			gchar *buf = g_strdup (cContent);
			gchar *str = strchr (buf, '?');
			if (str != NULL)
				*str = '\0';
			
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
			gchar *cFileName = (*cContent == '/' ? g_strdup (cContent) : g_uri_unescape_string (cContent, NULL));  // compared to g_filename_from_uri, g_uri_unescape_string will just unescape the string, without trying to check the URI; this way, something like trash://xyz will work (the "trash:/" is kept, which doesn't disturb g_path_get_basename)
			cName = g_path_get_basename (cFileName);
			g_free (cFileName);
		}
	}
	else
	{
		cName = cairo_dock_cut_string (cContent, 20);  // we only display the first 20 chars.
	}
	g_return_val_if_fail (cName != NULL, NULL);
	
	//\_______________________ On ecrit toutes les infos dans un fichier de conf.
	GList *pIconsList = CD_APPLET_MY_ICONS_LIST;
	Icon *icon = cairo_dock_get_last_icon (pIconsList);
	if (icon)
		fOrder = icon->fOrder + 1;
	
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
			g_string_printf (sConfFilePath, "%s/%s", myConfig.cStackDir, cName);
		else
			g_string_printf (sConfFilePath, "%s/%s.%d", myConfig.cStackDir, cName, i);
		i ++;
	} while (g_file_test (sConfFilePath->str, G_FILE_TEST_EXISTS));
	
	cairo_dock_write_keys_to_file (pKeyFile, sConfFilePath->str);
	if (pHtmlLink)
		pHtmlLink->cConfFilePath = g_strdup (sConfFilePath->str);
	
	//\_______________________ On cree une icone a partir du fichier de cle precedemment remplit.
	Icon *pIcon = cd_stack_build_one_icon (myApplet, pKeyFile);
	if (pIcon != NULL)
	{
		pIcon->cDesktopFileName = g_path_get_basename (sConfFilePath->str);
		
		if (myConfig.iSortType == CD_STACK_SORT_BY_NAME)
		{
			_set_icon_order (pIcon, myApplet, (GCompareFunc) cairo_dock_compare_icons_name);
		}
		else if (myConfig.iSortType == CD_STACK_SORT_BY_TYPE)
		{
			_set_icon_order (pIcon, myApplet, (GCompareFunc) cairo_dock_compare_icons_extension);
		}
	}
	
	g_key_file_free (pKeyFile);
	g_string_free (sConfFilePath, TRUE);
	return pIcon;
}

void cd_stack_create_and_load_item (CairoDockModuleInstance *myApplet, const gchar *cContent)
{
	//\_______________________ On cree l'item.
	Icon *pIcon = _cd_stack_create_new_item (myApplet, cContent);
	if (pIcon == NULL)  // peut arriver si l'icone est filtree.
		return ;
	
	//\_______________________ On le charge et on le rajoute au container.
	CD_APPLET_ADD_ICON_IN_MY_ICONS_LIST (pIcon);
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
