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

#ifndef __CD_APPLET_INDICATOR3__
#define  __CD_APPLET_INDICATOR3__

#include "indicator-applet3.h"

void cd_indicator_generic_indicator_reload (IndicatorObject *pIndicator, IndicatorObjectEntry *pEntry, gpointer data);

void cd_indicator_generic_load_one_indicator (GldiModuleInstance *myApplet);

void cd_indicator_generic_indicator_stop (GldiModuleInstance *myApplet);

#endif /* __CD_APPLET_INDICATOR3__ */
