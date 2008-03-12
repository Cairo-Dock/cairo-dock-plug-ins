/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include "stdlib.h"

#include "rendering-config.h"
#include "rendering-caroussel.h"
#include "rendering-parabole.h"
#include "rendering-3D-plane.h"
#include "rendering-rainbow.h"
#include "rendering-init.h"

#define MY_APPLET_CONF_FILE "rendering.conf"
#define MY_APPLET_USER_DATA_DIR "rendering"

double my_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.

cairo_surface_t *my_pFlatSeparatorSurface[2];
double my_fSeparatorColor[4];

double my_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
double my_iGapOnEllipse;  // regle la profondeur du caroussel.
gboolean my_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.

double my_fParaboleCurvature;  // puissance de x.
double my_fParaboleRatio;  // hauteur/largeur.
double my_fParaboleMagnitude;
int my_iParaboleTextGap;
gboolean my_bDrawTextWhileUnfolding;

int my_iSpaceBetweenRows = 10;
int my_iSpaceBetweenIcons = 0;
double my_fRainbowMagnitude = .3;
int my_iRainbowNbIconsMin = 1;
double my_fRainbowConeOffset = (60./180.*G_PI);


CD_APPLET_DEFINITION("rendering", 1, 4, 7, CAIRO_DOCK_CATEGORY_DESKTOP)


static void _load_flat_separator (gboolean bFlatSeparator, CairoDock *pDock)
{
	if (bFlatSeparator)
	{
		cairo_t *pSourceContext = cairo_dock_create_context_from_window (pDock);
		my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = cd_rendering_create_flat_separator_surface (pSourceContext, 150, 150);
		my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], pSourceContext, 150, 150, -G_PI / 2);
		cairo_destroy (pSourceContext);
	}
	else
	{
		my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = NULL;
		my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = NULL;
	}
}

void init (GKeyFile *pKeyFile, Icon *pIcon, CairoDockContainer *pContainer, gchar *cConfFilePath, GError **erreur)
{
	//g_print ("%s (%s)\n", __func__, MY_APPLET_DOCK_VERSION);
	//\_______________ On lit le fichier de conf.
	gboolean bFlatSeparator;
	read_conf_file (pKeyFile, &bFlatSeparator);
	
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer ();
	
	cd_rendering_register_3D_plane_renderer ();
	
	cd_rendering_register_parabole_renderer ();
	
	cd_rendering_register_rainbow_renderer ();  // pas encore ...
	
	cairo_dock_set_all_views_to_default ();
	
	//\_______________ On charge le separateur plat.
	_load_flat_separator (bFlatSeparator, g_pMainDock);
}


void stop (void)
{
	cairo_dock_remove_renderer (MY_APPLET_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_3D_PLANE_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_PARABOLIC_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_RAINBOW_VIEW_NAME);
	
	reset_data ();
	
	cairo_dock_reset_all_views ();  // inutile de faire cairo_dock_set_all_views_to_default () puisqu'on ne peut desactiver un module qu'en validant la config du dock.
}


gboolean reload (GKeyFile *pKeyFile, gchar *cConfFilePath, CairoDockContainer *pNewContainer)
{
	if (pKeyFile != NULL)
	{
		reset_data ();
		
		gboolean bFlatSeparator;
		read_conf_file (pKeyFile, &bFlatSeparator);
		
		cairo_dock_set_all_views_to_default ();
		
		_load_flat_separator (bFlatSeparator, g_pMainDock);
	}
	return TRUE;
}
