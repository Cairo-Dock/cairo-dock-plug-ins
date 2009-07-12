
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
