/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <math.h>
#include <cairo-dock.h>

#include "rendering-3D-plane.h"
#include "rendering-config.h"

extern double my_fInclinationOnHorizon;

extern double my_fForegroundRatio;
extern double my_iGapOnEllipse;
extern gboolean my_bRotateIconsOnEllipse;

extern double my_fParabolePower;
extern double my_fParaboleFactor;
extern double my_fSeparatorColor[4];

extern double my_fParaboleCurvature;
extern double my_fParaboleRatio;
extern double my_fParaboleAmplitude;

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
	
	double fInclinationAngle  = cairo_dock_get_double_key_value (pKeyFile, "Inclinated Plane", "inclination", &bFlushConfFileNeeded, 35, NULL, NULL);
	my_fInclinationOnHorizon = tan (fInclinationAngle * G_PI / 180.);
	
	*bFlatSeparator = cairo_dock_get_boolean_key_value (pKeyFile, "Inclinated Plane", "flat separator", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	double couleur[4] = {0.9,0.9,1.0,1.0};
	cairo_dock_get_double_list_key_value (pKeyFile, "Inclinated Plane", "separator color", &bFlushConfFileNeeded, my_fSeparatorColor, 4, couleur, NULL, NULL);
	
	
	my_iGapOnEllipse = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "gap on ellipse", &bFlushConfFileNeeded, 10, NULL, NULL);
	
	my_bRotateIconsOnEllipse = ! cairo_dock_get_boolean_key_value (pKeyFile, "Caroussel", "show face", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	my_fForegroundRatio = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "foreground ratio", &bFlushConfFileNeeded, .5, NULL, NULL);
	
	
	my_fParaboleCurvature = cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "curvature", &bFlushConfFileNeeded, .5, NULL, NULL);
	
	my_fParaboleRatio = cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "ratio", &bFlushConfFileNeeded, 5, NULL, NULL);
	
	my_fParaboleAmplitude = cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "wave amplitude", &bFlushConfFileNeeded, .2, NULL, NULL);
	
	if (! bFlushConfFileNeeded)
		bFlushConfFileNeeded = cairo_dock_conf_file_needs_update (pKeyFile, MY_APPLET_VERSION);
	if (bFlushConfFileNeeded)
		cairo_dock_flush_conf_file (pKeyFile, cConfFilePath, MY_APPLET_SHARE_DATA_DIR);
	
	g_key_file_free (pKeyFile);
}
