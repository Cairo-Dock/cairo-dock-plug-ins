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
#include "rendering-init.h"

#define MY_APPLET_CONF_FILE "rendering.conf"
#define MY_APPLET_USER_DATA_DIR "rendering"

double my_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.

cairo_surface_t *my_pFlatSeparatorSurface[2];
double my_fSeparatorColor[4];

double my_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
double my_iGapOnEllipse;  // regle la profondeur du caroussel.
gboolean my_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.

double my_fParabolePower = .5;
double my_fParaboleFactor = .33;


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
	return pVisitCard;
}


Icon *init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s (%s)\n", __func__, MY_APPLET_DOCK_VERSION);
	//\_______________ On verifie la presence des fichiers necessaires.
	*cConfFilePath = cairo_dock_check_conf_file_exists (MY_APPLET_USER_DATA_DIR, MY_APPLET_SHARE_DATA_DIR, MY_APPLET_CONF_FILE);
	
	
	//\_______________ On lit le fichier de conf.
	gboolean bFlatSeparator;
	cd_rendering_read_conf_file (*cConfFilePath, &bFlatSeparator);
	
	
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer ();
	
	cd_rendering_register_3D_plane_renderer ();
	
	cd_rendering_register_parabole_renderer ();  // pas encore ...
	
	cairo_dock_set_all_views_to_default ();
	
	if (bFlatSeparator && g_bUseSeparator)
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
	
	return NULL;
}

void stop (void)
{
	cairo_dock_remove_renderer (MY_APPLET_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_3D_PLANE_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_PARABOLIC_VIEW_NAME);
	
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
	my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = NULL;
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
	my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = NULL;
	
	cairo_dock_reset_all_views ();
}


