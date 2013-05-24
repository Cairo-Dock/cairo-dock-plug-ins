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
#include <time.h>
#include <math.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-etpan.h"
#include "cd-mail-applet-notifications.h"


CD_APPLET_ON_CLICK_BEGIN
	/**if (myIcon->Xid != 0)
	{
		if (cairo_dock_get_current_active_window () == myIcon->Xid && myTaskBar.bMinimizeOnClick)
			cairo_dock_minimize_xwindow (myIcon->Xid);
		else
			cairo_dock_show_xwindow (myIcon->Xid);
	}
	else*/
	{
		/* if a specific mail application has been specified for this account, use this one */
		gchar *cMailAppToLaunch = NULL;
		if (myData.pMailAccounts->len == 1) // 1 seul compte => pas de sous-dock, donc pas d'icone sur laquelle recuperer la commande.
		{
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, 0);
			if (pMailAccount)
				cMailAppToLaunch = pMailAccount->cMailApp;
		}
		else if( CD_APPLET_CLICKED_ICON && CD_APPLET_CLICKED_ICON->cCommand &&
			strlen(CD_APPLET_CLICKED_ICON->cCommand)>0)
		{
			cMailAppToLaunch = CD_APPLET_CLICKED_ICON->cCommand;
		}
		if (cMailAppToLaunch == NULL)
			cMailAppToLaunch = myConfig.cMailApplication;
		if (cMailAppToLaunch != NULL)
		{
			gboolean r = cairo_dock_launch_command (cMailAppToLaunch);
			if (!r)
			{
				cd_warning ("when couldn't execute '%s'", cMailAppToLaunch);
				gldi_dialog_show_temporary_with_icon_printf (D_("A problem occured\nIf '%s' is not your usual mail application,\nyou can change it in the configuration panel of this module"), myIcon, myContainer, 5000, "same icon", cMailAppToLaunch);
			}
		}
		else
		{
			gldi_dialog_show_temporary_with_icon (D_("No mail application is defined,\nyou can define it in the configuration panel of this module"), myIcon, myContainer, 5000, "same icon");
		}
	}
CD_APPLET_ON_CLICK_END



static void _cd_mail_update_account (GtkMenuItem *menu_item, CDMailAccount *pMailAccount)
{
	if( pMailAccount )
	{
		if (cairo_dock_task_is_running (pMailAccount->pAccountMailTimer))
		{
			cd_debug ("account is being checked, wait a second\n");
			return;
		}

		GldiModuleInstance *myApplet = pMailAccount->pAppletInstance;
		Icon *pIcon = (pMailAccount->icon ? pMailAccount->icon : myIcon);
		GldiContainer *pContainer = (pMailAccount->icon ? CD_APPLET_MY_ICONS_LIST_CONTAINER : myContainer);
		cairo_dock_set_quick_info (pIcon, pContainer, "...");
		
		cairo_dock_launch_task(pMailAccount->pAccountMailTimer);
	}
}

static void _cd_mail_force_update(GldiModuleInstance *myApplet)
{
	guint i;
	if (myData.pMailAccounts != NULL)
	{
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			if( pMailAccount )
			{
				_cd_mail_update_account(NULL, pMailAccount);
			}
		}
	}
}

static void _cd_mail_update_all_accounts (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	_cd_mail_force_update (myApplet);
}

CD_APPLET_ON_MIDDLE_CLICK_BEGIN

    _cd_mail_force_update(myApplet);

CD_APPLET_ON_MIDDLE_CLICK_END

static void _cd_mail_launch_mail_appli (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	cairo_dock_launch_command (myConfig.cMailApplication);
}

