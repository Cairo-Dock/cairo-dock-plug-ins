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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <cairo.h>

#include "rendering-commons.h"
#include "rendering-rainbow.h"

extern int my_iSpaceBetweenRows;
extern int my_iSpaceBetweenIcons;
extern double my_fRainbowMagnitude;
extern int my_iRainbowNbIconsMin;
extern double my_fRainbowConeOffset;
extern double my_fRainbowColor[4];
extern double my_fRainbowLineColor[4];

static double *pCosSinTab = NULL;
static GLfloat *pVertexTab = NULL;
static GLfloat* pColorTab = NULL;

typedef struct _CDRainbowData
{
	gboolean bUseDrawCoords;
	int iLastRow;
} CDRainbowData;

/* Allocate or get CDRainbowData for a dock. */
static CDRainbowData *_get_rainbow_data (CairoDock *pDock)
{
	CDRainbowData *pData = NULL;
	if (pDock->pRendererData) pData = (CDRainbowData*)pDock->pRendererData;
	else
	{
		pData = g_new (CDRainbowData, 1);
		pDock->pRendererData = pData;
		pData->bUseDrawCoords = FALSE;
		pData->iLastRow = -1;
	}
	
	return pData;
}

static void _free_rainbow_data (CairoDock *pDock)
{
	if (pDock->pRendererData)
	{
		g_free (pDock->pRendererData);
		pDock->pRendererData = NULL;
	}
}

