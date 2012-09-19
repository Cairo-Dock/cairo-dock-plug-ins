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

static void cd_rendering_calculate_max_dock_size_rainbow (CairoDock *pDock)
{
	pDock->fMagnitudeMax = my_fRainbowMagnitude;
	cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth);
	
	double fMaxScale =  1. + my_fRainbowMagnitude * myIconsParam.fAmplitude;
	int iMaxIconWidth = pDock->iMaxIconHeight + my_iSpaceBetweenIcons;
	double fCone = G_PI - 2 * my_fRainbowConeOffset;
	int iNbIcons = g_list_length (pDock->icons);
	int iMinRadius = MIN (my_iRainbowNbIconsMin, iNbIcons) * iMaxIconWidth * fMaxScale / fCone;
	
	int iNbRows = (int) ceil (sqrt (2 * iNbIcons / fCone / fMaxScale) + .5);  /// approximation, utiliser la formule complete...
	
	pDock->iMaxDockHeight = iNbRows * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale + iMinRadius;
	pDock->iMaxDockWidth = 2 * (pDock->iMaxDockHeight * cos (my_fRainbowConeOffset));
	cd_debug ("iNbRows : %d => %dx%d (iMaxIconHeight = %d ; iMinRadius = %d ; fMaxScale = %.2f)\n", iNbRows, pDock->iMaxDockWidth, pDock->iMaxDockHeight, pDock->iMaxIconHeight, iMinRadius, fMaxScale);
	
	pDock->iDecorationsWidth = 0;
	pDock->iDecorationsHeight = 0;
	
	pDock->iMinDockWidth = pDock->fFlatDockWidth;
	pDock->iMinDockHeight = pDock->iMaxIconHeight;
	
	pDock->iActiveWidth = pDock->iMaxDockWidth;
	pDock->iActiveHeight = pDock->iMaxDockHeight;
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
		//\____________________ On fait un clip du cone.
		cairo_move_to (pCairoContext, 0., pDock->container.iHeight * (1 - sin (my_fRainbowConeOffset)));
		cairo_line_to (pCairoContext, pDock->container.iWidth/2, pDock->container.iHeight);
		cairo_line_to (pCairoContext, pDock->container.iWidth, pDock->container.iHeight * (1 - sin (my_fRainbowConeOffset)));
		cairo_line_to (pCairoContext, pDock->container.iWidth, 0.);
		cairo_line_to (pCairoContext, 0., 0.);
		cairo_close_path (pCairoContext);
		cairo_clip (pCairoContext);
		
		//\____________________ On dessine chaque rayure dedans.
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
				if (fCurrentRadius == 0)  // 1er coup.
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
	
	//\____________________ On dessine le cadre.
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
		cairo_set_line_width (pCairoContext, myDocksParam.iDockLineWidth);
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
	
	//\____________________ On dessine la ficelle qui les joint.
	
	//\____________________ On dessine les icones avec leurs etiquettes.
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

		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pDock, pCairoContext, fDockMagnitude, TRUE);
		cairo_restore (pCairoContext);

		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}


static void cd_rendering_get_polar_coords (CairoDock *pDock, double *fRadius, double *fTheta)
{
	double x = pDock->container.iMouseX - pDock->container.iWidth / 2, y = (pDock->container.bDirectionUp ? pDock->container.iHeight - pDock->container.iMouseY : pDock->container.iMouseY);
	
	*fRadius = sqrt (x * x + y * y);
	*fTheta = atan2 (x, y);
}

