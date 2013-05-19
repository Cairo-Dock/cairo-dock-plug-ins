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

#ifndef __APPLET_NVIDIA__
#define __APPLET_NVIDIA__

#include <cairo-dock.h>


void cd_sysmonitor_get_nvidia_data (GldiModuleInstance *myApplet);


void cd_sysmonitor_get_nivdia_info (GldiModuleInstance *myApplet, GString *pInfo);


void cd_nvidia_alert (GldiModuleInstance *myApplet);


#endif
