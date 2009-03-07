#include "applet-dcop.h"

/***************************************************************************/
/// Fonctions de lecture de pipe dcop

#include <stdio.h>

FILE *popen(const char *command, const char *type);

int pclose(FILE *stream);   

gchar *cd_dcop_get_string (const char *cCommand) {
	FILE *pPipe = popen (cCommand,"r");
	if (!pPipe)
		return NULL;
	gchar *cRead = (gchar *) malloc (512*sizeof(gchar));
	if (!fgets(cRead,512,pPipe)) {
		g_free(cRead);
		pclose(pPipe);
		return NULL;
	}
	pclose(pPipe);
	strtok (cRead,"\n");
	return cRead;
}

gint cd_dcop_get_int (const char *cCommand) {
	FILE *pPipe = popen (cCommand,"r");
	if (!pPipe)
		return -1;
	gchar *cRead = (gchar *) malloc (128*sizeof(gchar));
	if (!fgets(cRead,128,pPipe)) {
		g_free(cRead);
		pclose(pPipe);
		return -1;
	}
	pclose(pPipe);
	gint iRet = (gint)atoi(cRead);
	g_free(cRead);
	return iRet;
}

gboolean cd_dcop_get_boolean (const char *cCommand) {
	FILE *pPipe = popen (cCommand,"r");
	if (!pPipe)
		return FALSE;
	gchar *cRead = (gchar *) malloc (56*sizeof(gchar));
	if (!fgets(cRead,56,pPipe)) {
		g_free(cRead);
		pclose(pPipe);
		return FALSE;
	}
	pclose(pPipe);
	strtok (cRead,"\n");
	gboolean bRet = g_strcasecmp(cRead,"true")==0;
	g_free(cRead);
	return bRet;
}
/***************************************************************************/
