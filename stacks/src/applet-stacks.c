/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

******************************************************************************/
#include <string.h>
#include <cairo-dock.h>

#include "applet-struct.h"
#include "applet-stacks.h"
 

//ajouter dans applet facility?
static gboolean _isin (gchar **cString, gchar *cCompar) {
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

static gboolean _useLocalDir (void) {
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

void cd_stacks_mklink (const gchar *cURI) {
	cd_message ("%s (%s)", __func__,  cURI);
	if (! myConfig.bLocalDir && ! _useLocalDir())
		return;
	
	GError *erreur = NULL;
	gchar *cFilePath = g_filename_from_uri (cURI, NULL, &erreur);
	if (erreur != NULL) {
		cd_warning ("URI (%s) seems not valid [%s], halt.", cURI, erreur->message);
		g_error_free (erreur);
		CD_APPLET_MAKE_TEMPORARY_EMBLEM_CLASSIC (CAIRO_DOCK_EMBLEM_BROKEN, CAIRO_DOCK_EMBLEM_UPPER_LEFT, 5000);
		CD_APPLET_REDRAW_MY_ICON;
		return;
	}
	
	gchar *cCommand = g_strdup_printf("ln -s \"%s\" \"%s/stacks\"", cFilePath, g_cCairoDockDataDir);
	cd_debug("Stacks: will use '%s'", cCommand);
	int r = system (cCommand);  // c'est une commande quasi-immediate donc pas vraiment besoin de le lancer en asynchrone.
	g_print ("retour : %d\n", r);  // quel est le retour en cas d'erreur ?
	g_free (cCommand);
}

void cd_stacks_clean_local (void) {
	cd_message ("");
	//gchar *cCommand = g_strdup_printf("cd %s/stacks && rm -R *", g_cCairoDockDataDir);
	gchar *cCommand = g_strdup_printf("rm -rf %s/stacks/*", g_cCairoDockDataDir);
	cd_debug("Stacks: will use '%s'", cCommand);
	system (cCommand);
	g_free(cCommand);
}

void cd_stacks_run_dir (void) {
	gint i=0;
	gchar *cURI, *cDirectory;
	while (myConfig.cMonitoredDirectory[i] != NULL) {
		cDirectory = g_strdup (myConfig.cMonitoredDirectory[i]);
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
		}
		
		///cURI = g_strdup_printf("file://%s", cDirectory);
		cURI = g_filename_to_uri (cDirectory, NULL, NULL);
		cairo_dock_fm_launch_uri (cURI);
		
		g_free (cDirectory);
		g_free (cURI);
		
		if (myConfig.bLocalDir && i == 0)
			break; //Solution temporaire
		i++;
	}
}

GList* cd_stacks_mime_filter (GList *pIconsList) {
	Icon *pIcon;
	GList *pFilteredList=NULL, *pElement=NULL;
	for (pElement = pIconsList; pElement != NULL; pElement = pElement->next) {
		pIcon = pElement->data;
		if (_isin(myConfig.cMimeTypes, pIcon->acFileName) == FALSE) {
			//cd_debug ("Adding %s (%s) to filtered list", pIcon->acName, pIcon->acFileName);
			pFilteredList = g_list_append (pFilteredList, pElement->data);
		}
	}
	
	return pFilteredList;
}

void cd_stacks_remove_monitors (void) {
	cd_debug ("Removing all old monitors");
	gint i=0, j=0;
	while (myData.cMonitoredDirectory[i] != NULL) {
		gchar *cDirectory = g_strdup(myData.cMonitoredDirectory[i]);
		if (cDirectory == NULL)
			break;
		
		if (strcmp (cDirectory, "_LocalDirectory_") == 0) {
			g_free (cDirectory);
			cDirectory = g_strdup_printf("%s/stacks", g_cCairoDockDataDir);
		}
		cairo_dock_fm_remove_monitor_full (cDirectory, TRUE, NULL);
		g_free (cDirectory);
		i++;
	}
}
