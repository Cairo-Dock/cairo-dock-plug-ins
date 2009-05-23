
#ifndef __APPLET_SESSION__
#define  __APPLET_SESSION__

#include <cairo-dock.h>
#include "applet-struct.h"


void cd_do_open_session (void);

void cd_do_close_session (void);

void cd_do_exit_session (void);

#define cd_do_session_is_waiting_for_input(...) (myData.sCurrentText != NULL)
#define cd_do_session_is_closing(...) (myData.iCloseTime != 0)
#define cd_do_session_is_running(...) (cd_do_session_is_waiting_for_input () || cd_do_session_is_closing ())
#define cd_do_session_is_in_navigation_mode(...) (myData.pCharList == NULL)
#define cd_do_session_is_in_selection_mode(...) (myData.pCharList != NUUL)

void cd_do_free_char (CDChar *pChar);
void cd_do_free_char_list (GList *pCharList);


void cd_do_load_pending_caracters (void);

void cd_do_compute_final_coords (void);

void cd_do_launch_appearance_animation (void);

void cd_do_delete_invalid_caracters (void);


#endif
