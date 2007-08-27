/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <time.h>
#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>
#include "cairo-dock.h"

#include "cd-clock-config.h"
#include "cd-clock-struct.h"
#include "cd-clock-draw.h"


extern Icon *my_pIcon;
extern cairo_t *my_pCairoContext;
extern GtkWidget *my_pWidget;
extern CairoDock *my_pDock;

extern gboolean my_bShowDate;
extern gboolean my_bShowSeconds;
extern gboolean my_b24Mode;
extern int my_bOldStyle;
extern int my_iTheme;
extern GHashTable *my_pThemeTable;

extern cairo_surface_t *g_pBackgroundSurface;
extern cairo_surface_t *g_pForegroundSurface;
extern RsvgDimensionData my_DimensionData;
extern RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];

static char cDateBuffer[50];


gboolean cd_clock_update_with_time (Icon *icon)
{
	time_t epoch = (time_t) time (NULL);
	struct tm epoch_tm;
	localtime_r (&epoch, &epoch_tm);
	
	if (my_bOldStyle)
		cd_clock_draw_old_fashionned_clock (my_pCairoContext, (int) icon->fWidth, (int) icon->fHeight, (1 + g_fAmplitude), &epoch_tm);
	else
		cd_clock_draw_text (my_pCairoContext, &epoch_tm);
	
	
	if (! g_pMainDock->bAtBottom || ! g_bAutoHide)
		cairo_dock_redraw_my_icon (icon, my_pDock);
	
	return TRUE;
}


void cd_clock_draw_text (cairo_t *pSourceContext, struct tm *pTime)
{
	GString *sFormat = g_string_new ("");
	
	if (my_bShowSeconds)
		g_string_printf (sFormat, "%T");
	else
		g_string_printf (sFormat, "%R");
	
	if (my_bShowDate)
		g_string_append (sFormat, "\n%a%d%b");
	
	strftime (cDateBuffer, 50, sFormat->str, pTime);
	g_string_free (sFormat, TRUE);
	
	
	cairo_save (pSourceContext);
	
	cairo_set_tolerance (pSourceContext, 0.1);
	
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	
	
	PangoLayout *pLayout = pango_cairo_create_layout (pSourceContext);
	PangoFontDescription *pDesc = pango_font_description_new ();
	
	pango_font_description_set_absolute_size (pDesc, g_iLabelSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, g_cLabelPolice);
	pango_font_description_set_weight (pDesc, g_iLabelWeight);
	pango_font_description_set_style (pDesc, g_iLabelStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	pango_layout_set_text (pLayout, cDateBuffer, -1);
	//g_print ("%s\n", cDateBuffer);
	
	
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ink.width + 2,
		ink.height + 2);
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	cairo_set_source_rgb (pCairoContext, 0.0, 0.0, 0.5);
	cairo_translate (pCairoContext, -ink.x, -ink.y);
	
	pango_cairo_show_layout (pCairoContext, pLayout);
	
	//double fTextXOffset = log.width / 2. - ink.x;
	//double fTextYOffset = log.height     - ink.y;
	double fMaxScale = 1 + g_fAmplitude;
	
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	cairo_scale (pSourceContext, my_pIcon->fWidth * fMaxScale / ink.width, my_pIcon->fHeight * fMaxScale / ink.height);
	cairo_set_source_surface (pSourceContext,
		pNewSurface,
		0,
		0);
	cairo_paint (pSourceContext);
	
	cairo_destroy (pCairoContext);
	
	g_object_unref (pLayout);
	cairo_restore (pSourceContext);
}



void draw_background (cairo_t* pDrawingContext, int iWidth, int iHeight)
{
	/* clear context */
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) my_DimensionData.width,
		(double) iHeight / (double) my_DimensionData.height);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	cairo_paint (pDrawingContext);
	
	/* draw stuff */
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_DROP_SHADOW], pDrawingContext);
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_FACE], pDrawingContext);
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_MARKS], pDrawingContext);
}

void draw_foreground (cairo_t* pDrawingContext, int iWidth, int iHeight)
{
	/* clear context */
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) my_DimensionData.width,
		(double) iHeight / (double) my_DimensionData.height);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	cairo_paint (pDrawingContext);
	
	/* draw stuff */
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_FACE_SHADOW], pDrawingContext);
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_GLASS], pDrawingContext);
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_FRAME], pDrawingContext);
}

