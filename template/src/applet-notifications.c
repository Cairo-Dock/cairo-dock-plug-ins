
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <glib/gi18n.h>

#include "applet-notifications.h"

CD_APPLET_INCLUDE_MY_VARS

extern CairoDockDesktopEnv my_logout_iDesktopEnv;


CD_APPLET_ABOUT (_D("This is the CD_APPLET_NAME applet\n made by CD_MY_NAME for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN
	if (my_logout_iDesktopEnv == CAIRO_DOCK_GNOME)
	{
		system ("gnome-session-save --kill --gui");
	}
	else if (my_logout_iDesktopEnv == CAIRO_DOCK_KDE)
	{
		GtkWidget *dialog = gtk_message_dialog_new (NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"Log out ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer == GTK_RESPONSE_YES)
		{
			system ("dcop ksmserver default logout 0 0 0");  // kdmctl shutdown reboot forcenow  // kdeinit_shutdown
		}
	}
CD_APPLET_ON_CLICK_END


CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("Logout", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END
