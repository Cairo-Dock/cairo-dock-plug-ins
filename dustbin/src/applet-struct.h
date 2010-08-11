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


#ifndef __CD_DUSTBIN_STRUCT__
#define  __CD_DUSTBIN_STRUCT__

#include <cairo-dock.h>

typedef enum {
	CD_DUSTBIN_INFO_NONE,
	CD_DUSTBIN_INFO_NB_TRASHES,
	CD_DUSTBIN_INFO_NB_FILES,
	CD_DUSTBIN_INFO_WEIGHT
	} CdDustbinInfotype;

struct _AppletConfig {
	gchar *cThemePath;
	gchar *cEmptyUserImage;
	gchar *cFullUserImage;
	CdDustbinInfotype iQuickInfoType;
	gboolean bAskBeforeDelete;
	} ;

struct _AppletData {
	CairoDockTask *pTask;  // tache de la mesure.
	// shared memory
	gsize _iMeasure;
	// end of shared memory
	gsize iMeasure;
	gchar *cDustbinPath;  // donnee constante.
	gboolean bMonitoringOK;
	gboolean bDisplayFullIcon;
	} ;

#define CD_DUSTBIN_DIALOG_DURATION 4000

#endif