cairo_surface_t* update_surface (cairo_surface_t* pOldSurface,
								cairo_t* pSourceContext,
								int iWidth,
								int iHeight,
								SurfaceKind kind)
{
	cairo_surface_t* pNewSurface = NULL;
	cairo_t* pDrawingContext = NULL;
	
	if (pOldSurface != NULL)
		cairo_surface_destroy (pOldSurface);
	pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iWidth,
		iHeight);
	if (cairo_surface_status (pNewSurface) != CAIRO_STATUS_SUCCESS)
		return NULL;
	
	pDrawingContext = cairo_create (pNewSurface);
	if (cairo_status (pDrawingContext) != CAIRO_STATUS_SUCCESS)
		return NULL;
	
	switch (kind)
	{
		case KIND_BACKGROUND :
			draw_background (pDrawingContext, iWidth, iHeight);
		break;
		
		case KIND_FOREGROUND :
			draw_foreground (pDrawingContext, iWidth, iHeight);
		break;
	}
	
	cairo_destroy (pDrawingContext);
	
	return pNewSurface;
}


void cd_clock_draw_old_fashionned_clock (cairo_t *pSourceContext, int width, int height, double fMaxScale, struct tm *pTime)
{
	//g_print ("%s (%dx%d)\n", __func__, width, height);
	static double fHalfX;
	static double fHalfY;
	static double fShadowOffsetX = -0.75f;
	static double fShadowOffsetY = 0.75f;
	static cairo_text_extents_t textExtents;
	
	fHalfX = my_DimensionData.width / 2.0f;
	fHalfY = my_DimensionData.height / 2.0f;
	
	int g_iSeconds = pTime->tm_sec;
	int g_iMinutes = pTime->tm_min;
	int g_iHours = pTime->tm_hour;
	
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	
	cairo_set_source_surface (pSourceContext, g_pBackgroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
	
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	cairo_save (pSourceContext);
	cairo_scale (pSourceContext,
		(double) width / (double) my_DimensionData.width * fMaxScale,
		(double) height / (double) my_DimensionData.height * fMaxScale);
		
	cairo_translate (pSourceContext, fHalfX, fHalfY);
	cairo_rotate (pSourceContext, -G_PI/2.0f);
	
	if (my_bShowDate)
	{
		cairo_save (pSourceContext);
		cairo_set_source_rgb (pSourceContext, 1.0f, 0.5f, 0.0f);
		cairo_set_line_width (pSourceContext, 5.0f);
		cairo_text_extents (pSourceContext, cDateBuffer, &textExtents);
		cairo_rotate (pSourceContext, (G_PI/180.0f) * 90.0f);
		cairo_move_to (pSourceContext,
			-textExtents.width / 2.0f,
			2.0f * textExtents.height);
		
		strftime (cDateBuffer, 50, "%a%d%b", pTime);
		cairo_show_text (pSourceContext, cDateBuffer);
		cairo_restore (pSourceContext);
	}
	
	cairo_save (pSourceContext);
	cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (pSourceContext, (G_PI/ 12.0f * g_iHours + (G_PI/ 720.0f) * g_iMinutes));

	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_HOUR_HAND_SHADOW], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	cairo_save (pSourceContext);
	cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iMinutes);
	
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_MINUTE_HAND_SHADOW], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	if (my_bShowSeconds)
	{
		cairo_save (pSourceContext);
		cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
		cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iSeconds);
		
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_SECOND_HAND_SHADOW], pSourceContext);
		
		cairo_restore (pSourceContext);
	}
	
	cairo_save (pSourceContext);
	cairo_rotate (pSourceContext, (g_iHours % 12) * G_PI/6 + g_iMinutes * G_PI/360.0f);
	
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_HOUR_HAND], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	cairo_save (pSourceContext);
	cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iMinutes);
	
	rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_MINUTE_HAND], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	if (my_bShowSeconds)
	{
		cairo_save (pSourceContext);
		cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iSeconds);
		
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_SECOND_HAND], pSourceContext);
		cairo_restore (pSourceContext);
	}
	
	cairo_restore (pSourceContext);
	
	cairo_set_source_surface (pSourceContext, g_pForegroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
}
