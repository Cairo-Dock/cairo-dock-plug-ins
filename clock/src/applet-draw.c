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

#include <stdlib.h>
#define __USE_POSIX
#include <time.h>
#include <signal.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-draw.h"

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];

#define GAPX .12
#define GAPY .02
///#define MAX_RATIO 2.
#define MIN_TEXT_HEIGHT 12.  // the text should be at least 12 pixels height, or it would be hard to read for a lot of people.

static void _outlined_pango_cairo (GldiModuleInstance *myApplet, PangoLayout *pLayout)
{
	cairo_save (myDrawContext);
	cairo_set_source_rgba (myDrawContext,
		myConfig.fOutlineColor[0],
		myConfig.fOutlineColor[1],
		myConfig.fOutlineColor[2],
		myConfig.fOutlineColor[3]);
	cairo_set_line_width (myDrawContext, myConfig.iOutlineWidth);
	pango_cairo_layout_path (myDrawContext, pLayout);
	cairo_stroke (myDrawContext);
	cairo_restore (myDrawContext);
}

void cd_clock_draw_text (GldiModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();
	/**g_return_if_fail (myDrawContext != NULL);
	//\______________ On efface le dessin courant.
	cairo_dock_erase_cairo_context (myDrawContext);*/
	
	if (myData.pNumericBgSurface != NULL)
	{
		cairo_set_source_surface (myDrawContext, myData.pNumericBgSurface, 0., 0.);
		cairo_paint (myDrawContext);
	}
	if (myConfig.bUseDefaultColors)
		gldi_style_colors_set_text_color (myDrawContext);
	else
		cairo_set_source_rgba (myDrawContext, myConfig.textDescription.fColorStart[0], myConfig.textDescription.fColorStart[1], myConfig.textDescription.fColorStart[2], myConfig.textDescription.fColorStart[3]);
	
	//\______________ On defini le texte a dessiner.
	// layout
	PangoFontDescription *pDesc = myConfig.textDescription.fd;
	pango_font_description_set_absolute_size (pDesc, myIcon->fHeight * 72 / myData.fDpi * PANGO_SCALE); // pixel converted to point, converted to pango dimension.
	
	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	pango_layout_set_font_description (pLayout, pDesc);
	
	// format de l'heure
	const gchar *cFormat;
	if (myConfig.b24Mode)
	{
		if (myConfig.bShowSeconds)
			cFormat = "%T";
		else
			cFormat = "%R";
	}
	else
	{
		if (myConfig.bShowSeconds)
			cFormat = "%r";  // equivalent a %I:%M:%S %p
		else
			cFormat = "%I:%M %p";
	}
	
	strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, cFormat, pTime);
	pango_layout_set_text (pLayout, s_cDateBuffer, -1);
	PangoRectangle log;
	pango_layout_get_pixel_extents (pLayout, NULL, &log);
	if (myConfig.iOutlineWidth)
	{
		log.width += myConfig.iOutlineWidth / 2;
		log.height += myConfig.iOutlineWidth / 2;
	}
	
	//\______________ On dessine le texte.
	cairo_save (myDrawContext);
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		PangoLayout *pLayout2 = pango_cairo_create_layout (myDrawContext);
		pango_layout_set_font_description (pLayout2, pDesc);
		
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a %d %b", pTime);
		pango_layout_set_text (pLayout2, s_cDateBuffer, -1);
		PangoRectangle log2;
		pango_layout_get_pixel_extents (pLayout2, NULL, &log2);
		if (myConfig.iOutlineWidth)
		{
			log2.width += myConfig.iOutlineWidth / 2;
			log2.height += myConfig.iOutlineWidth / 2;
		}
		
		double h=0, w=0, fZoomX=0, fZoomY=0;  // parametres d'affichage 2 lignes
		double h_=0, w_=0, fZoomX_=0, fZoomY_=0;  // parametres d'affichage 1 ligne
		if (myData.iTextLayout == CD_TEXT_LAYOUT_2_LINES || myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)
		{
			h = log.height + log2.height + GAPY * iHeight;
			w = MAX (log.width, log2.width);
			fZoomX = (double) iWidth / w;
			fZoomY = (double) iHeight / h;
			/**if (myDock && fZoomY > MAX_RATIO * fZoomX)  // we limit the deformation
				fZoomY = MAX_RATIO * fZoomX;
			
			if (myConfig.fTextRatio < 1)
				fZoomY *= myConfig.fTextRatio;*/
			// keep the ratio of the text, until 12px height.
			fZoomX = MIN (fZoomX, fZoomY) * myConfig.fTextRatio;
			fZoomY = fZoomX;
			if (fZoomY * h < MIN_TEXT_HEIGHT)
				fZoomY = MIN_TEXT_HEIGHT / h;
		}
		if (myData.iTextLayout == CD_TEXT_LAYOUT_1_LINE || myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)
		{
			h_ = MAX (log.height, log2.height);
			w_ = log.width + log2.width + GAPX * iWidth;
			fZoomX_ = (double) iWidth / w_;
			fZoomY_ = (double) iHeight / h_;
			/**if (myDock && fZoomY_ > MAX_RATIO * fZoomX_)  // we limit the deformation
				fZoomY_ = MAX_RATIO * fZoomX_;
			
			if (myConfig.fTextRatio < 1)
				fZoomY_ *= myConfig.fTextRatio;
			
			if (fZoomY_ > fZoomX_)
			{
				double fMaxScale = cairo_dock_get_icon_max_scale (myIcon);
				fZoomY_ = MAX (fZoomX_, 16. * fMaxScale / h_);  // en mode horizontal, on n'a pas besoin que le texte remplisse toute la hauteur de l'icone. 16 pixels de haut sont suffisant pour etre lisible.
			}*/
			// keep the ratio of the text, until 12px height.
			fZoomX_ = MIN (fZoomX_, fZoomY_) * myConfig.fTextRatio;
			fZoomY_ = fZoomX_;
			if (fZoomY_ * h_ < MIN_TEXT_HEIGHT)
				fZoomY_ = MIN_TEXT_HEIGHT / h_;
		}
		
		if (myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)  // si l'orientation n'est pas encore definie, on la definit de facon a ne pas changer (si on est tres proche de la limite, la taille du texte pourrait changer suffisamment pour nous faire passer d'une orientation a l'autre.
		{
			double def = (fZoomX > fZoomY ? fZoomX / fZoomY : fZoomY / fZoomX);  // deformation.
			double def_ = (fZoomX_ > fZoomY_ ? fZoomX_ / fZoomY_ : fZoomY_ / fZoomX_);
			if (def > def_)  // deformation plus grande en mode 2 lignes => on passe en mode 1 ligne.
				myData.iTextLayout = CD_TEXT_LAYOUT_2_LINES;
			else
				myData.iTextLayout = CD_TEXT_LAYOUT_1_LINE;
		}
		
		if (myData.iTextLayout == CD_TEXT_LAYOUT_1_LINE)  // mode 1 line
		{
			cairo_translate (myDrawContext, (iWidth - fZoomX_ * w_) / 2, (iHeight - fZoomY_ * h_)/2);  // text will be centred.
			cairo_scale (myDrawContext, fZoomX_, fZoomY_);
			if (myConfig.iOutlineWidth)
				_outlined_pango_cairo (myApplet, pLayout2);
			pango_cairo_show_layout (myDrawContext, pLayout2);
			
			cairo_restore (myDrawContext);
			cairo_save (myDrawContext);
			
			cairo_translate (myDrawContext, (iWidth + fZoomX_ * w_) / 2 - fZoomX_ * log.width, (iHeight - fZoomY_ * h_)/2);
			cairo_scale (myDrawContext, fZoomX_, fZoomY_);
			if (myConfig.iOutlineWidth)
				_outlined_pango_cairo (myApplet, pLayout);
			pango_cairo_show_layout (myDrawContext, pLayout);
		}
		else  // mode 2 lines
		{
			cairo_translate (myDrawContext, (iWidth - fZoomX * log.width) / 2, (iHeight - fZoomY * h)/2);  // text will be centred.
			cairo_scale (myDrawContext, fZoomX, fZoomY);
			if (myConfig.iOutlineWidth)
				_outlined_pango_cairo (myApplet, pLayout);
			pango_cairo_show_layout (myDrawContext, pLayout);
			
			cairo_restore (myDrawContext);
			cairo_save (myDrawContext);
			
			cairo_translate (myDrawContext, (iWidth - fZoomX * log2.width) / 2, (iHeight + fZoomY * GAPY)/2);
			cairo_scale (myDrawContext, fZoomX, fZoomY);
			if (myConfig.iOutlineWidth)
				_outlined_pango_cairo (myApplet, pLayout2);
			pango_cairo_show_layout (myDrawContext, pLayout2);
		}
		g_object_unref (pLayout2);
	}
	else  // only the hour with 1 line.
	{
		double fZoomX = (double) iWidth / log.width;
		double fZoomY = (double) iHeight / log.height;
		/**if (myDock && fZoomY > MAX_RATIO * fZoomX)  // we limit the deformation
			fZoomY = MAX_RATIO * fZoomX;
		
		if (myConfig.fTextRatio < 1)
			fZoomY *= myConfig.fTextRatio;*/
		// keep the ratio of the text, until 12px height.
		fZoomX = MIN (fZoomX, fZoomY) * myConfig.fTextRatio;
		fZoomY = fZoomX;
		if (fZoomY * log.height < MIN_TEXT_HEIGHT)
			fZoomY = MIN_TEXT_HEIGHT / log.height;
		
		cairo_translate (myDrawContext,
			(iWidth - fZoomX * log.width)/2,
			(iHeight - fZoomY * log.height)/2);  // text will be centred.
		cairo_scale (myDrawContext, fZoomX, fZoomY);
		if (myConfig.iOutlineWidth)
			_outlined_pango_cairo (myApplet, pLayout);
		pango_cairo_show_layout (myDrawContext, pLayout);
	}
	cairo_restore (myDrawContext);
	g_object_unref (pLayout);
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}


