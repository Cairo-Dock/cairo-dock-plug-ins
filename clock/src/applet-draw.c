/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

**********************************************************************************/
#include <stdlib.h>
#define __USE_POSIX
#include <time.h>
#include <signal.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-digital.h" //Digital html like renderer
#include "applet-draw.h"

CD_APPLET_INCLUDE_MY_VARS


#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];


void cd_clock_free_alarm (CDClockAlarm *pAlarm)
{
	g_free (pAlarm->cMessage);
	g_free (pAlarm);
}


gboolean cd_clock_update_with_time (CairoDockModuleInstance *myApplet)
{
	static gboolean bBusy = FALSE;
	if (bBusy)
		return TRUE;
	bBusy = TRUE;
	
	struct tm epoch_tm;
	time_t epoch = (time_t) time (NULL);
	if (myConfig.cLocation != NULL)
	{
		g_setenv ("TZ", myConfig.cLocation, TRUE);
		tzset ();
	}
	localtime_r (&epoch, &epoch_tm);
	if (myConfig.cLocation != NULL)
	{
		if (myData.cSystemLocation != NULL)
			g_setenv ("TZ", myData.cSystemLocation, TRUE);
		else
			g_unsetenv ("TZ");
	}
	
	Icon *icon = myIcon;
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	double fRatio = (myDock ? myDock->fRatio : 1);
	if (myConfig.bOldStyle)
		cd_clock_draw_old_fashionned_clock (myApplet, (int) icon->fWidth / fRatio, (int) icon->fHeight / fRatio, fMaxScale, &epoch_tm);
	else
		cd_clock_draw_text (myApplet, (int) icon->fWidth / fRatio, (int) icon->fHeight / fRatio, fMaxScale, &epoch_tm);
	
	if (myDock && myDock->bUseReflect)
	{
		cairo_surface_t *pReflet = icon->pReflectionBuffer;
		icon->pReflectionBuffer = NULL;
		cairo_surface_destroy (pReflet);
		
		icon->pReflectionBuffer = cairo_dock_create_reflection_surface (icon->pIconBuffer,
			myDrawContext,
			(myDock->bHorizontalDock ? icon->fWidth : icon->fHeight) / fRatio * (1 + g_fAmplitude),
			(myDock->bHorizontalDock ? icon->fHeight : icon->fWidth) / fRatio * (1 + g_fAmplitude),
			myDock->bHorizontalDock,
			1 + g_fAmplitude,
			myDock->bDirectionUp);
	}
	
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_LABEL && myConfig.cLocation == NULL && (epoch_tm.tm_mday != myData.iLastCheckedDay || epoch_tm.tm_mon != myData.iLastCheckedMonth || epoch_tm.tm_year != myData.iLastCheckedYear))
	{
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a %d %b", &epoch_tm);
		CD_APPLET_SET_NAME_FOR_MY_ICON (s_cDateBuffer);
		
		myData.iLastCheckedDay = epoch_tm.tm_mday;
		myData.iLastCheckedMonth = epoch_tm.tm_mon;
		myData.iLastCheckedYear = epoch_tm.tm_year;
	}
	
	CD_APPLET_REDRAW_MY_ICON;
	
	if (!myConfig.bShowSeconds || epoch_tm.tm_min != myData.iLastCheckedMinute)  // un g_timeout de 1min ne s'effectue pas forcement à exectement 1 minute d'intervalle, et donc pourrait "sauter" la minute de l'alarme, d'ou le test sur bShowSeconds dans le cas ou l'applet ne verifie que chaque minute.
	{
		myData.iLastCheckedMinute = epoch_tm.tm_min;
		CDClockAlarm *pAlarm;
		int i;
		for (i = 0; i < myConfig.pAlarms->len; i ++)
		{
			pAlarm = g_ptr_array_index (myConfig.pAlarms, i);
			
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
					cd_message ("Dring ! %s", pAlarm->cMessage);
					cairo_dock_show_temporary_dialog (pAlarm->cMessage, myIcon, myContainer, 60e3);
					if (pAlarm->cCommand != NULL)
					{
						if (myData.iAlarmPID > 0)
						{
							kill (myData.iAlarmPID, 1);
							myData.iAlarmPID = 0;
						}
						GError *erreur = NULL;
						gchar **argv = g_strsplit (pAlarm->cCommand, " ", -1);
						g_spawn_async (NULL,
							argv,
							NULL,
							0,
							NULL,
							NULL,
							&myData.iAlarmPID,
							&erreur);
						if (erreur != NULL)
						{
							cd_warning ("Attention : when trying to execute '%s' : %s", pAlarm->cCommand, erreur->message);
							g_error_free (erreur);
							myData.iAlarmPID = 0;
						}
						g_strfreev (argv);
						cd_message (" --> child_pid : %d", myData.iAlarmPID);
					}
				}
				
				if (bRemoveAlarm)
				{
					cd_message ("Cette alarme ne sera pas répétée");
					g_ptr_array_remove_index (myConfig.pAlarms, i);
					cd_clock_free_alarm (pAlarm);
					/// A FAIRE : effacer l'heure dans le fichier de conf pour cette alarme.
				}
			}
		}
	}
	
	bBusy = FALSE;
	return TRUE;
}

