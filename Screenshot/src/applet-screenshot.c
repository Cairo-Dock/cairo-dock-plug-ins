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

#include <cairo-xlib.h>

#include "applet-struct.h"
#include "applet-screenshot.h"

#define _MARGIN 3

static void cd_screenshot_free_options (CDScreenshotOptions *pOptions);

  ///////////////
 /// DRAWING ///
///////////////

static gboolean _render_step_cairo (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	CD_APPLET_LEAVE_IF_FAIL (iHeight != 0, TRUE);
	
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN_CAIRO (FALSE);

	// image precedente.
	if (myData.pOldImage != NULL)
	{
		cairo_dock_apply_image_buffer_surface_with_offset (myData.pOldImage, myDrawContext, 0, 0, 1. - f);
	}
	
	// image courante.
	if (myData.pCurrentImage != NULL)
	{
		int x = (iWidth - myData.pCurrentImage->iWidth) / 2;
		int y = (iHeight - myData.pCurrentImage->iHeight) / 2;
		cairo_dock_apply_image_buffer_surface_with_offset (myData.pCurrentImage, myDrawContext, x, y, f);
	}
	
	CD_APPLET_FINISH_DRAWING_MY_ICON_CAIRO;
	CD_APPLET_LEAVE (TRUE);
}

static gboolean _render_step_opengl (Icon *pIcon, CairoDockModuleInstance *myApplet)
{
	g_return_val_if_fail (myData.pCurrentImage != NULL, FALSE);
	CD_APPLET_ENTER;
	double f = CD_APPLET_GET_TRANSITION_FRACTION ();
	
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	
	// image precedente.
	if (myData.pOldImage != NULL)
	{
		_cairo_dock_set_alpha (1. - f);
		cairo_dock_apply_image_buffer_texture_with_offset (myData.pOldImage, 0, 0);
	}
	
	// image courante.
	if (myData.pCurrentImage != NULL)
	{
		_cairo_dock_set_alpha (f);
		cairo_dock_apply_image_buffer_texture_with_offset (myData.pCurrentImage, 0, 0);
	}
	
	_cairo_dock_disable_texture ();
	
	CD_APPLET_LEAVE (TRUE);
}

  ////////////
 /// MENU ///
////////////

void cd_screenshot_free_apps_list (CairoDockModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) g_free, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

static void _cd_open (GtkMenuItem *pMenuItem, gpointer data)
{
	cairo_dock_fm_launch_uri (myData.cCurrentUri);
}

static void _cd_launch_with (GtkMenuItem *pMenuItem, const gchar *cExec)
{
	cairo_dock_launch_command_printf ("%s \"%s\"", NULL, cExec, myData.cCurrentUri);
}

static void _cd_open_parent (GtkMenuItem *pMenuItem, gpointer data)
{
	gchar *cFolder = g_path_get_dirname (myData.cCurrentUri);
	cairo_dock_fm_launch_uri (cFolder);
	g_free (cFolder);
}

static void _cd_copy_location (GtkMenuItem *pMenuItem, gpointer data)
{
	GtkClipboard *pClipBoard;
	pClipBoard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);  // GDK_SELECTION_PRIMARY
	gtk_clipboard_set_text (pClipBoard, myData.cCurrentUri, -1);
}


  //////////////////
 /// SCREENSHOT ///
//////////////////

static gchar *_make_image_name (const gchar *cFolder, const gchar *cFileName)
{
	const gchar *cDestinationDir = cFolder ? cFolder : g_getenv ("HOME");
	if (cFileName)
		return g_strdup_printf ("%s/%s.png", cDestinationDir, cFileName);

	char s_cDateBuffer[21];
	time_t epoch = (time_t) time (NULL);
	struct tm currentTime;
	localtime_r (&epoch, &currentTime);
	
	strftime (s_cDateBuffer, 20, "%Y-%m-%d %H:%M:%S", &currentTime);
	return g_strdup_printf ("%s/%s %s.png", cDestinationDir,
		D_("Screenshot from"), s_cDateBuffer);
}

