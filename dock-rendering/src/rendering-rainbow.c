/*********************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

*********************************************************************************/
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

void cd_rendering_calculate_max_dock_size_rainbow (CairoDock *pDock)
{
	pDock->fMagnitudeMax = my_fRainbowMagnitude;
	pDock->pFirstDrawnElement = cairo_dock_calculate_icons_positions_at_rest_linear (pDock->icons, pDock->fFlatDockWidth, pDock->iScrollOffset);
	
	double fMaxScale =  1. + my_fRainbowMagnitude * g_fAmplitude;
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
}


void cd_rendering_render_rainbow (cairo_t *pCairoContext, CairoDock *pDock)
{
	//g_print ("pDock->fFoldingFactor : %.2f\n", pDock->fFoldingFactor);
	
	//\____________________ On trace le cadre.
	
	//\____________________ On dessine les decorations dedans.
	
	//\____________________ On dessine le cadre.
	
	//\____________________ On dessine la ficelle qui les joint.
	
	//\____________________ On dessine les icones avec leurs etiquettes.
	double fRatio = pDock->fRatio;
	///cairo_dock_render_icons_linear (pCairoContext, pDock, fRatio);
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	if (pFirstDrawnElement == NULL)
		return;

	double fDockMagnitude = cairo_dock_calculate_magnitude (pDock->iMagnitudeIndex)/** * pDock->fMagnitudeMax*/;
	Icon *icon;
	int iWidth = pDock->iCurrentWidth;
	gboolean bHorizontalDock = pDock->bHorizontalDock;
	GList *ic = pFirstDrawnElement;
	do
	{
		icon = ic->data;

		cairo_save (pCairoContext);
		cairo_dock_render_one_icon (icon, pCairoContext, bHorizontalDock, fRatio, fDockMagnitude, pDock->bUseReflect, ! mySystem.bTextAlwaysHorizontal, pDock->iCurrentWidth, pDock->bDirectionUp);
		
		if (mySystem.bTextAlwaysHorizontal && icon->pTextBuffer != NULL && icon->fScale > 1.01 && (! mySystem.bLabelForPointedIconOnly || icon->bPointed) && icon->iCount == 0)  // 1.01 car sin(pi) = 1+epsilon :-/
		{
			double fOffsetX = -icon->fTextXOffset + icon->fWidthFactor * icon->fWidth * icon->fScale / 2;
			if (fOffsetX < - icon->fDrawX)
				fOffsetX = - icon->fDrawX;
			else if (icon->fDrawX + fOffsetX + icon->iTextWidth > iWidth)
				fOffsetX = iWidth - icon->iTextWidth - icon->fDrawX;
			if (icon->fOrientation != 0 && ! mySystem.bTextAlwaysHorizontal)
			{
				cairo_rotate (pCairoContext, icon->fOrientation);
			}
			/*if (fRatio < 1)  // bof, finalement spa top de reduire le texte.
				cairo_scale (pCairoContext,
					fRatio,
					fRatio);*/
			if (! bHorizontalDock && mySystem.bTextAlwaysHorizontal)
			{
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					0,
					0);
			}
			else if (bHorizontalDock)
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					fOffsetX,
					pDock->bDirectionUp ? -myLabels.iconTextDescription.iSize : icon->fHeight * icon->fScale - icon->fTextYOffset);
			else
				cairo_set_source_surface (pCairoContext,
					icon->pTextBuffer,
					pDock->bDirectionUp ? -myLabels.iconTextDescription.iSize : icon->fHeight * icon->fScale - icon->fTextYOffset,
					fOffsetX);
			
			double fMagnitude;
			if (mySystem.bLabelForPointedIconOnly)
			{
				fMagnitude = fDockMagnitude;  // (icon->fScale - 1) / g_fAmplitude / sin (icon->fPhase);  // sin (phi ) != 0 puisque fScale > 1.
			}
			else
			{
				fMagnitude = (icon->fScale - 1) / g_fAmplitude / pDock->fMagnitudeMax;
				fMagnitude *= (fMagnitude * mySystem.fLabelAlphaThreshold + 1) / (mySystem.fLabelAlphaThreshold + 1);
			}
			cairo_paint_with_alpha (pCairoContext, fMagnitude);
		}
		
		cairo_restore (pCairoContext);

		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
}