void cd_clock_draw_analogic (GldiModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();
	///g_return_if_fail (myDrawContext != NULL);
	//g_print ("%s (%dx%d)\n", __func__, width, height);
	double fHalfX;
	double fHalfY;
	double fShadowOffsetX = -0.75f;
	double fShadowOffsetY = 0.75f;
	cairo_text_extents_t textExtents;
	
	fHalfX = myData.DimensionData.width / 2.0f;
	fHalfY = myData.DimensionData.height / 2.0f;
	
	int iSeconds = pTime->tm_sec;
	int iMinutes = pTime->tm_min;
	int iHours = pTime->tm_hour;
	
	/**cairo_set_source_rgba (myDrawContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);*/
	cairo_save (myDrawContext);
	
	cairo_set_source_surface (myDrawContext, myData.pBackgroundSurface, 0.0f, 0.0f);
	cairo_paint (myDrawContext);
	
	cairo_scale (myDrawContext,
		(double) iWidth / (double) myData.DimensionData.width,
		(double) iHeight / (double) myData.DimensionData.height);
		
	cairo_translate (myDrawContext, fHalfX, fHalfY);
	
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		cairo_save (myDrawContext);
		cairo_set_source_rgba (myDrawContext, myConfig.fDateColor[0], myConfig.fDateColor[1], myConfig.fDateColor[2], myConfig.fDateColor[3]);
		cairo_set_line_width (myDrawContext, 8.0f);
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a%d%b", pTime);
		cairo_text_extents (myDrawContext, s_cDateBuffer, &textExtents);
		cairo_move_to (myDrawContext,
			-textExtents.width / 2.0f,
			2.0f * textExtents.height);
		
		cairo_show_text (myDrawContext, s_cDateBuffer);
		cairo_restore (myDrawContext);
	}
	
	cairo_save (myDrawContext);
	cairo_translate (myDrawContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (myDrawContext, (iHours % 12 + iMinutes/60.) * G_PI/6 - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND_SHADOW], myDrawContext);
	cairo_restore (myDrawContext);
	
	cairo_save (myDrawContext);
	cairo_translate (myDrawContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (myDrawContext, (G_PI/30.0f) * (iMinutes + iSeconds/60.) - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND_SHADOW], myDrawContext);
	cairo_restore (myDrawContext);
	
	if (myConfig.bShowSeconds)
	{
		cairo_save (myDrawContext);
		cairo_translate (myDrawContext, fShadowOffsetX, fShadowOffsetY);
		cairo_rotate (myDrawContext, (G_PI/30.0f) * iSeconds - G_PI/2.0f);
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND_SHADOW], myDrawContext);
		cairo_restore (myDrawContext);
	}
	
	cairo_save (myDrawContext);
	cairo_rotate (myDrawContext, (iHours % 12 + iMinutes/60.) * G_PI/6 - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND], myDrawContext);
	cairo_restore (myDrawContext);
	
	cairo_save (myDrawContext);
	cairo_rotate (myDrawContext, (G_PI/30.0f) * (iMinutes + iSeconds/60.) - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND], myDrawContext);
	cairo_restore (myDrawContext);
	
	if (myConfig.bShowSeconds)
	{
		cairo_save (myDrawContext);
		cairo_rotate (myDrawContext, (G_PI/30.0f) * iSeconds - G_PI/2.0f);
		
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND], myDrawContext);
		cairo_restore (myDrawContext);
	}
	
	cairo_restore (myDrawContext);
	cairo_set_source_surface (myDrawContext, myData.pForegroundSurface, 0.0f, 0.0f);
	cairo_paint (myDrawContext);
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}


