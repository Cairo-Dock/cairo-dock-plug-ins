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


#ifndef __CD_DUSTBIN_TRASHES_MANAGER__
#define  __CD_DUSTBIN_TRASHES_MANAGER__

#include <cairo-dock.h>
#include "applet-struct.h"


gpointer cd_dustbin_threaded_calculation (gpointer data);

void cd_dustbin_remove_all_messages (void);

void cd_dustbin_add_message (gchar *cURI, CdDustbin *pDustbin);

gboolean cd_dustbin_is_calculating (void);


int cd_dustbin_count_trashes (const gchar *cDirectory);

void cd_dustbin_measure_directory (const gchar *cDirectory, CdDustbinInfotype iInfoType, CdDustbin *pDustbin, int *iNbFiles, int *iSize);

void cd_dustbin_measure_one_file (const gchar *cFilePath, CdDustbinInfotype iInfoType, CdDustbin *pDustbin, int *iNbFiles, int *iSize);

void cd_dustbin_measure_all_dustbins (int *iNbFiles, int *iSize);


void cd_dustbin_delete_trash (GtkMenuItem *menu_item, const gchar *cDirectory);

void cd_dustbin_show_trash (GtkMenuItem *menu_item, const gchar *cDirectory);


gboolean cd_dustbin_is_monitored (const gchar *cDustbinPath);

gboolean cd_dustbin_add_one_dustbin (gchar *cDustbinPath, int iAuthorizedWeight);

void cd_dustbin_free_dustbin (CdDustbin *pDustbin);

void cd_dustbin_remove_all_dustbins (void);


#endif
