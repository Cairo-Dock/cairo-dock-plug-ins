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

typedef struct {
	gchar *cPath;
	gint iNbTrashes;
	gint iNbFiles;
	gint iSize;
	gint iAuthorizedWeight;
	} CdDustbin;

typedef struct {
	gchar *cURI;
	CdDustbin *pDustbin;
	} CdDustbinMessage;


struct _AppletConfig {
	gchar **cAdditionnalDirectoriesList;
	gchar *cThemePath;
	gchar *cEmptyUserImage;
	gchar *cFullUserImage;
	CdDustbinInfotype iQuickInfoType;
	int iGlobalSizeLimit;
	int iSizeLimit;
	gboolean bAskBeforeDelete;
	
	double fCheckInterval;
	gchar *cDefaultBrowser;
	} ;

struct _AppletData {
	GList *pDustbinsList;
	gchar *cDialogIconPath;
	cairo_surface_t *pEmptyBinSurface;
	cairo_surface_t *pFullBinSurface;
	int iNbTrashes;
	int iNbFiles;
	int iSize;
	int iQuickInfoValue;
	
	int iState;
	int iSidCheckTrashes;
	} ;

#define CD_DUSTBIN_DIALOG_DURATION 4000

#endif
