/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <stdlib.h>

#include "file-manager-menu-functions.h"


void file_manager_about (GtkMenuItem *menu_item, gpointer *data)
{
	GtkWidget *pWidget = data[0];
	GtkWidget *pMessageDialog = gtk_message_dialog_new (GTK_WINDOW (pWidget),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"This is the truc applet made by Me (me@myadress.zglub) for Cairo-Dock");
	
	gtk_dialog_run (GTK_DIALOG (pMessageDialog));
	gtk_widget_destroy (pMessageDialog);
}
