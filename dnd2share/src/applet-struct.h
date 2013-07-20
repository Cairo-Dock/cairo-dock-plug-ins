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

#define CD_NB_SITES 8


typedef struct _CDUploadedItem {
	gchar *cItemName;  // nom de l'item, c'est aussi le nom du groupe dans le fichier historique et de la copie locale si l'option est activee (de la forme "item-$timestamp").
	gint iSiteID;  // pour savoir quel site on a utilise => nous donne le backend.
	gchar **cDistantUrls;  // il peut y en avoir plusieurs (differentes tailles, etc), le backend sait lesquelles il s'agit.
	time_t iDate;  // date de l'upload, permet de classer les items entre eux et de leur donner un nom unique.
	gchar *cLocalPath;  // chemin du fichier sur le disque dur au moment de son upload.
	gchar *cFileName;  // nom du fichier (et nom affiche pour l'utilisateur).
	CDFileType iFileType;
	} CDUploadedItem;

typedef void (* CDUploadFunc) (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError);

typedef struct _CDSiteBackend {
	const gchar *cSiteName;  // nom du site, pour affichage
	gint iNbUrls;  // nombre d'URLs qu'il renvoie.
	gchar **cUrlLabels;  // description de chacune de ces URL.
	gint iPreferedUrlType;  // celle qui est la plus utile, eventuellement a mettre en conf.
	CDUploadFunc upload;  // la fonction d'upload, threadee.
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
	CDSiteBackend backends[CD_NB_FILE_TYPES][CD_NB_SITES];
	CDSiteBackend *pCurrentBackend[CD_NB_FILE_TYPES];
	int iNbSitesForType[CD_NB_FILE_TYPES];
	
	CairoDockTask *pTask;  // current upload task.
	
	GList *pUpoadedItems;  // une liste de CDUploadedItem*
	gchar *cLastURL;  // la derniere URL a avoir ete copiee dans le clipboard; on pourra y acceder par clic gauche sur l'icone.
	gint iCurrentItemNum;  // le numero de l'item correspondant dans la liste. C'est plus sur que de pointer directement dans la liste, au cas ou elle changerait.
	gchar *cTmpFilePath;
	} ;


#endif
