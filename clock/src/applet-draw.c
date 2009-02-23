/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.
The analogic display comes from Cairo-Clock.

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

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];


void cd_clock_free_alarm (CDClockAlarm *pAlarm)
{
	g_free (pAlarm->cMessage);
	g_free (pAlarm);
}


gboolean cd_clock_update_with_time (CairoDockModuleInstance *myApplet)
{
	//\________________ On recupere l'heure courante.
	time_t epoch = (time_t) time (NULL);
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
	
	//\________________ On dessine avec cette heure.
	myData.iSmoothAnimationStep = 0;
	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	double fRatio = (myDock ? myDock->fRatio : 1);
	int iWidth = (int) myIcon->fWidth / fRatio * fMaxScale;
	int iHeight = (int) myIcon->fHeight / fRatio * fMaxScale;
	if (myConfig.bOldStyle)
	{
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cd_clock_render_analogic_to_texture (myApplet, iWidth, iHeight, &myData.currentTime);
		else
			cd_clock_draw_analogic (myApplet, iWidth, iHeight, &myData.currentTime);
	}
	else
	{
		cd_clock_draw_text (myApplet, iWidth, iHeight, &myData.currentTime);
	}
	
	if (!myConfig.bOldStyle)  // on ne gere pas encore le dessin de la vue numerique en opengl, donc on fait une copie a partir de la surface cairo.
	{
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
			cairo_dock_update_icon_texture (myIcon);
	}
	
	if (myDock && myDock->bUseReflect && (! CD_APPLET_MY_CONTAINER_IS_OPENGL || !myConfig.bOldStyle))  // les reflets pour cairo.
	{
		cairo_surface_destroy (myIcon->pReflectionBuffer);
		myIcon->pReflectionBuffer = cairo_dock_create_reflection_surface (myIcon->pIconBuffer,
			myDrawContext,
			(myDock->bHorizontalDock ? myIcon->fWidth : myIcon->fHeight) / fRatio * (1 + g_fAmplitude),
			(myDock->bHorizontalDock ? myIcon->fHeight : myIcon->fWidth) / fRatio * (1 + g_fAmplitude),
			myDock->bHorizontalDock,
			1 + g_fAmplitude,
			myDock->bDirectionUp);
	}
	
	//\________________ On change la date si necessaire.
	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_LABEL && myConfig.cLocation == NULL && (myData.currentTime.tm_mday != myData.iLastCheckedDay || myData.currentTime.tm_mon != myData.iLastCheckedMonth || myData.currentTime.tm_year != myData.iLastCheckedYear))
	{
		strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, "%a %d %b", &myData.currentTime);
		CD_APPLET_SET_NAME_FOR_MY_ICON (s_cDateBuffer);
		
		myData.iLastCheckedDay = myData.currentTime.tm_mday;
		myData.iLastCheckedMonth = myData.currentTime.tm_mon;
		myData.iLastCheckedYear = myData.currentTime.tm_year;
	}
	
	//\________________ On redessine notre icone.
	CD_APPLET_REDRAW_MY_ICON;
	
	//\_______________ On lance l'animation "smooth" si ce n'etait pas deja fait.
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myConfig.bOldStyle)
		cairo_dock_launch_animation (myContainer);
	
	//\________________ On teste les alarmes.
	if (!myConfig.bShowSeconds || myData.currentTime.tm_min != myData.iLastCheckedMinute)  // un g_timeout de 1min ne s'effectue pas forcement à exectement 1 minute d'intervalle, et donc pourrait "sauter" la minute de l'alarme, d'ou le test sur bShowSeconds dans le cas ou l'applet ne verifie que chaque minute.
	{
		myData.iLastCheckedMinute = myData.currentTime.tm_min;
		CDClockAlarm *pAlarm;
		int i;
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
	
	return TRUE;
}

/*void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime) {
	cd_clock_draw_frames (myApplet);
	cd_clock_put_text_on_frames (myApplet, width, height, fMaxScale, pTime);
}*/

