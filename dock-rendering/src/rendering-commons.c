/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>
#include "rendering-commons.h"

extern cairo_surface_t *my_pFlatSeparatorSurface[2];
extern double my_fSeparatorColor[4];
extern GLuint my_iFlatSeparatorTexture;

cairo_surface_t *cd_rendering_create_flat_separator_surface (cairo_t *pSourceContext, int iWidth, int iHeight)
{
	cairo_pattern_t *pStripesPattern = cairo_pattern_create_linear (0.0f,
		iHeight,
		0.,
		0.);
	g_return_val_if_fail (cairo_pattern_status (pStripesPattern) == CAIRO_STATUS_SUCCESS, NULL);
	
	cairo_pattern_set_extend (pStripesPattern, CAIRO_EXTEND_REPEAT);
	
	double fStep = 1.;
	double y = 0, h0 = (fStep * (1 + sqrt (1 + 4. * iHeight / fStep)) / 2) - 1, hk = h0;
	int k = 0;
	for (k = 0; k < h0 / fStep; k ++)
	{
		//g_print ("step : %f ; y = %.2f\n", 1.*hk / iHeight, y);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			0.,
			0.,
			0.,
			0.);
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			my_fSeparatorColor[0],
			my_fSeparatorColor[1],
			my_fSeparatorColor[2],
			my_fSeparatorColor[3]);
		y += 1.*hk / iHeight;
		cairo_pattern_add_color_stop_rgba (pStripesPattern,
			y,
			my_fSeparatorColor[0],
			my_fSeparatorColor[1],
			my_fSeparatorColor[2],
			my_fSeparatorColor[3]);
		hk -= fStep;
	}
	
	cairo_surface_t *pNewSurface = _cairo_dock_create_blank_surface (pSourceContext,
		iWidth,
		iHeight);
	cairo_t *pImageContext = cairo_create (pNewSurface);
	cairo_set_source (pImageContext, pStripesPattern);
	cairo_paint (pImageContext);
	
	cairo_pattern_destroy (pStripesPattern);
	cairo_destroy (pImageContext);
	
	return pNewSurface;
}

void cd_rendering_load_flat_separator (CairoContainer *pContainer)
{
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
	cairo_surface_destroy (my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL]);
	
	cairo_t *pSourceContext = cairo_dock_create_context_from_window (pContainer);
	my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL] = cd_rendering_create_flat_separator_surface (pSourceContext, 300, 150);
	my_pFlatSeparatorSurface[CAIRO_DOCK_VERTICAL] = cairo_dock_rotate_surface (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL], pSourceContext, 300, 150, -G_PI / 2);
	cairo_destroy (pSourceContext);
	
	if (g_bUseOpenGL)
		my_iFlatSeparatorTexture = cairo_dock_create_texture_from_surface (my_pFlatSeparatorSurface[CAIRO_DOCK_HORIZONTAL]);
}



double cd_rendering_interpol (double x, double *fXValues, double *fYValues)
{
	double y;
	int i, i_inf=0, i_sup=RENDERING_INTERPOLATION_NB_PTS-1;
	do
	{
		i = (i_inf + i_sup) / 2;
		if (fXValues[i] < x)
			i_inf = i;
		else
			i_sup = i;
	}
	while (i_sup - i_inf > 1);
	
	double x_inf = fXValues[i_inf];
	double x_sup = fXValues[i_sup];
	return (x_sup != x_inf ? ((x - x_inf) * fYValues[i_sup] + (x_sup - x) * fYValues[i_inf]) / (x_sup - x_inf) : fYValues[i_inf]);
}
