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


#define CD_RENDERING_CONF_FILE "rendering.conf"
#define CD_RENDERING_USER_DATA_DIR "rendering"

double my_rendering_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.

double my_rendering_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
double my_rendering_iGapOnEllipse;  // regle la profondeur du caroussel.
gboolean my_rendering_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.

double my_rendering_fParabolePower = .5;
double my_rendering_fParaboleFactor = .33;


gchar *cd_rendering_pre_init (void)
{
	return g_strdup_printf ("%s/%s", CD_RENDERING_SHARE_DATA_DIR, CD_RENDERING_README_FILE);
}


Icon *cd_rendering_init (CairoDock *pDock, gchar **cConfFilePath, GError **erreur)
{
	//g_print ("%s ()\n", __func__);
	//\_______________ On verifie la presence des fichiers necessaires.
	*cConfFilePath = cairo_dock_check_conf_file_exists (CD_RENDERING_USER_DATA_DIR, CD_RENDERING_SHARE_DATA_DIR, CD_RENDERING_CONF_FILE);
	
	
	//\_______________ On lit le fichier de conf.
	cd_rendering_read_conf_file (*cConfFilePath);
	
	
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer ();
	
	cd_rendering_register_3D_plane_renderer ();
	
	//cd_rendering_register_parabole_renderer ();
	
	cairo_dock_set_all_views_to_default ();
	
	return NULL;
}

void cd_rendering_stop (void)
{
	cairo_dock_remove_renderer (CD_RENDERING_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_3D_PLANE_VIEW_NAME);
	//cairo_dock_remove_renderer (CD_RENDERING_PARABOLIC_VIEW_NAME);
	
	cairo_dock_reset_all_views ();
}


