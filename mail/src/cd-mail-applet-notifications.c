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


void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet);



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

    cd_mail_force_update(myApplet);

CD_APPLET_ON_MIDDLE_CLICK_END

static void _cd_mail_update_account (GtkMenuItem *menu_item, CDMailAccount *pMailAccount)
{
  if( pMailAccount )
  {
    cairo_dock_launch_measure(pMailAccount->pAccountMailTimer);
  }
}

CD_APPLET_ON_BUILD_MENU_BEGIN

	 GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
        if(myData.pMailAccounts && myData.pMailAccounts->len > 0)
        {
          guint i;
          
          GtkWidget *pRefreshAccountSubMenu = CD_APPLET_ADD_SUB_MENU (_("Refresh a mail account"), pSubMenu);

          /* add a "update account" item for each mailbox */
          for (i = 0; i < myData.pMailAccounts->len; i ++)
          {
            CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
            CD_APPLET_ADD_IN_MENU_WITH_DATA (pMailAccount->name, _cd_mail_update_account, pRefreshAccountSubMenu, pMailAccount);
          }
        }

		CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
CD_APPLET_ON_BUILD_MENU_END


void cd_mail_force_update(CairoDockModuleInstance *myApplet)
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


/*
 * Extraire les donnees des mails (nombre, titres, resume eventuellement)
 * et les placer dans une structure dediee a l'affichage
 */
void cd_mail_read_folder_data(CDMailAccount *pMailAccount)
{
  if( !pMailAccount ) return;
  CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;
  
  if( pMailAccount->dirtyfied == TRUE )
  {
    cd_debug( "cd_mail: The mailbox %s has changed", pMailAccount->name );

    myData.bNewMailFound = TRUE;
  }
}

void
_mail_draw_main_icon (CairoDockModuleInstance *myApplet, gchar **mailbox_names, guint *new_message_counts)
{
	g_return_if_fail (myDrawContext != NULL);

	if (myData.iNbUnreadMails <= 0)
	{
	    cairo_dock_remove_dialog_if_any (myIcon);
	    if( myData.bNewMailFound )
	    {
        cairo_dock_show_temporary_dialog (_("No unread mail in your mailboxes"), myIcon, myContainer, 1000);
      }
      
      {
          g_free (myIcon->acFileName);

          //Chargement de l'image "pas de mail"
          myIcon->acFileName = g_strdup(myConfig.cNoMailUserImage);
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

        {
          g_free (myIcon->acFileName);

            //Chargement de l'image "il y a un des mails"
          myIcon->acFileName = g_strdup(myConfig.cHasMailUserImage);
        }
	}

  if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
  {
    if( myData.iNbUnreadMails > 0 )
    {
      cairo_dock_launch_animation (myContainer);
    }

    cd_mail_render_3D_to_texture (myApplet);
  }
  else
  {
    CD_APPLET_SET_IMAGE_ON_MY_ICON (myIcon->acFileName);
  }

  CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iNbUnreadMails);
}

gboolean cd_mail_update_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation)
{
  static gboolean bSkipThisFrame = TRUE;
  
	if (pContainer != myContainer)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;

  bSkipThisFrame = !bSkipThisFrame;
  
  if( bSkipThisFrame && bContinueAnimation )
  {
    *bContinueAnimation = TRUE;
    return CAIRO_DOCK_LET_PASS_NOTIFICATION;
  }
	
  if( myData.iNbUnreadMails > 0 || myData.current_rotX != 0 )
  {
    myData.current_rotX += 5;
  }
  if( myData.iNbUnreadMails > 0 || (myData.current_rotY % 90) != 0 )
  {
    myData.current_rotY += 2;
  }

  if( myData.current_rotX>=360.f ) myData.current_rotX -= 360.f;
  if( myData.current_rotY>=360.f ) myData.current_rotY -= 360.f;

  cd_mail_render_3D_to_texture (myApplet);

  if( bContinueAnimation )
  {
    if( myData.iNbUnreadMails > 0 || myData.current_rotX != 0 || (myData.current_rotY % 90) != 0 )
    {
      *bContinueAnimation = TRUE;
    }
    else
    {
      *bContinueAnimation = FALSE;
    }
  }
  
  return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}

void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet)
{
	if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
		return ;

	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	double fRatio = (myDock ? myDock->fRatio : 1);
	int iWidth = (int) myIcon->fWidth / fRatio * fMaxScale;
	int iHeight = (int) myIcon->fHeight / fRatio * fMaxScale;

  cd_debug( "iWidth=%d iHeight=%d", iWidth, iHeight);
  
	glPushMatrix ();

  glScalef(0.8*iWidth, 0.8*iHeight, 1.0);
  glTranslatef(0., 0., -1.0);

  glRotatef(myData.current_rotX, 1.0f, 0.0f, 0.0f);  /* rotate on the X axis */
  glRotatef(myData.current_rotY, 0.0f, 1.0f, 0.0f);  /* rotate on the Y axis */
//  glRotatef(30.f, 0.0f, 0.0f, 1.0f);  /* rotate on the Z axis */

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend le cube transparent.
  glAlphaFunc ( GL_GREATER, 0.1 ) ;
  glEnable ( GL_ALPHA_TEST ) ;

	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	//glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, myData.iNbUnreadMails > 0?myData.iHasMailTexture:myData.iNoMailTexture);
	//glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // type de generation des coordonnees de la texture
	//glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	//glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	//glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glCallList (myData.iCubeCallList);
//	glCallList (myData.iCapsuleCallList);

	glDisable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);

  glDisable ( GL_ALPHA_TEST ) ;
	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
	glPopMatrix ();

	cairo_dock_end_draw_icon (myIcon, myContainer);
	CD_APPLET_REDRAW_MY_ICON;
}

void cd_mail_update_account_status( CDMailAccount *pUpdatedMailAccount )
{
  if( !pUpdatedMailAccount ) return;
  cd_mail_update_status( pUpdatedMailAccount->pAppletInstance );
}

gboolean cd_mail_update_status( CairoDockModuleInstance *myApplet )
{
	CDMailAccount *pMailAccount;
  GList *pIconList = NULL;
  Icon *pIcon;

  gchar **mailbox_names = NULL;
  guint *new_message_counts = NULL;
  guint i;

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

  _mail_draw_main_icon(myApplet, mailbox_names, new_message_counts);

  if (myDesklet)
      gtk_widget_queue_draw (myDesklet->pWidget);
  else
      CD_APPLET_REDRAW_MY_ICON;

  g_strfreev(mailbox_names);
  g_free(new_message_counts);

  myData.bNewMailFound = FALSE;

  return TRUE;
}