static double _calculate_wave_offset (int x_abs, int iMaxIconHeight, double fMagnitude, double fFlatDockWidth, int iWidth, double fAlign, double fFoldingFactor, double fRatio)
{
	int iIconNumber = (x_abs + .5*my_iSpaceBetweenRows) / (iMaxIconHeight + my_iSpaceBetweenRows);
	
	int x_cumulated = iIconNumber * (iMaxIconHeight + my_iSpaceBetweenRows);
	cd_debug (" iIconNumber : %d ; x_cumulated : %d\n", iIconNumber, x_cumulated);
	double fXMiddle = x_cumulated + iMaxIconHeight / 2;
	double fPhase = (fXMiddle - x_abs) / myIconsParam.iSinusoidWidth / fRatio * G_PI + G_PI / 2;
	if (fPhase < 0)
	{
		fPhase = 0;
	}
	else if (fPhase > G_PI)
	{
		fPhase = G_PI;
	}
	double fScale = 1 + fMagnitude * myIconsParam.fAmplitude * sin (fPhase);
	double fX = x_cumulated - 0*(fFlatDockWidth - iWidth) / 2 + (1 - fScale) * (x_abs - x_cumulated + .5*my_iSpaceBetweenRows);
	fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1. - fFoldingFactor);
	
	while (iIconNumber > 0)
	{
		iIconNumber --;
		x_cumulated = iIconNumber * (iMaxIconHeight + my_iSpaceBetweenRows);
		//cd_debug ("  %d) x_cumulated = %d\n", iIconNumber, x_cumulated);
		fXMiddle = x_cumulated + iMaxIconHeight / 2;
		fPhase = (fXMiddle - x_abs) / myIconsParam.iSinusoidWidth / fRatio * G_PI + G_PI / 2;
		if (fPhase < 0)
		{
			fPhase = 0;
		}
		else if (fPhase > G_PI)
		{
			fPhase = G_PI;
		}
		fScale = 1 + fMagnitude * myIconsParam.fAmplitude * sin (fPhase);
		
		fX = fX - (iMaxIconHeight + my_iSpaceBetweenRows) * fScale;
		fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1. - fFoldingFactor);
	}
	return -fX;
}
static double cd_rendering_calculate_wave_position (CairoDock *pDock, double fCurvilignAbscisse, double fMagnitude)
{
	cd_debug ("%s (%.2f)\n", __func__, fCurvilignAbscisse);
	
	if (pDock->icons == NULL || fCurvilignAbscisse <= 0)
		return 0;
	double fWaveOffset, fWaveExtrema;
	double x_abs = fCurvilignAbscisse;
	int nb_iter = 0;
	double fRatio = pDock->container.fRatio;
	
	do
	{
		//cd_debug ("  x_abs : %.2f\n", x_abs);
		fWaveOffset = _calculate_wave_offset (x_abs, pDock->iMaxIconHeight, fMagnitude, pDock->fFlatDockWidth, pDock->fFlatDockWidth, 0*pDock->fAlign, pDock->fFoldingFactor, fRatio);
		
		fWaveExtrema = fWaveOffset + x_abs;
		if (fWaveExtrema >= 0)
			x_abs += (fCurvilignAbscisse - fWaveExtrema) / 2;
		else
			x_abs = MAX (0, x_abs - (fCurvilignAbscisse - fWaveExtrema) / 2);
		if (x_abs > (int) pDock->fFlatDockWidth)
		{
			x_abs = (int) pDock->fFlatDockWidth;
			break ;
		}
		//cd_debug ("  -> fWaveOffset : %.2f, fWaveExtrema : %.2f\n", fWaveOffset, fWaveExtrema);
		
		nb_iter ++;
	}
	while (fabs (fWaveExtrema - fCurvilignAbscisse) > 1 && nb_iter < 15);
	
	return x_abs;
}

