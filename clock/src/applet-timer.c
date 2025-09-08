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

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-calendar.h"
#include "applet-draw.h"
#include "applet-timer.h"

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];


void cd_clock_free_alarm (CDClockAlarm *pAlarm)
{
	g_free (pAlarm->cMessage);
	g_free (pAlarm);
}

static void _dialog_destroyed (gpointer ptr)
{
	CDClockTask *pTask = (CDClockTask*)ptr;
	pTask->pWarningDialog = NULL;
}

static void _set_warning_repetition (int iClickedButton, GtkWidget *pInteractiveWidget, CDClockTask *pTask, CairoDialog *pDialog);
static gboolean _task_warning (CDClockTask *pTask, const gchar *cMessage)
{
	cd_debug ("%s (%s)", __func__, cMessage);
	GldiModuleInstance *myApplet = pTask->pApplet;

	GtkWidget *pScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 1, 60, 1);  // 1mn-60mn et 1 cran/mn.
	gtk_scale_set_digits (GTK_SCALE (pScale), 0);
	gtk_range_set_value (GTK_RANGE (pScale), pTask->iWarningDelay != 0 ? pTask->iWarningDelay : 15);  // 15mn par defaut.
	g_object_set (pScale, "width-request", CAIRO_DIALOG_MIN_SCALE_WIDTH, NULL);

	GtkWidget *pExtendedWidget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *label = gtk_label_new (D_("1mn"));
	gtk_box_pack_start (GTK_BOX (pExtendedWidget), label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (pExtendedWidget), pScale, FALSE, FALSE, 0);
	label = gtk_label_new (D_("1h"));
	gtk_box_pack_start (GTK_BOX (pExtendedWidget), label, FALSE, FALSE, 0);
	
	if (pTask->pWarningDialog) gldi_object_unref (GLDI_OBJECT(pTask->pWarningDialog));
	
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	attr.cText = (gchar *)cMessage;
	attr.bUseMarkup = TRUE;
	attr.cImageFilePath = (gchar *)MY_APPLET_SHARE_DATA_DIR"/icon-task.png";
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) _set_warning_repetition;
	attr.pInteractiveWidget = pExtendedWidget;
	attr.pUserData = pTask;
	attr.pFreeDataFunc = _dialog_destroyed;
	attr.iTimeLength = (pTask->iWarningDelay != 0 ? MIN (pTask->iWarningDelay-.1, 15.) : 15) * 60e3;  // on laisse le dialogue visible le plus longtemps possible, jusqu'a 15mn.
	const gchar *cDefaultActionButtons[3] = {"ok", "cancel", NULL};
	attr.cButtonsImage = cDefaultActionButtons;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	pTask->pWarningDialog = gldi_dialog_new (&attr);
	
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
	cd_debug ("%s (%d)", __func__, iClickedButton);
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
	GldiModuleInstance *myApplet = pTask->pApplet;
	CD_APPLET_STOP_DEMANDING_ATTENTION;
}

static inline void _get_current_time (time_t epoch, GldiModuleInstance *myApplet)
{
	if (myConfig.cLocation != NULL)
	{
		if (!myData.tz)
		{
			myData.tz = g_time_zone_new_identifier (myConfig.cLocation);
			if (!myData.tz)
				cd_warning ("cannot parse time zone identifier: %s\n", myConfig.cLocation);
		}
	}
	
	GDateTime *now = myData.tz ? g_date_time_new_now (myData.tz) : g_date_time_new_now_local ();
	
	myData.currentTime.tm_sec  = g_date_time_get_second (now);
	myData.currentTime.tm_min  = g_date_time_get_minute (now);
	myData.currentTime.tm_hour = g_date_time_get_hour (now);
	myData.currentTime.tm_mday = g_date_time_get_day_of_month (now);
	myData.currentTime.tm_mon  = g_date_time_get_month (now) - 1;
	myData.currentTime.tm_year = g_date_time_get_year (now) - 1900;
	myData.currentTime.tm_wday = g_date_time_get_day_of_week (now);
	if (myData.currentTime.tm_wday == 7) myData.currentTime.tm_wday = 0; // Sunday
	myData.currentTime.tm_isdst = g_date_time_is_daylight_savings (now);
	// not used: tm_yday, tm_gmtoff, tm_zone, we leave these unset
	
	g_date_time_unref (now);
}

void cd_clock_init_time (GldiModuleInstance *myApplet)
{
	time_t epoch = (time_t) time (NULL);
	_get_current_time (epoch, myApplet);
}