static void cd_rendering_get_polar_coords (CairoDock *pDock, double *fRadius, double *fTheta)
{
	double x = pDock->iMouseX - pDock->iCurrentWidth / 2, y = (pDock->bDirectionUp ? pDock->iCurrentHeight - pDock->iMouseY : pDock->iMouseY);
	
	*fRadius = sqrt (x * x + y * y);
	*fTheta = atan2 (x, y);
}

static double _calculate_wave_offset (int x_abs, int iMaxIconHeight, double fMagnitude, double fFlatDockWidth, int iWidth, double fAlign, double fFoldingFactor, double fRatio)
{
	int iIconNumber = (x_abs + .5*my_iSpaceBetweenRows) / (iMaxIconHeight + my_iSpaceBetweenRows);
	
	int x_cumulated = iIconNumber * (iMaxIconHeight + my_iSpaceBetweenRows);
	cd_debug (" iIconNumber : %d ; x_cumulated : %d\n", iIconNumber, x_cumulated);
	double fXMiddle = x_cumulated + iMaxIconHeight / 2;
	double fPhase = (fXMiddle - x_abs) / myIcons.iSinusoidWidth / fRatio * G_PI + G_PI / 2;
	if (fPhase < 0)
	{
		fPhase = 0;
	}
	else if (fPhase > G_PI)
	{
		fPhase = G_PI;
	}
	double fScale = 1 + fMagnitude * g_fAmplitude * sin (fPhase);
	double fX = x_cumulated - 0*(fFlatDockWidth - iWidth) / 2 + (1 - fScale) * (x_abs - x_cumulated + .5*my_iSpaceBetweenRows);
	fX = fAlign * iWidth + (fX - fAlign * iWidth) * (1. - fFoldingFactor);
	
	while (iIconNumber > 0)
	{
		iIconNumber --;
		x_cumulated = iIconNumber * (iMaxIconHeight + my_iSpaceBetweenRows);
		//cd_debug ("  %d) x_cumulated = %d\n", iIconNumber, x_cumulated);
		fXMiddle = x_cumulated + iMaxIconHeight / 2;
		fPhase = (fXMiddle - x_abs) / myIcons.iSinusoidWidth / fRatio * G_PI + G_PI / 2;
		if (fPhase < 0)
		{
			fPhase = 0;
		}
		else if (fPhase > G_PI)
		{
			fPhase = G_PI;
		}
		fScale = 1 + fMagnitude * g_fAmplitude * sin (fPhase);
		
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
	double fRatio = pDock->fRatio;
	
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
	
	float x_cumulated = 0, fXMiddle, fDeltaExtremum;
	double fPhase, fScale, fX;
	int iNumRow, iPointedRow=-1;
	for (iNumRow = 0; iNumRow < iNbRows; iNumRow ++)
	{
		//cd_debug (" ligne %d\n", iNumRow);
		fXMiddle = x_cumulated + .5*iMaxIconHeight;
		
		fPhase = (fXMiddle - x_abs) / myIcons.iSinusoidWidth / fRatio * G_PI + G_PI / 2;
		if (fPhase < 0)
			fPhase = 0;
		else if (fPhase > G_PI)
			fPhase = G_PI;
		fScale = 1 + fMagnitude * g_fAmplitude * sin (fPhase);
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

Icon *cd_rendering_calculate_icons_rainbow (CairoDock *pDock)
{
	if (pDock->icons == NULL)
		return NULL;
	
	//\____________________ On se place en coordonnees polaires.
	double fMaxScale =  1. + my_fRainbowMagnitude * g_fAmplitude;
	int iMaxIconWidth = pDock->iMaxIconHeight + my_iSpaceBetweenIcons;
	double fCone = G_PI - 2 * my_fRainbowConeOffset;
	int iNbIcons = g_list_length (pDock->icons);
	int iMinRadius = MIN (my_iRainbowNbIconsMin, iNbIcons) * iMaxIconWidth * fMaxScale / fCone;
	double fRatio = pDock->fRatio;
	double w = pDock->iCurrentWidth;
	double h = pDock->iCurrentHeight;
	double fRadius, fTheta;
	cd_debug (" mouse : (%d ; %d)\n", pDock->iMouseX, pDock->iMouseY);
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
	Icon* icon, *prev_icon;
	GList* ic;
	
	GList *pFirstDrawnElement = (pDock->pFirstDrawnElement != NULL ? pDock->pFirstDrawnElement : pDock->icons);
	ic = pFirstDrawnElement;
	Icon *pFirstIcon = pFirstDrawnElement->data;
	
	int iNbRow = -1, iNbIconsOnRow = 0, iNbInsertedIcons = 0;
	double fCurrentRadius=0, fNormalRadius=0, fCurrentTheta, fThetaStart, fDeltaTheta, fCurrentScale;
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
		
		icon->fX = fCurrentRadius + (pDock->bDirectionUp ? pDock->iMaxIconHeight * fCurrentScale : 0);
		
		fCurrentTheta = fThetaStart + iNbInsertedIcons * fDeltaTheta;
		icon->fOrientation = (pDock->bDirectionUp ? fCurrentTheta : - fCurrentTheta);
		
		if (pPointedIcon == NULL && fRadius < fCurrentRadius + (pDock->iMaxIconHeight + my_iSpaceBetweenRows) * fCurrentScale && fRadius > fCurrentRadius && fTheta > fCurrentTheta - fDeltaTheta/2 && fTheta < fCurrentTheta + fDeltaTheta/2)
		{
			icon->bPointed = TRUE;
			pPointedIcon = icon;
			cd_debug (" POINTED ICON : %s\n", pPointedIcon->acName);
		}
		else
			icon->bPointed = FALSE;
		
		icon->fDrawY = icon->fX * cos (fCurrentTheta) + icon->fWidth/2 * fCurrentScale * sin (fCurrentTheta);
		if (pDock->bDirectionUp)
			icon->fDrawY = pDock->iCurrentHeight - icon->fDrawY;
		icon->fDrawX = icon->fX * sin (fCurrentTheta) - icon->fWidth/2 * fCurrentScale * cos (fCurrentTheta) + pDock->iCurrentWidth / 2;
		cd_debug (" %.2fdeg ; (%.2f;%.2f)\n", fCurrentTheta/G_PI*180, icon->fDrawX, icon->fDrawY);
		
		icon->fScale = fCurrentScale;
		icon->fAlpha = 1;
		icon->fWidthFactor = 1.;
		icon->fHeightFactor = 1.;
		
		prev_icon = icon;
		
		cairo_dock_manage_animations (icon, pDock);
		
		iNbInsertedIcons ++;
		
		ic = cairo_dock_get_next_element (ic, pDock->icons);
	} while (ic != pFirstDrawnElement);
	g_free (pScales);
	
	CairoDockMousePositionType iMousePositionType;
	//g_print ("fRadius : %.2f ; limite : %.2f\n", fRadius, fCurrentRadius + pDock->iMaxIconHeight * fCurrentScale);
	if (! pDock->bInside || fRadius > fCurrentRadius + pDock->iMaxIconHeight * fCurrentScale + myLabels.iconTextDescription.iSize - (pDock->fFoldingFactor > 0 ? 20 : 0) || fTheta < - G_PI/2 + my_fRainbowConeOffset || fTheta > G_PI/2 - my_fRainbowConeOffset)
	{
		cd_debug ("<<< on sort du demi-disque >>>\n");
		iMousePositionType = CAIRO_DOCK_MOUSE_OUTSIDE;
	}
	else
	{
		iMousePositionType = CAIRO_DOCK_MOUSE_INSIDE;
	}
	
	cairo_dock_manage_mouse_position (pDock, iMousePositionType);
	
	//cairo_dock_mark_avoiding_mouse_icons_linear (pDock);
	
	
	return (iMousePositionType == CAIRO_DOCK_MOUSE_INSIDE ? pPointedIcon : NULL);
}


void cd_rendering_register_rainbow_renderer (const gchar *cRendererName)
{
	CairoDockRenderer *pRenderer = g_new0 (CairoDockRenderer, 1);
	pRenderer->cReadmeFilePath = g_strdup_printf ("%s/readme-rainbow-view", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->cPreviewFilePath = g_strdup_printf ("%s/preview-rainbow.png", MY_APPLET_SHARE_DATA_DIR);
	pRenderer->calculate_max_dock_size = cd_rendering_calculate_max_dock_size_rainbow;
	pRenderer->calculate_icons = cd_rendering_calculate_icons_rainbow;
	pRenderer->render = cd_rendering_render_rainbow;
	pRenderer->render_optimized = NULL;
	pRenderer->set_subdock_position = cairo_dock_set_subdock_position_linear;
	
	cairo_dock_register_renderer (cRendererName, pRenderer);
}
