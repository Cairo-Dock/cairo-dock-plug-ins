
#ifndef __CD_DUSTBIN_TRASHES_MANAGER__
#define  __CD_DUSTBIN_TRASHES_MANAGER__

#include <cairo-dock.h>
#include "applet-struct.h"


gpointer cd_dustbin_threaded_calculation (gpointer data);

void cd_dustbin_remove_all_messages (void);

void cd_dustbin_add_message (gchar *cURI, CdDustbin *pDustbin);



int cd_dustbin_count_trashes (gchar *cDirectory);

void cd_dustbin_measure_directory (gchar *cDirectory, CdDustbinInfotype iInfoType, CdDustbin *pDustbin, int *iNbFiles, int *iSize);

void cd_dustbin_measure_one_file (gchar *cFilePath, CdDustbinInfotype iInfoType, CdDustbin *pDustbin, int *iNbFiles, int *iSize);

void cd_dustbin_measure_all_dustbins (int *iNbFiles, int *iSize);


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, gchar *cDirectory);

void cd_dustbin_show_trash (GtkMenuItem *menu_item, gchar *cDirectory);


gboolean cd_dustbin_is_monitored (gchar *cDustbinPath);

gboolean cd_dustbin_add_one_dustbin (gchar *cDustbinPath, int iAuthorizedWeight);

void cd_dustbin_free_dustbin (CdDustbin *pDustbin);

void cd_dustbin_remove_all_dustbins (void);


#endif
