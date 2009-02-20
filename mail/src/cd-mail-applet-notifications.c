/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <time.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-notifications.h"


CD_APPLET_INCLUDE_MY_VARS



CD_APPLET_ABOUT (D_("This is the mail applet\n made by Christophe Chapuis for Cairo-Dock"))


#define _add_icon(account_name, associated_command, nbUnreadMails, i)\
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
		pIcon->acCommand = g_strdup (associated_command);\
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
        cairo_dock_show_temporary_dialog (D_("A problem occured\nIf '%s' is not your usual mail application,\nyou can change it in the conf panel of this module"), myIcon, myContainer, 5000, myConfig.cMailApplication);
      }
    }

CD_APPLET_ON_CLICK_END

CD_APPLET_ON_MIDDLE_CLICK_BEGIN

    cd_mail_force_update();

CD_APPLET_ON_MIDDLE_CLICK_END

void _cd_mail_update_account(CDMailAccount *pMailAccount)
{
  if( pMailAccount )
  {
    cairo_dock_launch_measure(pMailAccount->pAccountMailTimer);
  }
}

CD_APPLET_ON_BUILD_MENU_BEGIN

	CD_APPLET_ADD_SUB_MENU ("mail", pSubMenu, CD_APPLET_MY_MENU);
        if(myData.pMailAccounts && myData.pMailAccounts->len > 0)
        {
          int i;
          
          CD_APPLET_ADD_SUB_MENU (_("Refresh a mail account"), pRefreshAccountSubMenu, pSubMenu);

          /* add a "update account" item for each mailbox */
          for (i = 0; i < myData.pMailAccounts->len; i ++)
          {
            CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
            CD_APPLET_ADD_IN_MENU_WITH_DATA (pMailAccount->name, _cd_mail_update_account, pRefreshAccountSubMenu, pMailAccount);
          }
        }

		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


void cd_mail_force_update(void)
{
	int i;
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


/*
 * Extraire les donnees des mails (nombre, titres, resume eventuellement)
 * et les placer dans une structure dediee a l'affichage
 */
void cd_mail_read_folder_data(CDMailAccount *pMailAccount)
{
  if( pMailAccount->dirtyfied == TRUE )
  {
    cd_debug( "cd_mail: The mailbox %s has changed", pMailAccount->name );

    myData.bNewMailFound = TRUE;
  }
}

void
_mail_draw_main_icon (gchar **mailbox_names, guint *new_message_counts)
{
	g_return_if_fail (myDrawContext != NULL);

	if (myData.iNbUnreadMails <= 0)
	{
	    cairo_dock_remove_dialog_if_any (myIcon);
	    if( myData.bNewMailFound )
	    {
        cairo_dock_show_temporary_dialog (_("No unread mail in your mailboxes"), myIcon, myContainer, 1000);
      }
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
            CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName);
        }
	}
	else
	{
	    if( myData.bNewMailFound )
	    {
          GString *ttip_str = g_string_sized_new(32);
          gint i;

          /* don't play more often than every 4 seconds... */
          time_t currentTime = time(NULL);
          if(currentTime-myConfig.timeEndOfSound > 4 &&
             myData.bNewMailFound == TRUE)
          {
            cairo_dock_play_sound(myConfig.cNewMailUserSound);
            myConfig.timeEndOfSound = time(NULL);
          }

          g_string_append_printf(ttip_str, "%s %d %s", _("You have"), myData.iNbUnreadMails,myData.iNbUnreadMails>1?_("new mails :"):_("new mail :"));

          for(i = 0; mailbox_names[i]; i++) {
              if(new_message_counts[i] > 0) {
                  g_string_append_printf(ttip_str, "\n    %d in %s",
                          new_message_counts[i], mailbox_names[i]);
              }
          }

          cairo_dock_remove_dialog_if_any (myIcon);
          cairo_dock_show_temporary_dialog (ttip_str->str, myIcon, myContainer, 5000);

          g_string_free(ttip_str, TRUE);
        }
/*	    if( myData.pHasMailSurface )
	    {
            CD_APPLET_SET_SURFACE_ON_MY_ICON (myData.pHasMailSurface)
	    }
        else */
        {
            //Chargement de l'image "il y a un des mails"
            CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHasMailUserImage);
        }
	}

  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iNbUnreadMails);
}

void cd_mail_update_status( gpointer *data )
{
	CDMailAccount *pMailAccount;
  GList *pIconList = NULL;
  Icon *pIcon;

  gchar **mailbox_names = NULL;
  guint *new_message_counts = NULL;
  gint i;

  myData.iNbUnreadMails = 0;
	
	if (myData.pMailAccounts != NULL)
	{
    mailbox_names = g_new0( gchar* , myData.pMailAccounts->len+1 );
    new_message_counts = g_new0( guint, myData.pMailAccounts->len );
	  
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			if( pMailAccount )
			{
        mailbox_names[i] = g_strdup( pMailAccount->name );
        new_message_counts[i] = pMailAccount->iNbUnseenMails;
        
			  myData.iNbUnreadMails += pMailAccount->iNbUnseenMails;
      }
		}
	}

  cd_message( "cd_mail_update_status: %d new messages !", myData.iNbUnreadMails );

  if( mailbox_names[0] && mailbox_names[1] )
  {
      for(i = 0; mailbox_names[i]; i++)
      {
          _add_icon (mailbox_names[i], mailbox_names[i], new_message_counts[i], i);
      }
  }

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
            CD_APPLET_CREATE_MY_SUBDOCK (pIconList, NULL);
        }
    }
    else  // on a deja notre sous-dock, on remplace juste ses icones.
    {
        cd_message ("  rechargement du sous-dock mail");
        if (pIconList == NULL)  // inutile de le garder.
        {
            CD_APPLET_DESTROY_MY_SUBDOCK;
        }
        else
        {
            CD_APPLET_LOAD_ICONS_IN_MY_SUBDOCK (pIconList);
        }
    }
  }
  else
  {
    if (myIcon->pSubDock != NULL)
    {
      CD_APPLET_DESTROY_MY_SUBDOCK;
    }
    myDesklet->icons = pIconList;

    if(NULL == mailbox_names[0] || NULL == mailbox_names[1])
    {
      cairo_dock_set_desklet_renderer_by_name (myDesklet, "Simple", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, NULL);
    }
    else
    {        
      gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
      cairo_dock_set_desklet_renderer_by_name (myDesklet, "Caroussel", NULL, CAIRO_DOCK_LOAD_ICONS_FOR_DESKLET, pConfig);
    }

    myDrawContext = cairo_create (myIcon->pIconBuffer);
  }

  _mail_draw_main_icon(mailbox_names, new_message_counts);

  if (myDesklet)
      gtk_widget_queue_draw (myDesklet->pWidget);
  else
      CD_APPLET_REDRAW_MY_ICON;

  g_strfreev(mailbox_names);
  g_free(new_message_counts);

  myData.bNewMailFound = FALSE;
}

