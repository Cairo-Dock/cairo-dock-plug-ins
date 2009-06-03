#ifndef __CD_DND2SHARE__
#define  __CD_DND2SHARE__

#include <cairo-dock.h>
#include "applet-struct.h"

/*
void cd_dnd2share_check_number_of_stored_pictures (void);

void cd_dnd2share_extract_urls_from_log (void);

void cd_dnd2share_get_urls_from_stored_file (void);

void cd_dnd2share_new_picture (gchar *cDroppedPicturePath);

void cd_dnd2share_delete_picture (void);

void cd_dnd2share_delete_all_pictures (void);

void cd_dnd2share_copy_url_into_clipboard (gint iUrlNumberInList);
*/


void cd_dnd2share_free_uploaded_item (CDUploadedItem *pItem);

void cd_dnd2share_build_history (void);

void cd_dnd2share_clear_history (void);


void cd_dnd2share_launch_upload (const gchar *cFilePath, CDFileType iFileType);


void cd_dnd2share_clear_working_directory (void);

void cd_dnd2share_clear_copies_in_working_directory (void);

void cd_dnd2share_set_working_directory_size (int iNbItems);

void cd_dnd2share_clean_working_directory (void);


void cd_dnd2share_copy_url_to_clipboard (const gchar *cURL);

gchar *cd_dnd2share_get_prefered_url_from_item (CDUploadedItem *pItem);

void cd_dnd2share_set_current_url_from_item (CDUploadedItem *pItem);


#endif // __CD_DND2SHARE__
