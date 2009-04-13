
#ifndef __APPLET_SESSION__
#define  __APPLET_SESSION__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_open_session (void);

void cd_do_close_session (void);

void cd_do_exit_session (void);


void cd_do_free_char (CDChar *pChar);
void cd_do_free_char_list (GList *pCharList);


#endif
