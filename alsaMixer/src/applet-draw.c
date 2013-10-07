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
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-generic.h"
#include "applet-draw.h"


void cd_update_icon (void)
{
	gboolean bNeedRedraw = FALSE;
	
	// update the volume info
	switch (myConfig.iVolumeDisplay)
	{
		case VOLUME_ON_LABEL :
			CD_APPLET_SET_NAME_FOR_MY_ICON_PRINTF ("%s: %d%%", myData.mixer_card_name?myData.mixer_card_name:D_("Volume"), myData.iCurrentVolume);
		break;
		
		case VOLUME_ON_ICON :
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d%%", myData.iCurrentVolume);
			bNeedRedraw = TRUE;
		break;
		
		default :
		break;
	}
	
	// update the icon representation
	switch (myConfig.iVolumeEffect)  // set the icon if needed
	{
		case VOLUME_EFFECT_NONE :
		case VOLUME_EFFECT_BAR :
			if (myData.bMuteImage < 0 || (myData.bIsMute != myData.bMuteImage))
			{
				if (myData.bIsMute)
					CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cMuteIcon, "mute.svg");
				else
					CD_APPLET_SET_USER_IMAGE_ON_MY_ICON (myConfig.cDefaultIcon, "default.svg");
				myData.bMuteImage = myData.bIsMute;
				bNeedRedraw = FALSE;
			}
		break;
		
		default :
		break;
	}
	switch (myConfig.iVolumeEffect)  // render the value
	{
		case VOLUME_EFFECT_BAR :
		case VOLUME_EFFECT_GAUGE :
		{
			double fPercent;
			if (myData.bIsMute)
				fPercent = CAIRO_DATA_RENDERER_UNDEF_VALUE;
			else
				fPercent = (double) myData.iCurrentVolume / 100.;
			CD_APPLET_RENDER_NEW_DATA_ON_MY_ICON (&fPercent);
			bNeedRedraw = FALSE;
		}
		break;
		
		default :
		break;
	}
	
	if (bNeedRedraw)
		CD_APPLET_REDRAW_MY_ICON;
	
	if (myData.pScale)
	{
		cd_mixer_set_volume_with_no_callback (myData.pScale, myData.iCurrentVolume);
	}
}
