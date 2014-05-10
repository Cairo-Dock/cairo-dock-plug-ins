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


#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


typedef enum _CDFileType {
	CD_UNKNOWN_TYPE=0,
	CD_TYPE_TEXT,
	CD_TYPE_IMAGE,
	CD_TYPE_VIDEO,
	CD_TYPE_FILE,
	CD_NB_FILE_TYPES
	} CDFileType;

#define CD_NB_SITES_MAX 5 // max number here above
// number of sites for the each type in the config file
#define CD_NB_SITES_TEXT 5
#define CD_NB_SITES_IMG 4
#define CD_NB_SITES_VID 2
#define CD_NB_SITES_FILE 3


typedef struct _CDUploadedItem {
	gchar *cItemName;     // name of the item (also the group name in the history file and the local copy if needed -> looks like "item-$timestamp").
	gint iSiteID;         // to link it with the right backend.
	gchar **cDistantUrls; // can have several url (with different size, etc) => check backend.
	time_t iDate;         // date of the upload, to sort items and it's unique.
	gchar *cLocalPath;    // Path of the file when the user has uploaded it.
	gchar *cFileName;     // name of the file (which will be displayed to the user).
	CDFileType iFileType;
	} CDUploadedItem;

typedef void (* CDUploadFunc) (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError);

typedef struct _CDSiteBackend {
	const gchar *cSiteName; // name of the website (display)
	gint iNbUrls;           // number of URL returned.
	gchar **cUrlLabels;     // description of each URL.
	gint iPreferedUrlType;  // the most 'useful' URL
	CDUploadFunc upload;    // the upload function (launched in a separated thread).
	} CDSiteBackend;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bEnableDialogs;
	gdouble dTimeDialogs;
	guint iNbItems;
	gint iLimitRate;
	gboolean bkeepCopy;
	gboolean bUseOnlyFileType;
	gboolean bDisplayLastImage;
	gint iPreferedSite[CD_NB_FILE_TYPES];
	gchar *cIconAnimation;
	gchar *cCustomScripts[CD_NB_FILE_TYPES];
	gchar *cLocalDir;
	gboolean bAnonymous;
	gint iTinyURLService;
	gboolean bUseTinyAsDefault;
	} ;

typedef struct _CDSharedMemory {
	gchar *cCurrentFilePath;
	CDFileType iCurrentFileType;
	gboolean bTempFile;
	CDUploadFunc upload;  // we don't keep a pointer to the current backend in the shard memory, because it can be destroyed meanwhile. so we just copy the part that is useful.
	gint iNbUrls;
	gint iTinyURLService;
	gchar *cLocalDir;
	gboolean bAnonymous;
	gint iLimitRate;
	gchar **cResultUrls;
	GError *pError;
	} CDSharedMemory;


//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gchar *cWorkingDirPath;
	CDSiteBackend backends[CD_NB_FILE_TYPES][CD_NB_SITES_MAX];
	CDSiteBackend *pCurrentBackend[CD_NB_FILE_TYPES];
	int iNbSitesForType[CD_NB_FILE_TYPES];

	CairoDockTask *pTask;  // current upload task.

	GList *pUpoadedItems;  // list of CDUploadedItem*
	gchar *cLastURL;       // the last copied URL -> for the left click
	gint iCurrentItemNum;  // the number of the current item in the list (safer than using a pointer in the list if this list is modified)
	gchar *cTmpFilePath;
	} ;


#endif
