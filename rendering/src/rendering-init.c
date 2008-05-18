/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include "stdlib.h"

#include "rendering-config.h"
#include "rendering-caroussel.h"
#include "rendering-parabole.h"
#include "rendering-3D-plane.h"
#include "rendering-rainbow.h"
#include "rendering-diapo.h"
#include "rendering-diapo-simple.h"
#include "rendering-curve.h"
#include "rendering-desklet-tree.h"
#include "rendering-desklet-caroussel.h"
#include "rendering-desklet-simple.h"
#include "rendering-desklet-controler.h"
#include "rendering-init.h"

#define MY_APPLET_CONF_FILE "rendering.conf"
#define MY_APPLET_USER_DATA_DIR "rendering"

double my_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.
CDSpeparatorType my_iDrawSeparator3D;

cairo_surface_t *my_pFlatSeparatorSurface[2] = {NULL, NULL};
double my_fSeparatorColor[4];

double my_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
double my_iGapOnEllipse;  // regle la profondeur du caroussel.
gboolean my_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.

double my_fParaboleCurvature;  // puissance de x.
double my_fParaboleRatio;  // hauteur/largeur.
double my_fParaboleMagnitude;
int my_iParaboleTextGap;
gboolean my_bDrawTextWhileUnfolding;

int my_iSpaceBetweenRows;
int my_iSpaceBetweenIcons;
double my_fRainbowMagnitude;
int my_iRainbowNbIconsMin;
double my_fRainbowConeOffset;

gint     my_diapo_iconGapX;
gint     my_diapo_iconGapY;
gdouble  my_diapo_fScaleMax;
gint     my_diapo_sinW;
gboolean my_diapo_lineaire;
gboolean  my_diapo_wide_grid;
gboolean  my_diapo_text_only_on_pointed;

gdouble  my_diapo_color_frame_start[4];
gdouble  my_diapo_color_frame_stop[4];
gboolean my_diapo_fade2bottom;
gboolean my_diapo_fade2right;
guint    my_diapo_arrowWidth;
guint    my_diapo_arrowHeight;
gdouble  my_diapo_arrowShift;
guint    my_diapo_lineWidth;
guint    my_diapo_radius;
gdouble  my_diapo_color_border_line[4];
gboolean my_diapo_draw_background;

gint     my_diapo_simple_iconGapX;
gint     my_diapo_simple_iconGapY;
gdouble  my_diapo_simple_fScaleMax;
gint     my_diapo_simple_sinW;
gboolean my_diapo_simple_lineaire;
gboolean  my_diapo_simple_wide_grid;
gboolean  my_diapo_simple_text_only_on_pointed;
gboolean my_diapo_simple_display_all_icons;

gdouble  my_diapo_simple_color_frame_start[4];
gdouble  my_diapo_simple_color_frame_stop[4];
gboolean my_diapo_simple_fade2bottom;
gboolean my_diapo_simple_fade2right;
guint    my_diapo_simple_arrowWidth;
guint    my_diapo_simple_arrowHeight;
gdouble  my_diapo_simple_arrowShift;
guint    my_diapo_simple_lineWidth;
guint    my_diapo_simple_radius;
gdouble  my_diapo_simple_color_border_line[4];
gboolean my_diapo_simple_draw_background;
gboolean my_diapo_display_all_icons;

gdouble my_curve_curvitude;
CDSpeparatorType my_curve_iDrawSeparator3D;
double my_curve_fSeparatorColor[4];

CD_APPLET_PRE_INIT_BEGIN("rendering", 1, 5, 4, CAIRO_DOCK_CATEGORY_DESKTOP)
	rendering_register_tree_desklet_renderer ();
	rendering_register_caroussel_desklet_renderer ();
	rendering_register_simple_desklet_renderer ();
	rendering_register_controler_desklet_renderer ();
CD_APPLET_PRE_INIT_END


static void _load_flat_separator (CairoContainer *pContainer)
{
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
	if (my_iDrawSeparator3D == CD_FLAT_SEPARATOR)
	{
		cairo_t *pSourceContext = cairo_dock_create_context_from_window (pContainer);
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

void init (GKeyFile *pKeyFile, Icon *pIcon, CairoContainer *pContainer, gchar *cConfFilePath, GError **erreur)
{
	//g_print ("%s (%s)\n", __func__, MY_APPLET_DOCK_VERSION);
	//\_______________ On lit le fichier de conf.
	read_conf_file (pKeyFile);
	
	//\_______________ On enregistre les vues.
	cd_rendering_register_caroussel_renderer ();
	
	cd_rendering_register_3D_plane_renderer ();
	
	cd_rendering_register_parabole_renderer ();
	
	cd_rendering_register_rainbow_renderer ();
	
	cd_rendering_register_diapo_renderer (); 

	cd_rendering_register_diapo_simple_renderer (); 
	
	cd_rendering_register_curve_renderer (); 
	
	cairo_dock_set_all_views_to_default ();
	
	//\_______________ On charge le separateur plat.
	_load_flat_separator (CAIRO_CONTAINER (g_pMainDock));
}


void stop (void)
{
	cairo_dock_remove_renderer (MY_APPLET_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_3D_PLANE_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_PARABOLIC_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_RAINBOW_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_DIAPO_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_DIAPO_SIMPLE_VIEW_NAME);
	cairo_dock_remove_renderer (MY_APPLET_CURVE_VIEW_NAME);
	reset_data ();
	
	cairo_dock_reset_all_views ();  // inutile de faire cairo_dock_set_all_views_to_default () puisqu'on ne peut desactiver un module qu'en validant la config du dock.
}


gboolean reload (GKeyFile *pKeyFile, gchar *cConfFilePath, CairoContainer *pNewContainer)
{
	if (pKeyFile != NULL)
	{
		reset_data ();
		
		read_conf_file (pKeyFile);
		
		cairo_dock_set_all_views_to_default ();
		
		_load_flat_separator (CAIRO_CONTAINER (g_pMainDock));
	}
	return TRUE;
}
