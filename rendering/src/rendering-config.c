/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <cairo-dock.h>

#include "rendering-3D-plane.h"
#include "rendering-config.h"

extern double my_rendering_fInclinationOnHorizon;

extern double my_rendering_fForegroundRatio;
extern double my_rendering_iGapOnEllipse;
extern gboolean my_rendering_bRotateIconsOnEllipse;

extern double my_rendering_fParabolePower;
extern double my_rendering_fParaboleFactor;
extern double my_fSeparatorColor[4];


void cd_rendering_read_conf_file (gchar *cConfFilePath, gboolean *bFlatSeparator)
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
	
	double fInclinationAngle  = cairo_dock_get_double_key_value (pKeyFile, "Inclinated Plane", "inclination", &bFlushConfFileNeeded, 35);
	my_rendering_fInclinationOnHorizon = tan (fInclinationAngle * G_PI / 180.);
	
	my_rendering_fForegroundRatio = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "foreground ratio", &bFlushConfFileNeeded, .5);
	
	*bFlatSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Inclinated Plane", "flat separator", &bFlushConfFileNeeded, FALSE);
	
	double couleur[4] = {0.9,0.9,1.0,1.0};
	cairo_dock_get_double_list_key_value (pKeyFile, "Inclinated Plane", "separator color", &bFlushConfFileNeeded, my_fSeparatorColor, 4, couleur);
	
	my_rendering_iGapOnEllipse = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "gap on ellipse", &bFlushConfFileNeeded, 10);
	
	my_rendering_bRotateIconsOnEllipse = ! cairo_dock_get_boolean_key_value (pKeyFile, "Caroussel", "show face", &bFlushConfFileNeeded, FALSE);
	
	//if (! bFlushConfFileNeeded)  // pour l'instant on n'a pas de traduction en francais...
	//	bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile);
	if (bFlushConfFileNeeded)
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, MY_APPLET_SHARE_DATA_DIR);
	
	g_key_file_free (pKeyFile);
}