static gchar *_make_missed_task_message (CDClockTask *pTask, GldiModuleInstance *myApplet)
{
	//g_print ("%s (%s)\n", __func__, pTask->cID);
	struct tm st;
	memset (&st, 0, sizeof (st));
	st.tm_min = pTask->iMinute;
	st.tm_hour = pTask->iHour;
	st.tm_mday = pTask->iDay;
	st.tm_mon = pTask->iMonth;
	st.tm_year = pTask->iYear - 1900;
	st.tm_sec = 0;
	st.tm_isdst = myData.currentTime.tm_isdst;
	char cDateBuffer[200+1];
	memset (cDateBuffer, 0, 200);
	const gchar *cFormat;
	if (myConfig.b24Mode)
		cFormat = "%a %d %b, %R";
	else
		cFormat = "%a %d %b, %I:%M %p";
	strftime (cDateBuffer, 200, cFormat, &st);
	return g_strdup_printf ("%s\n\n %s\n %s\n\n %s",
		D_("The following task has felt due:"),
		cDateBuffer,
		pTask->cTitle?pTask->cTitle:D_("No title"),
		pTask->cText?pTask->cText:"");
}
static void _on_next_missed_task (int iClickedButton, GtkWidget *pInteractiveWidget, GldiModuleInstance *myApplet, CairoDialog *pDialog)
{
	g_return_if_fail (myData.pMissedTasks != NULL);
	//g_print ("%s ()\n", __func__);
	
	// acknowledge this task
	CDClockTask *pTask = myData.pMissedTasks->data;
	pTask->bAcknowledged = TRUE;
	myData.pBackend->update_task (pTask, myApplet);
	
	// jump to next task.
	if (iClickedButton == -1 || iClickedButton == 1)  // 'enter' or 2nd button
	{
		myData.pMissedTasks = g_list_delete_link (myData.pMissedTasks, myData.pMissedTasks);
		if (myData.pMissedTasks != NULL)
		{
			// display next task.
			pTask = myData.pMissedTasks->data;
			//g_print ("display task '%s'\n", pTask->cID);
			gchar *cMessage = _make_missed_task_message (pTask, myApplet);
			gldi_dialog_set_message (pDialog, cMessage);
			g_free (cMessage);
			
			// remove 'next' button if no more task will follow.
			if (myData.pMissedTasks->next == NULL && pDialog->pButtons != NULL && pDialog->iNbButtons > 1)
			{
				// remove 'next' button
				cairo_surface_t *pSurface;
				GLuint iTexture;
				int i = 1;
				pSurface = pDialog->pButtons[i].pSurface;
				if (pSurface != NULL)
				{
					cairo_surface_destroy (pSurface);
					pDialog->pButtons[i].pSurface = NULL;
				}
				iTexture = pDialog->pButtons[i].iTexture;
				if (iTexture != 0)
				{
					_cairo_dock_delete_texture (iTexture);
					pDialog->pButtons[i].iTexture = 0;
				}
				pDialog->iNbButtons = 1;  // only the 'ok' button will stay.
				
				// transform 'cancel' into 'ok'
				i = 0;
				pDialog->pButtons[i].iDefaultType = 1;
			}
			gldi_object_ref (GLDI_OBJECT(pDialog));  // keep the dialog alive.
		}
	}
	else  // dismiss next missed tasks and let the dialog close itself.
	{
		g_list_free (myData.pMissedTasks);
		myData.pMissedTasks = NULL;
	}
}