static void cd_rendering_calculate_max_dock_size_rainbow (CairoDock *pDock)
{
	pDock->fMagnitudeMax = my_fRainbowMagnitude;
	cairo_dock_calculate_icons_positions_at_rest_linear (pDock);
	
	double fMaxScale =  1. + my_fRainbowMagnitude * myIconsParam.fAmplitude;
	int iMaxIconWidth = pDock->iMaxIconHeight + my_iSpaceBetweenIcons;
	double fCone = G_PI - 2 * my_fRainbowConeOffset;
	int iNbIcons = g_list_length (pDock->icons);
	int iMinRadius = MIN (my_iRainbowNbIconsMin, iNbIcons) * iMaxIconWidth * fMaxScale / fCone;
	
	int iNbRows = 0;
	int iNbTotal = 0;
	for (; iNbTotal < iNbIcons; iNbRows++)
	{
		double fNormalRadius = iMinRadius + iNbRows * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale;
		double fDeltaTheta = 2 * atan (iMaxIconWidth * fMaxScale / 2 / fNormalRadius);
		int iNbIconsOnRow = MAX ((int) (fCone / fDeltaTheta), 1); // ensure that we do not get into an infinite loop
		iNbTotal += iNbIconsOnRow;
	}
	
	pDock->iMaxDockHeight = iNbRows * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale + iMinRadius + 2 * myIconsParam.iLabelSize;
	pDock->iMaxDockWidth = 2 * (pDock->iMaxDockHeight * cos (my_fRainbowConeOffset));
	cd_debug ("iNbRows : %d => %dx%d (iMaxIconHeight = %d ; iMinRadius = %d ; fMaxScale = %.2f)", iNbRows, pDock->iMaxDockWidth, pDock->iMaxDockHeight, (int)pDock->iMaxIconHeight, iMinRadius, fMaxScale);
	
	pDock->iDecorationsWidth = 0;
	pDock->iDecorationsHeight = 0;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth;
	pDock->iMinDockHeight = pDock->iMaxIconHeight;
	
	pDock->iActiveWidth = pDock->iMaxDockWidth;
	pDock->iActiveHeight = pDock->iMaxDockHeight;
	
	
	//\____________________ We calculate the minimize position of all icons when the mouse is outside the dock
	// These are used by default until we have updated them in cd_rendering_calculate_icons_rainbow ()
	if (!pDock->icons) return;
	
	Icon* icon;
	GList* ic;
	
	GList *pFirstDrawnElement = pDock->icons;
	ic = pFirstDrawnElement;
	
	int iNbRow = -1, iNbIconsOnRow = 0, iNbInsertedIcons = 0;
	double fCurrentRadius=0, fThetaStart=0, fDeltaTheta=0, fCurrentScale=1;
	double t = 0;
	do
	{
		icon = ic->data;
		
		if (iNbInsertedIcons == iNbIconsOnRow)
		{
			iNbRow ++;
			if (iNbRow >= iNbRows)
				break ;
			iNbInsertedIcons = 0;
			t = iNbRow * (pDock->iMaxIconHeight + my_iSpaceBetweenRows);
			fCurrentRadius = iMinRadius + t;
			fCurrentScale = 1.0;
			
			double fNormalRadius = iMinRadius + iNbRow * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale;
			fDeltaTheta = 2 * atan (iMaxIconWidth * fMaxScale / 2 / fNormalRadius);
			iNbIconsOnRow = (int) (fCone / fDeltaTheta);
			fThetaStart = - G_PI/2 + my_fRainbowConeOffset + (fCone - iNbIconsOnRow * fDeltaTheta) / 2 + fDeltaTheta / 2;
		}
		
		double x0 = fCurrentRadius + (pDock->container.bDirectionUp ? pDock->iMaxIconHeight * fCurrentScale : 0);
		double fCurrentTheta = fThetaStart + iNbInsertedIcons * fDeltaTheta;
		icon->fYAtRest = x0 * cos (fCurrentTheta) + icon->fWidth/2 * fCurrentScale * sin (fCurrentTheta);
		if (pDock->container.bDirectionUp)
			icon->fYAtRest = pDock->iMaxDockHeight - icon->fYAtRest;
		icon->fXAtRest = x0 * sin (fCurrentTheta) - icon->fWidth/2 * fCurrentScale * cos (fCurrentTheta);
		
		iNbInsertedIcons ++;
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	
	CDRainbowData *pData = _get_rainbow_data (pDock);
	pData->bUseDrawCoords = FALSE;
}

static void cd_rendering_get_minimize_pos_rainbow (Icon *icon, CairoDock *pDock, double *pX, double *pY)
{
	if (pDock->pRendererData)
	{
		CDRainbowData *pData = (CDRainbowData*)pDock->pRendererData;
		if (pData->bUseDrawCoords)
		{
			*pY = icon->fDrawY + 0.5 * icon->fHeight; // adjust position a bit since it is inexact anyway
			*pX = icon->fDrawX;
			return;
		}
	}
	
	*pY = icon->fYAtRest;
	*pX = icon->fXAtRest + pDock->container.iWidth / 2;
}

static void cd_rendering_render_rainbow (cairo_t *pCairoContext, CairoDock *pDock)
{
	//g_print ("pDock->fFoldingFactor : %.2f\n", pDock->fFoldingFactor);
	double fMaxScale =  1. + my_fRainbowMagnitude * myIconsParam.fAmplitude;
	double fRadius=0;
	if (my_fRainbowColor[3] != 0 && pDock->icons != NULL)
	{
		cairo_save (pCairoContext);
		if (! pDock->container.bIsHorizontal)
		{
			cairo_translate (pCairoContext, pDock->container.iHeight/2, pDock->container.iWidth/2);
			cairo_rotate (pCairoContext, -G_PI/2);
			cairo_translate (pCairoContext, -pDock->container.iWidth/2, -pDock->container.iHeight/2);
		}
		if (!pDock->container.bDirectionUp)
		{
			cairo_translate (pCairoContext, 0., pDock->container.iHeight);
			cairo_scale (pCairoContext, 1., -1.);
		}
		//\____________________ We do a clip of the cone.
		cairo_move_to (pCairoContext, 0., pDock->container.iHeight * (1 - sin (my_fRainbowConeOffset)));
		cairo_line_to (pCairoContext, pDock->container.iWidth/2, pDock->container.iHeight);
		cairo_line_to (pCairoContext, pDock->container.iWidth, pDock->container.iHeight * (1 - sin (my_fRainbowConeOffset)));
		cairo_line_to (pCairoContext, pDock->container.iWidth, 0.);
		cairo_line_to (pCairoContext, 0., 0.);
		cairo_close_path (pCairoContext);
		cairo_clip (pCairoContext);
		
		//\____________________ We draw each scratch in it.
		cairo_pattern_t *pGradationPattern = cairo_pattern_create_radial (pDock->container.iWidth/2,
			pDock->container.iHeight,
			0.,
			pDock->container.iWidth/2,
			pDock->container.iHeight,
			pDock->container.iHeight);
		g_return_if_fail (cairo_pattern_status (pGradationPattern) == CAIRO_STATUS_SUCCESS);
		
		cairo_pattern_set_extend (pGradationPattern, CAIRO_EXTEND_NONE);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			0.,
			0.,
			0.,
			0.,
			0.);
		
		GList *pFirstDrawnElement = pDock->icons;
		GList *ic = pFirstDrawnElement;
		double fCurrentRadius=0;
		Icon *icon;
		do
		{
			icon = ic->data;
			fRadius = icon->fX - (pDock->container.bDirectionUp ? pDock->iMaxIconHeight * fMaxScale : 0);
			if (fRadius != fCurrentRadius)
			{
				if (fCurrentRadius == 0)  // First hit.
				{
					cairo_pattern_add_color_stop_rgba (pGradationPattern,
						(fRadius - my_iSpaceBetweenRows/2)/ pDock->container.iHeight,
						0.,
						0.,
						0.,
						0.);
				}
				
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					(fRadius + .5 * pDock->iMaxIconHeight * fMaxScale)/ pDock->container.iHeight,
					my_fRainbowColor[0],
					my_fRainbowColor[1],
					my_fRainbowColor[2],
					my_fRainbowColor[3]);
				cairo_pattern_add_color_stop_rgba (pGradationPattern,
					(fRadius + pDock->iMaxIconHeight * fMaxScale + my_iSpaceBetweenRows/2)/ pDock->container.iHeight,
					0.,
					0.,
					0.,
					0.);
				fCurrentRadius = fRadius;
			}
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		cairo_set_source (pCairoContext, pGradationPattern);
		cairo_paint (pCairoContext);
		cairo_pattern_destroy (pGradationPattern);
		cairo_restore (pCairoContext);
	}
	
	//\____________________ We draw the frame/border.
	if (fRadius == 0)
	{
		Icon *icon = cairo_dock_get_last_icon (pDock->icons);
		if (icon)
			fRadius = icon->fX - (pDock->container.bDirectionUp ? pDock->iMaxIconHeight * fMaxScale : 0);
	}
	fRadius += .5 * pDock->iMaxIconHeight * fMaxScale;
	if (my_fRainbowLineColor[3] != 0)
	{
		cairo_save (pCairoContext);
		if (! pDock->container.bIsHorizontal)
		{
			cairo_translate (pCairoContext, pDock->container.iHeight/2, pDock->container.iWidth/2);
			cairo_rotate (pCairoContext, -G_PI/2);
			cairo_translate (pCairoContext, -pDock->container.iWidth/2, -pDock->container.iHeight/2);
		}
		if (!pDock->container.bDirectionUp)
		{
			cairo_translate (pCairoContext, 0., pDock->container.iHeight);
			cairo_scale (pCairoContext, 1., -1.);
		}
		cairo_set_line_width (pCairoContext, _get_dock_linewidth());
		cairo_move_to (pCairoContext, pDock->container.iWidth/2 - fRadius * cos (my_fRainbowConeOffset), pDock->container.iHeight - fRadius * sin (my_fRainbowConeOffset));
		cairo_line_to (pCairoContext, pDock->container.iWidth/2, pDock->container.iHeight);
		cairo_line_to (pCairoContext, pDock->container.iWidth/2 + fRadius * cos (my_fRainbowConeOffset), pDock->container.iHeight - fRadius * sin (my_fRainbowConeOffset));
		cairo_set_source_rgba (pCairoContext,
			my_fRainbowLineColor[0],
			my_fRainbowLineColor[1],
			my_fRainbowLineColor[2],
			my_fRainbowLineColor[3]);
		cairo_stroke (pCairoContext);
		cairo_restore (pCairoContext);
	}
	
	//\____________________ We draw the icons with their labels.
	///cairo_dock_render_icons_linear (pCairoContext, pDock, fRatio);
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;

	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex)/** * pDock->fMagnitudeMax*/;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;

		if (!CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			cairo_save (pCairoContext);
			cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
			cairo_restore (pCairoContext);
		}

		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}


