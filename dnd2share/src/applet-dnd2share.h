#ifndef __CD_DND2SHARE__
#define  __CD_DND2SHARE__


#include <cairo-dock.h>
#include "applet-struct.h"



void cd_dnd2share_check_number_of_stored_pictures (void);

void cd_dnd2share_extract_urls_from_log (void);

void cd_dnd2share_get_urls_from_stored_file (void);

void cd_dnd2share_new_picture (gchar *cDroppedPicturePath);

void cd_dnd2share_delete_picture (void);

void cd_dnd2share_delete_all_pictures (void);

void cd_dnd2share_copy_url_into_clipboard (gint iUrlNumberInList);

#endif // __CD_DND2SHARE__
