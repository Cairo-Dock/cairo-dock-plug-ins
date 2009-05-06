/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Christophe Chapuis (for any bug report, please mail me to tofe@users.berlios.de)

******************************************************************************/

#include <string.h>
#include <math.h>
#include <cairo-dock.h>
#include <libetpan/libetpan.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-accounts.h"
#include "cd-mail-applet-etpan.h"

#define _add_icon(pMailAccount)\
	if (pMailAccount->name != NULL) {\
		pIcon = g_new0 (Icon, 1);\
		pIcon->acName = g_strdup (pMailAccount->name);\
		pIcon->acFileName = g_strdup (pMailAccount->iNbUnseenMails > 0 ? myConfig.cHasMailUserImage : myConfig.cNoMailUserImage);\
		if (pMailAccount->iNbUnseenMails>0)\
			pIcon->cQuickInfo = g_strdup_printf ("%d", pMailAccount->iNbUnseenMails);\
		else\
			pIcon->cQuickInfo = g_strdup ("...");\
		pIcon->fOrder = i;\
		pIcon->fScale = 1.;\
		pIcon->fAlpha = 1.;\
		pIcon->fWidthFactor = 1.;\
		pIcon->fHeightFactor = 1.;\
		pIcon->acCommand = g_strdup ("none");\
		pIcon->cParentDockName = g_strdup (myIcon->acName);\
		cd_debug (" + %s (%s)\n", pIcon->acName, pIcon->acFileName);\
		pIconList = g_list_append (pIconList, pIcon);\
		pMailAccount->icon = pIcon; }

void cd_mail_acquire_folder_data(CDMailAccount *pMailAccount)
{
	if( ! pMailAccount )
		return ;
	int r = 0;

	pMailAccount->dirtyfied = FALSE;

	/* get the folder structure */

	// create the folder, if not yet done
	if( pMailAccount->folder == NULL )
	{
		pMailAccount->folder = mailfolder_new(pMailAccount->storage, pMailAccount->name, NULL);
	}

	if( pMailAccount->storage && pMailAccount->folder )
	{
		/* Ensure the connection is alive */
		r = mailfolder_connect(pMailAccount->folder);
	    
		/* Fix initialization for feed storage */
		if( pMailAccount->driver == FEED_STORAGE )
		{
			if( pMailAccount->folder && pMailAccount->folder->fld_session && pMailAccount->folder->fld_session->sess_data )
				((struct feed_session_state_data *) (pMailAccount->folder->fld_session->sess_data))->feed_last_update = (time_t) -1;
		}

		/* retrieve the stats */
		if (r == MAIL_NO_ERROR)
		{
			uint32_t result_messages;
			uint32_t result_recent;
			uint32_t result_unseen;
		    
			//if( MAIL_NO_ERROR == mailsession_unseen_number(pMailAccount->folder->fld_session, pMailAccount->name, &result_unseen) )
			if( MAIL_NO_ERROR == mailfolder_status(pMailAccount->folder,
													&result_messages, &result_recent, &result_unseen) )
			{
				pMailAccount->iPrevNbUnseenMails = pMailAccount->iNbUnseenMails;
				if( pMailAccount->iNbUnseenMails != (guint)result_unseen )
				{
					pMailAccount->iNbUnseenMails = (guint)result_unseen;
					pMailAccount->dirtyfied = TRUE;
				}
			}

			cd_debug( "result_messages = %d, result_recent = %d, result_unseen = %d", result_messages, result_recent, result_unseen );

			mailfolder_disconnect(pMailAccount->folder);
			mailstorage_disconnect(pMailAccount->storage);
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
		
	}
}

gboolean cd_mail_update_account_status( CDMailAccount *pUpdatedMailAccount )
{
	if( !pUpdatedMailAccount ) return TRUE;
	CairoDockModuleInstance *myApplet = pUpdatedMailAccount->pAppletInstance;
	GList *pIconList = CD_APPLET_MY_ICONS_LIST;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	Icon *pIcon = pUpdatedMailAccount->icon;
	g_return_val_if_fail (pIcon != NULL, TRUE);
	
	//\_______________________ On met a jour l'icone du compte.
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	if (pUpdatedMailAccount->iNbUnseenMails > 0)
	{
		cairo_dock_set_quick_info_full (myDrawContext, pIcon, pContainer, "%d", pUpdatedMailAccount->iNbUnseenMails);
		
		cairo_dock_set_image_on_icon (pIconContext, myConfig.cHasMailUserImage, pIcon, pContainer);
	}
	else
	{
		cairo_dock_set_quick_info (myDrawContext, NULL, pIcon, cairo_dock_get_max_scale (pContainer));
		
		cairo_dock_set_image_on_icon (pIconContext, myConfig.cNoMailUserImage, pIcon, pContainer);
	}
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	
	//\_______________________ On met a jour l'icone principale.
	if (pUpdatedMailAccount->iPrevNbUnseenMails != pUpdatedMailAccount->iNbUnseenMails)  // des mails en plus ou en moins.
	{
		myData.iPrevNbUnreadMails = myData.iNbUnreadMails;
		myData.iNbUnreadMails += pUpdatedMailAccount->iNbUnseenMails - pUpdatedMailAccount->iPrevNbUnseenMails;
		cd_mail_draw_main_icon (myApplet);
	}
	
	return TRUE;
}



void cd_mail_load_icons( CairoDockModuleInstance *myApplet )
{
	CDMailAccount *pMailAccount;
	GList *pIconList = NULL;
	Icon *pIcon;
	guint i;
	int iNbIcons = 0;
	
	myData.iPrevNbUnreadMails = 0;
	myData.iNbUnreadMails = 0;
	
	//\_______________________ On construit la liste des icones.
	if (myData.pMailAccounts != NULL)
	{
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			if( !pMailAccount )
				continue;
			
			myData.iNbUnreadMails += pMailAccount->iNbUnseenMails;  // a priori c'est a 0.
			_add_icon (pMailAccount);
			iNbIcons ++;
		}
	}
	g_print ( "%s () : %d messages initialement\n", __func__, myData.iNbUnreadMails );
	
	//\_______________________ On efface l'ancienne liste.
	CD_APPLET_DELETE_MY_ICONS_LIST;
	
	//\_______________________ On charge la nouvelle liste.
	gpointer pConfig[2] = {GINT_TO_POINTER (FALSE), GINT_TO_POINTER (FALSE)};
	CD_APPLET_LOAD_MY_ICONS_LIST (pIconList, myConfig.cRenderer, (iNbIcons > 1 ? "Caroussel" : "Simple"), (iNbIcons > 1 ? pConfig : NULL));
	
	//\_______________________ On redessine l'icone principale.
	gchar *cNewImage;
	if (myData.iNbUnreadMails > 0)
	{
		cNewImage = myConfig.cHasMailUserImage;
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iNbUnreadMails);
	}
	else
	{
		cNewImage = myConfig.cNoMailUserImage;
	}
	CD_APPLET_SET_IMAGE_ON_MY_ICON (cNewImage);
	
	/*if (myDesklet)
		gtk_widget_queue_draw (myDesklet->pWidget);
	else*/
	CD_APPLET_REDRAW_MY_ICON;
}