static void cd_rendering_get_polar_coords (CairoDock *pDock, double *fRadius, double *fTheta)
{
	double x = pDock->container.iMouseX - pDock->container.iWidth / 2, y = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->container.iMouseY : pDock->container.iMouseY);
	
	*fRadius = sqrt (x * x + y * y);
	*fTheta = atan2 (x, y);
}

/**
 * Functions to scale the rainbow's radius so that the currently pointed row is zoomed.
 * We do this by assuming a position-dependent scaling factor to the radius. In the following,
 * "t" is a curve parameter that corresponds to unscaled coordinates, and "x" is the real
 * radius. The scaling factor is dependent on t:
 *    s(t) = | 1                             if t < t0 - W/2 or t > t0 + W/2
 *           | A * cos (pi * (t - t0) / W)   if t \in [t0 - W/2; t0 + W/2]
 * where t0 is a parameter that corresponds to the "center point", i.e. the mouse position
 * in unscaled coordinates, while W is the width of the scaling effect and A is its amplitude.
 * These are calculated as:
 *    W = myIconsParam.iSinusoidWidth * pDock->container.fRatio;
 *    A = myIconsParam.fAmplitude * my_fRainbowMagnitude * cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
 * Below, the function _get_current_scale () calculates s(t) for a set of parameters (t0 and dock parameters).
 * 
 * We convert to real radius coordinates by integrating s along t:
 *    x(t) = \int_0^t s(z) d z
 * The result depends on whether t0 is larger than W/2.
 * If t0 > W/2, we have:
 *    x(t) = |  t                                                                   if t <  t0 - W/2
 *           |  t + A * W * ( 1 + sin (pi * (t - t0 / W) ) ) / pi                   if t \in [t0 - W/2; t0 + W/2]
 *           |  t + 2 * A * W / pi                                                  if t >  t0 + W/2
 * If t0 < W/2, then:
 *    x(t) = | t + A * W * ( sin (pi * t0 / W) + sin (pi * (t - t0 / W) ) ) / pi    if t <  t0 + W/2
 *           | t + A * W * ( sin (pi * t0 / W) + 1 ) / pi                           if t >= t0 + W/2
 * Below, the function _get_current_radius () calculates x(t) for a set of parameters (t0 and dock parameters).
 * 
 * To calculte t0, we need to invert the above function. If x0 is the current mouse position (in polar
 * coordinates, i.e. the current radius), we can do the following:
 *   If x0 > W / 2 + A * W / pi  =>  t0 = x0 - A * W / pi
 *   Otherwise, we need to solve the following equation:
 *      t0 + A * W * sin (pi * t0 / W) / pi - x0 = 0
 *   We know that:
 *      t0 \in [ MAX(0, x0 - A * W / pi); x0 ]
 *      and also that the left hand side of the above equation is a monotonously
 *      increasing function on this interval, meaning that there can be only one
 *      zero crossing
 *   This means that we can do a binary search starting from the two endpoints until
 *   we find a candidate t0 where the left hand side is close enough to zero (we do
 *   not need to be very precise).
 * Below, the _calculate_t0 () function does this for a given x0 and dock parameters.
 * This is called whenever the mouse position changes and the resulting t0 is used
 * for later calculations.
 * 
 * Additional complications:
 * We scale coordinates by (1.0 - pDock->fFoldingFactor) to take into account the
 * dock opening / closing animation.
 * We start our coordinate system not from the half-circle center, but from the radius
 * that corresponds to the first icon position (fMinIconRadius below). This is done
 * so that the radius of the first ring of icons never changes. This means that:
 *   -- for x0 >= fMinIconRadius, we simply subtract this value (and later add back as necessary)
 *   -- for x0 <  fMinIconRadius, we calculate an "artificial" t0, so that we have zero scaling
 *      (achieved by setting t0 == -W / 2) when x0 == 0 and we have t0 == 0 if x0 == fMinIconRadius
 */
