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

#include "applet-struct.h"
#include "applet-screenshot.h"
#include "applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	// take screenshot
	myData.bFromShortkey = FALSE;
	cd_screenshot_take (NULL);
CD_APPLET_ON_CLICK_END


static void _on_got_answer (int iClickedButton, GtkWidget *pInteractiveWidget, gpointer data, CairoDialog *pDialog)
{
	CD_APPLET_ENTER;
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		myData.bFromShortkey = TRUE;
		CDScreenshotOptions *pOptions = cd_screenshot_get_options_from_widget (pInteractiveWidget);
		if (pOptions->bActiveWindow && pOptions->iDelay == 0)
			pOptions->iDelay = 1;
		cd_screenshot_take (pOptions);
	}
	CD_APPLET_LEAVE ();

}
static void _take_screenshot_with_options (void)
{
	GtkWidget *pWidget = cd_screenshot_build_options_widget ();
	myData.pDialog = gldi_dialog_show (D_("Screenshot"), myIcon, myContainer,
		0,
		MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE,
		pWidget, (CairoDockActionOnAnswerFunc)_on_got_answer, NULL, (GFreeFunc)NULL);
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	_take_screenshot_with_options ();
CD_APPLET_ON_MIDDLE_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Screenshot with options"), D_("middle-click"));
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_PROPERTIES, _take_screenshot_with_options, CD_APPLET_MY_MENU);
	g_free (cLabel);
CD_APPLET_ON_BUILD_MENU_END


void cd_screenshot_on_keybinding_pull (const gchar *keystring, gpointer user_data)
{
	myData.bFromShortkey = TRUE;
	cd_screenshot_take (NULL);
}
