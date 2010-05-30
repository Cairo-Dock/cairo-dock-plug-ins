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
#include "applet-config.h"
#include "applet-digital.h" //Digital html like renderer
#include "applet-calendar.h"
#include "applet-draw.h"

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];


void cd_clock_free_alarm (CDClockAlarm *pAlarm)
{
	g_free (pAlarm->cMessage);
	g_free (pAlarm);
}

static void _set_warning_repetition (int iClickedButton, GtkWidget *pInteractiveWidget, CDClockTask *pTask, CairoDialog *pDialog);
static gboolean _task_warning (CDClockTask *pTask, const gchar *cMessage)
{
	cd_debug ("%s (%s)", __func__, cMessage);
	CairoDockModuleInstance *myApplet = pTask->pApplet;
	
	GtkWidget *pScale = gtk_hscale_new_with_range (1, 60, 1);  // 1mn-60mn et 1 cran/mn.
	gtk_scale_set_digits (GTK_SCALE (pScale), 0);
	gtk_range_set_value (GTK_RANGE (pScale), pTask->iWarningDelay != 0 ? pTask->iWarningDelay : 15);  // 15mn par defaut.
	gtk_widget_set (pScale, "width-request", CAIRO_DIALOG_MIN_SCALE_WIDTH, NULL);
	
	GtkWidget *pExtendedWidget = gtk_hbox_new (FALSE, 0);
	GtkWidget *label = gtk_label_new (D_("1mn"));
	GtkWidget *pAlign = gtk_alignment_new (1., 1., 0., 0.);
	gtk_container_add (GTK_CONTAINER (pAlign), label);
	gtk_box_pack_start (GTK_BOX (pExtendedWidget), pAlign, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (pExtendedWidget), pScale, FALSE, FALSE, 0);
	label = gtk_label_new (D_("1h"));
	pAlign = gtk_alignment_new (1., 1., 0., 0.);
	gtk_container_add (GTK_CONTAINER (pAlign), label);
	gtk_box_pack_start (GTK_BOX (pExtendedWidget), pAlign, FALSE, FALSE, 0);
	
	cairo_dock_dialog_unreference (pTask->pWarningDialog);
	myDialogs.dialogTextDescription.bUseMarkup = TRUE;
	pTask->pWarningDialog = cairo_dock_show_dialog_full (cMessage,
		myIcon, myContainer,
		(pTask->iWarningDelay != 0 ? MIN (pTask->iWarningDelay-.1, 15.) : 15) * 60e3,  // on laisse le dialogue visible le plus longtemps possible, jusqu'a 15mn.
		MY_APPLET_SHARE_DATA_DIR"/icon-task.png",
		pExtendedWidget,
		(CairoDockActionOnAnswerFunc) _set_warning_repetition,
		pTask,
		NULL);
	myDialogs.dialogTextDescription.bUseMarkup = FALSE;
	
	CD_APPLET_DEMANDS_ATTENTION (NULL, 3600);  // ~ 1h, pour si on loupe le dialogue.
	return TRUE;
}
static gboolean _task_warning_repeat (CDClockTask *pTask, const gchar *cMessage)
{
	gchar *cText = g_strdup_printf ("%s %d:%02d\n<b>%s</b>\n %s\n\n%s",
		D_("The following task was scheduled at"), pTask->iHour, pTask->iMinute,
		pTask->cTitle?pTask->cTitle:D_("No title"),
		pTask->cText?pTask->cText:"",
		D_("Repeat this message every:"));
	_task_warning (pTask, cText);
	g_free (cText);
	return TRUE;
}
static void _set_warning_repetition (int iClickedButton, GtkWidget *pInteractiveWidget, CDClockTask *pTask, CairoDialog *pDialog)
{
	g_print ("%s (%d)\n", __func__, iClickedButton);
	GList *cl = gtk_container_get_children (GTK_CONTAINER (pInteractiveWidget));
	g_return_if_fail (cl != NULL && cl->next != NULL);
	GtkWidget *pScale = cl->next->data;
	g_return_if_fail (pScale != NULL);
	
	int dt = gtk_range_get_value (GTK_RANGE (pScale));
	if (dt == 0 || (iClickedButton != 0 && iClickedButton != -1))
	{
		if (pTask->iSidWarning != 0)
		{
			g_source_remove (pTask->iSidWarning);
			pTask->iSidWarning = 0;
		}
	}
	else
	{
		if (pTask->iSidWarning != 0 && dt != pTask->iWarningDelay)
		{
			g_source_remove (pTask->iSidWarning);
			pTask->iSidWarning = 0;
		}
		if (pTask->iSidWarning == 0)
		{
			pTask->iSidWarning = g_timeout_add_seconds (dt*60, (GSourceFunc) _task_warning_repeat, pTask);
			pTask->iWarningDelay = dt;
		}
	}
	pTask->pWarningDialog = NULL;
	CairoDockModuleInstance *myApplet = pTask->pApplet;
	CD_APPLET_STOP_DEMANDING_ATTENTION;
}

