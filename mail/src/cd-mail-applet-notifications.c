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
				cairo_dock_launch_task(pMailAccount->pAccountMailTimer);
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
		CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;
		Icon *pIcon = (pMailAccount->icon ? pMailAccount->icon : myIcon);
		CairoContainer *pContainer = (pMailAccount->icon ? CD_APPLET_MY_ICONS_LIST_CONTAINER : myContainer);
		cairo_dock_set_quick_info (myDrawContext, "...", pIcon, cairo_dock_get_max_scale (pContainer));
		
		cairo_dock_launch_task(pMailAccount->pAccountMailTimer);
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

void _cd_mail_show_current_mail(CDMailAccount *pMailAccount)
{
	CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;
	GList *l = pMailAccount->pUnseenMessageList;
	gchar *cMessage;
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
	CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;

	myData.iCurrentlyShownMail--;
	_cd_mail_show_current_mail(pMailAccount);
}

void _cd_mail_show_next_mail_cb(GtkWidget *widget, CDMailAccount *pMailAccount)
{
	CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;

	myData.iCurrentlyShownMail++;
	_cd_mail_show_current_mail(pMailAccount);
}

void _cd_mail_close_preview_cb(GtkWidget *widget, CDMailAccount *pMailAccount)
{
	CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;

	if( myData.pMessagesDialog != NULL )
	{
		cairo_dock_dialog_unreference (myData.pMessagesDialog);
		myData.pMessagesDialog = NULL;
	}
}

GtkWidget *cd_mail_messages_container_new(CDMailAccount *pMailAccount)
{
	CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;

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
	GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

	GtkWidget *pTextView = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(pTextView), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(pTextView), FALSE);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(pTextView), GTK_WRAP_WORD );

	myData.pTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(pTextView));

	GtkWidget* pScrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(pScrolledWindow), pTextView );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(pScrolledWindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	
	gtk_box_pack_start(GTK_BOX(vbox), pScrolledWindow, TRUE, TRUE, 0);
	
	GtkWidget *hbox = gtk_hbox_new(TRUE, 0);
	myData.pPrevButton = gtk_button_new_from_stock( GTK_STOCK_GO_BACK );
	GtkWidget *pCloseButton = gtk_button_new_from_stock( GTK_STOCK_CLOSE );
	myData.pNextButton = gtk_button_new_from_stock( GTK_STOCK_GO_FORWARD );

	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(myData.pPrevButton), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(pCloseButton), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(myData.pNextButton), FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	
	// then we need to put the callbacks
	gtk_signal_connect( GTK_OBJECT(myData.pPrevButton), "clicked", G_CALLBACK(_cd_mail_show_prev_mail_cb), (gpointer)pMailAccount );
	gtk_signal_connect( GTK_OBJECT(myData.pNextButton), "clicked", G_CALLBACK(_cd_mail_show_next_mail_cb), (gpointer)pMailAccount );
	gtk_signal_connect( GTK_OBJECT(pCloseButton),       "clicked", G_CALLBACK(_cd_mail_close_preview_cb), (gpointer)pMailAccount );

	GList *l = pMailAccount->pUnseenMessageList;
	gchar *cMessage;
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
	if (myData.pMailAccounts == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;

	CDMailAccount *pMailAccount;
	guint i;
	int r;
	for (i = 0; i < myData.pMailAccounts->len; i++)
	{
		pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
		if (pMailAccount->name && (myData.pMailAccounts->len == 1 || strcmp (pMailAccount->name, CD_APPLET_CLICKED_ICON->acName) == 0))
			break ;
	}
	if (i == myData.pMailAccounts->len || pMailAccount == NULL)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	
	if (cairo_dock_task_is_running (pMailAccount->pAccountMailTimer))
	{
		g_print ("account is being checked, wait a second\n");
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;
	}
	
	/* Ensure the connection is alive */
	r = mailfolder_connect(pMailAccount->folder);
	if (r != MAIL_NO_ERROR)  // no connexion, we keep the previous satus.
	{
		cd_warning ("mail : couldn't connect to '%s'", pMailAccount->name);
		pMailAccount->bError = TRUE;
	}
	
	g_print( "Displaying messages\n" );
	{
		if( myData.pMessagesDialog == NULL )
		{
			myData.pMessagesDialog = cairo_dock_show_dialog_full (_("Mail"),
				myIcon, myContainer,
				0,
				"same icon",
				cd_mail_messages_container_new(pMailAccount),
				NULL, NULL, NULL);
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
	
	GList *l, *l_Uid;
	gchar *cMessage;
	gchar *cMessageUid;
	mailmessage *pMessage;
	for (i = 1, l = pMailAccount->pUnseenMessageList, l_Uid = pMailAccount->pUnseenMessageUid; l != NULL && l_Uid != NULL; l = l->next, l_Uid = l_Uid->next, i++)
	{
		cMessage = l->data;
		cMessageUid = l_Uid->data;
		pMessage = NULL;
		
		// on marque le compte comme lu.
		if( !pMailAccount->bError )
		{
			struct mail_flags *pFlags = NULL;

			// on marque le message comme lu.
			cd_message ("Fetching message number %d...\n", i);
			
			r = mailfolder_get_message_by_uid (pMailAccount->folder, cMessageUid, &pMessage);  /// or result_messages - i ?...
			if (r != MAIL_NO_ERROR || pMessage == NULL)
			{
				cd_warning ("couldn't get the message number %d", i);
				continue;
			}
			r = mailmessage_get_flags (pMessage, &pFlags);
			if (r != MAIL_NO_ERROR || pFlags == NULL)
			{
				cd_warning ("couldn't get the message flags !", i);
				mailmessage_free (pMessage);
				continue;
			}
			
			pFlags->fl_flags &= ~MAIL_FLAG_NEW;
			pFlags->fl_flags |= MAIL_FLAG_SEEN;
			
			r = mailmessage_check( pMessage );
			mailmessage_free (pMessage);
		}
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
