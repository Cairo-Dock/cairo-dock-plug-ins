/**
* This file is a part of the Cairo-Dock project
*
* Copyright : (C) see the 'copyright' file.
* E-mail    : see the 'copyright' file.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdlib.h"

#include "rendering-config.h"
#include "rendering-caroussel.h"
#include "rendering-parabole.h"
#include "rendering-3D-plane.h"
#include "rendering-rainbow.h"
//#include "rendering-diapo.h"
#include "rendering-slide.h"
#include "rendering-curve.h"
#include "rendering-panel.h"
#include "rendering-commons.h"
#include "rendering-init.h"

#define CD_RENDERING_CAROUSSEL_VIEW_NAME N_("Caroussel")
#define CD_RENDERING_3D_PLANE_VIEW_NAME N_("3D plane")
#define CD_RENDERING_PARABOLIC_VIEW_NAME N_("Parabolic")
#define CD_RENDERING_RAINBOW_VIEW_NAME N_("Rainbow")
//#define CD_RENDERING_DIAPO_VIEW_NAME "Slide"
#define CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME N_("Slide")
#define CD_RENDERING_CURVE_VIEW_NAME N_("Curve")
#define CD_RENDERING_PANEL_VIEW_NAME N_("Panel")


extern int iVanishingPointY;  // distance du point de fuite au plan (au niveau du point de contact du plan et des icones).

double my_fInclinationOnHorizon;  // inclinaison de la ligne de fuite vers l'horizon.
cairo_surface_t *my_pFlatSeparatorSurface[2] = {NULL, NULL};
GLuint my_iFlatSeparatorTexture = 0;

extern double my_fForegroundRatio;  // fraction des icones presentes en avant-plan (represente donc l'etirement en profondeur de l'ellipse).
extern double my_iGapOnEllipse;  // regle la profondeur du caroussel.
extern gboolean my_bRotateIconsOnEllipse;  // tourner les icones de profil ou pas.
extern double my_fScrollAcceleration;
extern double my_fScrollSpeed;

extern double my_fParaboleCurvature;  // puissance de x.
extern double my_fParaboleRatio;  // hauteur/largeur.
extern double my_fParaboleMagnitude;
extern int my_iParaboleTextGap;
extern gboolean my_bDrawTextWhileUnfolding;
extern gboolean my_bParaboleCurveOutside;

extern int my_iSpaceBetweenRows;
extern int my_iSpaceBetweenIcons;
extern double my_fRainbowMagnitude;
extern int my_iRainbowNbIconsMin;
extern double my_fRainbowConeOffset;
extern double my_fRainbowColor[4];
extern double my_fRainbowLineColor[4];

extern gdouble  my_diapo_simple_max_size;
extern gint     my_diapo_simple_iconGapX;
extern gint     my_diapo_simple_iconGapY;
extern gdouble  my_diapo_simple_fScaleMax;
extern gint     my_diapo_simple_sinW;
extern gboolean my_diapo_simple_lineaire;
extern gboolean  my_diapo_simple_wide_grid;
extern gboolean  my_diapo_simple_text_only_on_pointed;
extern gboolean my_diapo_simple_display_all_labels;

extern gboolean my_diapo_simple_use_default_colors;
extern gdouble  my_diapo_simple_color_frame_start[4];
extern gdouble  my_diapo_simple_color_frame_stop[4];
extern gboolean my_diapo_simple_fade2bottom;
extern gboolean my_diapo_simple_fade2right;
extern gint    my_diapo_simple_arrowWidth;
extern gint    my_diapo_simple_arrowHeight;
//gdouble  my_diapo_simple_arrowShift;
extern gint    my_diapo_simple_lineWidth;
extern gint    my_diapo_simple_radius;
extern gdouble  my_diapo_simple_color_border_line[4];
extern gboolean my_diapo_simple_draw_background;
extern gdouble  my_diapo_simple_color_scrollbar_line[4];
extern gdouble  my_diapo_simple_color_scrollbar_inside[4];
extern gdouble  my_diapo_simple_color_grip[4];

extern gdouble my_fCurveCurvature;
extern gint my_iCurveAmplitude;

extern gdouble my_fPanelRadius;
extern gdouble my_fPanelInclination;
extern gdouble my_fPanelRatio;
static gdouble s_fPreviousPanelRatio;
extern gboolean my_bPanelPhysicalSeparator;

GldiColor my_fSeparatorColor;


static gboolean on_style_changed (G_GNUC_UNUSED gpointer data)
{
	if (my_diapo_simple_use_default_colors)  // update slide params
	{
		cd_debug ("style changed update Slide...");
		
		my_diapo_simple_radius = myStyleParam.iCornerRadius;
		my_diapo_simple_lineWidth = myStyleParam.iLineWidth;
	}
	
	if (myIconsParam.bSeparatorUseDefaultColors
	&& (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] != NULL || my_iFlatSeparatorTexture != 0)
	&& g_pMainDock)
	{
		cd_debug ("update flat separators...");
		cd_rendering_load_flat_separator (CAIRO_CONTAINER(g_pMainDock));
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

CD_APPLET_DEFINE_BEGIN ("dock rendering",
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_THEME,
	"This module adds different views to your dock.\n"
	"Any dock or sub-dock can be displayed with the view of your choice.\n"
	"Currently, 3D-plane, Caroussel, Parabolic, Rainbow, Slide, and Curve views are provided.",
	"Fabounet (Fabrice Rey) &amp; parAdOxxx_ZeRo")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE;
	CD_APPLET_SET_CONTAINER_TYPE (CAIRO_DOCK_MODULE_IS_PLUGIN);
	CD_APPLET_EXTEND_MANAGER ("Backends");
	
	memset (&my_fSeparatorColor, 0, sizeof(GldiColor));
	cd_rendering_register_3D_plane_renderer (CD_RENDERING_3D_PLANE_VIEW_NAME);
	
	cd_rendering_register_parabole_renderer (CD_RENDERING_PARABOLIC_VIEW_NAME);
	
	cd_rendering_register_rainbow_renderer (CD_RENDERING_RAINBOW_VIEW_NAME);
	
	cd_rendering_register_diapo_simple_renderer (CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME);  // By Paradoxxx_Zero
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_LEAVE_DOCK,
		(GldiNotificationFunc) cd_slide_on_leave,
		GLDI_RUN_FIRST, NULL);  // on l'enregistre ici, et non pas sur le container, pour intercepter la fermeture du dock lorsque l'on en sort en tirant la scrollbar.
	gldi_object_register_notification (&myStyleMgr,
		NOTIFICATION_STYLE_CHANGED,
		(GldiNotificationFunc) on_style_changed,
		GLDI_RUN_FIRST, NULL);  // register first, so that we update our params before the Dock manager update docks size (in case the linewidth has changed)
	
	cd_rendering_register_curve_renderer (CD_RENDERING_CURVE_VIEW_NAME);  // By Paradoxxx_Zero and Fabounet
	
	cd_rendering_register_panel_renderer (CD_RENDERING_PANEL_VIEW_NAME);
	s_fPreviousPanelRatio = my_fPanelRatio;
CD_APPLET_DEFINE_END


CD_APPLET_INIT_BEGIN
	//\_______________ On enregistre les vues.
	///cd_rendering_register_caroussel_renderer 		(CD_RENDERING_CAROUSSEL_VIEW_NAME);
	
	/*cd_rendering_register_3D_plane_renderer 		(CD_RENDERING_3D_PLANE_VIEW_NAME);
	
	cd_rendering_register_parabole_renderer 		(CD_RENDERING_PARABOLIC_VIEW_NAME);
	
	cd_rendering_register_rainbow_renderer 		(CD_RENDERING_RAINBOW_VIEW_NAME);
	
	//cd_rendering_register_diapo_renderer 			(CD_RENDERING_DIAPO_VIEW_NAME);  // By Paradoxxx_Zero

	cd_rendering_register_diapo_simple_renderer 	(CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME);  // By Paradoxxx_Zero
	gldi_object_register_notification (&myDockObjectMgr,
		NOTIFICATION_LEAVE_DOCK,
		(GldiNotificationFunc) cd_slide_on_leave,
		GLDI_RUN_FIRST, NULL);  // on l'enregistre ici, et non pas sur le container, pour intercepter la fermeture du dock lorsque l'on en sort en tirant la scrollbar.
	
	cd_rendering_register_curve_renderer 			(CD_RENDERING_CURVE_VIEW_NAME);  // By Paradoxxx_Zero and Fabounet
	
	cd_rendering_register_panel_renderer 			(CD_RENDERING_PANEL_VIEW_NAME);
	
	if (! cairo_dock_is_loading ())  // plug-in active a la main (en-dehors du chargement du theme).
	{
		cairo_dock_set_all_views_to_default (0);
	}
	else
		gtk_widget_queue_draw (g_pMainDock->container.pWidget);*/
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	/*cairo_dock_remove_renderer (CD_RENDERING_CAROUSSEL_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_3D_PLANE_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_PARABOLIC_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_RAINBOW_VIEW_NAME);
	//cairo_dock_remove_renderer (CD_RENDERING_DIAPO_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_DIAPO_SIMPLE_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_CURVE_VIEW_NAME);
	cairo_dock_remove_renderer (CD_RENDERING_PANEL_VIEW_NAME);
	
	gldi_object_remove_notification (&myDockObjectMgr,
                NOTIFICATION_LEAVE_DOCK,
		(GldiNotificationFunc) cd_slide_on_leave, NULL);
	
	cairo_dock_reset_all_views ();
	gtk_widget_queue_draw (g_pMainDock->container.pWidget);*/
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (CD_APPLET_MY_CONFIG_CHANGED)
	{
		cairo_dock_set_all_views_to_default (0);
		
		// reload icons surface/texture in case their size have changed.
		if (s_fPreviousPanelRatio != my_fPanelRatio)
		{
			s_fPreviousPanelRatio = my_fPanelRatio;
			cairo_dock_reload_buffers_in_all_docks (TRUE);
		}
		
		gldi_docks_redraw_all_root ();
	}
CD_APPLET_RELOAD_END