static double _get_current_scale (CairoDock *pDock, double t, double t0)
{
	double fRatio = pDock->container.fRatio;
	double W = myIconsParam.iSinusoidWidth * fRatio;
	double scale = 1.0;
	if (t > t0 - 0.5 * W && t < t0 + 0.5 * W)
	{
		double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double A = myIconsParam.fAmplitude * my_fRainbowMagnitude * fMagnitude;
		scale += A * cos (G_PI * (t - t0) / W);
	}
	return scale * (1.0 - pDock->fFoldingFactor);
}

static double _get_current_radius (CairoDock *pDock, double t, double t0)
{
	double fRatio = pDock->container.fRatio;
	double W = myIconsParam.iSinusoidWidth * fRatio;
	double x = t;
	if (t > t0 - 0.5 * W)
	{
		double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
		double A = myIconsParam.fAmplitude * my_fRainbowMagnitude * fMagnitude;
		
		if (t0 < 0.5 * W) x += A * W * sin (G_PI * t0 / W) / G_PI;
		else x += A * W / G_PI;
		
		if (t < t0 + 0.5 * W) x += A * W * sin (G_PI * (t - t0) / W) / G_PI;
		else x += A * W / G_PI;
	}
	
	return x * (1.0 - pDock->fFoldingFactor);
}

static double _calculate_t0 (CairoDock *pDock, double x)
{
	double fRatio = pDock->container.fRatio;
	double W = myIconsParam.iSinusoidWidth * fRatio;
	if (pDock->fFoldingFactor > 0.999) return -0.5 * W; // dock is almost hidden, avoid any scaling
	
	if (x <= 0.0) return 0.0;
	
	x /= (1.0 - pDock->fFoldingFactor);
	
	double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex);
	double A = myIconsParam.fAmplitude * my_fRainbowMagnitude * fMagnitude;
	
	double t0 = x - A * W / G_PI;
	if (t0 >= 0.5 * W) return t0;
	
	// we need to solve the following equation for t0:
	// t0 + A*W*sin(G_PI*t0/W)/G_PI - x = 0
	// we know that: t0 < x and t0 > 0 and t0 > x - A * W / G_PI
	if (t0 < 0.0) t0 = 0.0;
	double delta = 0.5 * (x - t0);
	int nb_iter = 0;
	
	do
	{
		t0 += delta;
		double res = t0 + A * W * sin (G_PI * t0 / W) / G_PI - x;
		if (res < -1e-6) delta = 0.5 * fabs (delta);
		else if (res > 1e-6) delta = -0.5 * fabs (delta);
		else break; // already close enough
		nb_iter++;
	} while (nb_iter < 15);
	
	return t0;
}

