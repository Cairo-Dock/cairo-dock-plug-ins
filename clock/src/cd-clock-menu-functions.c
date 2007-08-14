/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

**********************************************************************************/
#include <stdlib.h>

#include "cd-clock-menu-functions.h"


void cd_clock_launch_time_admin (GtkMenuItem *menu_item, gpointer *data)
{
	system ("gksu time-admin &");
}


void cd_clock_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pMessageDialog = gtk_message_dialog_new (NULL,
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is a clock applet made by Fabrice Rey (fabounet_03@yahoo.fr) for Cairo-Dock.\n The analogic representation is a port of the well-known Cairo-Clock from MacSlow (http://macslow.thepimp.net).");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}
