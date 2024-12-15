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

#include <cairo-xlib.h>
#include <gdk/gdkx.h>

#include "applet-struct.h"
#include "applet-screenshot.h"

#define _MARGIN 3

static void cd_screenshot_free_options (CDScreenshotOptions *pOptions);

  ///////////////
 /// DRAWING ///
///////////////

static gboolean _render_step_cairo (Icon *pIcon, GldiModuleInstance *myApplet)
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

static gboolean _render_step_opengl (Icon *pIcon, GldiModuleInstance *myApplet)
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

void cd_screenshot_free_apps_list (GldiModuleInstance *myApplet)
{
	if (myData.pAppList != NULL)
	{
		g_list_foreach (myData.pAppList, (GFunc) g_strfreev, NULL);
		g_list_free (myData.pAppList);
		myData.pAppList = NULL;
	}
}

static void _cd_open (GtkMenuItem *pMenuItem, gpointer data)
{
	cairo_dock_fm_launch_uri (myData.cCurrentUri);
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
	gchar *cOutputFile;

	const gchar *cDestinationDir;
	if (cFolder && g_file_test (cFolder, G_FILE_TEST_IS_DIR))
		cDestinationDir = cFolder;
	else if (myConfig.cDirPath && g_file_test (myConfig.cDirPath, G_FILE_TEST_IS_DIR))
		cDestinationDir = myConfig.cDirPath;
	else
		cDestinationDir = g_getenv ("HOME");

	if (cFileName)
		cOutputFile = g_strdup_printf ("%s/%s.png", cDestinationDir, cFileName);
	else
	{
		char s_cDateBuffer[21];
		time_t epoch = (time_t) time (NULL);
		struct tm currentTime;
		localtime_r (&epoch, &currentTime);
		
		strftime (s_cDateBuffer, 20, "%Y-%m-%d %H:%M:%S", &currentTime);
		cOutputFile = g_strdup_printf ("%s/%s %s.png", cDestinationDir,
			D_("Screenshot from"), s_cDateBuffer);
	}

	// if the file exists, add a number
	if (g_file_test (cOutputFile, G_FILE_TEST_EXISTS))
	{
		int i = 1;
		gchar *cOriginalOutputFile = g_strdup (cOutputFile);
		gchar *cDot = strrchr (cOriginalOutputFile, '.'); // end with .png
		cDot[0] = '\0'; // without the extension
		do
		{
			g_free (cOutputFile);
			cOutputFile = g_strdup_printf ("%s-%d.png", cOriginalOutputFile, i);
			i++;
		} while (g_file_test (cOutputFile, G_FILE_TEST_EXISTS));
		g_free (cOriginalOutputFile);
	}

	return cOutputFile;
}

static gchar *_make_screenshot (gboolean bActiveWindow, const gchar *cFolder, const gchar *cFileName)
{
	// create a surface that points on the root window.
	Display *display = gdk_x11_get_default_xdisplay ();
	Screen *screen = XDefaultScreenOfDisplay (display);
	Visual *visual = DefaultVisualOfScreen (screen);
	int w, h;
	Window Xid;
	if (bActiveWindow)
	{
		GldiWindowActor *pActiveWindow = gldi_windows_get_active ();
		Xid = gldi_window_get_id (pActiveWindow);  // cairo_dock_get_active_xwindow ()
		Window root_return;
		int x_return=1, y_return=1;
		unsigned int width_return, height_return, border_width_return, depth_return;
		XGetGeometry (display, Xid,
			&root_return,
			&x_return, &y_return,
			&width_return, &height_return,
			&border_width_return, &depth_return);  // we can't use the data from the WindowActor, because it takes into account the window border.
		w = width_return;
		h = height_return;
	}
	else
	{
		Xid = DefaultRootWindow (display);
		w = g_desktopGeometry.Xscreen.width;
		h = g_desktopGeometry.Xscreen.height;
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
			w0, h0);  // we must duplicate the surface, because it's an Xlib surface, not a data surface (plus it's way too large anyway).
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
	CD_APPLET_REMOVE_TRANSITION_ON_MY_ICON;
	CD_APPLET_STOP_DEMANDING_ATTENTION;
	CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->cFileName);
}

