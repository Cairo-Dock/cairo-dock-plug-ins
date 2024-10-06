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
#include <math.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "cd-mail-applet-config.h"
#include "cd-mail-applet-notifications.h"
#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-accounts.h"
#include "cd-mail-applet-etpan.h"
#include "cd-mail-applet-init.h"


CD_APPLET_DEFINE2_BEGIN (N_("mail"),
	CAIRO_DOCK_MODULE_DEFAULT_FLAGS,
	CAIRO_DOCK_CATEGORY_APPLET_INTERNET,
	N_("This applet is very useful to warn you when you get new e-mails\n"
	"It can check in any kind of mailbox (yahoo, gmail, etc)\n"
	"Left-click to launch the preferred mail application,\n"
	"Middle-click to refresh all the mailboxes."),
	"Tofe (Christophe Chapuis) &amp; Fabounet (Fabrice Rey)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_mail_load_custom_widget;
	pInterface->save_custom_widget = cd_mail_save_custom_widget;
CD_APPLET_DEFINE2_END


GLuint cd_mail_load_cube_calllist (void)
{
	GLuint iCallList = glGenLists (1);
	glNewList(iCallList, GL_COMPILE); // Go pour la compilation de la display list
	glPolygonMode (GL_FRONT, GL_FILL);
	
	double a = .5 / sqrt (2);
	glBegin(GL_QUADS);
	// Front Face (note that the texture's corners have to match the quad's corners)
  //glColor3f(1.0f,0.5f,0.0f);
	glNormal3f(0,0,1);
	glTexCoord2d(0., 0.); glVertex3f(-a,  a,  a);  // Bottom Left Of The Texture and Quad
	glTexCoord2d(1., 0.); glVertex3f( a,  a,  a);  // Bottom Right Of The Texture and Quad
	glTexCoord2d(1., 1.); glVertex3f( a, -a,  a);  // Top Right Of The Texture and Quad
	glTexCoord2d(0., 1.); glVertex3f(-a, -a,  a);  // Top Left Of The Texture and Quad
	// Back Face
	glNormal3f(0,0,-1);
	glTexCoord2d(1., 0.); glVertex3f( -a, a, -a);  // Bottom Right Of The Texture and Quad
	glTexCoord2d(1., 1.); glVertex3f( -a, -a, -a);  // Top Right Of The Texture and Quad
	glTexCoord2d(0., 1.); glVertex3f(a, -a, -a);  // Top Left Of The Texture and Quad
	glTexCoord2d(0., 0.); glVertex3f(a, a, -a);  // Bottom Left Of The Texture and Quad
	// Top Face
  //glColor3f(1.0f,0.f,1.0f);
	glNormal3f(0,1,0);
	glTexCoord2d(0., 1.); glVertex3f(-a,  a,  a);  // Top Left Of The Texture and Quad
	glTexCoord2d(0., 0.); glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
	glTexCoord2d(1., 0.); glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
	glTexCoord2d(1., 1.); glVertex3f( a,  a,  a);  // Top Right Of The Texture and Quad
	// Bottom Face
	glNormal3f(0,-1,0);
	glTexCoord2d(1., 1.); glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
	glTexCoord2d(0., 1.); glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
	glTexCoord2d(0., 0.); glVertex3f(-a, -a,  a);  // Bottom Left Of The Texture and Quad
	glTexCoord2d(1., 0.); glVertex3f( a, -a,  a);  // Bottom Right Of The Texture and Quad
	// Right face
  //glColor3f(0.f,0.5f,1.0f);
	glNormal3f(1,0,0);
	glTexCoord2d(1., 0.);  glVertex3f( a,  a, -a);  // Bottom Right Of The Texture and Quad
	glTexCoord2d(1., 1.);  glVertex3f( a, -a, -a);  // Top Right Of The Texture and Quad
	glTexCoord2d(0., 1.);  glVertex3f( a, -a,  a);  // Top Left Of The Texture and Quad
	glTexCoord2d(0., 0.);  glVertex3f( a,  a,  a);  // Bottom Left Of The Texture and Quad
	// Left Face
	glNormal3f(-1,0,0);
	glTexCoord2d(0., 0.);  glVertex3f(-a,  a, -a);  // Bottom Left Of The Texture and Quad
	glTexCoord2d(1., 0.);  glVertex3f(-a,  a,  a);  // Bottom Right Of The Texture and Quad
	glTexCoord2d(1., 1.);  glVertex3f(-a, -a,  a);  // Top Right Of The Texture and Quad
	glTexCoord2d(0., 1.);  glVertex3f(-a, -a, -a);  // Top Left Of The Texture and Quad
	glEnd();
	
	glEndList(); // Fini la display list
	return iCallList;
}


static void _load_theme (GldiModuleInstance *myApplet, GError **erreur)
{
	//\_______________ On charge le theme si necessaire, avec en priorite les images utilisateur.
	if (myConfig.cNoMailUserImage != NULL)
	{
		gchar *cPath = cairo_dock_search_icon_s_path (myConfig.cNoMailUserImage, MAX (myIcon->image.iWidth, myIcon->image.iHeight));
		if (! g_file_test (cPath, G_FILE_TEST_EXISTS))
		{
			g_free (myConfig.cNoMailUserImage);
			myConfig.cNoMailUserImage = NULL;
		}
		g_free (cPath);
	}
	if (myConfig.cHasMailUserImage != NULL)
	{
		gchar *cPath = cairo_dock_search_icon_s_path (myConfig.cHasMailUserImage, MAX (myIcon->image.iWidth, myIcon->image.iHeight));
		if (! g_file_test (cPath, G_FILE_TEST_EXISTS))
		{
			g_free (myConfig.cHasMailUserImage);
			myConfig.cHasMailUserImage = NULL;
		}
		g_free (cPath);
	}
	if (myConfig.cThemePath != NULL && (myConfig.cNoMailUserImage == NULL || myConfig.cHasMailUserImage == NULL))
	{
		GError *tmp_erreur = NULL;
		GDir *dir = g_dir_open (myConfig.cThemePath, 0, &tmp_erreur);
		if (tmp_erreur != NULL)
		{
			g_propagate_error (erreur, tmp_erreur);
			return ;
		}
		
		const gchar *cElementName;
		gchar *cElementPath;
		while ((cElementName = g_dir_read_name (dir)) != NULL)
		{
			cElementPath = g_strdup_printf ("%s/%s", myConfig.cThemePath, cElementName);
			cd_debug ("  Mail theme item: %s", cElementPath);
			if (strncmp (cElementName, "no_mail", 7) == 0 && myConfig.cNoMailUserImage == NULL)
			{
				myConfig.cNoMailUserImage = cElementPath;
			}
			else if (strncmp (cElementName, "has_mail", 8) == 0 && myConfig.cHasMailUserImage == NULL)
			{
				myConfig.cHasMailUserImage = cElementPath;
			}
			else if (strncmp (cElementName, "new_mail_sound", 14) == 0 && myConfig.cNewMailUserSound == NULL)
			{
				myConfig.cNewMailUserSound = cElementPath;
			}
			else
			{
				g_free (cElementPath);
			}
		}
		g_dir_close (dir);
	}
	if (myConfig.cNoMailUserImage == NULL || myConfig.cHasMailUserImage == NULL || myConfig.cNewMailUserSound == NULL)
	{
		cd_warning ("mail : couldn't find images, this theme is not valid");
	}
	
	// textures et calllist.
	if (myData.iNoMailTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iNoMailTexture);
		myData.iNoMailTexture = 0;
	}
	if (myData.iHasMailTexture != 0)
	{
		_cairo_dock_delete_texture (myData.iHasMailTexture);
		myData.iHasMailTexture = 0;
	}
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
	{
		if (myConfig.cNoMailUserImage != NULL)
			myData.iNoMailTexture = cairo_dock_create_texture_from_image(myConfig.cNoMailUserImage);
		if (myConfig.cHasMailUserImage != NULL)
			myData.iHasMailTexture = cairo_dock_create_texture_from_image(myConfig.cHasMailUserImage);

		if (myData.iCubeCallList == 0)
			myData.iCubeCallList = cd_mail_load_cube_calllist();
	}

	// on cree le repertoire de l'historique si necessaire.
	myData.cWorkingDirPath = g_strdup_printf ("%s/mail", g_cCairoDockDataDir);
	if (! g_file_test (myData.cWorkingDirPath, G_FILE_TEST_EXISTS))
	{
		cd_debug ("Plugin Mail : le dossier '%s' n'existe pas encore -> On le cree", myData.cWorkingDirPath);
		if (g_mkdir (myData.cWorkingDirPath, 7*8*8+7*8+0) != 0)
		{
			cd_warning ("couldn't create directory '%s' !\nNo read history will be available.", myData.cWorkingDirPath);
		}
	}
}

CD_APPLET_INIT_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}

	//\_______________ On charge surfaces et textures.
	GError *erreur = NULL;
	_load_theme (myApplet, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("mail : %s", erreur->message);
		g_error_free (erreur);
		return;
	}
	
	//\_______________ On initialise tous les comptes et on lance un timer pour chacun.
	myData.iPrevNbUnreadMails = G_MAXUINT;
	cd_mail_init_accounts(myApplet);
	
	//\_______________ On s'abonne aux notifications.
	CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_REGISTER_FOR_SCROLL_EVENT;
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
	{
		CD_APPLET_REGISTER_FOR_UPDATE_ICON_EVENT;
	}
CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN

	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_SCROLL_EVENT;
	CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_EVENT;
	
	///CD_APPLET_MANAGE_APPLICATION (NULL);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN

	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED )
	{
		CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_EVENT;
		
		GError *erreur = NULL;
		_load_theme (myApplet, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("mail : when trying to load theme : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
		{
			CD_APPLET_REGISTER_FOR_UPDATE_ICON_EVENT;
		}
		
		cd_mail_init_accounts(myApplet);  // la config vient d'etre rechargee, donc les comptes sont tout neufs.
	}
	
CD_APPLET_RELOAD_END
