/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <cairo-dock.h>

#include "rendering-config.h"

extern double my_rendering_fInclination;
extern int my_rendering_iGapOnEllipse;
extern double my_rendering_fForegroundRatio;
extern gboolean my_rendering_bRotateIconsOnEllipse;


void cd_rendering_read_conf_file (gchar *cConfFilePath)
{
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new ();
	g_key_file_load_from_file (pKeyFile, cConfFilePath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	if (erreur != NULL)
	{
		g_print ("Attention : %s\n", erreur->message);
		g_error_free (erreur);
		return ;
	}
	
	
	double fAngle = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "angle", &bFlushConfFileNeeded, 30.);
	my_rendering_fInclination = tan (fAngle * G_PI / 180.);
	
	my_rendering_iGapOnEllipse = cairo_dock_get_integer_key_value (pKeyFile, "Caroussel", "gap on ellipse", &bFlushConfFileNeeded, 10);
	
	my_rendering_fForegroundRatio = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "foreground ratio", &bFlushConfFileNeeded, .5);
	
	my_rendering_bRotateIconsOnEllipse = ! cairo_dock_get_boolean_key_value (pKeyFile, "Caroussel", "show face", &bFlushConfFileNeeded, FALSE);
	
	//if (! bFlushConfFileNeeded)  // pour l'instant on n'a pas de traduction en francais...
	//	bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile);
	if (bFlushConfFileNeeded)
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, CD_RENDERING_SHARE_DATA_DIR);
	
	g_key_file_free (pKeyFile);
}