static void _take_screenshot (CDScreenshotOptions *pOptions)
{
	g_free (myData.cCurrentUri);
	myData.cCurrentUri = _make_screenshot (pOptions ? pOptions->bActiveWindow : FALSE,
		pOptions ? pOptions->cFolder : NULL,
		pOptions ? pOptions->cName   : NULL);
	
	if (myData.cCurrentUri)
	{
		// demands the attention; it helps localizing the menu and it shows the icon if the dock is hidden.
		if (myData.bFromShortkey)
		{
			CD_APPLET_DEMANDS_ATTENTION ("pulse", 1000);
		}
		
		// pop up the menu
		GtkWidget *pMenu = gldi_menu_new (myIcon);
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Copy the location"), GLDI_ICON_NAME_COPY, _cd_copy_location, pMenu, NULL);
		
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open"), GLDI_ICON_NAME_FILE, _cd_open, pMenu, NULL);

		if (myData.pAppList == NULL)
		{
			/* It's always a .png file always made by Cairo
			 *  ==> no need to recreate the list, to search for icons, etc.
			 * But we have to use the right icon and then reverse the list
			 */
			myData.pAppList = cairo_dock_fm_list_apps_for_file (myData.cCurrentUri);
			GList *a;
			gchar **pAppInfo; // name, icon, cmd
			gchar *cIconPath;
			for (a = myData.pAppList; a != NULL; a = a->next)
			{
				pAppInfo = a->data;
				if (pAppInfo[2] != NULL)
				{
					cIconPath = cairo_dock_search_icon_s_path (pAppInfo[2],
						cairo_dock_search_icon_size (GTK_ICON_SIZE_MENU));
					g_free (pAppInfo[2]);
					pAppInfo[2] = cIconPath;
				}
			}
			myData.pAppList = g_list_reverse (myData.pAppList); // it's normal, we received a reversed list
		}
		if (myData.pAppList)
		{
			cairo_dock_fm_add_open_with_submenu (myData.pAppList, myData.cCurrentUri, pMenu, D_("Open with"),
				GLDI_ICON_NAME_OPEN, NULL, NULL);
		}
		CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (D_("Open parent folder"), GLDI_ICON_NAME_DIRECTORY, _cd_open_parent, pMenu, NULL);
		
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
		gldi_dialog_show_temporary_with_icon (D_("Unable to take a screenshot"), myIcon, myContainer, 7000, MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
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

static GtkWidget * _add_label_in_new_hbox (const gchar *cLabel, const gchar *cTooltip, GtkWidget *pBox)
{
	GtkWidget *pHBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (pBox), pHBox, FALSE, FALSE, _MARGIN);
	
	GtkWidget *pLabel = gtk_label_new (cLabel);
	gldi_dialog_set_widget_text_color (pLabel);
	gtk_box_pack_start (GTK_BOX (pHBox), pLabel, FALSE, FALSE, _MARGIN);

	gtk_widget_set_tooltip_text (pLabel, cTooltip);

	return pHBox;
}

static void _cairo_dock_pick_a_file (G_GNUC_UNUSED GtkButton *button, GtkWidget *pEntry)
{
	GtkWidget* pFileChooserDialog = gtk_file_chooser_dialog_new (
		D_("Pick up a directory"),
		GTK_WINDOW (myContainer->pWidget), // pParentWindow,
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
		_("Ok"),
		GTK_RESPONSE_OK,
		_("Cancel"),
		GTK_RESPONSE_CANCEL,
		NULL);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (pFileChooserDialog),
		gtk_entry_get_text (GTK_ENTRY (pEntry)));
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (pFileChooserDialog),
		FALSE);

	gtk_widget_show (pFileChooserDialog);
	int answer = gtk_dialog_run (GTK_DIALOG (pFileChooserDialog));
	if (answer == GTK_RESPONSE_OK)
	{
		gchar *cDirPath = gtk_file_chooser_get_filename (
			GTK_FILE_CHOOSER (pFileChooserDialog));
		gtk_entry_set_text (GTK_ENTRY (pEntry), cDirPath);
		g_free (cDirPath);
	}
	gtk_widget_destroy (pFileChooserDialog);
}

