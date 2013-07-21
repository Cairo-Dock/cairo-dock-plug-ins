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

#include <stdlib.h>
#include <string.h>

#include "applet-struct.h"
#include "applet-dnd2share.h"
#include "applet-backend-custom.h"

#define NB_URLS 1
static const gchar *s_UrlLabels[NB_URLS] = {N_("Direct Link")};


static void _upload (CDFileType iCurrentFileType, const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	if (myConfig.cCustomScripts[iCurrentFileType] == NULL)
	{
		const gchar *cError = D_("No script set for this file type");
		cd_warning (cError);
		g_set_error (pError, 1, 1, cError);
		return;
	}

	// Upload the file
	gchar *cCommand = g_strdup_printf ("%s '%s'", myConfig.cCustomScripts[iCurrentFileType], cFilePath);
	gchar *cResult = cairo_dock_launch_command_sync (cCommand);
	g_free (cCommand);
	if (cResult == NULL || *cResult == '\0')
	{
		g_set_error (pError, 1, 1, DND2SHARE_GENERIC_ERROR_MSG);
		return;
	}
	
	if (cResult[strlen(cResult)-1] == '\r')
		cResult[strlen(cResult)-1] = '\0';
	if (cResult[strlen(cResult)-1] == '\n')
		cResult[strlen(cResult)-1] = '\0';

	// We take the last line in case of there are some log text before
	gchar *str = strrchr (cResult, '\n');
	if (str != NULL)
		str ++;
	else
		str = cResult;
	
	if (! cairo_dock_string_is_adress (str))
		cd_warning ("this address (%s) seems not valid !\nThe output was : '%s'", str, cResult);

	cResultUrls[0] = g_strdup (str);
	g_free (cResult);
}

static void upload_text (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	if (cFilePath == NULL || *cFilePath == '\0')
	{
		g_set_error (pError, 1, 1, D_("Your text is empty and couldn't be uploaded to this server"));
		return;
	}
	_upload (CD_TYPE_TEXT, cFilePath, cLocalDir, bAnonymous, iLimitRate, cResultUrls, pError);
}

static void upload_image (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	_upload (CD_TYPE_IMAGE, cFilePath, cLocalDir, bAnonymous, iLimitRate, cResultUrls, pError);
}

static void upload_video (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	_upload (CD_TYPE_VIDEO, cFilePath, cLocalDir, bAnonymous, iLimitRate, cResultUrls, pError);
}

static void upload_file (const gchar *cFilePath, gchar *cLocalDir, gboolean bAnonymous, gint iLimitRate, gchar **cResultUrls, GError **pError)
{
	_upload (CD_TYPE_FILE, cFilePath, cLocalDir, bAnonymous, iLimitRate, cResultUrls, pError);
}

static const CDUploadFunc upload_funcs[CD_NB_FILE_TYPES] = {upload_text, upload_image, upload_video, upload_file};

void cd_dnd2share_register_custom_backends (void)
{
	CDFileType t;
	for (t = 0; t < CD_NB_FILE_TYPES; t ++)
	{
		cd_dnd2share_register_new_backend (t,
			"custom",
			NB_URLS,
			s_UrlLabels,
			0,
			upload_funcs[t]);
	}
}
