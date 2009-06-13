
#ifndef __CD_APPLET_STRUCT__
#define  __CD_APPLET_STRUCT__

#include <cairo-dock.h>


typedef enum _CDFileType {
	CD_UNKNOWN_TYPE=0,
	CD_TYPE_TEXT,
	CD_TYPE_IMAGE,
	CD_TYPE_VIDEO,
	CD_NB_FILE_TYPES
	} CDFileType;

typedef enum _CDSiteId {
	CD_UPPIX=0,
	CD_IMAGEBIN,
	CD_NB_SITES
	} CDSiteId;

typedef struct _CDUploadedItem {
	gchar *cItemName;  // nom de l'item, c'est aussi le nom de la copie locale si l'option est activee ("item-$timestamp").
	CDSiteId iSiteID;  // pour savoir quel site on a utilise => nous donne le backend.
	gchar **cDistantUrls;  // il peut y en avoir plusieurs (differentes tailles, etc), le backend sait lesquelles il s'agit.
	time_t iDate;  // date de l'upload, permet de classer les items entre eux et de leur donner un nom unique.
	gchar *cLocalPath;  // chemin du fichier sur le disque dur au moment de son upload.
	gchar *cFileName;  // nom du fichier (et nom affiche pour l'utilisateur).
	CDFileType iFileType;
	} CDUploadedItem;

typedef void (* CDUploadFunc) (const gchar *cFilePath, CDFileType iFileType);

typedef struct _CDSiteBackend {
	const gchar *cSiteName;
	gint iNbUrls;
	const gchar **cUrlLabels;
	gint iPreferedUrlType;  /// eventuellement a mettre en conf...
	CDUploadFunc upload;
	} CDSiteBackend;


//\___________ structure containing the applet's configuration parameters.
struct _AppletConfig {
	gboolean bEnableDialogs;
	gdouble dTimeDialogs;
	gint iNbItems;
	gboolean bkeepCopy;
	gboolean bDisplayLastImage;
	CDSiteId iPreferedSite;
	gchar *cIconAnimation;
	} ;


//\___________ structure containing the applet's data, like surfaces, dialogs, results of calculus, etc.
struct _AppletData {
	gchar *cWorkingDirPath;
		
	CDSiteBackend backends[CD_NB_SITES];
	CDSiteBackend *pCurrentBackend;
	
	CairoDockMeasure *pMeasureTimer;
	gchar *cCurrentFilePath;  // memoire partagee avec le thread, a manipuler avec les precautions d'usage.
	CDFileType iCurrentFileType;  // idem
	gchar **cResultUrls;  // idem
	
	GList *pUpoadedItems;  // une liste de CDUploadedItem*
	gchar *cLastURL;  // la derniere URL a avoir ete copiee dans le clipboard; on pourra y acceder par clic gauche sur l'icone.
	gint iCurrentItemNum;  // le numero de l'item correspondant dans la liste. C'est plus sur que de pointer directement dans la liste, au cas ou elle changerait.
	} ;


#endif