static gchar *_make_screenshot (gboolean bActiveWindow, const gchar *cFolder, const gchar *cFileName)
{
	// create a surface that points on the root window.
	Display *display = cairo_dock_get_Xdisplay ();
	Screen *screen = XDefaultScreenOfDisplay (display);
	Visual *visual = DefaultVisualOfScreen (screen);
	int w, h;
	Window Xid;
	if (bActiveWindow)
	{
		Xid = cairo_dock_get_current_active_window();
		Window root_return;
		int x_return=1, y_return=1;
		unsigned int width_return, height_return, border_width_return, depth_return;
		XGetGeometry (display, Xid,
			&root_return,
			&x_return, &y_return,
			&width_return, &height_return,
			&border_width_return, &depth_return);  // we can't use the data from the Task manager, because it takes into account the window border.
		w = width_return;
		h = height_return;
	}
	else
	{
		Xid = cairo_dock_get_root_id();
		w = g_desktopGeometry.iXScreenWidth[CAIRO_DOCK_HORIZONTAL];
		h = g_desktopGeometry.iXScreenHeight[CAIRO_DOCK_HORIZONTAL];
	}
	cairo_surface_t *s = cairo_xlib_surface_create (display,
		Xid,
		visual,
		w, h);
	
	gchar *cName = NULL;
	if (s)
	{
		// save the surface on the disk
		cName = _make_image_name (cFolder, cFileName);
		cairo_surface_write_to_png (s, cName);
		
		// apply the surface on the icon, with a transition.
		int iWidth, iHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
		
		cairo_dock_free_image_buffer (myData.pCurrentImage);
		myData.pCurrentImage = g_new0 (CairoDockImageBuffer, 1);
		double ratio = MIN ((double)iWidth / w, (double)iHeight / h);  // keep ratio.
		int w0 = w * ratio;
		int h0 = h * ratio;
		cairo_surface_t *pSurface = cairo_dock_duplicate_surface (s,
			w, h,
			w0, h0);  // we must duplicate the surface, because it's an Xlib surface, not an data surface (plus it's way too large anyway).
		cairo_dock_load_image_buffer_from_surface (myData.pCurrentImage, pSurface, w0, h0);
		
		cairo_dock_free_image_buffer (myData.pOldImage);
		myData.pOldImage = cairo_dock_create_image_buffer (myIcon->cFileName, iWidth, iHeight, 0);  // maybe we could use the current icon image ...
		
		CD_APPLET_SET_TRANSITION_ON_MY_ICON (_render_step_cairo,
			_render_step_opengl,
			g_bUseOpenGL,  // bFastPace : vite si opengl, lent si cairo.
			2000,  // 2s transition
			TRUE);  // bRemoveWhenFinished
		
		cairo_surface_destroy (s);
	}
	return cName;
}

static void _on_menu_deactivated (G_GNUC_UNUSED GtkMenuShell *menu, G_GNUC_UNUSED gpointer data)
{
	CD_APPLET_STOP_DEMANDING_ATTENTION;
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->cFileName);
}

