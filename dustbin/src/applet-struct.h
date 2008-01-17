
#ifndef __CD_DUSTBIN_STRUCT__
#define  __CD_DUSTBIN_STRUCT__

typedef enum {
	CD_DUSTBIN_EMPTY = 0,
	CD_DUSTBIN_FULL
	} CdDustbinState;

typedef enum {
	CD_DUSTBIN_INFO_NONE = 0,
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

#define CD_DUSTBIN_DIALOG_DURATION 4000

#endif
