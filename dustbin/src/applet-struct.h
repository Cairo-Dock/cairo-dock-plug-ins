
#ifndef __CD_DUSTBIN_STRUCT__
#define  __CD_DUSTBIN_STRUCT__


typedef enum {
	CD_DUSTBIN_EMPTY = 0,
	CD_DUSTBIN_FULL
	} CdDustbinState;

typedef enum {
	CD_DUSTBIN_INFO_NONE = 0,
	CD_DUSTBIN_INFO_NUMBER,
	CD_DUSTBIN_INFO_WEIGHT
	} CdDustbinInfotype;


#endif