void cd_clock_render_analogic_to_texture (GldiModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime, double fFraction)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
	
	int iSeconds = pTime->tm_sec;
	int iMinutes = pTime->tm_min;
	int iHours = pTime->tm_hour;
	
	_cairo_dock_enable_texture ();
	//_cairo_dock_set_blend_over ();  // bof
	_cairo_dock_set_blend_alpha ();  // pas mal
	//_cairo_dock_set_blend_pbuffer ();
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // mieux, ne me demandez pas pourquoi...
	
	// draw texture bg
	_cairo_dock_apply_texture_at_size_with_alpha (myData.iBgTexture, iWidth, iHeight, 1.);
	
	//g_print ("%s (%d , %dx%d)\n", __func__, myData.iDateTexture, (int)myData.iDateWidth, (int)myData.iDateHeight);
	if (myData.iDateTexture != 0 && myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		glPushMatrix ();
		glTranslatef (0., - 3*myData.iDateHeight/2, 0.);
		cairo_dock_apply_texture_at_size (myData.iDateTexture, myData.iDateWidth, myData.iDateHeight);
		glPopMatrix ();
	}
	
	// hour
	glPushMatrix ();
	glRotatef (-(iHours % 12 + iMinutes/60.) * 30. + 90., 0., 0., 1.);
	glTranslatef (myData.iNeedleWidth/2 - myData.fNeedleScale * myData.iNeedleOffsetX, 0., 0.);
	cairo_dock_apply_texture_at_size (myData.iHourNeedleTexture, myData.iNeedleWidth, myData.iNeedleHeight+1);
	glPopMatrix ();
	
	// minute
	glPushMatrix ();
	glRotatef (-6. * (iMinutes + iSeconds/60.) + 90., 0., 0., 1.);
	glTranslatef (myData.iNeedleWidth/2 - myData.fNeedleScale * myData.iNeedleOffsetX, 0., 0.);
	cairo_dock_apply_texture_at_size (myData.iMinuteNeedleTexture, myData.iNeedleWidth, myData.iNeedleHeight+1);
	glPopMatrix ();
	
	// second
	if (myConfig.bShowSeconds)
	{
		glPushMatrix ();
		glRotatef (-6. * (iSeconds + fFraction) + 90., 0., 0., 1.);
		glTranslatef (myData.iNeedleWidth/2 - myData.fNeedleScale * myData.iNeedleOffsetX, 0., 0.);
		cairo_dock_apply_texture_at_size (myData.iSecondNeedleTexture, myData.iNeedleWidth, myData.iNeedleHeight+1);
		glPopMatrix ();
	}
	
	// draw texture fg
	cairo_dock_apply_texture_at_size (myData.iFgTexture, iWidth, iHeight);
	
	_cairo_dock_disable_texture ();
	
	CD_APPLET_FINISH_DRAWING_MY_ICON;
}
