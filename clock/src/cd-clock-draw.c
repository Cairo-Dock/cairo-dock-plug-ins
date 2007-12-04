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


extern Icon *myIcon;
extern cairo_t *myDrawContext;
extern CairoDock *myDock;

extern gboolean my_bShowDate;
extern gboolean my_bShowSeconds;
extern gboolean my_b24Mode;
extern int my_bOldStyle;
extern double my_fTextColor[4];

extern cairo_surface_t *my_pBackgroundSurface;
extern cairo_surface_t *my_pForegroundSurface;
extern RsvgDimensionData my_DimensionData;
extern RsvgHandle *my_pSvgHandles[CLOCK_ELEMENTS];
extern GPtrArray *my_pAlarms;

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];


void cd_clock_free_alarm (CDClockAlarm *pAlarm)
{
	g_free (pAlarm->cMessage);
	g_free (pAlarm);
}


gboolean cd_clock_update_with_time (Icon *icon)
{
	static gboolean bBusy = FALSE;
	static int iLastCheckedMinute = -1;
	static struct tm epoch_tm;
	
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
	
	time_t epoch = (time_t) time (NULL);
	localtime_r (&epoch, &epoch_tm);
	
	if (my_bOldStyle)
		cd_clock_draw_old_fashionned_clock (myDrawContext, (int) icon->fWidth, (int) icon->fHeight, (1 + g_fAmplitude), &epoch_tm);
	else
		cd_clock_draw_text (myDrawContext, &epoch_tm);
	
	if (myDock->bUseReflect)
	{
		cairo_surface_t *pReflet = icon->pReflectionBuffer;
		icon->pReflectionBuffer = NULL;
		cairo_surface_destroy (pReflet);
		
		icon->pReflectionBuffer = cairo_dock_create_reflection_surface (icon->pIconBuffer,
			myDrawContext,
			(myDock->bHorizontalDock ? icon->fWidth : icon->fHeight) * (1 + g_fAmplitude),
			(myDock->bHorizontalDock ? icon->fHeight : icon->fWidth) * (1 + g_fAmplitude),
			myDock->bHorizontalDock);
	}
	
	
	cairo_dock_redraw_my_icon (icon, myDock);
	
	if (!my_bShowSeconds || epoch_tm.tm_min != iLastCheckedMinute)  // un g_timeout de 1min ne s'effectue pas forcement à exectement 1 minute d'intervalle, et donc pourrait "sauter" la minute de l'alarme, d'ou le test sur my_bShowSeconds dans le cas ou l'applet ne verifie que chaque minute.
	{
		iLastCheckedMinute = epoch_tm.tm_min;
		CDClockAlarm *pAlarm;
		int i;
		for (i = 0; i < my_pAlarms->len; i ++)
		{
			pAlarm = g_ptr_array_index (my_pAlarms, i);
			
			if (epoch_tm.tm_hour == pAlarm->iHour && epoch_tm.tm_min == pAlarm->iMinute)
			{
				gboolean bShowAlarm = FALSE, bRemoveAlarm = FALSE;
				if (pAlarm->iDayOfWeek > 0)
				{
					if (pAlarm->iDayOfWeek == 1)
						bShowAlarm = TRUE;
					else if (pAlarm->iDayOfWeek - 1 == epoch_tm.tm_wday)
						bShowAlarm = TRUE;
					else if (epoch_tm.tm_wday == 0 || epoch_tm.tm_wday == 6)  // week-end
					{
						if (pAlarm->iDayOfWeek == 9)
							bShowAlarm = TRUE;
					}
					else if (pAlarm->iDayOfWeek == 8)
						bShowAlarm = TRUE;
				}
				else if (pAlarm->iDayOfMonth > 0)
					bShowAlarm = (pAlarm->iDayOfMonth - 1 == epoch_tm.tm_mday);
				else  // c'est une alarme qui ne se repete pas.
				{
					bShowAlarm = TRUE;
					bRemoveAlarm = TRUE;
				}
				
				if (bShowAlarm)
				{
					g_print ("Dring ! %s\n", pAlarm->cMessage);
					cairo_dock_show_temporary_dialog (pAlarm->cMessage, myIcon, myDock, 60e3);
				}
				
				if (bRemoveAlarm)
				{
					g_print ("Cette alarme ne sera pas répétée\n");
					g_ptr_array_remove_index (my_pAlarms, i);
					cd_clock_free_alarm (pAlarm);
					/// A FAIRE : effacer l'heure dans le fichier de conf pour cette alarme.
				}
			}
		}
	}
	
	bBusy = FALSE;
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
	
	strftime (cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, sFormat->str, pTime);
	g_string_free (sFormat, TRUE);
	
	
	cairo_set_tolerance (pSourceContext, 0.5);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	
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
	cairo_set_source_rgba (pCairoContext, my_fTextColor[0], my_fTextColor[1], my_fTextColor[2], my_fTextColor[3]);
	cairo_translate (pCairoContext, -ink.x, -ink.y);
	
	pango_cairo_show_layout (pCairoContext, pLayout);
	cairo_destroy (pCairoContext);
	
	//double fTextXOffset = log.width / 2. - ink.x;
	//double fTextYOffset = log.height     - ink.y;
	double fMaxScale = 1 + g_fAmplitude;
	
	cairo_save (pSourceContext);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	cairo_scale (pSourceContext, myIcon->fWidth * fMaxScale / ink.width, myIcon->fHeight * fMaxScale / ink.height);
	cairo_set_source_surface (pSourceContext,
		pNewSurface,
		0,
		0);
	cairo_paint (pSourceContext);
	cairo_restore (pSourceContext);
	
	cairo_surface_destroy (pNewSurface);
	g_object_unref (pLayout);
}