static Icon *cd_rendering_calculate_icons_rainbow (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return NULL;
	
	//\____________________ We are using polar coordinates.
	double fMaxScale =  1. + my_fRainbowMagnitude * myIconsParam.fAmplitude;
	int iMaxIconWidth = pDock->iMaxIconHeight + my_iSpaceBetweenIcons;
	double fCone = G_PI - 2 * my_fRainbowConeOffset;
	int iNbIcons = g_list_length (pDock->icons);
	int iMinRadius = MIN (my_iRainbowNbIconsMin, iNbIcons) * iMaxIconWidth * fMaxScale / fCone;
	double fRadius, fTheta;
	cd_debug (" mouse : (%d ; %d)", pDock->container.iMouseX, pDock->container.iMouseY);
	cd_rendering_get_polar_coords (pDock, &fRadius, &fTheta);
	cd_debug (" polar : (%.2f ; %.2f)", fRadius, fTheta/G_PI*180.);
	
	double t0;
	const double W = myIconsParam.iSinusoidWidth * pDock->container.fRatio;
	double fMinIconRadius = iMinRadius + 0.5 * pDock->iMaxIconHeight;
	if (fRadius > fMinIconRadius) t0 = _calculate_t0 (pDock, fRadius - fMinIconRadius);
	else t0 = 0.5 * W * (fRadius / fMinIconRadius - 1.0);
	
	int iNbRows = round ((pDock->iMaxDockHeight - iMinRadius) / ((pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale));
	cd_debug ("iNbRows : %d, maxHeight: %d", iNbRows, pDock->iMaxDockHeight);
	
	//\____________________ We deduce the positions/stretching/alpha of all icons.
	Icon *pPointedIcon = NULL;
	Icon* icon;
	GList* ic;
	
	GList *pFirstDrawnElement = pDock->icons;
	ic = pFirstDrawnElement;
	
	int iNbRow = -1, iNbIconsOnRow = 0, iNbInsertedIcons = 0, iPointedRow = -1;
	double fCurrentRadius=0, fNormalRadius=0, fCurrentTheta, fThetaStart=0, fDeltaTheta=0, fCurrentScale=1;
	double t = 0;
	do
	{
		icon = ic->data;
		
		if (!CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			if (iNbInsertedIcons == iNbIconsOnRow)
			{
				iNbRow ++;
				if (iNbRow >= iNbRows)
					break ;
				iNbInsertedIcons = 0;
				t = iNbRow * (pDock->iMaxIconHeight + my_iSpaceBetweenRows);
				fCurrentRadius = iMinRadius + _get_current_radius (pDock, t, t0);
				fCurrentScale = _get_current_scale (pDock, t, t0);
				
				fNormalRadius = iMinRadius + iNbRow * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale;
				fDeltaTheta = 2 * atan (iMaxIconWidth * fMaxScale / 2 / fNormalRadius);
				iNbIconsOnRow = (int) (fCone / fDeltaTheta);
				fThetaStart = - G_PI/2 + my_fRainbowConeOffset + (fCone - iNbIconsOnRow * fDeltaTheta) / 2 + fDeltaTheta / 2;
				cd_debug ("We go to the line %d (%d icons, fThetaStart = %.2fdeg, fCurrentRadius = %.2f(%.2f), fDeltaTheta = %.2f, fCurrentScale = %.2f)", iNbRow, iNbIconsOnRow, fThetaStart/G_PI*180, fCurrentRadius, fNormalRadius, fDeltaTheta/G_PI*180, fCurrentScale);
			}
			
			icon->fX = fCurrentRadius + (pDock->container.bDirectionUp ? pDock->iMaxIconHeight * fCurrentScale : 0);
			
			fCurrentTheta = fThetaStart + iNbInsertedIcons * fDeltaTheta;
			icon->fOrientation = (pDock->container.bDirectionUp ? fCurrentTheta : - fCurrentTheta);
			if (! pDock->container.bIsHorizontal)
			{
				icon->fOrientation = -icon->fOrientation + 0;
			}
			if (pPointedIcon == NULL && fRadius < fCurrentRadius + (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fCurrentScale && fRadius > fCurrentRadius && fTheta > fCurrentTheta - fDeltaTheta/2 && fTheta < fCurrentTheta + fDeltaTheta/2)
			{
				icon->bPointed = TRUE;
				pPointedIcon = icon;
				iPointedRow = iNbRow;
				cd_debug (" POINTED ICON : %s", pPointedIcon->cName);
			}
			else
				icon->bPointed = FALSE;
			
			//if (pDock->container.bIsHorizontal)
			{
				icon->fDrawY = icon->fX * cos (fCurrentTheta) + icon->fWidth/2 * fCurrentScale * sin (fCurrentTheta);
				if (pDock->container.bDirectionUp)
					icon->fDrawY = pDock->container.iHeight - icon->fDrawY;
				icon->fDrawX = icon->fX * sin (fCurrentTheta) - icon->fWidth/2 * fCurrentScale * cos (fCurrentTheta) + pDock->container.iWidth / 2;
			}
			/*else
			{
				icon->fDrawX = icon->fX * cos (fCurrentTheta) + icon->fWidth/2 * fCurrentScale * sin (fCurrentTheta);
				if (!pDock->container.bDirectionUp)
					icon->fDrawX = pDock->container.iWidth - icon->fDrawX;
				icon->fDrawY = icon->fX * sin (fCurrentTheta) - icon->fWidth/2 * fCurrentScale * cos (fCurrentTheta) + pDock->container.iHeight / 2;
			}*/
			
			cd_debug (" %.2fdeg ; (%.2f;%.2f)", fCurrentTheta/G_PI*180, icon->fDrawX, icon->fDrawY);
			
			icon->fScale = fCurrentScale;
			icon->fAlpha = 1;
			icon->fWidthFactor = 1.;
			icon->fHeightFactor = 1.;
			
			iNbInsertedIcons ++;
		}
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	
	if (! pDock->container.bInside ||
		t0 > (iNbRows + 0.2) * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) ||
		((fTheta < - G_PI/2 + my_fRainbowConeOffset - G_PI / 18 || fTheta > G_PI/2 - my_fRainbowConeOffset + G_PI / 18) &&
			fRadius > iMinRadius + .5 * pDock->iMaxIconHeight * fMaxScale))
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
		pDock->bCanDrop = FALSE;
	}
	else
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
		if (pDock->bIsDragging)
			pDock->bCanDrop = TRUE;
	}
	
	// slight adjustment as there is a lot of change when approaching the origin
	if (iPointedRow == -1 && t0 < -0.25 * W) iPointedRow = -2;
	CDRainbowData *pData = _get_rainbow_data (pDock);
	if ( (! pData->bUseDrawCoords && iPointedRow >= 0) ||
		(pData->bUseDrawCoords && pData->iLastRow != iPointedRow))
	{
		// We trigger resetting icon minimize geometries if the row where the mouse points
		// to changed, or when the mouse first enters the subdock. This is necessary as
		// icons can move quite a lot if the amplitude is high.
		cd_debug ("iPointedRow: %d, iLastRow: %d", iPointedRow, pData->iLastRow);
		pData->bUseDrawCoords = TRUE;
		pData->iLastRow = iPointedRow;
		cairo_dock_trigger_set_WM_icons_geometry (pDock);
	}
	
	return pPointedIcon;
}