GtkWidget *cd_screenshot_build_options_widget (void)
{
	GtkWidget *pHBox;
	const gchar *cTooltip;
	GtkWidget *pBox = gtk_box_new (GTK_ORIENTATION_VERTICAL, CAIRO_DOCK_GUI_MARGIN);

	cTooltip = D_("in seconds.");
	pHBox = _add_label_in_new_hbox (D_("Delay"), cTooltip, pBox);

	GtkWidget *pScale;
	GtkAdjustment *pAdjustment = gtk_adjustment_new (0,
		0,  // min
		10,  // max
		1,  // step
		1,  // step
		0);
	pScale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (pAdjustment));
	gtk_scale_set_digits (GTK_SCALE (pScale), 0);
	g_object_set (pScale, "width-request", 100, NULL);
	gldi_dialog_set_widget_text_color (pScale);
	gtk_box_pack_end (GTK_BOX (pHBox), pScale, FALSE, FALSE, _MARGIN);
	g_object_set_data (G_OBJECT (pBox), "delay", pScale);
	gtk_widget_set_tooltip_text (pScale, cTooltip);
	
	
	cTooltip = D_("Grab the current window instead of the all screen");
	pHBox = _add_label_in_new_hbox (D_("Grab the current window"), cTooltip, pBox);

	GtkWidget *pCheckButton = gtk_check_button_new ();
	gtk_box_pack_end (GTK_BOX (pHBox), pCheckButton, FALSE, FALSE, _MARGIN);
	g_object_set_data (G_OBJECT (pBox), "current", pCheckButton);
	gtk_widget_set_tooltip_text (pCheckButton, cTooltip);
	
	
	cTooltip = D_("Let empty to use the default one.");
	pHBox = _add_label_in_new_hbox (D_("File name"), cTooltip, pBox);

	GtkWidget *pEntry = gtk_entry_new ();
	gtk_box_pack_end (GTK_BOX (pHBox), pEntry, FALSE, FALSE, _MARGIN);
	g_object_set_data (G_OBJECT (pBox), "name", pEntry);
	gtk_widget_set_tooltip_text (pEntry, cTooltip);
	
	
	cTooltip = D_("Let empty to use the default one.");
	pHBox = _add_label_in_new_hbox (D_("Directory"), cTooltip, pBox);

	pEntry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (pEntry), myConfig.cDirPath ? myConfig.cDirPath : g_getenv ("HOME"));
	g_object_set_data (G_OBJECT (pBox), "dir", pEntry);
	GtkWidget *pButtonFileChooser = gtk_button_new_from_icon_name (GLDI_ICON_NAME_OPEN, GTK_ICON_SIZE_BUTTON);
	g_signal_connect (G_OBJECT (pButtonFileChooser),
		"clicked",
		G_CALLBACK (_cairo_dock_pick_a_file),
		pEntry);
	gtk_box_pack_end (GTK_BOX (pHBox), pButtonFileChooser, FALSE, FALSE, _MARGIN);
	gtk_box_pack_end (GTK_BOX (pHBox), pEntry, FALSE, FALSE, _MARGIN);
	gtk_widget_set_tooltip_text (pEntry, cTooltip);
	
	return pBox;
}


CDScreenshotOptions *cd_screenshot_get_options_from_widget (GtkWidget *pWidget)
{
	CDScreenshotOptions *pOptions = g_new0 (CDScreenshotOptions, 1);
	
	GtkWidget *pScale = g_object_get_data (G_OBJECT (pWidget), "delay");
	GtkWidget *pEntry = g_object_get_data (G_OBJECT (pWidget), "name");
	GtkWidget *pCheckButton = g_object_get_data (G_OBJECT (pWidget), "current");
	GtkWidget *pEntryDir = g_object_get_data (G_OBJECT (pWidget), "dir");

	pOptions->iDelay = gtk_range_get_value (GTK_RANGE (pScale));
	const gchar *cName = gtk_entry_get_text (GTK_ENTRY (pEntry));
	pOptions->cName = (cName && *cName != '\0' ? g_strdup (cName) : NULL);
	pOptions->bActiveWindow = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (pCheckButton));
	cName = gtk_entry_get_text (GTK_ENTRY (pEntryDir));
	pOptions->cFolder = (cName && *cName != '\0' ? g_strdup (cName) : NULL);
	
	return pOptions;
}
