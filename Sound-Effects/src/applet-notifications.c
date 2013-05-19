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

#include "applet-struct.h"
#include "applet-sound.h"
#include "applet-notifications.h"


gboolean cd_sound_on_click (G_GNUC_UNUSED gpointer pUserData, Icon *pIcon, GldiContainer *pContainer, guint iButtonState)
{
	if (pIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (myData.pOnClickSound == NULL)
	{
		myData.pOnClickSound = cd_sound_load_sound_file (myConfig.cOnClickSound ? myConfig.cOnClickSound : MY_APPLET_SHARE_DATA_DIR"/on-click.wav");
	}
	cd_sound_play_sound (myData.pOnClickSound);
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_sound_on_middle_click (G_GNUC_UNUSED gpointer pUserData, Icon *pIcon, GldiContainer *pContainer)
{
	if (pIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (myData.pOnMiddleClickSound == NULL)
	{
		myData.pOnMiddleClickSound = cd_sound_load_sound_file (myConfig.cOnMiddleClickSound ? myConfig.cOnMiddleClickSound : MY_APPLET_SHARE_DATA_DIR"/on-middle-click.wav");
	}
	cd_sound_play_sound (myData.pOnMiddleClickSound);
	
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean cd_sound_on_hover (G_GNUC_UNUSED gpointer pUserData, Icon *pIcon, CairoDock *pDock, gboolean *bStartAnimation)
{
	if (pIcon == NULL)
		return GLDI_NOTIFICATION_LET_PASS;
	
	if (myData.pOnHoverSound == NULL)
	{
		myData.pOnHoverSound = cd_sound_load_sound_file (myConfig.cOnHoverSound ? myConfig.cOnHoverSound : MY_APPLET_SHARE_DATA_DIR"/on-hover.wav");
	}
	cd_sound_play_sound (myData.pOnHoverSound);
	
	
	return GLDI_NOTIFICATION_LET_PASS;
}