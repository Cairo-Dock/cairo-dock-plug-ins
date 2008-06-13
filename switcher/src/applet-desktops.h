
#ifndef __APPLET_DESKTOPS__
#define  __APPLET_DESKTOPS__


#include <cairo-dock.h>


void cd_switcher_get_current_desktop (void);


void cd_switcher_compute_nb_lines_and_columns (void);

void cd_switcher_compute_desktop_coordinates (int iNumDesktop, int iNumViewportX, int iNumViewportY, int *iNumLine, int *iNumColumn);

void cd_switcher_compute_desktop_from_coordinates (int iNumLine, int iNumColumn, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY);


int cd_switcher_compute_index (int iNumDesktop, int iNumViewportX, int iNumViewportY);

void cd_switcher_compute_viewports_from_index (int iIndex, int *iNumDesktop, int *iNumViewportX, int *iNumViewportY);


void cd_switcher_add_a_desktop (void);

void cd_switcher_remove_last_desktop (void);


#endif