static void _generate_cos_sin (double fConeOffset, double fDelta, double *pTabValues)
{
	int i, n = (int) ceil ((G_PI/2 - fConeOffset) / fDelta);
	pTabValues[2*n] = 0.;  // point at the middle.
	pTabValues[2*n+1] = 1.;
	
	double fTheta;
	for (i = 0; i < n; i ++)
	{
		fTheta = (i == 0 ? G_PI/2 - fConeOffset : (n - i) * fDelta);
		pTabValues[2*i] = sin (fTheta);
		pTabValues[2*i+1] = cos (fTheta);
		
		pTabValues[2*(2*n-i)] = - pTabValues[2*i];
		pTabValues[2*(2*n-i)+1] = pTabValues[2*i+1];
	}
}
static void _generate_sector_path (double fConeOffset, double fRadius1, double fRadius2, double fDelta, double *pCosSinTab, GLfloat *pTabValues)
{
	int i, n = (int) ceil ((G_PI/2 - fConeOffset) / fDelta), N = (2*n+1) * 2;
	for (i = 0; i < 2*n+1; i ++)
	{
		pTabValues[3*(2*i)] = fRadius1 * pCosSinTab[2*i+0];
		pTabValues[3*(2*i)+1] = fRadius1 * pCosSinTab[2*i+1];
		pTabValues[3*(2*i+1)] = fRadius2 * pCosSinTab[2*i+0];
		pTabValues[3*(2*i+1)+1] = fRadius2 * pCosSinTab[2*i+1];
		//g_print ("%.2f;%.2f\n", pTabValues[3*i], pTabValues[3*i+1]);
	}
	pTabValues[3*N] = pTabValues[3*0];
	pTabValues[3*N+1] = pTabValues[3*0+1];
	pTabValues[3*(N+1)] = pTabValues[3*1];
	pTabValues[3*(N+1)+1] = pTabValues[3*1+1];
}

