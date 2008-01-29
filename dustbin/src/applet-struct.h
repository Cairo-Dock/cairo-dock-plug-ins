
#ifndef __CD_DUSTBIN_STRUCT__
#define  __CD_DUSTBIN_STRUCT__

typedef enum {
	CD_DUSTBIN_EMPTY,
	CD_DUSTBIN_FULL
	} CdDustbinState;

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


typedef struct {
	double fCheckInterval;
	int iSidCheckTrashes;
	GList *pTrashDirectoryList;
	gchar **cAdditionnalDirectoriesList;
	int *pTrashState;
	cairo_surface_t *pEmptyBinSurface;
	cairo_surface_t *pFullBinSurface;
	gchar *cThemePath;
	int iState;
	int iNbTrashes;
	int iNbFiles;
	int iSize;
	CdDustbinInfotype iQuickInfoType;
	int iQuickInfoValue;
	gchar *cDefaultBrowser;
	gchar *cEmptyUserImage;
	gchar *cFullUserImage;
	int iGlobalSizeLimit;
	int iSizeLimit;
	gchar *cDialogIconPath;
	} AppletConfig;

#define CD_DUSTBIN_DIALOG_DURATION 4000

#endif