static void _take_screenshot (CDScreenshotOptions *pOptions)
{
	g_free (myData.cCurrentUri);
	myData.cCurrentUri = _make_screenshot (pOptions?pOptions->bActiveWindow:FALSE,
		NULL,
		pOptions?pOptions->cName:NULL);
	
	if (myData.cCurrentUri)
	{
		// demands the attention; it helps localizing the menu and it shows the icon if the dock is hidden.
		if (myData.bFromShortkey)
		{
			CD_APPLET_DEMANDS_ATTENTION ("pulse", 1000);
		}
		
		// pop up the menu
		GtkWidget *pMenu = gtk_menu_new ();
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Copy the location"), GTK_STOCK_COPY, _cd_copy_location, pMenu, NULL);
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open"), GTK_STOCK_OPEN, _cd_open, pMenu, NULL);

		GList *pApps = cairo_dock_fm_list_apps_for_file (myData.cCurrentUri);
		if (pApps != NULL)
		{
			GtkWidget *pSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Open with"), pMenu, GTK_STOCK_OPEN);

			cd_screenshot_free_apps_list (myApplet);

			GList *a;
			gchar **pAppInfo;
			gchar *cIconPath;
			for (a = pApps; a != NULL; a = a->next)
			{
				pAppInfo = a->data;
				myData.pAppList = g_list_prepend (myData.pAppList, pAppInfo[1]);

				if (pAppInfo[2] != NULL)
					cIconPath = cairo_dock_search_icon_s_path (pAppInfo[2], cairo_dock_search_icon_size (GTK_ICON_SIZE_LARGE_TOOLBAR));
				else
					cIconPath = NULL;
				CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (pAppInfo[0], cIconPath, _cd_launch_with, pSubMenu, pAppInfo[1]);
				g_free (cIconPath);
				g_free (pAppInfo[0]);
				g_free (pAppInfo[2]);
				g_free (pAppInfo);
			}
			g_list_free (pApps);
		}
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open parent folder"), GTK_STOCK_DIRECTORY, _cd_open_parent, pMenu, NULL);
		
		CD_APPLET_POPUP_MENU_ON_MY_ICON (pMenu);
		
		gtk_menu_shell_select_first (GTK_MENU_SHELL (pMenu), FALSE);  // must be done here, after the menu has been realized.
		
		// when the menu disappear, set the icon back to normal.
		g_signal_connect (G_OBJECT (pMenu),
			"deactivate",
			G_CALLBACK (_on_menu_deactivated),
			NULL);
	}
	else  // show an error message
	{
		cairo_dock_show_temporary_dialog_with_icon (D_("Unable to take a screenshot"), myIcon, myContainer, 7000, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
	}
}

static void cd_screenshot_free_options (CDScreenshotOptions *pOptions)
{
	if (!pOptions)
		return;
	g_free (pOptions->cFolder);
	g_free (pOptions->cName);
	g_free (pOptions);
}

void cd_screenshot_cancel (void)
{
	if (myData.iSidTakeWithDelay != 0)
		g_source_remove (myData.iSidTakeWithDelay);
	cd_screenshot_free_options (myData.pOptions);
	myData.pOptions = NULL;
	g_free (myData.cCurrentUri);
	myData.cCurrentUri = NULL;
}

	
static gboolean _take_screenshot_countdown (CDScreenshotOptions *pOptions)
{
	if (pOptions->iDelay > 0)
	{
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", pOptions->iDelay);
		CD_APPLET_REDRAW_MY_ICON;
		pOptions->iDelay --;
		return TRUE;
	}
	else
	{
		CD_APPLET_STOP_DEMANDING_ATTENTION;
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
		CD_APPLET_REDRAW_MY_ICON;
		_take_screenshot (pOptions);
		cd_screenshot_free_options (pOptions);
		myData.pOptions = NULL;
		myData.iSidTakeWithDelay = 0;
		return FALSE;
	}
}

void cd_screenshot_take (CDScreenshotOptions *pOptions)
{
	cd_screenshot_cancel ();
	
	if (pOptions && pOptions->iDelay > 0)
	{
		myData.pOptions = pOptions;
		_take_screenshot_countdown (pOptions);
		myData.iSidTakeWithDelay = g_timeout_add_seconds (1, (GSourceFunc)_take_screenshot_countdown, pOptions);
		CD_APPLET_DEMANDS_ATTENTION ("busy", 1000);  // this also shows the icon if the dock is hidden.
	}
	else
	{
		_take_screenshot (pOptions);
		cd_screenshot_free_options (pOptions);
	}
}


  //////////////////////
 /// OPTIONS WIDGET ///
//////////////////////

#if (GTK_MAJOR_VERSION < 3)
#define Adjustment GtkObject
#else
#define Adjustment GtkAdjustment
#endif

GtkWidget *cd_screenshot_build_options_widget (void)
{
	GtkWidget *pBox = _gtk_vbox_new (CAIRO_DOCK_GUI_MARGIN);
	
	GtkWidget *pHBox = _gtk_hbox_new (0);
	gtk_box_pack_start (GTK_BOX (pBox), pHBox, FALSE, FALSE, _MARGIN);
	
	GtkWidget *pLabel = gtk_label_new (D_("Delay"));
	gtk_box_pack_start (GTK_BOX (pHBox), pLabel, FALSE, FALSE, _MARGIN);
	
	GtkWidget *pScale;
	Adjustment *pAdjustment = gtk_adjustment_new (0,
		0,  // min
		10,  // max
		1,  // step
		1,  // step
		0);
	#if (GTK_MAJOR_VERSION < 3)
	pScale = gtk_hscale_new (GTK_ADJUSTMENT (pAdjustment));
	#else
	pScale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (pAdjustment));
	#endif
	gtk_scale_set_digits (GTK_SCALE (pScale), 0);
	g_object_set (pScale, "width-request", 100, NULL);
	gtk_box_pack_end (GTK_BOX (pHBox), pScale, FALSE, FALSE, _MARGIN);
	g_object_set_data (G_OBJECT (pBox), "delay", pScale);
	
	
	pHBox = _gtk_hbox_new (0);
	gtk_box_pack_start (GTK_BOX (pBox), pHBox, FALSE, FALSE, _MARGIN);
	
	pLabel = gtk_label_new (D_("Current window"));
	gtk_box_pack_start (GTK_BOX (pHBox), pLabel, FALSE, FALSE, _MARGIN);
	
	GtkWidget *pCheckButton = gtk_check_button_new ();
	gtk_box_pack_end (GTK_BOX (pHBox), pCheckButton, FALSE, FALSE, _MARGIN);
	g_object_set_data (G_OBJECT (pBox), "current", pCheckButton);
	
	
	pHBox = _gtk_hbox_new (0);
	gtk_box_pack_start (GTK_BOX (pBox), pHBox, FALSE, FALSE, _MARGIN);
	
	pLabel = gtk_label_new (D_("File name"));
	gtk_box_pack_start (GTK_BOX (pHBox), pLabel, FALSE, FALSE, _MARGIN);
	
	GtkWidget *pEntry = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (pHBox), pEntry, FALSE, FALSE, _MARGIN);
	g_object_set_data (G_OBJECT (pBox), "name", pEntry);
	
	return pBox;
}


CDScreenshotOptions *cd_screenshot_get_options_from_widget (GtkWidget *pWidget)
{
	CDScreenshotOptions *pOptions = g_new0 (CDScreenshotOptions, 1);
	
	GtkWidget *pScale = g_object_get_data (G_OBJECT (pWidget), "delay");
	GtkWidget *pEntry = g_object_get_data (G_OBJECT (pWidget), "name");
	GtkWidget *pCheckButton = g_object_get_data (G_OBJECT (pWidget), "current");

	pOptions->iDelay = gtk_range_get_value (GTK_RANGE (pScale));
	const gchar *cName = gtk_entry_get_text (GTK_ENTRY (pEntry));
	pOptions->cName = (cName && *cName != '\0' ? g_strdup (cName) : NULL);
	pOptions->bActiveWindow = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pCheckButton));
	pOptions->cFolder = NULL;  /// TODO  ...
	
	return pOptions;
}
