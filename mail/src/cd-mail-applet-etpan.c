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


void cd_mail_get_folder_data (CDMailAccount *pMailAccount)  ///Extraire les donnees des mails (nombre, titres, resume eventuellement) et les placer dans une structure dediee a l'affichage...
{
	if( ! pMailAccount )
		return ;
	pMailAccount->bError = FALSE;
	int r = 0;
	/* get the folder structure */

	// create the folder, if not yet done
	if( pMailAccount->folder == NULL )
	{
		pMailAccount->folder = mailfolder_new(pMailAccount->storage, pMailAccount->path, NULL);
	}

	if( pMailAccount->storage && pMailAccount->folder )
	{
		/* Ensure the connection is alive */
		r = mailfolder_connect(pMailAccount->folder);
	    if (r != MAIL_NO_ERROR)  // no connexion, we keep the previous satus.
		{
			cd_warning ("mail : couldn't connect to '%s'", pMailAccount->name);
			pMailAccount->bError = TRUE;
			return ;
		}
		//g_print ("connexion ok\n");
		
		/* Fix initialization for feed storage */
		if( pMailAccount->driver == FEED_STORAGE )
		{
			if( pMailAccount->folder && pMailAccount->folder->fld_session && pMailAccount->folder->fld_session->sess_data )
				((struct feed_session_state_data *) (pMailAccount->folder->fld_session->sess_data))->feed_last_update = (time_t) -1;
		}

		/* retrieve the stats */
		uint32_t result_messages;
		uint32_t result_recent;
		uint32_t result_unseen;
		
		//if( MAIL_NO_ERROR == mailsession_unseen_number(pMailAccount->folder->fld_session, pMailAccount->name, &result_unseen) )
		if( MAIL_NO_ERROR == mailfolder_status(pMailAccount->folder, &result_messages, &result_recent, &result_unseen) )
		{
			g_print ("mail : %d/%d/%d\n", result_messages, result_recent, result_unseen);
			pMailAccount->iPrevNbUnseenMails = pMailAccount->iNbUnseenMails;
			if( pMailAccount->iNbUnseenMails != (guint)result_unseen )  // nombre de messages non lus a change, on va supposer que cela provient soit de leur lecture, soit de leur arrivee, en excluant le cas ou arrivee = lecture, qui laisserait inchange le nombre de mails non lus.
			{
				pMailAccount->iNbUnseenMails = (guint)result_unseen;
				
				// On recupere les messages non lus.
				//if (myConfig.bShowMessageContent && pMailAccount->bInitialized)  // && pMailAccount->iNbUnseenMails > pMailAccount->iPrevNbUnseenMails
				CairoDockModuleInstance *myApplet = pMailAccount->pAppletInstance;
				if (myConfig.bShowMessageContent)
				{
					g_print ("getting %d message body...\n", pMailAccount->iNbUnseenMails);
					g_list_foreach (pMailAccount->pUnseenMessageList, (GFunc) g_free, NULL);
					g_list_free (pMailAccount->pUnseenMessageList);
					g_list_foreach (pMailAccount->pUnseenMessageUid, (GFunc) g_free, NULL);
					g_list_free (pMailAccount->pUnseenMessageUid);
					pMailAccount->pUnseenMessageList = NULL;
					pMailAccount->pUnseenMessageUid = NULL;
					
					mailmessage *pMessage;
					struct mailmime *pMailMime;
					struct mailimf_fields *pFields;
					struct mailimf_single_fields *pSingleFields;
					struct mailimf_from *pFrom;
					struct mailimf_subject *pSubject;
					struct mailimf_message_id *pUid;
					struct mailimf_mailbox *pFromMailBox;
					char *cRawBodyText, *cBodyText, *cFrom, *cSubject, *cMessage, *cUid;
					size_t length;
					guint i = 1;

					struct mailmessage_list * msg_list = NULL;
		      mailfolder_get_messages_list(pMailAccount->folder, &msg_list);
	
					guint iNbAccountsToCheck = 20;
					if( myConfig.iNbMaxShown != -1 )
					{
						 iNbAccountsToCheck = MIN (myConfig.iNbMaxShown, pMailAccount->iNbUnseenMails);
					}
					for (i = 1; iNbAccountsToCheck > 0; i ++)
					{
						cFrom = NULL;
						cSubject = NULL;
						cBodyText = NULL;
						cRawBodyText = NULL;
						cUid = NULL;
						pMessage = NULL;
						pSingleFields = NULL;
						struct mail_flags *pFlags = NULL;

						cd_message ("Fetching message number %d...\n", i);

						if (carray_count(msg_list->msg_tab) < i) {
							break;
						}

						pMessage = carray_get(msg_list->msg_tab, i-1);
						
						if (r != MAIL_NO_ERROR || pMessage == NULL)
						{
							cd_warning ("couldn't get the message number %d", i);
							iNbAccountsToCheck--;
							continue;
						}
						r = mailmessage_get_flags (pMessage, &pFlags);
						if (r != MAIL_NO_ERROR || pFlags == NULL)
						{
							cd_warning ("couldn't get the message flags !", i);
						}
						else
						{
							if( (pFlags->fl_flags & MAIL_FLAG_NEW) == 0 &&
									(pFlags->fl_flags & MAIL_FLAG_SEEN) > 0 )
							{
								continue;
							}
						}
						iNbAccountsToCheck--;

						
						r = mailmessage_get_bodystructure (pMessage, &pMailMime);
						if (r != MAIL_NO_ERROR)
						{
							cd_warning ("couldn't parse the message structure");
							continue;
						}
						r = mailmessage_fetch_body (pMessage, &cRawBodyText, &length);
						if (r != MAIL_NO_ERROR)
						{
							cd_warning ("couldn't fetch the body");
							continue;
						}
						else
						{
							if( pMailAccount->driver == FEED_STORAGE )
							{
								size_t cur_token = 0;

								r = mailmime_encoded_phrase_parse("UTF-8",
									cRawBodyText, length,
									&cur_token, "UTF-8",
									&cBodyText);
							}
							if (r != MAILIMF_NO_ERROR)
							{
							  cBodyText = g_strdup(cRawBodyText);
							}
							
							g_print (" -> '%s'\n", cBodyText);
						}
						r = mailmessage_fetch_envelope(pMessage, &pFields);
						if (r != MAIL_NO_ERROR)
						{
							cd_warning ("couldn't fetch the headers");
							continue;
						}
						pSingleFields = mailimf_single_fields_new (pFields);
						if (pSingleFields == NULL)
							continue;
						pFrom = pSingleFields->fld_from;
						if (pFrom != NULL)
						{
							pFromMailBox = (struct mailimf_mailbox *) clist_content(clist_begin (pFrom->frm_mb_list->mb_list));
							if (pFromMailBox->mb_display_name == NULL)
							{
								cFrom = g_strdup(pFromMailBox->mb_addr_spec);
							}
							else
							{
								size_t cur_token = 0;
								r = mailmime_encoded_phrase_parse("iso-8859-1",
									pFromMailBox->mb_display_name, strlen(pFromMailBox->mb_display_name),
									&cur_token, "UTF-8",
									&cFrom);
								if (r != MAILIMF_NO_ERROR) {
								  cFrom = g_strdup(pFromMailBox->mb_display_name);
								}
							}
						}
						pSubject = pSingleFields->fld_subject;
						if (pSubject != NULL)
						{
							size_t cur_token = 0;

							r = mailmime_encoded_phrase_parse("iso-8859-1",
								pSubject->sbj_value, strlen(pSubject->sbj_value),
								&cur_token, "UTF-8",
								&cSubject);
							if (r != MAILIMF_NO_ERROR) {
							  cSubject = g_strdup(pSubject->sbj_value);
							}
						}
						pUid = pSingleFields->fld_message_id;
						if (pUid != NULL)
						{
							cUid = pUid->mid_value;
						}
						g_print ("    cUid : %s\n", cUid);
						
						cMessage = g_strdup_printf ("From : %s\nSubject : %s\n%s", cFrom ? cFrom : D_("unknown"), cSubject ? cSubject : D_("no subject"), cBodyText ? cBodyText : "");
						pMailAccount->pUnseenMessageList = g_list_append (pMailAccount->pUnseenMessageList, cMessage);

						pMailAccount->pUnseenMessageUid = g_list_append (pMailAccount->pUnseenMessageUid, g_strdup(pMessage->msg_uid));

						g_print ("  Message preview: \n%s", cMessage);
						
						mailmessage_fetch_result_free (pMessage, cRawBodyText);
						mailimf_single_fields_free (pSingleFields);
						
						if( cFrom ) g_free(cFrom);
						if( cSubject ) g_free(cSubject);
						if( cBodyText ) g_free(cBodyText);
					}
					
					mailmessage_list_free(msg_list);
				}
			}
		}
		else
		{
			cd_warning ("mail : couldn't retrieve mails from '%s'", pMailAccount->name);
			pMailAccount->bError = TRUE;
		}
		cd_debug( "result_messages = %d, result_recent = %d, result_unseen = %d", result_messages, result_recent, result_unseen );

		mailfolder_disconnect(pMailAccount->folder);
		mailstorage_disconnect(pMailAccount->storage);  /// utile ?? il n'y a pas de mailstorage_connect ...
	}
}

