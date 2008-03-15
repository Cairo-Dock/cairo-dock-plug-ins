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

CD_APPLET_INCLUDE_MY_VARS

extern AppletConfig myConfig;
extern AppletData myData;


CD_APPLET_ABOUT (_D("This is the mail applet\n made by Christophe Chapuis for Cairo-Dock"))


CD_APPLET_ON_CLICK_BEGIN

    xfce_mailwatch_force_update(myData.mailwatch);

CD_APPLET_ON_CLICK_END

static void _cd_mail_add_account (GtkMenuItem *menu_item, gpointer *data)
{
    // display a dialog window to select the informations to show
    config_add_btn_clicked_cb(GTK_WIDGET(myContainer->pWidget), myData.mailwatch);

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

CD_APPLET_ON_BUILD_MENU_BEGIN
	CD_APPLET_ADD_SUB_MENU ("mail", pSubMenu, CD_APPLET_MY_MENU)
		CD_APPLET_ADD_IN_MENU (_("Add a new mail account"), _cd_mail_add_account, pSubMenu)
		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu)
CD_APPLET_ON_BUILD_MENU_END

void
mailwatch_new_messages_changed_cb(XfceMailwatch *mailwatch, gpointer arg, gpointer user_data)
{
    myData.iNbUnreadMails = GPOINTER_TO_UINT( arg );

    cd_message( "mailwatch_new_messages_changed_cb: %d new messages !", myData.iNbUnreadMails );
	if (myData.iNbUnreadMails <= 0)
	{
	    if( myData.pNoMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pNoMailSurface)
	    }
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

        cairo_dock_show_temporary_dialog (ttip_str->str, myIcon, myContainer, 5000);
        g_string_free(ttip_str, TRUE);

	    if( myData.pHasMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pHasMailSurface)
	    }
	}
    cd_message( "mailwatch_new_messages_changed_cb: Leaving." );
    CD_APPLET_REDRAW_MY_ICON
}