static inline void _get_current_time (time_t epoch, CairoDockModuleInstance *myApplet)
{
	if (myConfig.cLocation != NULL)
	{
		g_setenv ("TZ", myConfig.cLocation, TRUE);
		tzset ();
	}
	localtime_r (&epoch, &myData.currentTime);
	if (myConfig.cLocation != NULL)
	{
		if (myData.cSystemLocation != NULL)
			g_setenv ("TZ", myData.cSystemLocation, TRUE);
		else
			g_unsetenv ("TZ");
	}
}

void cd_clock_init_time (CairoDockModuleInstance *myApplet)
{
	time_t epoch = (time_t) time (NULL);
	_get_current_time (epoch, myApplet);
}

gboolean cd_clock_update_with_time (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	//\________________ On recupere l'heure courante.
	time_t epoch = (time_t) time (NULL);
	_get_current_time (epoch, myApplet);
	
	//\________________ On change la date si necessaire.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	gboolean bNewDate = (myData.currentTime.tm_mday != myData.iLastCheckedDay || myData.currentTime.tm_mon != myData.iLastCheckedMonth || myData.currentTime.tm_year != myData.iLastCheckedYear);
	if (bNewDate)
	{
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a %d %b", &myData.currentTime);
		myData.iLastCheckedDay = myData.currentTime.tm_mday;
		myData.iLastCheckedMonth = myData.currentTime.tm_mon;
		myData.iLastCheckedYear = myData.currentTime.tm_year;
	}
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOldStyle && myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
	{
		if (bNewDate || myData.iDateTexture == 0)
		{
			strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a %d %b", &myData.currentTime);
			
			if (myData.iDateTexture != 0)
				_cairo_dock_delete_texture (myData.iDateTexture);
			
			double fScale = (double) iWidth / (double) myData.DimensionData.width;
			CairoDockLabelDescription labelDescription;
			labelDescription.iSize = 10;
			labelDescription.cFont = (gchar*)"Sans";  // on peut caster car on ne liberera rien.
			labelDescription.iWeight = cairo_dock_get_pango_weight_from_1_9 (5);
			labelDescription.iStyle = PANGO_STYLE_NORMAL;
			labelDescription.fColorStart[0] = myConfig.fDateColor[0];
			labelDescription.fColorStart[1] = myConfig.fDateColor[1];
			labelDescription.fColorStart[2] = myConfig.fDateColor[2];
			memcpy (&labelDescription.fColorStop[0], &labelDescription.fColorStart[0], sizeof (labelDescription.fColorStop));
			labelDescription.fBackgroundColor[3] = 0;
			labelDescription.bOutlined = FALSE;
			labelDescription.iMargin = 0;
			cairo_surface_t *pDateSurface = cairo_dock_create_surface_from_text_full (s_cDateBuffer,
				&labelDescription,
				fScale,
				iWidth,
				&myData.iDateWidth, &myData.iDateHeight,
				NULL, NULL);
			myData.iDateWidth *= fScale;
			myData.iDateHeight *= fScale;
			//g_print ("date : %dx%d\n", myData.iDateWidth, myData.iDateHeight);
			myData.iDateTexture = cairo_dock_create_texture_from_surface (pDateSurface);
			cairo_surface_destroy (pDateSurface);
		}
	}
	if (bNewDate && myConfig.iShowDate == CAIRO_DOCK_INFO_ON_LABEL)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (s_cDateBuffer);
	}
	
	//\________________ On dessine avec cette heure.
	myData.iSmoothAnimationStep = 0;
	if (myConfig.bOldStyle)
	{
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cd_clock_render_analogic_to_texture (myApplet, iWidth, iHeight, &myData.currentTime, 0.);
		else
			cd_clock_draw_analogic (myApplet, iWidth, iHeight, &myData.currentTime);
	}
	else
	{
		cd_clock_draw_text (myApplet, iWidth, iHeight, &myData.currentTime);
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)  // on ne sait pas bien dessiner du texte, donc on le fait en cairo, et on transfere tout sur notre texture.
			cairo_dock_update_icon_texture (myIcon);
	}
	
	if (myDock && ! CD_APPLET_MY_CONTAINER_IS_OPENGL)  // les reflets pour cairo.
	{
		CD_APPLET_UPDATE_REFLECT_ON_MY_ICON;
	}
	
	//\________________ On redessine notre icone.
	CD_APPLET_REDRAW_MY_ICON;
	
	//\________________ On teste les alarmes et les taches.
	if (!myConfig.bShowSeconds || myData.currentTime.tm_min != myData.iLastCheckedMinute)  // un g_timeout de 1min ne s'effectue pas forcement a exectement 1 minute d'intervalle, et donc pourrait "sauter" la minute de l'alarme, d'ou le test sur bShowSeconds dans le cas ou l'applet ne verifie que chaque minute.
	{
		myData.iLastCheckedMinute = myData.currentTime.tm_min;
		
		// les alarmes.
		CDClockAlarm *pAlarm;
		guint i;
		for (i = 0; i < myConfig.pAlarms->len; i ++)
		{
			pAlarm = g_ptr_array_index (myConfig.pAlarms, i);
			
			if (myData.currentTime.tm_hour == pAlarm->iHour && myData.currentTime.tm_min == pAlarm->iMinute)
			{
				gboolean bShowAlarm = FALSE, bRemoveAlarm = FALSE;
				if (pAlarm->iDayOfWeek > 0)
				{
					if (pAlarm->iDayOfWeek == 1)
						bShowAlarm = TRUE;
					else if (pAlarm->iDayOfWeek - 1 == myData.currentTime.tm_wday)
						bShowAlarm = TRUE;
					else if (myData.currentTime.tm_wday == 0 || myData.currentTime.tm_wday == 6)  // week-end
					{
						if (pAlarm->iDayOfWeek == 9)
							bShowAlarm = TRUE;
					}
					else if (pAlarm->iDayOfWeek == 8)
						bShowAlarm = TRUE;
				}
				else if (pAlarm->iDayOfMonth > 0)
					bShowAlarm = (pAlarm->iDayOfMonth - 1 == myData.currentTime.tm_mday);
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
							cd_warning ("clock : when trying to execute '%s' : %s", pAlarm->cCommand, erreur->message);
							g_error_free (erreur);
							myData.iAlarmPID = 0;
						}
						g_strfreev (argv);
						cd_message (" --> child_pid : %d", myData.iAlarmPID);
					}
				}
				
				if (bRemoveAlarm)
				{
					cd_message ("Cette alarme ne sera pas repetee");
					g_ptr_array_remove_index (myConfig.pAlarms, i);
					cd_clock_free_alarm (pAlarm);
					/// A FAIRE : effacer l'heure dans le fichier de conf pour cette alarme.
				}
			}
		}
		
		// les taches.
		if (myData.pNextTask != NULL)
		{
			//g_print ("next task : %s\n", myData.pNextTask->cTitle);
			struct tm st;
			st.tm_min = myData.pNextTask->iMinute;
			st.tm_hour = myData.pNextTask->iHour;
			st.tm_mday = myData.pNextTask->iDay;
			st.tm_mon = myData.pNextTask->iMonth;
			st.tm_year = myData.pNextTask->iYear - 1900;
			st.tm_sec = 0;
			st.tm_isdst = myData.currentTime.tm_isdst;
			time_t t = mktime (&st);
			//g_print ("time : %ld, task : %ld\n", epoch, t);
			if (t < epoch)  // la tache est depassee.
			{
				myData.pNextTask = cd_clock_get_next_scheduled_task (myApplet);
			}
			else if (t < epoch + 15*60 && t >= epoch)
			{
				if (! myData.pNextTask->b15mnWarning)
				{
					//g_print ("15 mn warning\n");
					myData.pNextTask->b15mnWarning = TRUE;
					cairo_dock_show_temporary_dialog_with_icon_printf ("%s\n<b>%s</b>\n %s",
						myIcon, myContainer,
						60e3,
						MY_APPLET_SHARE_DATA_DIR"/icon-task.png",
						D_("This task will begin in 15 minutes:"),
						myData.pNextTask->cTitle?myData.pNextTask->cTitle:D_("No title"),
						myData.pNextTask->cText?myData.pNextTask->cText:"");
					CD_APPLET_DEMANDS_ATTENTION (NULL, 60);
				}
				else if (t < epoch + 60)
				{
					if (! myData.pNextTask->bFirstWarning)
					{
						//g_print ("first warning\n");
						myData.pNextTask->bFirstWarning = TRUE;
						gchar *cText = g_strdup_printf ("%s\n<b>%s</b>\n %s\n\n%s",
							D_("It's time for the following task:"),
							myData.pNextTask->cTitle?myData.pNextTask->cTitle:D_("No title"),
							myData.pNextTask->cText?myData.pNextTask->cText:"",
							D_("Repeat this message every:"));
						_task_warning (myData.pNextTask, cText);
						g_free (cText);
					}
				}
			}
			
			if (myData.pNextAnniversary != NULL)
			{
				if (!myData.pNextAnniversary->b1DayWarning && ! myData.pNextAnniversary->bFirstWarning && ! myData.pNextTask->b15mnWarning)
				{
					GDate* pCurrentDate = g_date_new_dmy (myData.currentTime.tm_mday, myData.currentTime.tm_mon + 1, myData.currentTime.tm_year+1900);
					GDate* pAnnivDate = g_date_new_dmy (myData.pNextAnniversary->iDay, myData.pNextAnniversary->iMonth + 1, myData.currentTime.tm_year+1900);
					gint iDaysToNextAnniversary = g_date_days_between (pCurrentDate, pAnnivDate);
					if (iDaysToNextAnniversary >= 0 && iDaysToNextAnniversary <= 1)
					{
						myData.pNextAnniversary->b1DayWarning = TRUE;
						gchar *cText = g_strdup_printf ("%s\n<b>%s</b>\n %s\n\n%s",
							iDaysToNextAnniversary == 0 ? D_("Today is the following anniversary:") : D_("Tomorrow is the following anniversary:"),
							myData.pNextTask->cTitle?myData.pNextTask->cTitle:D_("No title"),
							myData.pNextTask->cText?myData.pNextTask->cText:"",
							D_("Repeat this message every:"));
						_task_warning (myData.pNextTask, cText);
						g_free (cText);
						myData.pNextAnniversary = cd_clock_get_next_anniversary (myApplet);
					}
					g_date_free (pCurrentDate);
					g_date_free (pAnnivDate);
				}
			}
		}
	}
	
	CD_APPLET_LEAVE(TRUE);
	//return TRUE;
}

