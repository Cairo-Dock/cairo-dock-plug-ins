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

#include <math.h>
#include <cairo-dock.h>

#include "rendering-struct.h"
#include "rendering-commons.h"
#include "rendering-config.h"
#include "rendering-rainbow.h"

extern int iVanishingPointY;double my_fRainbowColor[4];
double my_fRainbowLineColor[4];

extern double my_fInclinationOnHorizon;
extern double my_fForegroundRatio;
extern double my_iGapOnEllipse;
extern gboolean my_bRotateIconsOnEllipse;
extern double my_fScrollAcceleration;
extern double my_fScrollSpeed;

extern double my_fParabolePower;
extern double my_fParaboleFactor;

extern double my_fParaboleCurvature;
extern double my_fParaboleRatio;
extern double my_fParaboleMagnitude;
extern int my_iParaboleTextGap;
extern gboolean my_bDrawTextWhileUnfolding;
extern gboolean my_bParaboleCurveOutside;

extern cairo_surface_t *my_pFlatSeparatorSurface[2];
extern GLuint my_iFlatSeparatorTexture;

extern int my_iSpaceBetweenRows;
extern int my_iSpaceBetweenIcons;
extern double my_fRainbowMagnitude;
extern int my_iRainbowNbIconsMin;
extern double my_fRainbowConeOffset;
extern double my_fRainbowColor[4];
extern double my_fRainbowLineColor[4];

/*extern gint     my_diapo_iconGapX;
extern gint     my_diapo_iconGapY;
extern gdouble  my_diapo_fScaleMax;
extern gint     my_diapo_sinW;
extern gboolean my_diapo_lineaire;
extern gboolean  my_diapo_wide_grid;
extern gboolean  my_diapo_text_only_on_pointed;

extern gdouble  my_diapo_color_frame_start[4];
extern gdouble  my_diapo_color_frame_stop[4];
extern gboolean my_diapo_fade2bottom;
extern gboolean my_diapo_fade2right;
extern guint    my_diapo_arrowWidth;
extern guint    my_diapo_arrowHeight;
extern gdouble  my_diapo_arrowShift;
extern guint    my_diapo_lineWidth;
extern guint    my_diapo_radius;
extern gdouble  my_diapo_color_border_line[4];
extern gboolean my_diapo_draw_background;
extern gboolean my_diapo_display_all_icons;*/

extern gint     my_diapo_simple_iconGapX;
extern gint     my_diapo_simple_iconGapY;
extern gdouble  my_diapo_simple_fScaleMax;
extern gint     my_diapo_simple_sinW;
extern gboolean my_diapo_simple_lineaire;
extern gboolean  my_diapo_simple_wide_grid;
//extern gboolean  my_diapo_simple_text_only_on_pointed;
extern gdouble  my_diapo_simple_color_frame_start[4];
extern gdouble  my_diapo_simple_color_frame_stop[4];
extern gboolean my_diapo_simple_fade2bottom;
extern gboolean my_diapo_simple_fade2right;
extern guint    my_diapo_simple_arrowWidth;
extern guint    my_diapo_simple_arrowHeight;
extern gdouble  my_diapo_simple_arrowShift;
extern guint    my_diapo_simple_lineWidth;
extern guint    my_diapo_simple_radius;
extern gdouble  my_diapo_simple_color_border_line[4];
extern gboolean my_diapo_simple_draw_background;
extern gboolean my_diapo_simple_display_all_icons;

extern gdouble my_fCurveCurvature;
extern gint my_iCurveAmplitude;

extern CDSpeparatorType my_iDrawSeparator3D;

