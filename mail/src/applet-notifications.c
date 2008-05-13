/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>

#include "applet-struct.h"
#include "applet-notifications.h"


extern void config_add_btn_clicked_cb(GtkWidget *w, XfceMailwatch *mailwatch);
extern gboolean config_do_edit_window_2 (GtkWidget *w, XfceMailwatch *mailwatch, XfceMailwatchMailbox *mailbox);

CD_APPLET_INCLUDE_MY_VARS



CD_APPLET_ABOUT (D_("This is the mail applet\n made by Christophe Chapuis for Cairo-Dock"))


#define _add_icon(account_name, nbUnreadMails, i)\
	if (account_name != NULL)\
	{\
		pIcon = g_new0 (Icon, 1);\
		pIcon->acName = g_strdup_printf ("%s", account_name);\
		pIcon->acFileName = g_strdup_printf ("%s", nbUnreadMails>0?myConfig.cHasMailUserImage:myConfig.cNoMailUserImage);\
		if (nbUnreadMails>0)\
			pIcon->cQuickInfo = g_strdup_printf ("%d", nbUnreadMails);\
		pIcon->fOrder = i;\
		pIcon->fScale = 1.;\
		pIcon->fAlpha = 1.;\
		pIcon->fWidthFactor = 1.;\
		pIcon->fHeightFactor = 1.;\
		pIcon->acCommand = g_strdup ("none");\
		pIcon->cParentDockName = g_strdup (myIcon->acName);\
		cd_debug (" + %s (%s)\n", pIcon->acName, pIcon->acFileName);\
		pIconList = g_list_append (pIconList, pIcon);\
	}

CD_APPLET_ON_CLICK_BEGIN

    // spawn the selected program
    if( myConfig.cMailApplication )
    {
		cd_message (">>> cd_mail: spawning %s\n", myConfig.cMailApplication);
		GError *erreur = NULL;
		g_spawn_command_line_async(myConfig.cMailApplication, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("Attention : when trying to execute '%s' : %s", myConfig.cMailApplication, erreur->message);
			g_error_free (erreur);
			//gchar *cTipMessage = g_strdup_printf ("A problem occured\nIf '%s' is not your usual file browser, you can change it in the conf panel of this module", myConfig.cDefaultBrowser);
			cairo_dock_show_temporary_dialog (D_("A problem occured\nIf '%s' is not your usual mail application,\nyou can change it in the conf panel of this module"), myIcon, myDock, 5000, myConfig.cMailApplication);
			//g_free (cTipMessage);
		}
    }

CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN

    xfce_mailwatch_force_update(myData.mailwatch);

CD_APPLET_ON_MIDDLE_CLICK_END

static void _cd_mail_add_account (GtkMenuItem *menu_item, gpointer *data)
{
    // display a dialog window to select the informations to show
    config_add_btn_clicked_cb(NULL, myData.mailwatch);

    GKeyFile *pKeyFile = g_key_file_new();

    if( g_key_file_load_from_file(pKeyFile,myIcon->pModule->cConfFilePath,
					     G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS,
					     NULL))
    {
        xfce_mailwatch_save_config(myData.mailwatch, pKeyFile);
        cairo_dock_write_keys_to_file (pKeyFile, myIcon->pModule->cConfFilePath);
    }
    g_key_file_free(pKeyFile);
}

static void _cd_mail_remove_account (GtkMenuItem *menu_item, gpointer *data)
{
    XfceMailwatchMailbox *mailbox = (XfceMailwatchMailbox*)( data );

    GKeyFile *pKeyFile = g_key_file_new();

    if( g_key_file_load_from_file(pKeyFile,myIcon->pModule->cConfFilePath,
					     G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS,
					     NULL))
    {
        cd_mailwatch_remove_account (myData.mailwatch, mailbox);
        xfce_mailwatch_save_config(myData.mailwatch, pKeyFile);
        cairo_dock_write_keys_to_file (pKeyFile, myIcon->pModule->cConfFilePath);
    }
    g_key_file_free(pKeyFile);

    xfce_mailwatch_force_update(myData.mailwatch);
}

static void _cd_mail_modify_account(GtkMenuItem *menu_item, gpointer *data)
{
    XfceMailwatchMailbox *mailbox = (XfceMailwatchMailbox*)( data );

    GKeyFile *pKeyFile = g_key_file_new();

    if( g_key_file_load_from_file(pKeyFile,myIcon->pModule->cConfFilePath,
					     G_KEY_FILE_KEEP_COMMENTS|G_KEY_FILE_KEEP_TRANSLATIONS,
					     NULL))
    {
        if( config_do_edit_window_2(NULL, myData.mailwatch, mailbox) )
        {
            xfce_mailwatch_save_config(myData.mailwatch, pKeyFile);
            cairo_dock_write_keys_to_file (pKeyFile, myIcon->pModule->cConfFilePath);
        }
    }
    g_key_file_free(pKeyFile);

    xfce_mailwatch_force_update(myData.mailwatch);
}

