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
//#include "rendering-rainbow.h"
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

CairoDockVisitCard *pre_init (void)
{
	CairoDockVisitCard *pVisitCard = g_new0 (CairoDockVisitCard, 1);
	pVisitCard->cModuleName = g_strdup ("rendering");
	pVisitCard->cReadmeFilePath = g_strdup_printf ("%s/%s", MY_APPLET_SHARE_DATA_DIR, MY_APPLET_README_FILE);
	pVisitCard->iMajorVersionNeeded = 1;
	pVisitCard->iMinorVersionNeeded = 4;
	pVisitCard->iMicroVersionNeeded = 6;
	pVisitCard->cGettextDomain = g_strdup (MY_APPLET_GETTEXT_DOMAIN);
	pVisitCard->cDockVersionOnCompilation = g_strdup (MY_APPLET_DOCK_VERSION);
	pVisitCard->cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE);
	return pVisitCard;
}


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

Icon *init (CairoDock *pDock, CairoDockModule *pModule, GError **erreur)
{
	//g_print ("%s (%s)\n", __func__, MY_APPLET_DOCK_VERSION);
	//\_______________ On lit le fichier de conf.
	gboolean bFlatSeparator;
	read_conf_file (pModule->cConfFilePath, &bFlatSeparator);
	
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer ();
	
	cd_rendering_register_3D_plane_renderer ();
	
	cd_rendering_register_parabole_renderer ();
	
	//cd_rendering_register_rainbow_renderer ();  // pas encore ...
	
	cairo_dock_set_all_views_to_default ();
	
	//\_______________ On charge le separateur plat.
	_load_flat_separator (bFlatSeparator, pDock);
	
	return NULL;
}


void stop (void)
{
	cairo_dock_remove_renderer (MY_APPLET_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_3D_PLANE_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_PARABOLIC_VIEW_NAME);
	//cairo_dock_remove_renderer (MY_APPLET_RAINBOW_VIEW_NAME);
	
	reset_data ();
	
	cairo_dock_reset_all_views ();  // inutile de faire cairo_dock_set_all_views_to_default () puisqu'on ne peut desactiver un module qu'en validant la config du dock.
}


gboolean reload (gchar *cConfFilePath)
{
	if (cConfFilePath != NULL)
	{
		reset_data ();
		
		gboolean bFlatSeparator;
		read_conf_file (cConfFilePath, &bFlatSeparator);
		
		cairo_dock_set_all_views_to_default ();
		
		_load_flat_separator (bFlatSeparator, g_pMainDock);
	}
	return TRUE;
}
