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

#ifndef __CD_DND2SHARE__
#define  __CD_DND2SHARE__

#include <cairo-dock.h>
#include "applet-struct.h"


#define DND2SHARE_GENERIC_ERROR_MSG D_("Couldn't upload the file, check that your internet connection is active.")
#define DND2SHARE_SET_GENERIC_ERROR_WEBSITE(cWebsite) g_set_error (pError, 1, 1, \
	D_("Couldn't upload the file to %s, check that your internet connection is active."), \
	cWebsite)
#define DND2SHARE_SET_GENERIC_ERROR_SERVICE(cService, cCommand) g_set_error (pError, 1, 1, \
	D_("Couldn't upload the file to %s.\nCheck that your internet connection is active and '%s' is correctly installed and running"), \
	cService, cCommand)


void cd_dnd2share_free_uploaded_item (CDUploadedItem *pItem);

void cd_dnd2share_build_history (void);

void cd_dnd2share_clear_history (void);


void cd_dnd2share_launch_upload (const gchar *cFilePath, CDFileType iFileType);


void cd_dnd2share_clear_working_directory (void);

void cd_dnd2share_clear_copies_in_working_directory (void);

void cd_dnd2share_set_working_directory_size (guint iNbItems);

void cd_dnd2share_clean_working_directory (void);


void cd_dnd2share_copy_url_to_clipboard (const gchar *cURL);
void cd_dnd2share_copy_url_to_primary (const gchar *cURL);

gchar *cd_dnd2share_get_prefered_url_from_item (CDUploadedItem *pItem);

void cd_dnd2share_set_current_url_from_item (CDUploadedItem *pItem);


void cd_dnd2share_remove_one_item (CDUploadedItem *pItem);


void cd_dnd2share_register_new_backend (CDFileType iFileType, const gchar *cSiteName, int iNbUrls, const gchar **cUrlLabels, int iPreferedUrlType, CDUploadFunc pUploadFunc);


#endif // __CD_DND2SHARE__