CD_APPLET_ON_BUILD_MENU_BEGIN

    GList *list_names = NULL, *list_data = NULL, *l = NULL, *l2 = NULL;
    guint i;

	CD_APPLET_ADD_SUB_MENU ("mail", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_("Add a new mail account"), _cd_mail_add_account, pSubMenu)

        cd_mailwatch_get_mailboxes_infos( myData.mailwatch, &list_names, &list_data );
        if( list_names && list_data )
        {
            CD_APPLET_ADD_SUB_MENU (_("Remove a mail account"), pRemoveAccountSubMenu, pSubMenu)

            /* add a "remove account" item for each mailbox */
            for(l = list_names, l2 = list_data; l && l2; l = l->next, l2 = l2->next) {
                CD_APPLET_ADD_IN_MENU_WITH_DATA (l->data, _cd_mail_remove_account, pRemoveAccountSubMenu, l2->data)
            }
            CD_APPLET_ADD_SUB_MENU (_("Modify a mail account"), pModifyAccountSubMenu, pSubMenu)

            /* add a "modify account" item for each mailbox */
            for(l = list_names, l2 = list_data; l && l2; l = l->next, l2 = l2->next) {
                CD_APPLET_ADD_IN_MENU_WITH_DATA (l->data, _cd_mail_modify_account, pModifyAccountSubMenu, l2->data)
            }
            g_list_free( list_names );
            g_list_free( list_data );
        }

		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

void
_mail_draw_main_icon (void)
{
	g_return_if_fail (myDrawContext != NULL);

	if (myData.iNbUnreadMails <= 0)
	{
	    cairo_dock_remove_dialog_if_any (myIcon);
        cairo_dock_show_temporary_dialog (_("No unread mail in your mailboxes"), myIcon, myContainer, 1000);
/*
	    if( myData.pNoMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pNoMailSurface)
        }
        else */
        {
            g_free (myIcon->acFileName);

            //Chargement de l'image "pas de mail"
            myIcon->acFileName = g_strdup(myConfig.cNoMailUserImage);
            CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName)
        }

        CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iNbUnreadMails)
	}
	else
	{
        GString *ttip_str = g_string_sized_new(32);
        gchar **mailbox_names = NULL;
        guint *new_message_counts = NULL;
        gint i;

        g_string_append_printf(ttip_str, "You have %d new mail%s:",myData.iNbUnreadMails,myData.iNbUnreadMails>1?"s":"");

        xfce_mailwatch_get_new_message_breakdown(myData.mailwatch,
                &mailbox_names, &new_message_counts);
        for(i = 0; mailbox_names[i]; i++) {
            if(new_message_counts[i] > 0) {
                g_string_append_printf(ttip_str, "\n    %d in %s",
                        new_message_counts[i], mailbox_names[i]);
            }
        }

        g_strfreev(mailbox_names);
        g_free(new_message_counts);

	    cairo_dock_remove_dialog_if_any (myIcon);
        cairo_dock_show_temporary_dialog (ttip_str->str, myIcon, myContainer, 5000);

        g_string_free(ttip_str, TRUE);

/*	    if( myData.pHasMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pHasMailSurface)
	    }
        else */
        {
            //Chargement de l'image "il y a un des mails"
            CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHasMailUserImage)
        }

        CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF (NULL)
	}
}

void
mailwatch_new_messages_changed_cb(XfceMailwatch *mailwatch, gpointer arg, gpointer user_data)
{
    myData.iNbUnreadMails = GPOINTER_TO_UINT( arg );

    cd_message( "mailwatch_new_messages_changed_cb: %d new messages !", myData.iNbUnreadMails );

    {
        GList *pIconList = NULL;
        Icon *pIcon;

        gchar **mailbox_names = NULL;
        guint *new_message_counts = NULL;
        gint i;

        xfce_mailwatch_get_new_message_breakdown(myData.mailwatch, &mailbox_names, &new_message_counts);
        if( mailbox_names[0] && mailbox_names[1] )
        {
            for(i = 0; mailbox_names[i]; i++)
            {
                _add_icon (mailbox_names[i], new_message_counts[i], i);
            }
        }

        g_strfreev(mailbox_names);
        g_free(new_message_counts);

        // on detruit l'ancienne liste d'icones (desklet ou sous-dock)
		if (myDesklet && myDesklet->icons != NULL)
		{
			g_list_foreach (myDesklet->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myDesklet->icons);
			myDesklet->icons = NULL;
		}
		if (myIcon->pSubDock != NULL)
		{
			g_list_foreach (myIcon->pSubDock->icons, (GFunc) cairo_dock_free_icon, NULL);
			g_list_free (myIcon->pSubDock->icons);
			myIcon->pSubDock->icons = NULL;
		}
		if( myDock )
		{
            if (myIcon->pSubDock == NULL)
            {
                if (pIconList != NULL)
                {
                    cd_message ("  creation du sous-dock mail");
                    CD_APPLET_CREATE_MY_SUBDOCK (pIconList, NULL)
                }
            }
            else  // on a deja notre sous-dock, on remplace juste ses icones.
            {
                cd_message ("  rechargement du sous-dock mail");
                if (pIconList == NULL)  // inutile de le garder.
                {
                    CD_APPLET_DESTROY_MY_SUBDOCK
                }
                else
                {
                    CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (pIconList)
                }
            }
		}
		else
		{
			if (myIcon->pSubDock != NULL)
			{
				CD_APPLET_DESTROY_MY_SUBDOCK
			}
			myDesklet->icons = pIconList;

            gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
            cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
			myDrawContext = cairo_create (myIcon->pIconBuffer);
		}
    }

    _mail_draw_main_icon();

    if (myDesklet)
        gtk_widget_queue_draw (myDesklet->pWidget);
    else
        CD_APPLET_REDRAW_MY_ICON
}

