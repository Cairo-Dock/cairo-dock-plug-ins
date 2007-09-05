/******************************************************************************

This file is a part of the cairo-dock program, 
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet_03@yahoo.fr)

******************************************************************************/
#include <string.h>

#include <xdg_vfs_client.h>

#include "file-manager-vfs-xdg.h"


void _file_manager_get_file_info (gchar *cURI, gchar **cIconName, gboolean *bIsDirectory, gboolean *bIsMountedPoint)
{
	g_print ("%s ()\n", __func__);
	XdgVfsSession * session = NULL;
	XdgVfsResult r = xdg_vfs_sess_start (&session, "this");
	if (r)
	{
		g_print ("Atention : xdg error : session start problem = %d\n", r);
		return;
	}
	
	r = xdg_vfs_sess_cmd_getFileInfo (session, cURI);
	
	XdgVfsItemType type;
	XdgVfsItem * item;
	while (xdg_vfs_sess_readItem(session, &type, &item, NULL, NULL) == XDGVFS_RESULT_CONTINUES) 
	{
		switch(type) 
		{
			case XDGVFS_ITEMTYPE_LS_HEAD:
			{
				XdgVfsSimpleHead * head = (XdgVfsSimpleHead*) item;
				g_print ("got ls header uri='%s'\n", head->uri);
				break;
			}
			
			case XDGVFS_ITEMTYPE_FILEINFO:
			{
				XdgVfsFileInfo * info = (XdgVfsFileInfo*) item;
				// fprintf(stdout, "got fileinfo uri='%s'\n", info->uri);	
				*cIconName = g_strdup (info->iconname);
				*bIsDirectory = (info->filetype == XDGVFS_FILE_TYPE_DIRECTORY);
				*bIsMountedPoint = (info->filetype == XDGVFS_FILE_TYPE_VFSMOUNTPOINT);
				break;
			}
			case XDGVFS_ITEMTYPE_NONE:
			{
				break;
			}
			default:
			{
				g_print ("unexpected item - type=%d\n", type);
				break;
			}
		}
		xdg_vfs_item_unref (item);
	}
	
	xdg_vfs_sess_close (session);
	return ;
}


GList *_file_manager_list_directory (gchar *cURI)
{
	g_print ("%s ()\n", __func__);
	XdgVfsSession * session = NULL;
	XdgVfsResult r = xdg_vfs_sess_start (&session, "this");
	if (r)
	{
		g_print ("Atention : xdg error : session start problem = %d\n", r);
		return NULL;
	}
	
	r = xdg_vfs_sess_cmd_listDirectory (session, cURI);
	
	XdgVfsItemType type;
	XdgVfsItem * item;
	GList *pIconList = NULL;
	Icon *icon;
	int iOrder = 0;
	
	while (xdg_vfs_sess_readItem (session, &type, &item, NULL, NULL) == XDGVFS_RESULT_CONTINUES)
	{
		switch(type)
		{
			case XDGVFS_ITEMTYPE_LS_HEAD:
			{
				XdgVfsSimpleHead * head = (XdgVfsSimpleHead*) item;
				g_print ("got ls header uri='%s'\n", head->uri);
				break;
			}
			
			case XDGVFS_ITEMTYPE_FILEINFO:
			{
				XdgVfsFileInfo * info = (XdgVfsFileInfo*) item;
				// fprintf(stdout, "got fileinfo uri='%s'\n", info->uri);	
				icon = g_new0 (Icon, 1);
				icon->bIsURI = TRUE;
				icon->iType = CAIRO_DOCK_LAUNCHER;
				icon->acDesktopFileName = g_strdup (info->uri);
				icon->acName = g_strdup (info->basename);
				icon->acFileName = g_strdup (info->iconname);
				icon->fOrder = iOrder ++;
				pIconList = g_list_prepend (pIconList, icon);
				break;
			}
			case XDGVFS_ITEMTYPE_NONE:
			{
				break;
			}
			default:
			{
				g_print ("unexpected item - type=%d\n", type);
				break;
			}
		}
		xdg_vfs_item_unref(item);
	}
	
	xdg_vfs_sess_close (session);
	return pIconList;
}




static void file_manager_just_launch_uri (gchar *cURI)
{
	gchar *cCommand = g_strdup_printf ("xdg-open %s", cURI);
	g_spawn_command_line_async (cCommand, NULL);
	g_free (cCommand);
}