static int cd_rendering_calculate_wave_on_each_lines (int x_abs, int iMaxIconHeight, double fMagnitude, double fFlatDockWidth, int iWidth, double fAlign, double fFoldingFactor, double fRatio, int iNbRows, double *pScales)  // (fScale,fX)
{
	if (iNbRows == 0)
		return 0;
	cd_debug ("%s (%d, %.2f, %.2f, %d)\n", __func__, x_abs, fMagnitude, fFoldingFactor, iNbRows);
	if (x_abs < 0 && iWidth > 0)  // ces cas limite sont la pour empecher les icones de retrecir trop rapidement quend on sort par les cotes.
		x_abs = -1;
	else if (x_abs > fFlatDockWidth && iWidth > 0)
		x_abs = fFlatDockWidth+1;
	
	float x_cumulated = 0, fXMiddle;
	double fPhase, fX, fScale = 0.0;
	int iNumRow, iPointedRow=-1;
	for (iNumRow = 0; iNumRow < iNbRows; iNumRow ++)
	{
		//cd_debug (" ligne %d\n", iNumRow);
		fXMiddle = x_cumulated + .5*iMaxIconHeight;
		
		fPhase = (fXMiddle - x_abs) / myIconsParam.iSinusoidWidth / fRatio * G_PI + G_PI / 2;
		if (fPhase < 0)
			fPhase = 0;
		else if (fPhase > G_PI)
			fPhase = G_PI;
		fScale = 1 + fMagnitude * myIconsParam.fAmplitude * sin (fPhase);
		pScales[2*iNumRow] = fScale;
		//cd_debug ("  fScale : %.2f\n", fScale);
		
		if (iPointedRow != -1)
		{
			fX = pScales[2*(iNumRow-1)+1] + (iMaxIconHeight + my_iSpaceBetweenRows) * pScales[2*(iNumRow-1)];
			fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1. - fFoldingFactor);
			pScales[2*iNumRow+1] = fX;
			//cd_debug ("  fX : %.2f (prev : %.2f , %.2f)\n", fX, pScales[2*(iNumRow-1)+1], pScales[2*(iNumRow-1)]);
		}
		
		if (x_cumulated + iMaxIconHeight + .5*my_iSpaceBetweenRows >= x_abs && x_cumulated - .5*my_iSpaceBetweenRows <= x_abs && iPointedRow == -1)  // on a trouve la ligne sur laquelle on pointe.
		{
			iPointedRow = iNumRow;
			fX = x_cumulated - 0*(fFlatDockWidth - iWidth) / 2 + (1 - fScale) * (x_abs - x_cumulated + .5*my_iSpaceBetweenRows);
			fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1. - fFoldingFactor);
			//cd_debug ("  ligne pointee : %d\n", iPointedRow);
			pScales[2*iNumRow+1] = fX;
			//cd_debug ("  fX : %.2f\n", fX);
		}
		
		x_cumulated += iMaxIconHeight;
	}
	
	if (iPointedRow == -1)  // on est en dehors du disque.
	{
		iPointedRow = iNbRows - 1;
		
		fX = x_cumulated - (fFlatDockWidth - iWidth) / 2 + (1 - fScale) * (iMaxIconHeight + .5*my_iSpaceBetweenRows);
		fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1 - fFoldingFactor);
		pScales[2*iNumRow+1] = fX;
		//cd_debug ("  fX : %.2f\n", fX);
	}
	
	for (iNumRow = iPointedRow-1; iNumRow >= 0; iNumRow --)
	{
		fX = pScales[2*(iNumRow+1)+1] - (iMaxIconHeight + my_iSpaceBetweenRows) * pScales[2*iNumRow];
		fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1. - fFoldingFactor);
		pScales[2*iNumRow+1] = fX;
		//cd_debug ("  fX : %.2f\n", fX);
	}
	
	fX = pScales[1];
	for (iNumRow = 0; iNumRow < iNbRows; iNumRow ++)
	{
		pScales[2*iNumRow+1] -= fX;
	}
	
	return iPointedRow;
}

