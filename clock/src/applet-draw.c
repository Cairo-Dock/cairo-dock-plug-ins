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
//#include "applet-digital.h" //Digital html like renderer
#include "applet-draw.h"

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];

#define GAPX .12
#define GAPY .02
#define MAX_RATIO 2.

void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	cairo_dock_erase_cairo_context (myDrawContext);
	if (myData.pNumericBgSurface != NULL)
	{
		cairo_set_source_surface (myDrawContext, myData.pNumericBgSurface, 0., 0.);
		cairo_paint (myDrawContext);
	}
	cairo_set_source_rgba (myDrawContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2], myConfig.fTextColor[3]);
	
	PangoFontDescription *pDesc = pango_font_description_new ();
	pango_font_description_set_absolute_size (pDesc, myIcon->fHeight * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, myConfig.cFont);
	pango_font_description_set_weight (pDesc, myConfig.iWeight);
	pango_font_description_set_style (pDesc, myConfig.iStyle);
	
	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	pango_layout_set_font_description (pLayout, pDesc);
	
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
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	cairo_save (myDrawContext);
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		PangoLayout *pLayout2 = pango_cairo_create_layout (myDrawContext);
		pango_layout_set_font_description (pLayout2, pDesc);
		
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a %d %b", pTime);
		pango_layout_set_text (pLayout2, s_cDateBuffer, -1);
		PangoRectangle ink2, log2;
		pango_layout_get_pixel_extents (pLayout2, &ink2, &log2);
		
		double h=0, w=0, fZoomX=0, fZoomY=0;
		double h_=0, w_=0, fZoomX_=0, fZoomY_=0;
		if (myData.iTextLayout == CD_TEXT_LAYOUT_2_LINES || myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)
		{
			h = ink.height + ink2.height + GAPY * iHeight;
			w = MAX (ink.width, ink2.width);
			fZoomX = (double) iWidth / w;
			fZoomY = (double) iHeight / h;
			if (myDock && fZoomY > MAX_RATIO * fZoomX)  // on ne garde pas le ratio car ca ferait un texte trop petit en hauteur, toutefois on limite un peu la deformation en hauteur.
				fZoomY = MAX_RATIO * fZoomX;
			
			if (fZoomX * w > myConfig.fTextRatio * iWidth)
			{
				fZoomY *= myConfig.fTextRatio * iWidth / w * fZoomX;
				fZoomX = myConfig.fTextRatio * iWidth / w;
			}
		}
		if (myData.iTextLayout == CD_TEXT_LAYOUT_1_LINE || myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)
		{
			h_ = MAX (ink.height, ink2.height);
			w_ = ink.width + ink2.width + GAPX * iWidth;
			fZoomX_ = (double) iWidth / w_;
			fZoomY_ = (double) iHeight / h_;
			if (myDock && fZoomY_ > MAX_RATIO * fZoomX_)  // on ne garde pas le ratio car ca ferait un texte trop petit en hauteur, toutefois on limite un peu la deformation en hauteur.
				fZoomY_ = MAX_RATIO * fZoomX_;
			
			if (fZoomX_ * w_ > myConfig.fTextRatio * iWidth)
			{
				fZoomY_ *= myConfig.fTextRatio * iWidth / w_ * fZoomX_;
				fZoomX_ = myConfig.fTextRatio * iWidth / w_;
			}
			if (fZoomY_ > fZoomX_)
			{
				double fMaxScale = cairo_dock_get_max_scale (myContainer);
				fZoomY_ = MAX (fZoomX_, 16. * fMaxScale / h_);  // en mode horizontal, on n'a pas besoin que le texte remplisse toute la hauteur de l'icone. 16 pixels de haut sont suffisant pour etre lisible.
			}
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
		
		if (myData.iTextLayout == CD_TEXT_LAYOUT_1_LINE)  // mode 1 ligne
		{
			cairo_translate (myDrawContext, (iWidth - fZoomX_ * w_) / 2, (iHeight - fZoomY_ * h_)/2);  // centre verticalement.
			cairo_scale (myDrawContext, fZoomX_, fZoomY_);
			cairo_translate (myDrawContext, -ink2.x, -ink2.y);
			pango_cairo_show_layout (myDrawContext, pLayout2);
			
			cairo_restore (myDrawContext);
			cairo_save (myDrawContext);
			
			cairo_translate (myDrawContext, (iWidth + fZoomX_ * w_) / 2 - fZoomX_ * ink.width, (iHeight - fZoomY_ * h_)/2);
			cairo_scale (myDrawContext, fZoomX_, fZoomY_);
			cairo_translate (myDrawContext, -ink.x, -ink.y);
			pango_cairo_show_layout (myDrawContext, pLayout);
		}
		else  // mode 2 lignes
		{
			cairo_translate (myDrawContext, (iWidth - fZoomX * ink.width) / 2, (iHeight - fZoomY * h)/2);  // centre verticalement.
			cairo_scale (myDrawContext, fZoomX, fZoomY);
			cairo_translate (myDrawContext, -ink.x, -ink.y);
			pango_cairo_show_layout (myDrawContext, pLayout);
			
			cairo_restore (myDrawContext);
			cairo_save (myDrawContext);
			
			cairo_translate (myDrawContext, (iWidth - fZoomX * ink2.width) / 2, (iHeight + fZoomY * GAPY)/2);
			cairo_scale (myDrawContext, fZoomX, fZoomY);
			cairo_translate (myDrawContext, -ink2.x, -ink2.y);
			pango_cairo_show_layout (myDrawContext, pLayout2);
		}
		g_object_unref (pLayout2);
	}
	else
	{
		double fZoomX = (double) (iWidth - 1) / ink.width;
		double fZoomY = (double) iHeight / ink.height;
		if (myDock && fZoomY > MAX_RATIO * fZoomX)  // on ne garde pas le ratio car ca ferait un texte trop petit en hauteur, toutefois on limite un peu la deformation en hauteur.
			fZoomY = MAX_RATIO * fZoomX;
		
		if (fZoomX * ink.width > myConfig.fTextRatio * iWidth)
		{
			fZoomY *= myConfig.fTextRatio * iWidth / (ink.width * fZoomX);
			fZoomX = myConfig.fTextRatio * iWidth / ink.width;
		}
		
		cairo_translate (myDrawContext, 0., (iHeight - fZoomY * ink.height)/2);  // centre verticalement.
		cairo_scale (myDrawContext, fZoomX, fZoomY);
		cairo_translate (myDrawContext, -ink.x, -ink.y);
		pango_cairo_show_layout (myDrawContext, pLayout);
	}
	cairo_restore (myDrawContext);
	g_object_unref (pLayout);
	pango_font_description_free (pDesc);
}