/*void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime) {
	cd_clock_draw_frames (myApplet);
	cd_clock_put_text_on_frames (myApplet, width, height, fMaxScale, pTime);
}*/

void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime)
{
	cairo_t *pSourceContext = myDrawContext;
	GString *sFormat = g_string_new ("");
	
	if (myConfig.b24Mode)
	{
		if (myConfig.bShowSeconds)
			g_string_printf (sFormat, "%%T");
		else
			g_string_printf (sFormat, " %%R");
	}
	else
	{
		if (myConfig.bShowSeconds)
			g_string_printf (sFormat, "%%r%s", pTime->tm_hour > 12 ? "PM" : "AM");
		else
			g_string_printf (sFormat, "%%I:%%M%s", pTime->tm_hour > 12 ? "PM" : "AM");
	}
	
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
		g_string_append (sFormat, "\n%a %d %b");
	
	strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, sFormat->str, pTime);
	g_string_free (sFormat, TRUE);
	
	cairo_set_tolerance (pSourceContext, 0.5);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	
	PangoLayout *pLayout = pango_cairo_create_layout (pSourceContext);
	PangoFontDescription *pDesc = pango_font_description_new ();
	
	pango_font_description_set_absolute_size (pDesc, g_iconTextDescription.iSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, myConfig.cFont);
	pango_font_description_set_weight (pDesc, g_iconTextDescription.iWeight);
	pango_font_description_set_style (pDesc, g_iconTextDescription.iStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	pango_layout_set_text (pLayout, s_cDateBuffer, -1);
	//g_print ("%s\n", s_cDateBuffer);
	
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	cairo_surface_t *pNewSurface = cairo_surface_create_similar (cairo_get_target (pSourceContext),
		CAIRO_CONTENT_COLOR_ALPHA,
		ink.width + 2,
		ink.height + 2);
	cairo_t *pCairoContext = cairo_create (pNewSurface);
	cairo_set_source_rgba (pCairoContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2], myConfig.fTextColor[3]);
	cairo_translate (pCairoContext, -ink.x, -ink.y);
	
	pango_cairo_show_layout (pCairoContext, pLayout);
	cairo_destroy (pCairoContext);
	
	//double fTextXOffset = log.width / 2. - ink.x;
	//double fTextYOffset = log.height     - ink.y;
	
	cairo_save (pSourceContext);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	cairo_scale (pSourceContext, width * fMaxScale / ink.width, height * fMaxScale / ink.height);
	cairo_set_source_surface (pSourceContext,
		pNewSurface,
		0,
		0);
	cairo_paint (pSourceContext);
	cairo_restore (pSourceContext);
	
	cairo_surface_destroy (pNewSurface);
	g_object_unref (pLayout);
}



void draw_background (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext, int iWidth, int iHeight)
{
	//g_print ("%s (%.2f, %.2f)\n", __func__, (double) iWidth / (double) myConfig.DimensionData.width, (double) iHeight / (double) myConfig.DimensionData.height);
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) myData.DimensionData.width,
		(double) iHeight / (double) myData.DimensionData.height);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_paint (pDrawingContext);
	
	if (myData.pSvgHandles[CLOCK_DROP_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_DROP_SHADOW], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_FACE] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_FACE], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_MARKS] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MARKS], pDrawingContext);
}