gboolean cd_mail_update_account_status( CDMailAccount *pUpdatedMailAccount )
{
	if( !pUpdatedMailAccount ) return TRUE;
	CairoDockModuleInstance *myApplet = pUpdatedMailAccount->pAppletInstance;
	CairoContainer *pContainer = CD_APPLET_MY_ICONS_LIST_CONTAINER;
	Icon *pIcon = pUpdatedMailAccount->icon;
	if (pIcon == NULL)  // cas d'un seul compte.
	{
		pIcon = myIcon;
		pContainer = myContainer;
	}
	g_return_val_if_fail (pIcon != NULL, TRUE);
	
	//\_______________________ On met a jour l'icone du compte.
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	if (pUpdatedMailAccount->bError)
	{
		cairo_dock_set_quick_info (myDrawContext, "N/A", pIcon, cairo_dock_get_max_scale (pContainer));
		
		cairo_dock_set_image_on_icon (pIconContext, myConfig.cNoMailUserImage, pIcon, pContainer);
	}
	else if (pUpdatedMailAccount->iNbUnseenMails > 0)
	{
		cairo_dock_set_quick_info_full (myDrawContext, pIcon, pContainer, "%d", pUpdatedMailAccount->iNbUnseenMails);
		
		cairo_dock_set_image_on_icon (pIconContext, myConfig.cHasMailUserImage, pIcon, pContainer);
	}
	else
	{
		cairo_dock_set_quick_info (myDrawContext, "0", pIcon, cairo_dock_get_max_scale (pContainer));
		
		cairo_dock_set_image_on_icon (pIconContext, myConfig.cNoMailUserImage, pIcon, pContainer);
	}
	cairo_destroy (pIconContext);
	
	//\_______________________ On met a jour l'icone principale.
	if (pUpdatedMailAccount->iPrevNbUnseenMails != pUpdatedMailAccount->iNbUnseenMails)  // des mails en plus ou en moins.
	{
		myData.iPrevNbUnreadMails = myData.iNbUnreadMails;
		myData.iNbUnreadMails += pUpdatedMailAccount->iNbUnseenMails - pUpdatedMailAccount->iPrevNbUnseenMails;
		cd_mail_draw_main_icon (myApplet, pUpdatedMailAccount->bInitialized);
	}
	cairo_dock_redraw_icon (pIcon, pContainer);
	
	pUpdatedMailAccount->bInitialized = TRUE;
	return TRUE;
}