void cd_clock_draw_analogic (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	//g_print ("%s (%dx%d)\n", __func__, width, height);
	cairo_t *pSourceContext = myDrawContext;
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
	
	//cairo_set_tolerance (pSourceContext, 0.1);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	cairo_save (pSourceContext);
	
	cairo_set_source_surface (pSourceContext, myData.pBackgroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
	
	cairo_scale (pSourceContext,
		(double) iWidth / (double) myData.DimensionData.width,
		(double) iHeight / (double) myData.DimensionData.height);
		
	cairo_translate (pSourceContext, fHalfX, fHalfY);
	
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		cairo_save (pSourceContext);
		cairo_set_source_rgba (pSourceContext, myConfig.fDateColor[0], myConfig.fDateColor[1], myConfig.fDateColor[2], myConfig.fDateColor[3]);
		cairo_set_line_width (pSourceContext, 8.0f);
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a%d%b", pTime);
		cairo_text_extents (pSourceContext, s_cDateBuffer, &textExtents);
		cairo_move_to (pSourceContext,
			-textExtents.width / 2.0f,
			2.0f * textExtents.height);
		
		cairo_show_text (pSourceContext, s_cDateBuffer);
		cairo_restore (pSourceContext);
	}
	
	cairo_save (pSourceContext);
	cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (pSourceContext, (iHours % 12 + iMinutes/60.) * G_PI/6 - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND_SHADOW], pSourceContext);
	cairo_restore (pSourceContext);
	
	cairo_save (pSourceContext);
	cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (pSourceContext, (G_PI/30.0f) * (iMinutes + iSeconds/60.) - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND_SHADOW], pSourceContext);
	cairo_restore (pSourceContext);
	
	if (myConfig.bShowSeconds)
	{
		cairo_save (pSourceContext);
		cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
		cairo_rotate (pSourceContext, (G_PI/30.0f) * iSeconds - G_PI/2.0f);
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND_SHADOW], pSourceContext);
		cairo_restore (pSourceContext);
	}
	
	cairo_save (pSourceContext);
	cairo_rotate (pSourceContext, (iHours % 12 + iMinutes/60.) * G_PI/6 - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND], pSourceContext);
	cairo_restore (pSourceContext);
	
	cairo_save (pSourceContext);
	cairo_rotate (pSourceContext, (G_PI/30.0f) * (iMinutes + iSeconds/60.) - G_PI/2.0f);
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND], pSourceContext);
	cairo_restore (pSourceContext);
	
	if (myConfig.bShowSeconds)
	{
		cairo_save (pSourceContext);
		cairo_rotate (pSourceContext, (G_PI/30.0f) * iSeconds - G_PI/2.0f);
		
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND], pSourceContext);
		cairo_restore (pSourceContext);
	}
	
	cairo_restore (pSourceContext);
	cairo_set_source_surface (pSourceContext, myData.pForegroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
}


void cd_clock_render_analogic_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime, double fFraction)
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