/*void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime) {
	cd_clock_draw_frames (myApplet);
	cd_clock_put_text_on_frames (myApplet, width, height, fMaxScale, pTime);
}*/

#define GAP 2
#define MAX_RATIO 2.
void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	GString *sFormat = g_string_new ("");
	
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
	
	if (myConfig.b24Mode)
	{
		if (myConfig.bShowSeconds)
			g_string_assign (sFormat, "%T");
		else
			g_string_assign (sFormat, "%R");
	}
	else
	{
		if (myConfig.bShowSeconds)
			g_string_assign (sFormat, "%r");  // equivalent a %I:%M:%S %p
		else
			g_string_printf (sFormat, "%%I:%%M %%p");
	}
	
	strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, sFormat->str, pTime);
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
		
		double h = ink.height + ink2.height + GAP;
		double fZoomX = MIN ((double) (iWidth - 1) / ink.width, (double) (iWidth - 1) / ink2.width);
		double fZoomY = (double) iHeight / h;
		if (myDock && fZoomY > MAX_RATIO * fZoomX)  // on ne garde pas le ratio car ca ferait un texte trop petit en hauteur, toutefois on limite un peu la deformation en hauteur.
			fZoomY = MAX_RATIO * fZoomX;
		
		if (fZoomX * MAX (ink.width, ink2.width) > myConfig.fTextRatio * iWidth)
		{
			fZoomY *= myConfig.fTextRatio * iWidth / (MAX (ink.width, ink2.width) * fZoomX);
			fZoomX = myConfig.fTextRatio * iWidth / MAX (ink.width, ink2.width);
		}
		
		cairo_translate (myDrawContext, (iWidth - fZoomX * ink.width) / 2, (iHeight - fZoomY * h)/2);  // centre verticalement.
		cairo_scale (myDrawContext, fZoomX, fZoomY);
		cairo_translate (myDrawContext, -ink.x, -ink.y);
		pango_cairo_show_layout (myDrawContext, pLayout);
		
		cairo_restore (myDrawContext);
		cairo_save (myDrawContext);
		
		cairo_translate (myDrawContext, (iWidth - fZoomX * ink2.width) / 2, (iHeight + fZoomY * GAP)/2);
		cairo_scale (myDrawContext, fZoomX, fZoomY);
		cairo_translate (myDrawContext, -ink2.x, -ink2.y);
		pango_cairo_show_layout (myDrawContext, pLayout2);
		
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



void cd_clock_draw_analogic_opengl (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime, double fFraction)
{
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
	
	// heure
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
	
	// seconde
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
}

void cd_clock_render_analogic_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime, double fFraction)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
	
	cd_clock_draw_analogic_opengl (myApplet, iWidth, iHeight, pTime, fFraction);
	
	CD_APPLET_FINISH_DRAWING_MY_ICON;
}
