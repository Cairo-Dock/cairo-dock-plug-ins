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
#include <time.h>
#include <signal.h>
#include <math.h>
#include <locale.h> // setlocale()
#include <langinfo.h> // nl_langinfo()
#include <ctype.h> // isdigit()

#include "applet-struct.h"
#include "applet-draw.h"

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer1[CD_CLOCK_DATE_BUFFER_LENGTH+1];
static char s_cDateBuffer2[CD_CLOCK_DATE_BUFFER_LENGTH+1];
static char s_cCmbBuffer[2*CD_CLOCK_DATE_BUFFER_LENGTH+1];

#define GAPY .02
///#define MAX_RATIO 2.
#define MIN_TEXT_HEIGHT 12.  // the text should be at least 12 pixels height, or it would be hard to read for a lot of people.

/** Get a format string that is suitable to pass to strftime() to format
 * the date to be displayed on the icon or as a label.
 * We need this function as the default date format (%x) is not sufficient.
 * Specifically, we want to (1) omit the year so that the label can be shorter;
 * (2) use the name of the month to avoid confusion with the day; and (3) include
 * the weekday as well. However, what is a good way to represent this information
 * varies by language.
 */
const char *cd_clock_get_date_format (void)
{
	static const char *cFormat = NULL;
	
	if (cFormat) return cFormat;
	
	const char *cLocale = setlocale (LC_TIME, NULL); // contrary to the name, this will get the current locale setting
	
	// Chinese and Japanese: %b and %B are equivalent to %m月, better to specify explicitly, and also use
	// the 日 character to indicate the day of the month
	// note: %- is a glibc extension to print a number without padding; it is also supported by musl, see:
	// https://git.musl-libc.org/cgit/musl/tree/src/time/strftime.c#n235
	// and the BSD libc implementation: https://man.freebsd.org/cgi/man.cgi?strftime
	// However, it might cause problems on systems with other libc implementations (e.g. ulibc-ng, newlib, etc.).
	if (!strncmp (cLocale, "ja", 2))
		cFormat = "%-m月%e日 (%a)"; // Japanese: one character abbreviation of weekday should be clear
	else if (!strncmp (cLocale, "zh", 2))
		cFormat = "%-m月%e日 (%A)"; // Chinese: use the full weekday names, as %a only gives numbers / 日
	else
	{
		// try to determine if months should go before days or otherwise
		const char *tmp = nl_langinfo (D_FMT);
		gboolean month_first = FALSE;
		
		for (; *tmp; ++tmp) if (*tmp == '%')
		{
			gboolean found = FALSE;
			++tmp;
			// flags supported by glibc
			if (*tmp == '-' || *tmp == '_' || *tmp == '0' || *tmp == '^' || *tmp == '#') ++tmp;
			// field width supported by glibc
			while (isdigit (*tmp)) ++tmp;
			// alternate number selector flag supported by glibc
			if (*tmp == 'O') ++tmp;
			
			switch (*tmp)
			{
				case 'b':
				case 'h':
				case 'B':
				case 'm':
				case 'D':
					month_first = TRUE;
					found = TRUE;
					break;
				case 'd':
				case 'e':
				case 'F':
				case 0:
					found = TRUE;
					break;
				default:
					break;
			}
			
			if (found) break;
		}
		
		cFormat = month_first ? "%b %e (%a)" : "%e %b (%a)";
	}
	return cFormat;
}

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

	if (myData.pNumericBgSurface != NULL)
	{
		cairo_set_source_surface (myDrawContext, myData.pNumericBgSurface, 0., 0.);
		cairo_paint (myDrawContext);
	}
	if (myConfig.bUseDefaultColors)
		gldi_style_colors_set_text_color (myDrawContext);
	else
		gldi_color_set_cairo (myDrawContext, &myConfig.textDescription.fColorStart);

	//\______________ We define the text that we have to draw.
	const char *cDateFormat = cd_clock_get_date_format ();
	// hour's format
	const gchar *cTimeFormat;
	if (myConfig.b24Mode)
	{
		if (myConfig.bShowSeconds)
			cTimeFormat = "%T";
		else
			cTimeFormat = "%R";
	}
	else
	{
		if (myConfig.bShowSeconds)
			cTimeFormat = "%r";  // same as %I:%M:%S %p
		else
			cTimeFormat = "%I:%M %p";
	}


	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		size_t off = 0;
		if (myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)
		{
			// we separately format date and time and will combine them in two different ways
			strftime (s_cDateBuffer1, CD_CLOCK_DATE_BUFFER_LENGTH, cDateFormat, pTime);
			strftime (s_cDateBuffer2, CD_CLOCK_DATE_BUFFER_LENGTH, cTimeFormat, pTime);
			// we start with the one line version
			snprintf (s_cCmbBuffer, 2 * CD_CLOCK_DATE_BUFFER_LENGTH + 1, "%s %s", s_cDateBuffer1, s_cDateBuffer2);
		}
		else if (myData.iTextLayout == CD_TEXT_LAYOUT_2_LINES)
		{
			// time goes first
			off = strftime (s_cCmbBuffer, 2 * CD_CLOCK_DATE_BUFFER_LENGTH, cTimeFormat, pTime);
			s_cCmbBuffer[off] = '\n'; // note: there is at least one more character (as the buffer size is +1 longer than given to strftime)
			++off;
			strftime (s_cCmbBuffer + off, 2 * CD_CLOCK_DATE_BUFFER_LENGTH + 1 - off, cDateFormat, pTime);
		}
		else
		{
			// one line layout, date goes first
			off = strftime (s_cCmbBuffer, 2 * CD_CLOCK_DATE_BUFFER_LENGTH, cDateFormat, pTime);
			s_cCmbBuffer[off] = ' ';
			++off;
			strftime (s_cCmbBuffer + off, 2 * CD_CLOCK_DATE_BUFFER_LENGTH + 1 - off, cTimeFormat, pTime);
		}
	}
	else strftime (s_cCmbBuffer, 2 * CD_CLOCK_DATE_BUFFER_LENGTH - 1, cTimeFormat, pTime);
	
	// layout
	PangoFontDescription *pDesc = myConfig.textDescription.fd;
	pango_font_description_set_absolute_size (pDesc, myIcon->fHeight * 72 / myData.fDpi * PANGO_SCALE); // pixel converted to point, converted to pango dimension.

	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_layout_set_alignment (pLayout, PANGO_ALIGN_CENTER);

	pango_layout_set_text (pLayout, s_cCmbBuffer, -1);
	PangoRectangle log;
	pango_layout_get_pixel_extents (pLayout, NULL, &log);
	if (myConfig.iOutlineWidth)
	{
		log.width += myConfig.iOutlineWidth / 2;
		log.height += myConfig.iOutlineWidth / 2;
	}
	
	// scaling with the current layout
	double fZoomX = (double) iWidth / log.width;
	double fZoomY = (double) iHeight / log.height;
	// keep the ratio of the text, until 12px height.
	fZoomX = MIN (fZoomX, fZoomY);
	fZoomY = fZoomX * myConfig.fTextRatio;
	if (fZoomY * log.height < MIN_TEXT_HEIGHT)
		fZoomY = MIN_TEXT_HEIGHT / log.height;
	
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON && myData.iTextLayout == CD_TEXT_LAYOUT_AUTO)
	{
		// If the orientation is no longer defined, we define it just once at startup (if we are close
		// to the limit, the size of the text could change enough to change the layout).
		// We test both layouts for this -- note: currently, the bufer includes the one line layout,
		// we should test the two line alternative.
		snprintf (s_cCmbBuffer, 2 * CD_CLOCK_DATE_BUFFER_LENGTH + 1, "%s\n%s", s_cDateBuffer2, s_cDateBuffer1);

		PangoLayout *pLayout2 = pango_cairo_create_layout (myDrawContext);
		pango_layout_set_font_description (pLayout2, pDesc);
		pango_layout_set_alignment (pLayout2, PANGO_ALIGN_CENTER);

		pango_layout_set_text (pLayout2, s_cCmbBuffer, -1);
		PangoRectangle log2;
		pango_layout_get_pixel_extents (pLayout2, NULL, &log2);
		if (myConfig.iOutlineWidth)
		{
			log2.width += myConfig.iOutlineWidth / 2;
			log2.height += myConfig.iOutlineWidth / 2;
		}
		
		// scaling with the current layout
		double fZoomX2 = (double) iWidth / log2.width;
		double fZoomY2 = (double) iHeight / log2.height;
		// keep the ratio of the text, until 12px height.
		fZoomX2 = MIN (fZoomX2, fZoomY2);
		fZoomY2 = fZoomX2 * myConfig.fTextRatio;
		if (fZoomY2 * log2.height < MIN_TEXT_HEIGHT)
			fZoomY2 = MIN_TEXT_HEIGHT / log2.height;
		
		// 1. check distortion, i.e. which case differs more from the expected ratio
		double exp_Y1 = fZoomX * myConfig.fTextRatio;
		double exp_Y2 = fZoomX2 * myConfig.fTextRatio;
		double def1 = (fZoomY  > exp_Y1 ? fZoomY  / exp_Y1 : exp_Y1 / fZoomY );  // deformation.
		double def2 = (fZoomY2 > exp_Y2 ? fZoomY2 / exp_Y2 : exp_Y2 / fZoomY2);
		if (def1 > def2 * 1.001) myData.iTextLayout = CD_TEXT_LAYOUT_2_LINES;
		else if (def2 > def1 * 1.001) myData.iTextLayout = CD_TEXT_LAYOUT_1_LINE;
		else
		{
			// 2. check which case shrinks the text more
			if (fZoomX < fZoomX2) myData.iTextLayout = CD_TEXT_LAYOUT_2_LINES;
			else myData.iTextLayout = CD_TEXT_LAYOUT_1_LINE;
		}
		
		if (myData.iTextLayout == CD_TEXT_LAYOUT_2_LINES)
		{
			g_object_unref (pLayout);
			pLayout = pLayout2;
			fZoomX = fZoomX2;
			fZoomY = fZoomY2;
			log.width = log2.width;
			log.height = log2.height;
		}
		else g_object_unref (pLayout2);
	}

	//\______________ We draw the text.
	cairo_save (myDrawContext);

	cairo_translate (myDrawContext,
		(iWidth - fZoomX * log.width)/2,
		(iHeight - fZoomY * log.height)/2);  // text will be centred.
	cairo_scale (myDrawContext, fZoomX, fZoomY);
	if (myConfig.iOutlineWidth)
		_outlined_pango_cairo (myApplet, pLayout);
	pango_cairo_show_layout (myDrawContext, pLayout);

	cairo_restore (myDrawContext);
	g_object_unref (pLayout);

	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
}


void cd_clock_draw_analogic (GldiModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO ();
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
		cairo_set_source_rgb (myDrawContext, myConfig.fDateColor[0], myConfig.fDateColor[1], myConfig.fDateColor[2]);
		cairo_set_line_width (myDrawContext, 8.0f);
		strftime (s_cDateBuffer1, CD_CLOCK_DATE_BUFFER_LENGTH, cd_clock_get_date_format (), pTime);
		cairo_text_extents (myDrawContext, s_cDateBuffer1, &textExtents);
		cairo_move_to (myDrawContext,
			-textExtents.width / 2.0f,
			2.0f * textExtents.height);

		cairo_show_text (myDrawContext, s_cDateBuffer1);
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
	//_cairo_dock_set_blend_over ();  // not good
	_cairo_dock_set_blend_alpha ();  // not bad
	//_cairo_dock_set_blend_pbuffer ();
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // better, do not ask me why...

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
