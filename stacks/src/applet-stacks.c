/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-stacks.h"
 
CD_APPLET_INCLUDE_MY_VARS

//ajouter dans applet facility?
gboolean _isin (gchar **cString, gchar *cCompar) {
	if (cString == NULL)
		return FALSE; //Nothing to search in
	
	int i=0;
	gchar *tmp=NULL;
	while (cString[i] != NULL) {
		tmp = g_strstr_len (cCompar, -1, cString[i]);
		if (tmp != NULL)
			return TRUE; //We found what we want
		i++;
	}
	
	return FALSE; //We didn't found anything
}

gboolean _useLocalDir (void) {
	int i = 0;
	while (myConfig.cMonitoredDirectory[i] != NULL) {
		if (strcmp (myConfig.cMonitoredDirectory[i], "_LocalDirectory_") == 0)
			return TRUE;
		i++;
	}
	return FALSE;
}

void cd_stacks_check_local (void) {
	gchar *cDirectory = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
	if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
		g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
		cd_debug("Stacks local directory made");
	}
	
	g_free (cDirectory);
}

void cd_stacks_mklink (const gchar *cFile) {
	cd_debug ("%s (%s)", __func__,  cFile);
	if (! myConfig.bLocalDir && ! _useLocalDir())
		return;
	
	cd_debug ("");
	GError *erreur = NULL;
	gchar *extension = strrchr (cFile, '.'), *cFileName = NULL, *cCommand = NULL, *cURI = NULL, *cHostname = NULL;
	
	/*if (extension != NULL && g_ascii_strcasecmp(extension, ".desktop") == 0) {
		cFileName = strrchr (cFile, '/');
		if (cFileName == NULL)
			cFileName = cFile;
		
		cURI = g_filename_from_uri (cFile, &cHostname, &erreur);
		cCommand = g_strdup_printf("cp \"%s\" \"/home/%s/.cairo-dock/stacks/%s\"", cURI, g_getenv ("USER"), cFileName);
		if (cCommand != NULL && cFile != NULL) {
			//cd_debug("Stacks: will use '%s'", cCommand);
			g_spawn_command_line_async (cCommand, &erreur);
			g_free(cCommand);
		}
		if (erreur != NULL) {
			cd_warning ("Attention : when trying to execute 'copy' : %s", erreur->message);
			g_error_free (erreur);
			CD_APPLET_MAKE_TEMPORARY_EMBLEM_CLASSIC (CAIRO_DOCK_EMBLEM_BROKEN, CAIRO_DOCK_EMBLEM_UPPER_LEFT, 5000);
			CD_APPLET_REDRAW_MY_ICON
		}
		return;
	}*/
	
	gchar *cBaseURI = g_strdup (cFile), *cIconName = NULL;
	gboolean bIsDirectory=FALSE;
	double fOrder=0;
	int iVolumeID=0;
	cairo_dock_fm_get_file_info (cBaseURI, &cFileName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_TYPE);
	cURI = g_filename_from_uri (cBaseURI, &cHostname, &erreur);
	
	if (cFileName == NULL) {
		cFileName = strrchr (cFile, '/');
		if (cFileName == NULL) {
			cd_warning ("Couldn't get filname with no path, halt.");
			CD_APPLET_MAKE_TEMPORARY_EMBLEM_CLASSIC (CAIRO_DOCK_EMBLEM_BROKEN, CAIRO_DOCK_EMBLEM_UPPER_LEFT, 5000);
			CD_APPLET_REDRAW_MY_ICON
			return;
		}
	}
	
	erreur = NULL;
	cCommand = g_strdup_printf("ln -s \"%s\" \"%s/stacks/%s\"", cURI, g_cCairoDockDataDir, cFileName);
	cd_debug("Stacks: linking %s in local dir", cFileName);
	if (cCommand != NULL && cFile != NULL) {
		//cd_debug("Stacks: will use '%s'", cCommand);
		g_spawn_command_line_async (cCommand, &erreur);
		g_free(cCommand);
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'link' : %s", erreur->message);
		g_error_free (erreur);
		CD_APPLET_MAKE_TEMPORARY_EMBLEM_CLASSIC (CAIRO_DOCK_EMBLEM_BROKEN, CAIRO_DOCK_EMBLEM_UPPER_LEFT, 5000);
		CD_APPLET_REDRAW_MY_ICON
	}
}

void cd_stacks_clean_local (void) {
	cd_debug ("%s", __func__);
	gchar *cCommand = g_strdup_printf("cd %s/stacks && rm -R *", g_cCairoDockDataDir);
	//cd_debug("Stacks: will use '%s'", cCommand);
	system (cCommand);
	g_free(cCommand);
}

void cd_stacks_run_dir (void) {
	gint i=0;
	gchar *cURI=NULL, *cDirectory=NULL;
	while (myConfig.cMonitoredDirectory[i] != NULL) {
		cDirectory = g_strdup (myConfig.cMonitoredDirectory[i]);
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
		}
		
		cURI = g_strdup_printf("file://%s", cDirectory);
		cairo_dock_fm_launch_uri(cURI);
		
		if (myConfig.bLocalDir && i == 0)
			break; //Solution temporaire
		i++;
	}
	g_free (cDirectory);
	g_free (cURI);
}

GList* cd_stacks_mime_filter (GList *pList) {
	if (pList == NULL)
		return NULL;
	
	GList *mList=NULL, *pElement=NULL;
  for (pElement = pList; pElement != NULL; pElement = pElement->next) {
  	Icon *pIcon = pElement->data;
    if (_isin(myConfig.cMimeTypes, pIcon->acFileName) == FALSE) {
    	//cd_debug ("Adding %s (%s) to filtered list", pIcon->acName, pIcon->acFileName);
    	mList = g_list_append(mList, pElement->data);
    }
  }
  
	return mList;
}