CD_APPLET_GET_CONFIG_BEGIN
	iVanishingPointY = cairo_dock_get_integer_key_value (pKeyFile, "Inclinated Plane", "vanishing point y", &bFlushConfFileNeeded, 0, NULL, NULL);

	double fInclinationAngle  = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "inclination", &bFlushConfFileNeeded, 35, NULL, NULL);
	my_fInclinationOnHorizon = tan (fInclinationAngle * G_PI / 180.);
	my_iGapOnEllipse = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "gap on ellipse", &bFlushConfFileNeeded, 10, NULL, NULL);
	my_bRotateIconsOnEllipse = ! cairo_dock_get_boolean_key_value (pKeyFile, "Caroussel", "show face", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	my_fForegroundRatio = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "foreground ratio", &bFlushConfFileNeeded, .5, NULL, NULL);
	my_fScrollSpeed = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "scroll speed", &bFlushConfFileNeeded, 10., NULL, NULL);
	my_fScrollAcceleration = cairo_dock_get_double_key_value (pKeyFile, "Caroussel", "scroll accel", &bFlushConfFileNeeded, .9, NULL, NULL);
	
	my_fParaboleCurvature = sqrt (cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "curvature", &bFlushConfFileNeeded, .7, NULL, NULL));  // c'est mieux proche de 1.
	my_fParaboleRatio = cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "ratio", &bFlushConfFileNeeded, 5, NULL, NULL);
	my_fParaboleMagnitude = cairo_dock_get_double_key_value (pKeyFile, "Parabolic", "wave magnitude", &bFlushConfFileNeeded, .2, NULL, NULL);
	my_iParaboleTextGap = cairo_dock_get_integer_key_value (pKeyFile, "Parabolic", "text gap", &bFlushConfFileNeeded, 3, NULL, NULL);
	my_bDrawTextWhileUnfolding = cairo_dock_get_boolean_key_value (pKeyFile, "Parabolic", "draw text", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_bParaboleCurveOutside = cairo_dock_get_boolean_key_value (pKeyFile, "Parabolic", "curve outside", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	
	
	my_iSpaceBetweenRows = cairo_dock_get_integer_key_value (pKeyFile, "Rainbow", "space between rows", &bFlushConfFileNeeded, 10, NULL, NULL);
	my_iSpaceBetweenIcons = cairo_dock_get_integer_key_value (pKeyFile, "Rainbow", "space between icons", &bFlushConfFileNeeded, 8, NULL, NULL);
	my_fRainbowMagnitude = cairo_dock_get_double_key_value (pKeyFile, "Rainbow", "wave magnitude", &bFlushConfFileNeeded, .3, NULL, NULL);
	my_iRainbowNbIconsMin = cairo_dock_get_integer_key_value (pKeyFile, "Rainbow", "nb icons min", &bFlushConfFileNeeded, 3, NULL, NULL);
	my_fRainbowConeOffset = G_PI * (1 - cairo_dock_get_double_key_value (pKeyFile, "Rainbow", "cone", &bFlushConfFileNeeded, 130, NULL, NULL) / 180) / 2;
	if (my_fRainbowConeOffset < 0) my_fRainbowConeOffset = 0;
	if (my_fRainbowConeOffset > G_PI/2) my_fRainbowConeOffset = G_PI/2;
	double bow_couleur[4] = {0.7,0.9,1.0,0.5};
	cairo_dock_get_double_list_key_value (pKeyFile, "Rainbow", "bow color", &bFlushConfFileNeeded, my_fRainbowColor, 4, bow_couleur, NULL, NULL);
	double line_couleur[4] = {0.5,1.0,0.9,0.6};
	cairo_dock_get_double_list_key_value (pKeyFile, "Rainbow", "line color", &bFlushConfFileNeeded, my_fRainbowLineColor, 4, line_couleur, NULL, NULL);
	
	
	/*my_diapo_iconGapX             = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "iconGapX",             &bFlushConfFileNeeded,     5, NULL, NULL);
	my_diapo_iconGapY             = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "iconGapY",             &bFlushConfFileNeeded,    10, NULL, NULL);
	my_diapo_fScaleMax            = cairo_dock_get_double_key_value  (pKeyFile, "Slide", "fScaleMax",            &bFlushConfFileNeeded,    2., NULL, NULL);
	my_diapo_sinW                 = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "sinW",                 &bFlushConfFileNeeded,   300, NULL, NULL);
	my_diapo_lineaire             = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "lineaire",             &bFlushConfFileNeeded, FALSE, NULL, NULL);
	my_diapo_wide_grid            = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "wide_grid",            &bFlushConfFileNeeded, FALSE, NULL, NULL);        
	my_diapo_text_only_on_pointed = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "text_only_on_pointed", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	gdouble color_frame_start[4] = {0.0, 0.0, 0.0, 1.0};
	cairo_dock_get_double_list_key_value (pKeyFile, "Slide", "color_frame_start", &bFlushConfFileNeeded, my_diapo_color_frame_start, 4, color_frame_start, NULL, NULL);
	gdouble color_frame_stop[4]  = {0.3, 0.3, 0.3, 0.6};
	cairo_dock_get_double_list_key_value (pKeyFile, "Slide", "color_frame_stop", &bFlushConfFileNeeded, my_diapo_color_frame_stop, 4, color_frame_stop, NULL, NULL);
	gdouble color_border_line[4] = {1., 1., 1., 0.5};
	cairo_dock_get_double_list_key_value (pKeyFile, "Slide", "color_border_line", &bFlushConfFileNeeded, my_diapo_color_border_line, 4, color_border_line, NULL, NULL);
	
	my_diapo_fade2bottom = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "fade2bottom", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_diapo_fade2right  = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "fade2right",  &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_diapo_arrowWidth  = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "arrowWidth",  &bFlushConfFileNeeded, 40,   NULL, NULL);
	my_diapo_arrowHeight = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "arrowHeight", &bFlushConfFileNeeded, 40,   NULL, NULL);
	my_diapo_arrowShift  = cairo_dock_get_double_key_value  (pKeyFile, "Slide", "arrowShift",  &bFlushConfFileNeeded, 30,   NULL, NULL) / 100;
	my_diapo_lineWidth   = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "lineWidth",   &bFlushConfFileNeeded, 5,    NULL, NULL);
	my_diapo_radius      = cairo_dock_get_integer_key_value (pKeyFile, "Slide", "radius",      &bFlushConfFileNeeded, 15,   NULL, NULL);
	my_diapo_draw_background = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "draw_background",  &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_diapo_display_all_icons = cairo_dock_get_boolean_key_value (pKeyFile, "Slide", "display_all_icons",  &bFlushConfFileNeeded, FALSE, NULL, NULL);*/
	
	
	my_diapo_simple_iconGapX             = MAX (30, cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_iconGapX",             &bFlushConfFileNeeded,    50, NULL, NULL));
	my_diapo_simple_iconGapY             = MAX (30, cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_iconGapY",             &bFlushConfFileNeeded,    50, NULL, NULL));
	my_diapo_simple_fScaleMax            = cairo_dock_get_double_key_value  (pKeyFile, "SimpleSlide", "simple_fScaleMax",            &bFlushConfFileNeeded,   2.0, NULL, NULL);
	my_diapo_simple_sinW                 = cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_sinW",                 &bFlushConfFileNeeded,   200, NULL, NULL);
	my_diapo_simple_lineaire             = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_lineaire",             &bFlushConfFileNeeded, FALSE, NULL, NULL);
	my_diapo_simple_wide_grid            = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_wide_grid",            &bFlushConfFileNeeded, FALSE, NULL, NULL);
	//my_diapo_simple_text_only_on_pointed = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_text_only_on_pointed", &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	gdouble color_frame_start_[4] = {0.0, 0.0, 0.0, 1.0};
	cairo_dock_get_double_list_key_value (pKeyFile, "SimpleSlide", "simple_color_frame_start", &bFlushConfFileNeeded, my_diapo_simple_color_frame_start, 4, color_frame_start_, NULL, NULL);
	gdouble color_frame_stop_[4]  = {0.3, 0.3, 0.3, 0.6};
	cairo_dock_get_double_list_key_value (pKeyFile, "SimpleSlide", "simple_color_frame_stop", &bFlushConfFileNeeded, my_diapo_simple_color_frame_stop, 4, color_frame_stop_, NULL, NULL);
	gdouble color_border_line_[4] = {1., 1., 1., 0.5};
	cairo_dock_get_double_list_key_value (pKeyFile, "SimpleSlide", "simple_color_border_line", &bFlushConfFileNeeded, my_diapo_simple_color_border_line, 4, color_border_line_, NULL, NULL);
	
	my_diapo_simple_fade2bottom = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_fade2bottom", &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_diapo_simple_fade2right  = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_fade2right",  &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_diapo_simple_arrowWidth  = cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_arrowWidth",  &bFlushConfFileNeeded, 40,   NULL, NULL);
	my_diapo_simple_arrowHeight = cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_arrowHeight", &bFlushConfFileNeeded, 40,   NULL, NULL);
	my_diapo_simple_arrowShift  = cairo_dock_get_double_key_value (pKeyFile, "SimpleSlide", "simple_arrowShift",  &bFlushConfFileNeeded, 30,   NULL, NULL) / 100.;
	my_diapo_simple_lineWidth   = cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_lineWidth",   &bFlushConfFileNeeded, 5,    NULL, NULL);
	my_diapo_simple_radius      = cairo_dock_get_integer_key_value (pKeyFile, "SimpleSlide", "simple_radius",      &bFlushConfFileNeeded, 15,   NULL, NULL);
	my_diapo_simple_draw_background = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_draw_background",  &bFlushConfFileNeeded, TRUE, NULL, NULL);
	my_diapo_simple_display_all_icons = cairo_dock_get_boolean_key_value (pKeyFile, "SimpleSlide", "simple_display_all_icons",  &bFlushConfFileNeeded, FALSE, NULL, NULL);
	
	
	my_fCurveCurvature = (double) cairo_dock_get_integer_key_value (pKeyFile, "Curve", "curvature", &bFlushConfFileNeeded, 50, NULL, NULL) / 100.;
	my_iCurveAmplitude = cairo_dock_get_integer_key_value (pKeyFile, "Curve", "amplitude", &bFlushConfFileNeeded, 20, NULL, NULL);
	
	if (g_key_file_has_group (pKeyFile, "Slide"))
	{
		g_key_file_remove_group (pKeyFile, "Slide", NULL);
		bFlushConfFileNeeded = TRUE;
	}
	
	cd_rendering_reload_rainbow_buffers ();
CD_APPLET_GET_CONFIG_END


CD_APPLET_RESET_CONFIG_BEGIN
	
CD_APPLET_RESET_CONFIG_END


CD_APPLET_RESET_DATA_BEGIN
	if (my_pFlatSeparatorSurface[0] != NULL)
	{
		cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
		my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = NULL;
		cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
		my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = NULL;
	}
	
	if (my_iFlatSeparatorTexture != 0)
	{
		glDeleteTextures (1, &my_iFlatSeparatorTexture);
		my_iFlatSeparatorTexture = 0;
	}
	my_iDrawSeparator3D = 0;
CD_APPLET_RESET_DATA_END

