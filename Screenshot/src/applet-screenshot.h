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

#ifndef __APPLET_SCREENSHOT__
#define  __APPLET_SCREENSHOT__

#include <cairo-dock.h>


void cd_screenshot_cancel (void);

void cd_screenshot_take (CDScreenshotOptions *pOptions);


void cd_screenshot_free_apps_list (GldiModuleInstance *myApplet);

GtkWidget *cd_screenshot_build_options_widget (void);

CDScreenshotOptions *cd_screenshot_get_options_from_widget (GtkWidget *pWidget);


#endif
