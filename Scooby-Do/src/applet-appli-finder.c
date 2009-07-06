/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Adapted from the Gnome-panel for Cairo-Dock by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#define __USE_BSD 1
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "applet-struct.h"
#include "applet-appli-finder.h"

static void _browse_dir (const gchar *cDirPath);


void cd_do_reset_applications_list (void)
{
	g_list_foreach (myData.pApplications, (GFunc) cairo_dock_free_icon, NULL);
	g_list_free (myData.pApplications);
	myData.pApplications = NULL;
	
	GList *m;
	for (m = myData.pMonitorList; m != NULL; m = m->next)
	{
		cairo_dock_fm_remove_monitor_full (m->data, TRUE, NULL);
		g_free (m->data);
	}
	g_list_free (myData.pMonitorList);
	myData.pMonitorList = NULL;
	
	myData.pCurrentApplicationToLoad = NULL;
	if (myData.iSidLoadExternAppliIdle != 0)
	{
		g_source_remove (myData.iSidLoadExternAppliIdle);
		myData.iSidLoadExternAppliIdle = 0;
	}
}

static int _compare_appli (Icon *pIcon1, Icon *pIcon2)
{
	if (pIcon1->acCommand == NULL)
		return -1;
	if (pIcon2->acCommand == NULL)
		return 1;
	return strcmp (pIcon1->acCommand, pIcon2->acCommand);
}
static void _cd_do_on_file_event (CairoDockFMEventType iEventType, const gchar *cURI, gpointer data)
{
	g_print ("la liste des applis a change dans %s!\n", cURI);
	// on reconstruit la liste des applis.
	cd_do_reset_applications_list ();
	_browse_dir ("/usr/share/applications");  // oui c'est bourrin, ca ne doit pas arriver tres souvent?
	myData.pApplications = g_list_sort (myData.pApplications, (GCompareFunc) _compare_appli);
}
static void _browse_dir (const gchar *cDirPath)
{
	//\_______________ On ouvre le repertoire.
	GError *erreur = NULL;
	GDir *dir = g_dir_open (cDirPath, 0, &erreur);
	if (erreur != NULL)
	{
		cd_warning (erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	//\_______________ On liste tous ses .desktop.
	gboolean bFoundOneAppli = FALSE;
	gchar *cPath, *str, *cCommand, *cIconName;
	Icon *pIcon;
	GKeyFile* pKeyFile;
	const gchar *cFileName;
	GList *pLocalItemList = NULL;
	do
	{
		cFileName = g_dir_read_name (dir);
		if (cFileName == NULL)
			break ;
		
		cPath = g_strdup_printf ("%s/%s", cDirPath, cFileName);
		if (g_file_test (cPath, G_FILE_TEST_IS_DIR))
		{
			_browse_dir (cPath);
			g_free (cPath);
		}
		else
		{
			pKeyFile = cairo_dock_open_key_file (cPath);
			if (pKeyFile == NULL)
			{
				g_free (cPath);
				continue;
			}
			cCommand = g_key_file_get_string (pKeyFile, "Desktop Entry", "Exec", NULL);
			if (cCommand == NULL)
			{
				g_key_file_free (pKeyFile);
				g_free (cPath);
				continue;
			}
			cIconName = g_key_file_get_string (pKeyFile, "Desktop Entry", "Icon", NULL);
			if (cIconName == NULL)
			{
				g_key_file_free (pKeyFile);
				g_free (cPath);
				continue;
			}
			pIcon = g_new0 (Icon, 1);
			pIcon->acDesktopFileName = cPath;
			pIcon->acFileName = cIconName;
			pIcon->acCommand = cCommand;
			str = strchr (pIcon->acCommand, '%');
			if (str != NULL)
				*str = '\0';
			g_print (" + %s\n", pIcon->acCommand);
			pIcon->cWorkingDirectory = g_key_file_get_string (pKeyFile, "Desktop Entry", "Path", NULL);
			myData.pApplications = g_list_prepend (myData.pApplications, pIcon);
			g_key_file_free (pKeyFile);
			if (! bFoundOneAppli)
				bFoundOneAppli = TRUE;
		}
	}
	while (1);
	g_dir_close (dir);
	
	//\_______________ On le surveille s'il a fourni au moins une appli.
	if (bFoundOneAppli)
	{
		gchar *cMonitoredPath = g_strdup (cDirPath);
		if (cairo_dock_fm_add_monitor_full (cMonitoredPath, TRUE, NULL, (CairoDockFMMonitorCallback) _cd_do_on_file_event, cMonitoredPath))
		{
			myData.pMonitorList = g_list_prepend (myData.pMonitorList, cMonitoredPath);
		}
	}
}
static int _same_command (Icon *pIcon1, Icon *pIcon2)
{
	return cairo_dock_strings_differ (pIcon1->acCommand, pIcon2->acCommand);
}
static gboolean _load_applis_buffer_idle (gpointer data)
{
	if (myData.pCurrentApplicationToLoad == NULL)
		return FALSE;
	int iNbAppliLoaded = 0;
	Icon *pIcon;
	cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	gboolean bLoadTexture = (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock));
	GList *a;
	for (a = myData.pCurrentApplicationToLoad; a != NULL && iNbAppliLoaded < 3; a = a->next)  // on en charge 3 d'un coup.
	{
		pIcon = a->data;
		if (pIcon->pIconBuffer == NULL)
		{
			pIcon->fWidth = 48.;
			pIcon->fHeight = 48.;
			pIcon->fScale = 1.;
			gchar *cIconPath = cairo_dock_search_icon_s_path (pIcon->acFileName);
			pIcon->pIconBuffer = cairo_dock_create_surface_for_icon (cIconPath, pCairoContext, 48., 48);
			g_free (cIconPath);
			if (bLoadTexture)
				pIcon->iIconTexture = cairo_dock_create_texture_from_surface (pIcon->pIconBuffer);
			iNbAppliLoaded ++;
		}
	}
	cairo_destroy (pCairoContext);
	g_print (" %d de plus chargee(s)\n", iNbAppliLoaded);
	myData.pCurrentApplicationToLoad = a;
	if (a == NULL)  // on est arrive au bout de la liste.
	{
		g_print ("toutes les applis sont chargees !\n");
		cairo_dock_redraw_container (CAIRO_CONTAINER (g_pMainDock));
		myData.iSidLoadExternAppliIdle = 0;
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
void cd_do_find_matching_applications (void)
{
	//\_______________ On liste les applis.
	if (myData.pApplications == NULL)  // on n'a pas encore liste les applications, on le fait maintenant.
	{
		_browse_dir ("/usr/share/applications");
		myData.pApplications = g_list_sort (myData.pApplications, (GCompareFunc) _compare_appli);  // on parcourt tout d'un coup (plutot que par exemple seulement les .desktop correspondant a la 1ere lettre car il y'a les sous-rep a parcourir, donc il faut de toute maniere se farcir la totale; de plus la commande peut differer du nom du .desktop.
	}
	
	if (myData.sCurrentText->len == 0)
		return ;
	
	//\_______________ On teste chaque appli qu'on rajoute a la liste si elle correspond.
	gboolean bFound = FALSE;
	Icon *pIcon;
	//cairo_t *pCairoContext = cairo_dock_create_context_from_window (CAIRO_CONTAINER (g_pMainDock));
	//gboolean bLoadTexture = (CAIRO_CONTAINER_IS_OPENGL (g_pMainDock));
	GList *a;
	for (a = myData.pApplications; a != NULL; a = a->next)
	{
		pIcon = a->data;
		if (pIcon->acCommand == NULL || g_strncasecmp (pIcon->acCommand, myData.sCurrentText->str, myData.sCurrentText->len) != 0)
		{
			if (bFound)
				break;
			else
				continue;
		}
		if (g_list_find_custom (myData.pMatchingIcons, pIcon, (GCompareFunc)_same_command) == NULL)
		{
			g_print (" on ajoute %s\n", pIcon->acCommand);
			myData.pMatchingIcons = g_list_prepend (myData.pMatchingIcons, pIcon);
			/*if (pIcon->pIconBuffer == NULL)
			{
				pIcon->fWidth = 48.;
				pIcon->fHeight = 48.;
				pIcon->fScale = 1.;
				gchar *cIconPath = cairo_dock_search_icon_s_path (pIcon->acFileName);
				pIcon->pIconBuffer = cairo_dock_create_surface_for_icon (cIconPath, pCairoContext, 48., 48);
				g_free (cIconPath);
				if (bLoadTexture)
					pIcon->iIconTexture = cairo_dock_create_texture_from_surface (pIcon->pIconBuffer);
			}*/
		}
	}
	//cairo_destroy (pCairoContext);
	myData.pMatchingIcons = g_list_reverse (myData.pMatchingIcons);
	
	/// lancer le chargement des icones en idle...
	myData.pCurrentApplicationToLoad = myData.pMatchingIcons;
	if (myData.iSidLoadExternAppliIdle == 0)
		myData.iSidLoadExternAppliIdle = g_idle_add (_load_applis_buffer_idle, NULL);
}