void draw_foreground (CairoDockModuleInstance *myApplet, cairo_t* pDrawingContext, int iWidth, int iHeight)
{
	//g_print ("%s (%.2f, %.2f)\n", __func__, (double) iWidth / (double) myConfig.DimensionData.width, (double) iHeight / (double) myConfig.DimensionData.height);
	cairo_scale (pDrawingContext,
		(double) iWidth / (double) myData.DimensionData.width,
		(double) iHeight / (double) myData.DimensionData.height);
	cairo_set_source_rgba (pDrawingContext, 1.0f, 1.0f, 1.0f, 0.0f);
	cairo_paint (pDrawingContext);
	
	if (myData.pSvgHandles[CLOCK_FACE_SHADOW] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_FACE_SHADOW], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_GLASS] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_GLASS], pDrawingContext);
	if (myData.pSvgHandles[CLOCK_FRAME] != NULL)
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_FRAME], pDrawingContext);
}

cairo_surface_t* update_surface (CairoDockModuleInstance *myApplet,
	cairo_surface_t* pOldSurface,
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
	cairo_set_operator (pDrawingContext, CAIRO_OPERATOR_OVER);
	
	switch (kind)
	{
		case KIND_BACKGROUND :
			draw_background (myApplet, pDrawingContext, iWidth, iHeight);
		break;
		
		case KIND_FOREGROUND :
			draw_foreground (myApplet, pDrawingContext, iWidth, iHeight);
		break;
	}
	
	cairo_destroy (pDrawingContext);
	
	return pNewSurface;
}


void cd_clock_draw_old_fashionned_clock (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime)
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
	
	int g_iSeconds = pTime->tm_sec;
	int g_iMinutes = pTime->tm_min;
	int g_iHours = pTime->tm_hour;
	
	//cairo_set_tolerance (pSourceContext, 0.1);
	cairo_set_source_rgba (pSourceContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (pSourceContext);
	cairo_set_operator (pSourceContext, CAIRO_OPERATOR_OVER);
	cairo_save (pSourceContext);
	
	cairo_set_source_surface (pSourceContext, myData.pBackgroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
	
	cairo_scale (pSourceContext,
		(double) width / (double) myData.DimensionData.width * fMaxScale,
		(double) height / (double) myData.DimensionData.height * fMaxScale);
		
	cairo_translate (pSourceContext, fHalfX, fHalfY);
	
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		cairo_save (pSourceContext);
		cairo_set_source_rgb (pSourceContext, 1.0f, 0.5f, 0.0f);
		cairo_set_line_width (pSourceContext, 8.0f);
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a%d%b", pTime);
		cairo_text_extents (pSourceContext, s_cDateBuffer, &textExtents);
		cairo_move_to (pSourceContext,
			-textExtents.width / 2.0f,
			2.0f * textExtents.height);
		
		cairo_show_text (pSourceContext, s_cDateBuffer);
		cairo_restore (pSourceContext);
	}
	
	cairo_rotate (pSourceContext, -G_PI/2.0f);
	cairo_save (pSourceContext);
	cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (pSourceContext, (G_PI/ 12.0f * g_iHours + (G_PI/ 360.0f) * g_iMinutes));

	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND_SHADOW], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	cairo_save (pSourceContext);
	cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
	cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iMinutes);
	
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND_SHADOW], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	if (myConfig.bShowSeconds)
	{
		cairo_save (pSourceContext);
		cairo_translate (pSourceContext, fShadowOffsetX, fShadowOffsetY);
		cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iSeconds);
		
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND_SHADOW], pSourceContext);
		
		cairo_restore (pSourceContext);
	}
	
	cairo_save (pSourceContext);
	cairo_rotate (pSourceContext, (g_iHours % 12) * G_PI/6 + g_iMinutes * G_PI/360.0f);
	
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_HOUR_HAND], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	cairo_save (pSourceContext);
	cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iMinutes);
	
	rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_MINUTE_HAND], pSourceContext);
	
	cairo_restore (pSourceContext);
	
	if (myConfig.bShowSeconds)
	{
		cairo_save (pSourceContext);
		cairo_rotate (pSourceContext, (G_PI/30.0f) * g_iSeconds);
		
		rsvg_handle_render_cairo (myData.pSvgHandles[CLOCK_SECOND_HAND], pSourceContext);
		cairo_restore (pSourceContext);
	}
	
	cairo_set_source_surface (pSourceContext, myData.pForegroundSurface, 0.0f, 0.0f);
	cairo_paint (pSourceContext);
	
	cairo_restore (pSourceContext);
}