static void _cd_mail_mark_all_as_read (GtkMenuItem *menu_item, GldiModuleInstance *myApplet)
{
	guint i;
	if (myData.pMailAccounts != NULL)
	{
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			if( pMailAccount )
			{
				cd_mail_mark_all_mails_as_read(pMailAccount);	
			}
		}
	}
	_cd_mail_force_update(myApplet);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	if(myData.pMailAccounts && myData.pMailAccounts->len > 0)
	{
		if (myData.pMailAccounts->len > 1)  // many accounts -> list them in a sub-menu
		{
			// add a "update account" item for each mailbox
			GtkWidget *pRefreshAccountSubMenu = CD_APPLET_ADD_SUB_MENU_WITH_IMAGE (D_("Refresh a mail account"), CD_APPLET_MY_MENU, GTK_STOCK_REFRESH);
			guint i;
			for (i = 0; i < myData.pMailAccounts->len; i ++)
			{
				CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
				CD_APPLET_ADD_IN_MENU_WITH_DATA (pMailAccount->name, _cd_mail_update_account, pRefreshAccountSubMenu, pMailAccount);
			}
			gchar *cLabel = g_strdup_printf ("%s (%s)", D_("Refresh all"), D_("middle-click"));
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (cLabel, GTK_STOCK_REFRESH, _cd_mail_update_all_accounts, CD_APPLET_MY_MENU, myApplet);
			g_free (cLabel);
		}
		else  // 1 account -> in main menu
		{
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, 0);
			gchar *cLabel = g_strdup_printf (D_("Refresh %s"), pMailAccount->name);
			CD_APPLET_ADD_IN_MENU_WITH_STOCK_AND_DATA (cLabel, GTK_STOCK_REFRESH, _cd_mail_update_account, CD_APPLET_MY_MENU, pMailAccount);
			g_free (cLabel);
		}
	}
	
	CD_APPLET_ADD_IN_MENU_WITH_STOCK (D_("Mark all emails as read"), GTK_STOCK_OK, _cd_mail_mark_all_as_read, CD_APPLET_MY_MENU);
	if (myConfig.cMailApplication)
	{
		gchar *cLabel = g_strdup_printf (D_("Launch %s"), myConfig.cMailApplication);
		CD_APPLET_ADD_IN_MENU_WITH_STOCK (cLabel, GTK_STOCK_EXECUTE, _cd_mail_launch_mail_appli, CD_APPLET_MY_MENU);
		g_free (cLabel);
	}
CD_APPLET_ON_BUILD_MENU_END

void _cd_mail_show_current_mail(CDMailAccount *pMailAccount)
{
	GldiModuleInstance *myApplet = pMailAccount->pAppletInstance;
	GList *l = pMailAccount->pUnseenMessageList;
	const gchar *cMessage = "";
	gint i = myData.iCurrentlyShownMail;

	if( myData.iCurrentlyShownMail < 0 )
		myData.iCurrentlyShownMail = 0;
	
	for( ; i > 0 && l != NULL; i-- )
	{
		if( l->next == NULL ) break;
		l = l->next;
	}
	if( i > 0 ) // just in case, to stay inside boundaries
	{
		myData.iCurrentlyShownMail -= i;
	}
	if( l )
		cMessage = l->data;
	gtk_text_buffer_set_text(myData.pTextBuffer, cMessage, -1);

	if( myData.iCurrentlyShownMail == 0 )
	{
		gtk_widget_set_sensitive( myData.pPrevButton, FALSE );		
	}
	else
	{
		gtk_widget_set_sensitive( myData.pPrevButton, TRUE );		
	}
	if( l->next == NULL )
	{
		gtk_widget_set_sensitive( myData.pNextButton, FALSE );
	}
	else
	{
		gtk_widget_set_sensitive( myData.pNextButton, TRUE );		
	}
}

void _cd_mail_show_prev_mail_cb(GtkWidget *widget, CDMailAccount *pMailAccount)
{
	GldiModuleInstance *myApplet = pMailAccount->pAppletInstance;

	myData.iCurrentlyShownMail--;
	_cd_mail_show_current_mail(pMailAccount);
}

void _cd_mail_show_next_mail_cb(GtkWidget *widget, CDMailAccount *pMailAccount)
{
	GldiModuleInstance *myApplet = pMailAccount->pAppletInstance;

	myData.iCurrentlyShownMail++;
	_cd_mail_show_current_mail(pMailAccount);
}

void _cd_mail_close_preview_cb(GtkWidget *widget, CDMailAccount *pMailAccount)
{
	GldiModuleInstance *myApplet = pMailAccount->pAppletInstance;

	if( myData.pMessagesDialog != NULL )
	{
		gldi_object_unref (GLDI_OBJECT(myData.pMessagesDialog));
		myData.pMessagesDialog = NULL;
	}
}

