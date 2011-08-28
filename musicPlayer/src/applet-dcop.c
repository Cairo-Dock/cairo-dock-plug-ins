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
