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

#include <libido/libido.h>

#include "common-defs.h"
#include "volume-widget.h"
#include "applet-struct.h"
#include "applet-menu.h"
#include "applet-notifications.h"


static inline void _show_menu (void)
{
	if (! cd_indicator_show_menu (myData.pIndicator))
		cairo_dock_show_temporary_dialog_with_icon (D_("The Sound service did not reply.\nPlease check that it is correctly installed."), myIcon, myContainer, 4000., "same icon");
}

//\___________ Define here the action to be taken when the user left-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons.
CD_APPLET_ON_CLICK_BEGIN
	_show_menu ();
CD_APPLET_ON_CLICK_END


//\___________ Same as ON_CLICK, but with middle-click.
CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	/// set mute...
	
CD_APPLET_ON_MIDDLE_CLICK_END


//\___________ Same as ON_CLICK, but with scroll. Moreover, CD_APPLET_SCROLL_UP tels you is the user scrolled up, CD_APPLET_SCROLL_DOWN the opposite.
CD_APPLET_ON_SCROLL_BEGIN
	if (myData.iCurrentState == UNAVAILABLE || myData.iCurrentState == MUTED)
		CD_APPLET_LEAVE (CAIRO_DOCK_LET_PASS_NOTIFICATION);

	GtkWidget* slider_widget = volume_widget_get_ido_slider(VOLUME_WIDGET(myData.volume_widget)); 
	GtkWidget* slider = ido_scale_menu_item_get_scale((IdoScaleMenuItem*)slider_widget);
	GtkRange* range = (GtkRange*)slider;
	CD_APPLET_LEAVE_IF_FAIL (GTK_IS_RANGE(range), CAIRO_DOCK_LET_PASS_NOTIFICATION);

	gdouble value = gtk_range_get_value(range);
	GtkAdjustment *adj = gtk_range_get_adjustment (GTK_RANGE (slider));
	//g_debug("indicator-sound-scroll - current slider value %f", value);
	if (CD_APPLET_SCROLL_UP) {
	value += 5;
	} else if (CD_APPLET_SCROLL_DOWN) {
	value -= 5;
	}
	//g_debug("indicator-sound-scroll - update slider with value %f", value);
	volume_widget_update(VOLUME_WIDGET(myData.volume_widget), value, "scroll updates");

	///sound_state_manager_show_notification (myApplet.state_manager, value);
CD_APPLET_ON_SCROLL_END


//\___________ Define here the entries you want to add to the menu when the user right-clicks on your icon or on its subdock or your desklet. The icon and the container that were clicked are available through the macros CD_APPLET_CLICKED_ICON and CD_APPLET_CLICKED_CONTAINER. CD_APPLET_CLICKED_ICON may be NULL if the user clicked in the container but out of icons. The menu where you can add your entries is available throught the macro CD_APPLET_MY_MENU; you can add sub-menu to it if you want.
CD_APPLET_ON_BUILD_MENU_BEGIN

CD_APPLET_ON_BUILD_MENU_END


void cd_sound_on_keybinding_pull (const gchar *keystring, CairoDockModuleInstance *myApplet)
{
	CD_APPLET_ENTER;
	_show_menu ();
	CD_APPLET_LEAVE();
}