void cd_clock_draw_text (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
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
	
	cairo_set_tolerance (myDrawContext, 0.5);
	cairo_set_source_rgba (myDrawContext, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	PangoFontDescription *pDesc = pango_font_description_new ();
	
	pango_font_description_set_absolute_size (pDesc, myLabels.iconTextDescription.iSize * PANGO_SCALE);
	pango_font_description_set_family_static (pDesc, myConfig.cFont);
	pango_font_description_set_weight (pDesc, myLabels.iconTextDescription.iWeight);
	pango_font_description_set_style (pDesc, myLabels.iconTextDescription.iStyle);
	pango_layout_set_font_description (pLayout, pDesc);
	pango_font_description_free (pDesc);
	
	pango_layout_set_text (pLayout, s_cDateBuffer, -1);
	//g_print ("%s\n", s_cDateBuffer);
	
	PangoRectangle ink, log;
	pango_layout_get_pixel_extents (pLayout, &ink, &log);
	
	cairo_save (myDrawContext);
	
	cairo_set_source_rgba (myDrawContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2], myConfig.fTextColor[3]);
	cairo_scale (myDrawContext, (double) iWidth / ink.width, (double) iHeight / ink.height);
	cairo_translate (myDrawContext, -ink.x, -ink.y);
	
	pango_cairo_show_layout (myDrawContext, pLayout);
	
	cairo_restore (myDrawContext);
	g_object_unref (pLayout);
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



void cd_clock_draw_analogic_opengl (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	int iSeconds = pTime->tm_sec;
	int iMinutes = pTime->tm_min;
	int iHours = pTime->tm_hour;
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // ne me demandez pas pourquoi...
	
	glEnable (GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glPolygonMode (GL_FRONT, GL_FILL);
	
	// draw texture bg
	glPushMatrix ();
	glScalef (iWidth, iHeight, 1.);
	cairo_dock_apply_texture (myData.iBgTexture);
	glPopMatrix ();
	
	// heure
	glPushMatrix ();
	glRotatef (-(iHours % 12 + iMinutes/60.) * 30. + 90., 0., 0., 1.);
	glTranslatef (myData.iNeedleWidth/2 - myData.fNeedleScale * myData.iNeedleOffsetX, 0., 0.);
	glScalef (myData.iNeedleWidth, myData.iNeedleHeight+1, 1.);
	cairo_dock_apply_texture (myData.iHourNeedleTexture);
	glPopMatrix ();
	
	// minute
	glPushMatrix ();
	glRotatef (-6. * (iMinutes + iSeconds/60.) + 90., 0., 0., 1.);
	glTranslatef (myData.iNeedleWidth/2 - myData.fNeedleScale * myData.iNeedleOffsetX, 0., 0.);
	glScalef (myData.iNeedleWidth, myData.iNeedleHeight+1, 1.);
	cairo_dock_apply_texture (myData.iMinuteNeedleTexture);
	glPopMatrix ();
	
	// seconde
	glPushMatrix ();
	glRotatef (-6. * (iSeconds + myData.iSmoothAnimationStep / 5.) + 90., 0., 0., 1.);
	glTranslatef (myData.iNeedleWidth/2 - myData.fNeedleScale * myData.iNeedleOffsetX, 0., 0.);
	glScalef (myData.iNeedleWidth, myData.iNeedleHeight+1, 1.);
	cairo_dock_apply_texture (myData.iSecondNeedleTexture);
	glPopMatrix ();
	
	// draw texture fg
	glScalef (iWidth, iHeight, 1.);
	cairo_dock_apply_texture (myData.iFgTexture);
	
	glDisable (GL_TEXTURE_2D);
	glDisable (GL_BLEND);
}

void cd_clock_render_analogic_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight, struct tm *pTime)
{
	if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
		return ;
	
	cd_clock_draw_analogic_opengl (myApplet, iWidth, iHeight, pTime);
	
	cairo_dock_end_draw_icon (myIcon, myContainer);
}

gboolean cd_clock_update_icon_slow (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
	if (pIcon != myIcon)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	*bContinueAnimation = TRUE;
	myData.iSmoothAnimationStep ++;
	if (myData.iSmoothAnimationStep > 5)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	// taille de la texture.
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	double fRatio = pContainer->fRatio;
	int iWidth = (int) pIcon->fWidth / fRatio * fMaxScale;
	int iHeight = (int) pIcon->fHeight / fRatio * fMaxScale;
	
	// render to texture
	cd_clock_render_analogic_to_texture (myApplet, iWidth, iHeight, &myData.currentTime);
	
	CD_APPLET_REDRAW_MY_ICON;
	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