void _file_manager_launch_uri (gchar *cURI)
{
	g_print ("%s ()\n", __func__);
	g_return_if_fail (cURI != NULL);
	
	gboolean bIsMounted;
	gchar *cNeededMountPointID = file_manager_is_mounting_point (cURI, &bIsMounted);
	if (cNeededMountPointID != NULL && ! bIsMounted)
	{
		GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (NULL),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			"Do you want to mount this point ?");
		int answer = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		if (answer != GTK_RESPONSE_YES)
			return ;
		
		gchar *cActivatedURI = _file_manager_mount_point_by_id (cNeededMountPointID);
		g_free (cNeededMountPointID);
		if (cActivatedURI != NULL)
			file_manager_just_launch_uri (cActivatedURI);
		g_free (cActivatedURI);
	}
	else
		file_manager_just_launch_uri (cURI);
}




gchar *_file_manager_is_mounting_point (gchar *cURI, gboolean *bIsMounted)
{
	g_print ("%s (%s)\n", __func__, cURI);
	XdgVfsSession * session = NULL;
	XdgVfsResult r = xdg_vfs_sess_start (&session, "this");
	if (r)
	{
		g_print ("Atention : xdg error : session start problem = %d\n", r);
		return NULL;
	}
	
	r = xdg_vfs_sess_cmd_getFileInfo (session, cURI);
	
	gchar *cMountPointID = NULL;
	XdgVfsItemType type;
	XdgVfsItem * item;
	while (xdg_vfs_sess_readItem(session, &type, &item, NULL, NULL) == XDGVFS_RESULT_CONTINUES)
	{
		switch (type)
		{
			case XDGVFS_ITEMTYPE_LS_HEAD:
			{
				XdgVfsSimpleHead * head = (XdgVfsSimpleHead*) item;
				g_print ("got ls header uri='%s'\n", head->uri);
				break;
			}
			
			case XDGVFS_ITEMTYPE_FILEINFO:
			{
				XdgVfsFileInfo * info = (XdgVfsFileInfo*) item;
				if (info->filetype == XDGVFS_FILE_TYPE_VFSMOUNTPOINT)
				{
					cMountPointID = g_strdup (info->mountpoint_id);
					*bIsMounted = info->is_mounted;
				}
				break;
			}
			case XDGVFS_ITEMTYPE_NONE:
			{
				break;
			}
			default:
			{
				g_print ("unexpected item - type=%d\n", type);
				break;
			}
		}
		xdg_vfs_item_unref (item);
	}
	
	xdg_vfs_sess_close (session);
	return cMountPointID;
}


gchar * _file_manager_mount_point_by_id (gchar *cMountPointID)
{
	g_print ("%s ()\n", __func__);
	XdgVfsSession * session = NULL;
	XdgVfsResult r = xdg_vfs_sess_start (&session, "this");
	if (r)
	{
		g_print ("Atention : xdg error : session start problem = %d\n", r);
		return NULL;
	}
	
	r = xdg_vfs_sess_cmd_mount (session, cMountPointID);
	if (r)
	{
		g_print ("Attention : unexpected error (%d) while trying to mount %s\n", r, cMountPointID);
		return NULL;
	}
	
	gchar *cActivatedURI = NULL;
	XdgVfsItemType type=0;
	XdgVfsItem * item=NULL;
	
	while ((r=xdg_vfs_sess_readItem(session, &type, &item, NULL, NULL)) == XDGVFS_RESULT_CONTINUES) 
	{
		switch (type)
		{
			case XDGVFS_ITEMTYPE_NONE:
			{
				break;
			}
			case XDGVFS_ITEMTYPE_FILEINFO:
			{
				XdgVfsFileInfo * info = (XdgVfsFileInfo*) item;
				fprintf(stderr, "got activation uri=%s\n", info->uri);
				if (!cActivatedURI)
					cActivatedURI = g_strdup (info->uri);
				break;
			}
			default:
			{
				fprintf(stderr, "unexpected item - type=%d\n", type);
				break;
			}
		}
		xdg_vfs_item_unref(item);
	}
	if (r)
	{
		g_print ("Attention : unexpected error (%d) while mounting %s\n", r, cMountPointID);
		g_free (cActivatedURI);
		return NULL;
	}
	
	return cActivatedURI;
}
