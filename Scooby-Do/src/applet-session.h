
#ifndef __APPLET_SESSION__
#define  __APPLET_SESSION__


#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_open_session (void);

void cd_do_close_session (void);

void cd_do_exit_session (void);


void cd_do_free_char (CDChar *pChar);
void cd_do_free_char_list (GList *pCharList);


void cd_do_load_pending_caracters (void);

void cd_do_compute_final_coords (void);

void cd_do_launch_appearance_animation (void);

void cd_do_delete_invalid_caracters (void);


#endif