gboolean cd_clock_update_with_time (GldiModuleInstance *myApplet)
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
			if (myData.iDateTexture != 0)
				_cairo_dock_delete_texture (myData.iDateTexture);
			
			double fScale = (double) iWidth / (double) myData.DimensionData.width;
			GldiTextDescription labelDescription;
			memset (&labelDescription, 0, sizeof (GldiTextDescription));
			gldi_text_description_set_font (&labelDescription, (gchar*)"Sans 8");  // casted and then set to null
			labelDescription.fColorStart.rgba.red = myConfig.fDateColor[0];
			labelDescription.fColorStart.rgba.green = myConfig.fDateColor[1];
			labelDescription.fColorStart.rgba.blue = myConfig.fDateColor[2];
			labelDescription.fColorStart.rgba.alpha = 1.;
			labelDescription.bNoDecorations = TRUE;
			cairo_surface_t *pDateSurface = cairo_dock_create_surface_from_text_full (s_cDateBuffer,
				&labelDescription,
				fScale,
				iWidth,
				&myData.iDateWidth, &myData.iDateHeight);
			//g_print ("date : %dx%d\n", myData.iDateWidth, myData.iDateHeight);
			myData.iDateTexture = cairo_dock_create_texture_from_surface (pDateSurface);
			cairo_surface_destroy (pDateSurface);
			labelDescription.cFont = NULL;
			gldi_text_description_reset (&labelDescription);
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
		///if (CD_APPLET_MY_CONTAINER_IS_OPENGL)  // on ne sait pas bien dessiner du texte, donc on le fait en cairo, et on transfere tout sur notre texture.
		///	cairo_dock_update_icon_texture (myIcon);
	}
	
	///CD_APPLET_REDRAW_MY_ICON;
	
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
					gldi_dialog_show_temporary (pAlarm->cMessage, myIcon, myContainer, 60e3);
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
		
		// display missed tasks.
		if (!myData.bTaskCheckedOnce)
		{
			myData.bTaskCheckedOnce = TRUE;
			myData.pMissedTasks = cd_clock_get_missed_tasks (myApplet);
		}
		if (myData.pMissedTasks != NULL)  // so if the dialog was closed before we could acknowledge all the tasks, it will re-open.
		{
			CDClockTask *pTask = myData.pMissedTasks->data;
			gchar *cMessage = _make_missed_task_message (pTask, myApplet);
			CairoDialogAttr attr;
			memset (&attr, 0, sizeof (CairoDialogAttr));
			attr.cText = cMessage;
			attr.bUseMarkup = TRUE;
			attr.cImageFilePath = (gchar *)MY_APPLET_SHARE_DATA_DIR"/icon-task.png";
			const gchar *cButtonsImage[3] = {"ok", NULL, NULL};
			if (myData.pMissedTasks->next != NULL)
			{
				cButtonsImage[0] = "cancel";
				cButtonsImage[1] = "next.png";
			}
			attr.cButtonsImage = cButtonsImage;
			attr.pActionFunc = (CairoDockActionOnAnswerFunc)_on_next_missed_task;
			attr.pUserData = myApplet;
			attr.pFreeDataFunc = NULL;
			attr.iTimeLength = 0;
			attr.pIcon = myIcon;
			attr.pContainer = myContainer;
			gldi_dialog_new (&attr);
			
			g_free (cMessage);
		}
		
		// display next task.
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
				// acknowledge this task
				myData.pNextTask->bAcknowledged = TRUE;
				myData.pBackend->update_task (myData.pNextTask, myApplet);
				
				// look for next task.
				myData.pNextTask = cd_clock_get_next_scheduled_task (myApplet);
			}
			else if (t < epoch + 15*60 && t >= epoch)
			{
				if (t < epoch + 60)
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
				else if (! myData.pNextTask->b15mnWarning)
				{
					//g_print ("15 mn warning\n");
					myData.pNextTask->b15mnWarning = TRUE;
					
					gchar *cText = g_strdup_printf ("%s\n<b>%s</b>\n %s",
						D_("This task will begin in 15 minutes:"),
						myData.pNextTask->cTitle?myData.pNextTask->cTitle:D_("No title"),
						myData.pNextTask->cText?myData.pNextTask->cText:"");
					
					CairoDialogAttr attr;
					memset (&attr, 0, sizeof (CairoDialogAttr));
					attr.cText = (gchar *)cText;
					attr.cImageFilePath = (gchar *)MY_APPLET_SHARE_DATA_DIR"/icon-task.png";
					attr.iTimeLength = 60e3;
					attr.bUseMarkup = TRUE;
					attr.pIcon = myIcon;
					attr.pContainer = myContainer;
					gldi_dialog_new (&attr);
					CD_APPLET_DEMANDS_ATTENTION (NULL, 60);
				}
			}
			
			// display next anniversary if it is scheduled in less than 1 day, because anniversary require time to prepare.
			if (myData.pNextAnniversary != NULL)
			{
				if (!myData.pNextAnniversary->b1DayWarning && ! myData.pNextAnniversary->bFirstWarning && ! myData.pNextAnniversary->b15mnWarning)
				{
					GDate* pCurrentDate = g_date_new_dmy (myData.currentTime.tm_mday, myData.currentTime.tm_mon + 1, myData.currentTime.tm_year+1900);
					GDate* pAnnivDate = g_date_new_dmy (myData.pNextAnniversary->iDay, myData.pNextAnniversary->iMonth + 1, myData.currentTime.tm_year+1900);
					gint iDaysToNextAnniversary = g_date_days_between (pCurrentDate, pAnnivDate);
					if (iDaysToNextAnniversary >= 0 && iDaysToNextAnniversary <= 1)
					{
						myData.pNextAnniversary->b1DayWarning = TRUE;
						gchar *cText = g_strdup_printf ("%s\n<b>%s</b>\n %s\n\n%s",
							iDaysToNextAnniversary == 0 ? D_("Today is the following anniversary:") : D_("Tomorrow is the following anniversary:"),
							myData.pNextAnniversary->cTitle ? myData.pNextAnniversary->cTitle : D_("No title"),
							myData.pNextAnniversary->cText  ? myData.pNextAnniversary->cText  : "",
							D_("Repeat this message every:"));
						_task_warning (myData.pNextAnniversary, cText);
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
}
