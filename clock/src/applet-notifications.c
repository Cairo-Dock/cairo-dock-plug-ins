/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>

#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS


void cd_clock_launch_time_admin (GtkMenuItem *menu_item, gpointer *data)
{
	system ("gksu time-admin &");
}


CD_APPLET_ABOUT ("This is a clock applet made by Fabrice Rey (fabounet_03@yahoo.fr) for Cairo-Dock.\n The analogic representation is a port of the well-known Cairo-Clock from MacSlow (http://macslow.thepimp.net).")


CD_APPLET_ON_CLICK_BEGIN
	GtkWidget *pDialog = gtk_dialog_new ();
	
	GtkWidget *pCalendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pDialog)->vbox), pCalendar);
	gtk_widget_show (pCalendar);
	
	gtk_dialog_run (GTK_DIALOG (pDialog));
	gtk_widget_destroy (pDialog);
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Clock", pSubMenu, CD_APPLET_MY_MENU)
	
	CD_APPLET_ADD_IN_MENU ("Set up time and date", cd_clock_launch_time_admin, pSubMenu)
	
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
