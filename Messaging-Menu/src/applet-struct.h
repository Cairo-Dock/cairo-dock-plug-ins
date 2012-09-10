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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>

#ifndef INDICATOR_MESSAGES_12_10
#include "indicator-applet.h"
#define FORCE_REMOVE_DOUBLE_SEPARATORS
// let's include the Dbus name shere, so that we don't duplicate the logic.
// we could put it directly in the .h, but having several .h will be easier to cope with further changes.
#if (INDICATOR_OLD_NAMES == 0)
#include "dbus-data.h"
#else
#include "dbus-data-old.h"
#endif

#else

#include "indicator-applet3.h"
#endif

//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gchar *cAnimationName;
	gchar *cShortkey;
	gchar *defaultTitle;
	#ifdef INDICATOR_MESSAGES_12_10
	gchar *cIndicatorName;
	#endif
	} ;

//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	#ifndef INDICATOR_MESSAGES_12_10
	CDAppletIndicator *pIndicator;
	#else
	IndicatorObject *pIndicator;
	IndicatorObjectEntry *pEntry;
	#endif
	CairoKeyBinding *pKeyBinding;
	} ;


#endif
