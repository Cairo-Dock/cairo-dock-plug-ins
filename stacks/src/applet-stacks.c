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

void cd_stacks_check_local(void) {
	gchar *cDirectory = g_strdup_printf("/home/%s/.cairo-dock/stacks", g_getenv ("USER"));
	if (! g_file_test (cDirectory, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_EXECUTABLE)) {
		g_mkdir_with_parents (cDirectory, 7*8*8+7*8+5);
		cd_debug("Stacks local directory made");
	}
	else
		cd_debug("Stacks local directory exists");
	
	g_free (cDirectory);
}

void cd_stacks_mklink(const gchar *cFile) {
	cd_debug("%s (%s)", __func__, cFile);
	if (!myConfig.bLocalDir)
		return;
	
	//(const gchar *cBaseURI, gchar **cName, gchar **cURI, gchar **cIconName, gboolean *bIsDirectory, int *iVolumeID, double *fOrder, CairoDockFMSortType iSortType)
	gchar *cBaseURI = g_strdup (cFile), *cIconName = NULL, *cURI = NULL, *cFileName = NULL;
	gboolean *bIsDirectory=NULL;
	double *fOrder=NULL;
	int *iVolumeID=NULL;
	cairo_dock_fm_get_file_info(cBaseURI, &cFileName, &cURI, &cIconName, &bIsDirectory, &iVolumeID, &fOrder, CAIRO_DOCK_FM_SORT_BY_TYPE);
	
	if (cFileName == NULL)
		return;
	
	gchar *cHostname = NULL;
	GError *erreur = NULL;
	cURI = g_filename_from_uri (cBaseURI, &cHostname, &erreur);
	
	erreur = NULL;
	gchar *cCommand = g_strdup_printf("ln -s \"%s\" \"/home/%s/.cairo-dock/stacks/%s\"", cURI, g_getenv ("USER"), cFileName);
	cd_debug("Stacks: linking %s in local dir", cFileName);
	if (cCommand != NULL && cFile != NULL) {
		cd_debug("Stacks: will use '%s'", cCommand);
		g_spawn_command_line_async (cCommand, &erreur);
		g_free(cCommand);
	}
	if (erreur != NULL) {
		cd_warning ("Attention : when trying to execute 'link' : %s", erreur->message);
		g_error_free (erreur);
	}
}

void cd_stacks_clean_local(void) {
	gchar *cCommand = g_strdup_printf("cd /home/%s/.cairo-dock/stacks/ && rm *", g_getenv ("USER"));
	cd_debug("Stacks: will use '%s'", cCommand);
	system (cCommand);
	g_free(cCommand);
}

void cd_stacks_run_dir(void) {
	gchar *cURI = g_strdup_printf("file://%s", myConfig.cMonitoredDirectory);
	cd_debug("Stacks: will use '%s'", cURI);
	cairo_dock_fm_launch_uri(cURI);
}
