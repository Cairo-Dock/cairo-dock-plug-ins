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
#include <string.h>
#include <cairo-dock.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-transitions.h"
#include "applet-slider.h"

static inline void _toggle_pause (CairoDockModuleInstance *myApplet)
{
	if (!myData.bPause) {
		myData.bPause = TRUE;  // coupera le timer.
	}
	else {
		myData.bPause = FALSE;
		cd_slider_next_slide (myApplet); //on relance le diapo
	}
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
static void _open_current_slide (CairoDockModuleInstance *myApplet)
{
	if (myData.pElement != NULL && myData.pElement->data != NULL)
	{
		SliderImage *pImage = myData.pElement->data;
		gchar *cImagePath = pImage->cPath;
		cd_debug ("opening %s ...", cImagePath);
		cairo_dock_fm_launch_uri (cImagePath);
	}
}
static void _open_current_folder (CairoDockModuleInstance *myApplet)
{
	if (myData.pElement != NULL && myData.pElement->data != NULL)
	{
		SliderImage *pImage = myData.pElement->data;
		gchar *cDirPath = g_path_get_dirname (pImage->cPath);
		cd_debug ("opening folder %s ...", cDirPath);
		cairo_dock_fm_launch_uri (cDirPath);
		g_free (cDirPath);
	}
}
static void _cd_slider_action (SliderClickOption iAction, CairoDockModuleInstance *myApplet)
{
	switch (iAction)
	{
		case SLIDER_PAUSE:
		default:
			_toggle_pause (myApplet);
		break;
		case SLIDER_OPEN_IMAGE:
			_open_current_slide (myApplet);
		break;
		case SLIDER_OPEN_FOLDER:
			_open_current_folder (myApplet);
		break;
	}
}
CD_APPLET_ON_CLICK_BEGIN
	_cd_slider_action (myConfig.iClickOption, myApplet);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_cd_slider_action (myConfig.iMiddleClickOption, myApplet);
CD_APPLET_ON_MIDDLE_CLICK_END


static gboolean _cd_slider_scroll_delayed (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	if (myData.iNbScroll == 0)
		CD_APPLET_LEAVE (FALSE);
	
	if (myConfig.bUseThread)
		cairo_dock_stop_task (myData.pMeasureImage);
	
	int i;
	if (myData.iNbScroll > 0)
	{
		if (myData.iTimerID == 0)  // en cours d'animation, on la finit en affichant l'image courante.
		{
			cd_slider_draw_default (myApplet);
			CD_APPLET_REDRAW_MY_ICON;
		}
		else
		{
			g_source_remove (myData.iTimerID);  //on coupe le timer en cours
			myData.iTimerID = 0;
		}
		for (i = 0; i < myData.iNbScroll-1; i ++)  // le 1er scroll n'implique rien.
		{
			if (myData.pElement == NULL)  // debut
				myData.pElement = myData.pList;
			else
				myData.pElement = cairo_dock_get_next_element (myData.pElement, myData.pList);
		}
	}
	else if (myData.iNbScroll < 0)
	{
		if (myData.iTimerID != 0)
		{
			g_source_remove(myData.iTimerID); //on coupe le timer en cours
			myData.iTimerID = 0;
		}
		for (i = 0; i <= -myData.iNbScroll; i ++)  // fait une fois de plus.
		{
			myData.pElement = cairo_dock_get_previous_element (myData.pElement, myData.pList);
		}
	}
	
	myData.iNbScroll = 0;
	myData.iScrollID = 0;
	cd_slider_next_slide (myApplet);
	CD_APPLET_LEAVE (FALSE);
}
CD_APPLET_ON_SCROLL_BEGIN
	if (myData.iScrollID != 0)
		g_source_remove (myData.iScrollID);
	
	if (CD_APPLET_SCROLL_DOWN)
	{
		myData.iNbScroll ++;
	}
	else if (CD_APPLET_SCROLL_UP)
	{
		myData.iNbScroll --;
	}
	
	myData.iScrollID = g_timeout_add (100, (GSourceFunc) _cd_slider_scroll_delayed, myApplet);
	CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
static void _free_app (gpointer *app)
{
	g_free (app[1]);
	g_free (app);
}
void cd_slider_free_apps_list (CairoDockModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) _free_app, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

static void _cd_slider_toogle_pause (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_toggle_pause (myApplet);
	CD_APPLET_LEAVE();
}
static void _cd_slider_open_selected (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	cairo_dock_fm_launch_uri (myData.cSelectedImagePath);
	CD_APPLET_LEAVE();
}
static void _cd_slider_launch_with (GtkMenuItem *pMenuItem, gpointer *app)
{
	CairoDockModuleInstance *myApplet = app[0];
	gchar *cExec = app[1];
	cairo_dock_launch_command_printf ("%s \"%s\"", NULL, cExec, myData.cSelectedImagePath);  // en esperant que l'appli gere les URI.
}
static void _cd_slider_run_dir (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	if (myData.cSelectedImagePath == NULL)
	{
		cairo_dock_fm_launch_uri (myConfig.cDirectory);
	}
	else
	{
		gchar *cDirPath = g_path_get_dirname (myData.cSelectedImagePath);
		cairo_dock_fm_launch_uri (cDirPath);
		g_free (cDirPath);
	}
	CD_APPLET_LEAVE();
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	// get the current image selected at the moment of the click.
	g_free (myData.cSelectedImagePath);
	if (myData.pElement != NULL && myData.pElement->data != NULL)
	{
		SliderImage *pImage = myData.pElement->data;
		myData.cSelectedImagePath = g_strdup (pImage->cPath);
	}
	else
		myData.cSelectedImagePath = NULL;
	
	gchar *cLabel;
	if (myConfig.iClickOption != SLIDER_PAUSE)
	{
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (myData.bPause ? D_("Play") : D_("Pause"), myData.bPause ? GTK_STOCK_MEDIA_PLAY : GTK_STOCK_MEDIA_PAUSE, _cd_slider_toogle_pause, CD_APPLET_MY_MENU);
	}
	
	if (myData.cSelectedImagePath != NULL)
	{
		if (myConfig.iClickOption != SLIDER_OPEN_IMAGE)
		{
			if (myConfig.iMiddleClickOption == SLIDER_OPEN_IMAGE)
				cLabel = g_strdup_printf ("%s (%s)", D_("Open current image"), D_("middle-click"));
			else
				cLabel = g_strdup (D_("Open current image"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_OPEN, _cd_slider_open_selected, CD_APPLET_MY_MENU);
			g_free (cLabel);
		}
		
		GList *pApps = cairo_dock_fm_list_apps_for_file (myData.cSelectedImagePath);
		if (pApps != NULL)
		{
			GtkWidget *pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Open with"), CD_APPLET_MY_MENU, GTK_STOCK_OPEN);
			
			cd_slider_free_apps_list (myApplet);

			gint iDesiredIconSize = cairo_dock_search_icon_size (GTK_ICON_SIZE_LARGE_TOOLBAR); // 24px
			GList *a;
			gchar **pAppInfo;
			gchar *cIconPath;
			gpointer *app;
			for (a = pApps; a != NULL; a = a->next)
			{
				pAppInfo = a->data;
				
				app = g_new0 (gpointer, 2);
				app[0] = myApplet;
				app[1] = g_strdup (pAppInfo[1]);
				//g_print (" + %s (%s ; %s)\n", pAppInfo[0], pAppInfo[1], pAppInfo[2]);
				myData.pAppList = g_list_prepend (myData.pAppList, app);
				
				if (pAppInfo[2] != NULL)
					cIconPath = cairo_dock_search_icon_s_path (pAppInfo[2], iDesiredIconSize);
				else
					cIconPath = NULL;
				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pAppInfo[0], cIconPath, _cd_slider_launch_with, pSubMenu, app);
				g_free (cIconPath);
				g_strfreev (pAppInfo);
			}
			g_list_free (pApps);
		}
	}
	
	if (myConfig.iMiddleClickOption == SLIDER_OPEN_FOLDER)
		cLabel = g_strdup_printf ("%s (%s)", D_("Browse images folder"), D_("middle-click"));
	else
		cLabel = g_strdup (D_("Browse images folder"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_DIRECTORY, _cd_slider_run_dir, CD_APPLET_MY_MENU);
	g_free (cLabel);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_UPDATE_ICON_BEGIN
	if (cd_slider_next_slide_is_scheduled (myApplet) || cairo_dock_task_is_running (myData.pMeasureImage))  // on est en attente d'une image, on quitte la boucle tout de suite.
		CD_APPLET_STOP_UPDATE_ICON;
	
	gboolean bContinueTransition = FALSE;
	switch (myData.iAnimation)
	{
		case SLIDER_FADE :
			bContinueTransition = cd_slider_fade (myApplet);
		break ;
		case SLIDER_BLANK_FADE :
			bContinueTransition = cd_slider_blank_fade (myApplet);
		break ;
		case SLIDER_FADE_IN_OUT :
			bContinueTransition = cd_slider_fade_in_out (myApplet);
		break ;
		case SLIDER_SIDE_KICK :
			bContinueTransition = cd_slider_side_kick (myApplet);
		break ;
		case SLIDER_DIAPORAMA :
			bContinueTransition = cd_slider_diaporama (myApplet);
		break ;
		case SLIDER_GROW_UP :
			bContinueTransition = cd_slider_grow_up (myApplet);
		break ;
		case SLIDER_SHRINK_DOWN :
			bContinueTransition = cd_slider_shrink_down (myApplet);
		break ;
		case SLIDER_CUBE :
			bContinueTransition = cd_slider_cube (myApplet);
		break ;
		default :
			CD_APPLET_STOP_UPDATE_ICON;
	}
	
	if (! bContinueTransition)  // la transition est finie, on dessine une derniere fois, on se place en attente de l'image suivante et on quitte la boucle.
	{
		cd_slider_schedule_next_slide (myApplet);
		CD_APPLET_PAUSE_UPDATE_ICON;
	}
CD_APPLET_ON_UPDATE_ICON_END