void cd_mail_draw_main_icon (CairoDockModuleInstance *myApplet, gboolean bSignalNewMessages)
{
	g_return_if_fail (myDrawContext != NULL);
	g_print ("%s ()\n", __func__);
	
	gchar *cNewImage = NULL;
	if (myData.iNbUnreadMails <= 0)  // plus de mail.
	{
		if (bSignalNewMessages)
		{
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (D_("No unread mail in your mailboxes"), myIcon, myContainer, 1500, "same icon");
		}
		//Chargement de l'image "pas de mail"
		CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cNoMailUserImage);
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON ("0");
	}
	else if (myData.iNbUnreadMails > myData.iPrevNbUnreadMails)  // de nouveaux mails.
	{
		if (bSignalNewMessages)
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
				GList *l;
				gchar *cMessage;
				CDMailAccount *pMailAccount;
				for (i = 0; i < myData.pMailAccounts->len; i++)
				{
					pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
					if( !pMailAccount ) continue;
					if (pMailAccount->iNbUnseenMails > 0)
					{
						g_string_append_printf(ttip_str, "\n   %d in %s", pMailAccount->iNbUnseenMails, pMailAccount->name);
						if (myConfig.bShowMessageContent)
						{
							for (l = pMailAccount->pUnseenMessageList; l != NULL; l = l->next)
							{
								cMessage = l->data;
								g_string_append_printf(ttip_str, "\n      %s", cMessage);
							}
							if( myConfig.iNbMaxShown != -1 && myConfig.iNbMaxShown < pMailAccount->iNbUnseenMails )
							{
								g_string_append_printf(ttip_str, "\n(more...)", cMessage);
							}
						}
					}
				}
			}
			cairo_dock_remove_dialog_if_any (myIcon);
			cairo_dock_show_temporary_dialog_with_icon (ttip_str->str, myIcon, myContainer, 5000, "same icon");

			g_string_free(ttip_str, TRUE);
		}
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet && bSignalNewMessages)
		{
			cairo_dock_launch_animation (myContainer);
		}
		else
		{
			CD_APPLET_SET_IMAGE_ON_MY_ICON (myConfig.cHasMailUserImage);
		}
		CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("%d", myData.iNbUnreadMails);
	}
	CD_APPLET_REDRAW_MY_ICON;
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