void cd_mail_draw_main_icon (CairoDockModuleInstance *myApplet)
{
	g_return_if_fail (myDrawContext != NULL);
	if (myData.iNbUnreadMails == myData.iPrevNbUnreadMails)  // rien de nouveau, rien a faire.
		return ;
	
	gchar *cNewImage = NULL;
	if (myData.iNbUnreadMails <= 0)  // plus de mail.
	{
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog_with_icon (D_("No unread mail in your mailboxes"), myIcon, myContainer, 1500, "same icon");
		
		//Chargement de l'image "pas de mail"
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cNoMailUserImage);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON (NULL);
	}
	else if (myData.iNbUnreadMails > 0)  // de nouveaux mails.
	{
		GString *ttip_str = g_string_sized_new(300);
		guint i;

		/* don't play more often than every 4 seconds... */
		time_t currentTime = time(NULL);
		if(currentTime-myData.timeEndOfSound > 4)
		{
			cairo_dock_play_sound(myConfig.cNewMailUserSound);
			myData.timeEndOfSound = time(NULL);
		}

		if (myData.iNbUnreadMails > 1)
			g_string_append_printf( ttip_str, D_("You have %d new mails :"), myData.iNbUnreadMails);
		else
			g_string_append_printf( ttip_str, D_("You have a new mail :"));

		if (myData.pMailAccounts != NULL)
		{
			CDMailAccount *pMailAccount;
			for (i = 0; i < myData.pMailAccounts->len; i++)
			{
				pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
				if( !pMailAccount ) continue;
				if (pMailAccount->iNbUnseenMails > 0) {
					g_string_append_printf(ttip_str, "\n    %d in %s",
						pMailAccount->iNbUnseenMails, pMailAccount->name);
				}
			}
		}
		cairo_dock_remove_dialog_if_any (myIcon);
		cairo_dock_show_temporary_dialog (ttip_str->str, myIcon, myContainer, 5000);

		g_string_free(ttip_str, TRUE);
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
		{
			cairo_dock_launch_animation (myContainer);
		}
		else
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHasMailUserImage);
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iNbUnreadMails);
	}
}


void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();

	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
  
	glPushMatrix ();

	glScalef(sqrt(2)/2*iWidth, sqrt(2)/2*iHeight, 1.0);  // faire tenir la diagonale du cube dans l'icone.
	glTranslatef(0., 0., -1.0);

	glRotatef(myData.current_rotX, 1.0f, 0.0f, 0.0f);  /* rotate on the X axis */
	glRotatef(myData.current_rotY, 0.0f, 1.0f, 0.0f);  /* rotate on the Y axis */
	//  glRotatef(30.f, 0.0f, 0.0f, 1.0f);  /* rotate on the Z axis */

	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_alpha ();
	glEnable(GL_DEPTH_TEST);
	glAlphaFunc ( GL_GREATER, 0.1 ) ;
	glEnable ( GL_ALPHA_TEST ) ;
	
	glBindTexture(GL_TEXTURE_2D, myData.iNbUnreadMails > 0?myData.iHasMailTexture:myData.iNoMailTexture);

	glCallList (myData.iCubeCallList);

	_cairo_dock_disable_texture ();
	glDisable ( GL_ALPHA_TEST ) ;
	glDisable (GL_DEPTH_TEST);
	glPopMatrix ();

	CD_APPLET_FINISH_DRAWING_MY_ICON;
}