GtkWidget *cd_mail_messages_container_new(CDMailAccount *pMailAccount)
{
	GldiModuleInstance *myApplet = pMailAccount->pAppletInstance;

	/*
	 * Appearance of the container:
	 * ____________________________
	 * | Subject: xxxxxx           |    <---- simple text area
	 * | From:    xxxxxx           |    <---- simple text area
	 * | bla bla blablabla bla .. ^|
	 * | .. blla blablabla bla .. ||    <---- this is a multi-line, scrollable text
	 * | bla ... abl  abla.       v|
	 * | <--       CLOSE       --> |    <---- 3 buttons, attached Left,Center,Right
	 * -----------------------------
	 */
	GtkWidget *vbox = _gtk_vbox_new (0);

	GtkWidget *pTextView = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(pTextView), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(pTextView), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pTextView), GTK_WRAP_WORD );

	myData.pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));

	GtkWidget* pScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pScrolledWindow), pTextView );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	
	gtk_box_pack_start(GTK_BOX(vbox), pScrolledWindow, TRUE, TRUE, 0);

	GtkWidget *hbox = _gtk_hbox_new(0);
	myData.pPrevButton = gtk_button_new_from_stock( GTK_STOCK_GO_BACK );
	GtkWidget *pCloseButton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
	myData.pNextButton = gtk_button_new_from_stock( GTK_STOCK_GO_FORWARD );

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(myData.pPrevButton), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(pCloseButton), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(myData.pNextButton), FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	
	// then we need to put the callbacks
	g_signal_connect( G_OBJECT(myData.pPrevButton), "clicked", G_CALLBACK(_cd_mail_show_prev_mail_cb), (gpointer)pMailAccount );
	g_signal_connect( G_OBJECT(myData.pNextButton), "clicked", G_CALLBACK(_cd_mail_show_next_mail_cb), (gpointer)pMailAccount );
	g_signal_connect( G_OBJECT(pCloseButton),       "clicked", G_CALLBACK(_cd_mail_close_preview_cb), (gpointer)pMailAccount );

	GList *l = pMailAccount->pUnseenMessageList;
	const gchar *cMessage = "";
	if( l )
		cMessage = l->data;

	gtk_text_buffer_set_text(myData.pTextBuffer, cMessage, -1);
	myData.iCurrentlyShownMail = 0;

	gtk_widget_set_sensitive( myData.pPrevButton, FALSE );
	if( l->next == NULL )
	{
		gtk_widget_set_sensitive( myData.pNextButton, FALSE );
	}

	return vbox;
}

CD_APPLET_ON_SCROLL_BEGIN
	if (myData.pMailAccounts == NULL || !myConfig.bShowMessageContent)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);

	CDMailAccount *pMailAccount = NULL;
	guint i;
	int r;
	for (i = 0; i < myData.pMailAccounts->len; i++)
	{
		pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
		if (pMailAccount->name && (myData.pMailAccounts->len == 1 || strcmp (pMailAccount->name, CD_APPLET_CLICKED_ICON->cName) == 0))
			break ;
	}
	if (i == myData.pMailAccounts->len || pMailAccount == NULL)
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	
	if (cairo_dock_task_is_running (pMailAccount->pAccountMailTimer))
	{
		cd_debug ("account is being checked, wait a second\n");
		CD_APPLET_LEAVE (GLDI_NOTIFICATION_LET_PASS);
	}
	
	/* Ensure the connection is alive */
	r = mailfolder_connect(pMailAccount->folder);
	if (r != MAIL_NO_ERROR)  // no connexion, we keep the previous satus.
	{
		cd_warning ("mail : couldn't connect to '%s'", pMailAccount->name);
		pMailAccount->bError = TRUE;
	}
	else
	{
		if( myData.pMessagesDialog == NULL )
		{
			cd_debug ( "Displaying messages" );
			if( pMailAccount->pUnseenMessageList != NULL )
			{
				myData.pMessagesDialog = gldi_dialog_show (D_("Mail"),
					myIcon, myContainer,
					0,
					"same icon",
					cd_mail_messages_container_new(pMailAccount),
					NULL, NULL, NULL);
			}
			else
				cd_debug ("Not Displaying messages, pUnseenMessageList empty");
		}
		else
		{
			// scroll one message
			if (CD_APPLET_SCROLL_DOWN)
			{
				_cd_mail_show_prev_mail_cb(NULL, pMailAccount);
			}
			else if (CD_APPLET_SCROLL_UP)
			{
				_cd_mail_show_next_mail_cb(NULL, pMailAccount);
			}
		}
	}

	cd_mail_mark_all_mails_as_read(pMailAccount);	
	
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