static void cd_rendering_render_rainbow_opengl (CairoDock *pDock)
{
	static double fDelta = 1.;
	int n = ceil (180./fDelta+1) + 1;  // nb points max, +1 for safety with rounded calculations.
	if (pCosSinTab == NULL)
	{
		pCosSinTab = g_new0 (double, n * 2);
		_generate_cos_sin (my_fRainbowConeOffset, fDelta/180.*G_PI, pCosSinTab);
	}
	if (pVertexTab == NULL)
		pVertexTab = g_new0 (GLfloat, (n * 2) * 3);
	if (pColorTab == NULL)
	{
		pColorTab = g_new0 (GLfloat, (n * 2) * 4);
		int i;
		for (i = 0; i < n; i ++)
		{
			pColorTab[4*2*i+0] = my_fRainbowColor[0];
			pColorTab[4*2*i+1] = my_fRainbowColor[1];
			pColorTab[4*2*i+2] = my_fRainbowColor[2];
			pColorTab[4*2*i+3] = my_fRainbowColor[3];
		}
	}
	
	double fMaxScale =  1. + my_fRainbowMagnitude * myIconsParam.fAmplitude;
	double fRadius=0;
	if (my_fRainbowColor[3] != 0 && pDock->icons != NULL)
	{
		//\____________________ We draw frame's borders.
		//\____________________ We draw decorations in it.
		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		///_cairo_dock_set_blend_alpha ();  // strange ...
		_cairo_dock_set_blend_over ();
		glEnable(GL_BLEND);
		
		glPolygonMode(GL_FRONT, GL_FILL);  // GL_FILL
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glLineWidth (1);
		
		int n = (int) ceil ((G_PI/2 - my_fRainbowConeOffset) / (fDelta/180.*G_PI)), N = (2*n+1) * 2;
		
		glPushMatrix ();
		if (! pDock->container.bIsHorizontal)
		{
			glTranslatef (pDock->container.iHeight/2, pDock->container.iWidth/2, 0.);
			glRotatef (90., 0., 0., 1.);
			glTranslatef (0., -pDock->container.iHeight/2, 0.);
		}
		else
			glTranslatef (pDock->container.iWidth/2, 0., 0.);
		if (!pDock->container.bDirectionUp)
		{
			glTranslatef (0., pDock->container.iHeight, 0.);
			glScalef (1., -1., 1.);
		}
		
		GList *pFirstDrawnElement = pDock->icons;
		GList *ic = pFirstDrawnElement;
		double fCurrentRadius=0;
		Icon *icon;
		do
		{
			//if (fRadius > 0)
			//	break ;
			icon = ic->data;
			fRadius = icon->fX - (pDock->container.bDirectionUp ? pDock->iMaxIconHeight * icon->fScale : 0);
			if (fRadius != fCurrentRadius)
			{
				_generate_sector_path (my_fRainbowConeOffset,
					fRadius + .5 * pDock->iMaxIconHeight * fMaxScale,
					fRadius - my_iSpaceBetweenRows/2,
					fDelta/180.*G_PI, pCosSinTab, pVertexTab);
				
				glVertexPointer(3, GL_FLOAT, 0, pVertexTab);
				glColorPointer(4, GL_FLOAT, 0, pColorTab);
				glDrawArrays(GL_QUAD_STRIP, 0, N);
				
				_generate_sector_path (my_fRainbowConeOffset,
					fRadius + .5 * pDock->iMaxIconHeight * fMaxScale,
					fRadius + pDock->iMaxIconHeight * fMaxScale + my_iSpaceBetweenRows/2,
					fDelta/180.*G_PI, pCosSinTab, pVertexTab);
				
				glVertexPointer(3, GL_FLOAT, 0, pVertexTab);
				glColorPointer(4, GL_FLOAT, 0, pColorTab);
				glDrawArrays(GL_QUAD_STRIP, 0, N);
				
				fCurrentRadius = fRadius;
			}
			ic = cairo_dock_get_next_element (ic, pDock->icons);
		} while (ic != pFirstDrawnElement);
		
		glPopMatrix ();
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisable(GL_POLYGON_SMOOTH);
		glDisable(GL_BLEND);
	}
	
	//\____________________ We draw the frame's borders.
	if (fRadius == 0)
	{
		Icon *icon = cairo_dock_get_last_icon (pDock->icons);
		if (icon)
			fRadius = icon->fX - (pDock->container.bDirectionUp ? pDock->iMaxIconHeight * fMaxScale : 0);
	}
	fRadius += .5 * pDock->iMaxIconHeight * fMaxScale;
	if (my_fRainbowLineColor[3] != 0)
	{
		glPushMatrix ();
		if (! pDock->container.bIsHorizontal)
		{
			glTranslatef (pDock->container.iHeight/2, pDock->container.iWidth/2, 0.);
			glRotatef (90., 0., 0., 1.);
			glTranslatef (0., -pDock->container.iHeight/2, 0.);
		}
		else
			glTranslatef (pDock->container.iWidth/2, 0., 0.);
		if (!pDock->container.bDirectionUp)
		{
			glTranslatef (0., pDock->container.iHeight, 0.);
			glScalef (1., -1., 1.);
		}
		GLfloat color[4*5] = {my_fRainbowLineColor[0], my_fRainbowLineColor[1], my_fRainbowLineColor[2], 0.,
		my_fRainbowLineColor[0], my_fRainbowLineColor[1], my_fRainbowLineColor[2], my_fRainbowLineColor[3],
		my_fRainbowLineColor[0], my_fRainbowLineColor[1], my_fRainbowLineColor[2], my_fRainbowLineColor[3],
		my_fRainbowLineColor[0], my_fRainbowLineColor[1], my_fRainbowLineColor[2], my_fRainbowLineColor[3],
		my_fRainbowLineColor[0], my_fRainbowLineColor[1], my_fRainbowLineColor[2], 0.};
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_FLOAT, 0, color);
		glVertexPointer(2, GL_FLOAT, 0, pVertexTab);
		
		pVertexTab[2*0+0] = - (fRadius + .5 * pDock->iMaxIconHeight * fMaxScale + my_iSpaceBetweenRows/2) * pCosSinTab[2*0];
		pVertexTab[2*0+1] = (fRadius + .5 * pDock->iMaxIconHeight * fMaxScale + my_iSpaceBetweenRows/2) * pCosSinTab[2*0+1];
		pVertexTab[2*1+0] = - fRadius * cos (my_fRainbowConeOffset);
		pVertexTab[2*1+1] = fRadius * sin (my_fRainbowConeOffset);
		pVertexTab[2*2+0] = 0.;
		pVertexTab[2*2+1] = 0.;
		pVertexTab[2*3+0] = - pVertexTab[2*1+0];
		pVertexTab[2*3+1] = pVertexTab[2*1+1];
		pVertexTab[2*4+0] = - pVertexTab[2*0+0];
		pVertexTab[2*4+1] = pVertexTab[2*0+1];
		
		cairo_dock_draw_current_path_opengl (_get_dock_linewidth(), my_fRainbowLineColor, 5);
		
		glDisableClientState(GL_COLOR_ARRAY);
		glPopMatrix ();
	}
	
	//\____________________ We draw icons.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;

	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex)/** * pDock->fMagnitudeMax*/;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		if (!CAIRO_DOCK_ICON_TYPE_IS_SEPARATOR (icon))
		{
			glPushMatrix ();
			cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, TRUE);  // ! mySystem.bTextAlwaysHorizontal
			glPopMatrix ();
		}
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}



void cd_rendering_reload_rainbow_buffers (void)
{
	g_free (pColorTab);
	pColorTab = NULL;
	g_free (pCosSinTab);
	pCosSinTab = NULL;
	g_free (pVertexTab);
	pVertexTab = NULL;
}


void cd_rendering_register_rainbow_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	// interface
	pRenderer->compute_size = cd_rendering_calculate_max_dock_size_rainbow;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_rainbow;
	pRenderer->render = cd_rendering_render_rainbow;
	pRenderer->render_optimized = NULL;
	pRenderer->render_opengl = cd_rendering_render_rainbow_opengl;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	pRenderer->free_data = _free_rainbow_data;
	pRenderer->get_minimize_pos = cd_rendering_get_minimize_pos_rainbow;
	// parametres
	pRenderer->cDisplayedName = D_ (cRendererName);
	pRenderer->cReadmeFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-rainbow-view");
	pRenderer->cPreviewFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-rainbow.jpg");
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
