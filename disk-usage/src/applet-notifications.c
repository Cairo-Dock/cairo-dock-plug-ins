/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-notifications.h"
#include "applet-hdd.h"

CD_APPLET_INCLUDE_MY_VARS


CD_APPLET_ABOUT (D_("This is the disk-usage applet\n made by Benjamin SANS for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	
	
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("disk-usage", pSubMenu, CD_APPLET_MY_MENU);
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_MIDDLE_CLICK_BEGIN
	if (myData.bAcquisitionOK)
	{
		//if (myData.pTopDialog != NULL || cairo_dock_remove_dialog_if_any (myIcon))
		//	return CAIRO_DOCK_INTERCEPT_NOTIFICATION;
		gchar *cFree  = cd_human_readable(myData.llFree);
		gchar *cUsed  = cd_human_readable(myData.llUsed);
		gchar *cTotal = cd_human_readable(myData.llTotal);
		gchar *cAvail = cd_human_readable(myData.llAvail);
		
		cairo_dock_show_temporary_dialog ("%s : %s\n%s : %s\n%s : %s\n%s : %s\n%s : %s\n%s : %s", myIcon, myContainer, 10e3, D_("Access point"), myConfig.cDevice, D_("Total size"), cTotal, D_("Used"), cUsed, D_("Free"), cFree,D_("Available"), cAvail, D_("Type"), myData.cType);
		g_free (cFree);
		g_free (cUsed);
		g_free (cTotal);
		g_free (cAvail);
	}
	else
		cairo_dock_show_temporary_dialog(D_("Data acquisition has failed"), myIcon, myContainer, 4e3);
CD_APPLET_ON_MIDDLE_CLICK_END
