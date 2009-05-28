/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <time.h>
#include <math.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-etpan.h"
#include "cd-mail-applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN

	if( myConfig.cMailApplication )
	{
		if (myIcon->Xid != 0)
		{
			if (cairo_dock_get_current_active_window () == myIcon->Xid && myTaskBar.bMinimizeOnClick)
				cairo_dock_minimize_xwindow (myIcon->Xid);
			else
				cairo_dock_show_xwindow (myIcon->Xid);
		}
		else
		{
			gboolean r = cairo_dock_launch_command (myConfig.cMailApplication);
			
			if (!r)
			{
				cd_warning ("when couldn't execute '%s'", myConfig.cMailApplication);
				cairo_dock_show_temporary_dialog (D_("A problem occured\nIf '%s' is not your usual mail application,\nyou can change it in the conf panel of this module"), myIcon, myContainer, 5000, myConfig.cMailApplication);
			}
		}
	}

CD_APPLET_ON_CLICK_END


static void _cd_mail_force_update(CairoDockModuleInstance *myApplet)
{
	guint i;
	if (myData.pMailAccounts != NULL)
	{
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			if( pMailAccount )
			{
				cairo_dock_launch_measure(pMailAccount->pAccountMailTimer);
			}
		}
	}
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN

    _cd_mail_force_update(myApplet);

CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_mail_update_account (GtkMenuItem *menu_item, CDMailAccount *pMailAccount)
{
	if( pMailAccount )
	{
		cairo_dock_launch_measure(pMailAccount->pAccountMailTimer);
	}
}
static void _cd_mail_launch_mail_appli (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cairo_dock_launch_command (myConfig.cMailApplication);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
    if(myData.pMailAccounts && myData.pMailAccounts->len > 0)
    {
        /* add a "update account" item for each mailbox */
        GtkWidget *pRefreshAccountSubMenu = CD_APPLET_ADD_SUB_MENU (D_("Refresh a mail account"), pSubMenu);
        
        guint i;
        for (i = 0; i < myData.pMailAccounts->len; i ++)
        {
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (pMailAccount->name, _cd_mail_update_account, pRefreshAccountSubMenu, pMailAccount);
        }
    }
	if (myConfig.cMailApplication)
	{
		gchar *cLabel = g_strdup_printf (D_("Launch %s"), myConfig.cMailApplication);
		CD_APPLET_ADD_IN_MENU (cLabel, _cd_mail_launch_mail_appli, pSubMenu);
		g_free (cLabel);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);

CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_SCROLL_BEGIN
	if (myData.pMailAccounts == NULL)
		return ;
	
	CDMailAccount *pMailAccount;
	guint i;
	for (i = 0; i < myData.pMailAccounts->len; i++)
	{
		pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
		if (pMailAccount->name && strcmp (pMailAccount->name, CD_APPLET_CLICKED_ICON->acName) == 0)
			break ;
	}
	if (i == myData.pMailAccounts->len || pMailAccount == NULL)
		return ;
	
	if (cairo_dock_measure_is_running (pMailAccount->pAccountMailTimer))
	{
		g_print ("account is being checked, wait a second\n");
		return ;
	}
	
	struct mail_flags *pFlags = NULL;
	mailmessage *pMessage = NULL;
	int r;
	GList *l;
	gchar *cMessage;
	for (l = pMailAccount->pUnseenMessageList; l != NULL; l = l->next)
	{
		cMessage = l->data;
		cairo_dock_remove_dialog_if_any (CD_APPLET_CLICKED_ICON);
		cairo_dock_show_temporary_dialog_with_icon (cMessage, CD_APPLET_CLICKED_ICON, CD_APPLET_CLICKED_CONTAINER, 10e3, "same icon");
		
		// on marque le message comme lu.
		///r = mailmessage_get_flags (pMessage, &pFlags);
	}
	
CD_APPLET_ON_SCROLL_END


CD_APPLET_ON_UPDATE_ICON_BEGIN
	double fSpeedX, fSpeedY;
	if (myData.iNbUnreadMails == 0)
	{
		fSpeedX = 2.;
		fSpeedY = 2.;
	}
	else
	{
		fSpeedX = 2 * MIN (5., sqrt (myData.iNbUnreadMails));
		fSpeedY = fSpeedX/2;
	}
	if( myData.iNbUnreadMails > 0 || myData.current_rotX != 0 )  // mails non lus ou on finit la rotation en cours.
	{
		myData.current_rotX += fSpeedX;
	}
	if( myData.iNbUnreadMails > 0 || myData.current_rotY != 0 )  // mails non lus ou on finit la rotation en cours.
	{
		myData.current_rotY += fSpeedY;
	}

	if( myData.current_rotX>=360.f )
	{
		if (myData.iNbUnreadMails > 0)
			myData.current_rotX -= 360.f;  // on se ramene juste dans [0;360[
		else
			myData.current_rotX = 0;  // on s'arrete la.
	}
	if( myData.current_rotY>=360.f )
	{
		if (myData.iNbUnreadMails > 0)
			myData.current_rotY -= 360.f;
		else
			myData.current_rotY = 0;
	}
	
	cd_mail_render_3D_to_texture (myApplet);

	if( myData.iNbUnreadMails <= 0 && myData.current_rotX == 0 && myData.current_rotY == 0 )
	{
		CD_APPLET_STOP_UPDATE_ICON;
	}
CD_APPLET_ON_UPDATE_ICON_END