static Icon *cd_rendering_calculate_icons_rainbow (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return NULL;
	
	//\____________________ On se place en coordonnees polaires.
	double fMaxScale =  1. + my_fRainbowMagnitude * myIconsParam.fAmplitude;
	int iMaxIconWidth = pDock->iMaxIconHeight + my_iSpaceBetweenIcons;
	double fCone = G_PI - 2 * my_fRainbowConeOffset;
	int iNbIcons = g_list_length (pDock->icons);
	int iMinRadius = MIN (my_iRainbowNbIconsMin, iNbIcons) * iMaxIconWidth * fMaxScale / fCone;
	double fRatio = pDock->container.fRatio;
	double fRadius, fTheta;
	cd_debug (" mouse : (%d ; %d)\n", pDock->container.iMouseX, pDock->container.iMouseY);
	cd_rendering_get_polar_coords (pDock, &fRadius, &fTheta);
	cd_debug (" polar : (%.2f ; %.2f)\n", fRadius, fTheta/G_PI*180.);
	
	double fCurvilignAbscisse = (fTheta > -G_PI/2 && fTheta < G_PI/2 ? MAX (0, fRadius - iMinRadius): 0);
	
	//\____________________ On en deduit ou appliquer la vague pour que la crete soit a la position du curseur.
	Icon *pPointedIcon = NULL;
	double fMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex) * pDock->fMagnitudeMax;
	int x_abs = (int) round (cd_rendering_calculate_wave_position (pDock, fCurvilignAbscisse, fMagnitude));
	cd_debug (" => x_abs : %d (fMagnitude:%.2f ; fFoldingFactor:%.2f)\n", x_abs, fMagnitude, pDock->fFoldingFactor);
	
	//\_______________ On en deduit l'amplitude de chaque ligne.
	int iNbRows = round ((pDock->iMaxDockHeight- iMinRadius) / ((pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale));
	cd_debug ("iNbRows : %d\n", iNbRows);
	double fFlatDockWidth = iNbRows * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale;
	double *pScales = g_new0 (double, 2*iNbRows+2);
	cd_rendering_calculate_wave_on_each_lines (x_abs, pDock->iMaxIconHeight, fMagnitude, fFlatDockWidth, fFlatDockWidth, 0*pDock->fAlign, pDock->fFoldingFactor, fRatio, iNbRows, pScales);
	
	//\____________________ On en deduit les position/etirements/alpha des icones.
	Icon* icon;
	GList* ic;
	
	GList *pFirstDrawnElement = pDock->icons;
	ic = pFirstDrawnElement;
	
	int iNbRow = -1, iNbIconsOnRow = 0, iNbInsertedIcons = 0;
	double fCurrentRadius=0, fNormalRadius=0, fCurrentTheta, fThetaStart=0, fDeltaTheta=0, fCurrentScale=1;
	do
	{
		icon = ic->data;
		
		if (iNbInsertedIcons == iNbIconsOnRow)
		{
			iNbRow ++;
			if (iNbRow == iNbRows)
				break ;
			iNbInsertedIcons = 0;
			fCurrentRadius = iMinRadius * (1 - pDock->fFoldingFactor) + pScales[2*iNbRow+1];
			fCurrentScale = pScales[2*iNbRow] * (1 - pDock->fFoldingFactor);
			fNormalRadius = iMinRadius + iNbRow * (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fMaxScale;
			fDeltaTheta = 2 * atan (iMaxIconWidth * fMaxScale / 2 / fNormalRadius);
			iNbIconsOnRow = (int) (fCone / fDeltaTheta);
			fThetaStart = - G_PI/2 + my_fRainbowConeOffset + (fCone - iNbIconsOnRow * fDeltaTheta) / 2 + fDeltaTheta / 2;
			cd_debug ("on passe a la ligne %d (%d icones, fThetaStart = %.2fdeg, fCurrentRadius = %.2f(%.2f), fDeltaTheta = %.2f, fCurrentScale = %.2f)\n", iNbRow, iNbIconsOnRow, fThetaStart/G_PI*180, fCurrentRadius, fNormalRadius, fDeltaTheta/G_PI*180, fCurrentScale);
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
			cd_debug (" POINTED ICON : %s\n", pPointedIcon->cName);
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
		
		cd_debug (" %.2fdeg ; (%.2f;%.2f)\n", fCurrentTheta/G_PI*180, icon->fDrawX, icon->fDrawY);
		
		icon->fScale = fCurrentScale;
		icon->fAlpha = 1;
		icon->fWidthFactor = 1.;
		icon->fHeightFactor = 1.;
		
		iNbInsertedIcons ++;
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	g_free (pScales);
	
	//g_print ("fRadius : %.2f ; limite : %.2f\n", fRadius, fCurrentRadius + pDock->iMaxIconHeight * fCurrentScale);
	if (! pDock->container.bInside ||
		fRadius > fCurrentRadius + pDock->iMaxIconHeight * fCurrentScale + myIconsParam.iLabelSize - (pDock->fFoldingFactor > 0 ? 20 : 0) ||
		((fTheta < - G_PI/2 + my_fRainbowConeOffset || fTheta > G_PI/2 - my_fRainbowConeOffset) && fRadius > iMinRadius + .5 * pDock->iMaxIconHeight * fMaxScale))
	{
		cd_debug ("<<< on sort du demi-disque >>>\n");
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
		pDock->bCanDrop = FALSE;
	}
	else
	{
		pDock->iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
		if (pDock->bIsDragging)
			pDock->bCanDrop = TRUE;
	}
	
	return pPointedIcon;
}


static void _generate_cos_sin (double fConeOffset, double fDelta, double *pTabValues)
{
	int i, n = (int) ceil ((G_PI/2 - fConeOffset) / fDelta);
	pTabValues[2*n] = 0.;  // point au milieu.
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
		//\____________________ On trace le cadre.
		//\____________________ On dessine les decorations dedans.
		glEnable (GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		///_cairo_dock_set_blend_alpha ();  // qqch cloche ...
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
	
	//\____________________ On dessine le cadre.
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
		
		cairo_dock_draw_current_path_opengl (myDocksParam.iDockLineWidth, my_fRainbowLineColor, 5);
		
		glDisableClientState(GL_COLOR_ARRAY);
		glPopMatrix ();
	}
	
	//\____________________ On dessine les icones.
	GList *pFirstDrawnElement = cairo_dock_get_first_drawn_element_linear (pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;

	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex)/** * pDock->fMagnitudeMax*/;
	Icon *icon;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;
		
		glPushMatrix ();
		cairo_dock_render_one_icon_opengl (icon, pDock, fDockMagnitude, TRUE);  // ! mySystem.bTextAlwaysHorizontal
		glPopMatrix ();
		
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
	// parametres
	pRenderer->cDisplayedName = D_ (cRendererName);
	pRenderer->cReadmeFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/readme-rainbow-view");
	pRenderer->cPreviewFilePath = g_strdup (MY_APPLET_SHARE_DATA_DIR"/preview-rainbow.jpg");
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