void draw_background (cairo_t* pDrawingContext, int iWidth, int iHeight)
{
	//g_print ("%s (%.2f, %.2f)\n", __func__, (double) iWidth / (double) my_DimensionData.width, (double) iHeight / (double) my_DimensionData.height);
	/* clear context */
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) my_DimensionData.width,
		(double) iHeight / (double) my_DimensionData.height);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	cairo_paint (pDrawingContext);
	
	/* draw stuff */
	if (my_pSvgHandles[CLOCK_DROP_SHADOW] != NULL)
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_DROP_SHADOW], pDrawingContext);
	if (my_pSvgHandles[CLOCK_FACE] != NULL)
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_FACE], pDrawingContext);
	if (my_pSvgHandles[CLOCK_MARKS] != NULL)
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_MARKS], pDrawingContext);
}

void draw_foreground (cairo_t* pDrawingContext, int iWidth, int iHeight)
{
	//g_print ("%s (%.2f, %.2f)\n", __func__, (double) iWidth / (double) my_DimensionData.width, (double) iHeight / (double) my_DimensionData.height);
	/* clear context */
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) my_DimensionData.width,
		(double) iHeight / (double) my_DimensionData.height);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	cairo_paint (pDrawingContext);
	
	/* draw stuff */
	if (my_pSvgHandles[CLOCK_FACE_SHADOW] != NULL)
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_FACE_SHADOW], pDrawingContext);
	if (my_pSvgHandles[CLOCK_GLASS] != NULL)
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_GLASS], pDrawingContext);
	if (my_pSvgHandles[CLOCK_FRAME] != NULL)
		rsvg_handle_render_cairo (my_pSvgHandles[CLOCK_FRAME], pDrawingContext);
}

cairo_surface_t* update_surface (cairo_surface_t* pOldSurface,
								cairo_t* pSourceContext,
								int iWidth,
								int iHeight,
								SurfaceKind kind)
{
	//g_print ("%s (%dx%d)\n", __func__, iWidth, iHeight);
	cairo_surface_t* pNewSurface = NULL;
	cairo_t* pDrawingContext = NULL;
	
	if (pOldSurface != NULL)
		cairo_surface_destroy (pOldSurface);
	pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		iWidth,
		iHeight);
	g_return_val_if_fail (cairo_surface_status (pNewSurface) == CAIRO_STATUS_SUCCESS, NULL);
	
	pDrawingContext = cairo_create (pNewSurface);
	g_return_val_if_fail (cairo_status (pDrawingContext) == CAIRO_STATUS_SUCCESS, NULL);
	
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
	double fHalfX;
	double fHalfY;
	double fShadowOffsetX = -0.75f;
	double fShadowOffsetY = 0.75f;
	cairo_text_extents_t textExtents;
	
	fHalfX = my_DimensionData.width / 2.0f;
	fHalfY = my_DimensionData.height / 2.0f;
	
	int g_iSeconds = pTime->tm_sec;
	int g_iMinutes = pTime->tm_min;
	int g_iHours = pTime->tm_hour;
	
	//cairo_set_tolerance (pSourceContext, 0.1);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	cairo_set_source_surface (pSourceContext, my_pBackgroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
	
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
		cairo_set_line_width (pSourceContext, 8.0f);
		strftime (cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a%d%b", pTime);
		cairo_text_extents (pSourceContext, cDateBuffer, &textExtents);
		cairo_rotate (pSourceContext, (G_PI/180.0f) * 90.0f);
		cairo_move_to (pSourceContext,
			-textExtents.width / 2.0f,
			2.0f * textExtents.height);
		
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
	
	cairo_set_source_surface (pSourceContext, my_pForegroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
}
