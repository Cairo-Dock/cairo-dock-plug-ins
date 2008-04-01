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
extern double my_fParaboleMagnitude;
extern int my_iParaboleTextGap;
extern gboolean my_bDrawTextWhileUnfolding;

extern cairo_surface_t *my_pFlatSeparatorSurface[2];

extern int my_iSpaceBetweenRows;
extern int my_iSpaceBetweenIcons;
extern double my_fRainbowMagnitude;
extern int my_iRainbowNbIconsMin;
extern double my_fRainbowConeOffset;

void read_conf_file (GKeyFile *pKeyFile, gboolean *bFlatSeparator)
{
	gboolean bFlushConfFileNeeded = FALSE;  // si un champ n'existe pas, on le rajoute au fichier de conf.
	
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
	
	my_fParaboleMagnitude = cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "wave magnitude", &bFlushConfFileNeeded, .2, NULL, NULL);
	
	my_iParaboleTextGap = cairo_dock_get_integer_key_value (pKeyFile, "Parabolic", "text gap", &bFlushConfFileNeeded, 3, NULL, NULL);
	
	my_bDrawTextWhileUnfolding  = cairo_dock_get_boolean_key_value (pKeyFile, "Parabolic", "draw text", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	
	
	my_iSpaceBetweenRows = cairo_dock_get_integer_key_value (pKeyFile, "Rainbow", "space between rows", &bFlushConfFileNeeded, 10, NULL, NULL);
	
	my_iSpaceBetweenIcons = cairo_dock_get_integer_key_value (pKeyFile, "Rainbow", "space between icons", &bFlushConfFileNeeded, 8, NULL, NULL);
	
	my_fRainbowMagnitude = cairo_dock_get_double_key_value (pKeyFile, "Rainbow", "wave magnitude", &bFlushConfFileNeeded, .3, NULL, NULL);
	
	my_iRainbowNbIconsMin = cairo_dock_get_integer_key_value (pKeyFile, "Rainbow", "nb icons min", &bFlushConfFileNeeded, 3, NULL, NULL);
	
	my_fRainbowConeOffset = G_PI * (1 - cairo_dock_get_double_key_value (pKeyFile, "Rainbow", "cone", &bFlushConfFileNeeded, 130, NULL, NULL) / 180) / 2;
}


void reset_data (void)
{
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
	my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = NULL;
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
	my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = NULL;
}
