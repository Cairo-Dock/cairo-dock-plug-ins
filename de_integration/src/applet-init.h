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


#ifndef __DE_INTEGRATION_INIT__
#define __DE_INTEGRATION_INIT__


#include <cairo-dock.h>


CD_APPLET_H


struct _AppletConfig {
	CairoDockDesktopEnv iEnvRequested;
} ;

struct _AppletData {
	CairoDockDesktopEnv iCurrentEnv;
} ;


void cd_gnome_integration_init (void);
void cd_gnome_integration_stop (void);
const char *cd_gnome_integration_get_status (void);

void cd_kde_integration_init (void);

void cd_xfce_integration_init (void);
void cd_xfce_integration_stop (void);
const char *cd_xfce_integration_get_status (void);

void cd_cosmic_integration_init (void);

CD_APPLET_CONFIG_H

#endif

