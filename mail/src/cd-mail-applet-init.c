/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "cd-mail-applet-config.h"
#include "cd-mail-applet-notifications.h"
#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-init.h"


CD_APPLET_PRE_INIT_BEGIN (N_("mail"),
	2, 0, 0,
	CAIRO_DOCK_CATEGORY_ACCESSORY,
	N_("This applet is very useful to warn you when you get new e-mails\n"
	"It can check in any kind of mailbox (yahoo, gmail, etc)"),
	"Tofe (Christophe Chapuis)")
	CD_APPLET_DEFINE_COMMON_APPLET_INTERFACE
	pInterface->load_custom_widget = cd_mail_load_custom_widget;
	pInterface->save_custom_widget = cd_mail_save_custom_widget;
CD_APPLET_PRE_INIT_END


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

#define RADIAN (G_PI / 180.0)  // Conversion Radian/Degres

GLuint cd_mail_load_capsule_calllist (void)
{
	GLuint iCallList = glGenLists (1);
	int        deg, deg2, iter, nb_iter=20;
	float        amp, rayon, c=2.;
	
	rayon        = 1.0f/c;
	amp        = 90.0 / nb_iter;
	deg2        = 0;
	
	glNewList(iCallList, GL_COMPILE); // Go pour la compilation de la display list
	glPolygonMode (GL_FRONT, GL_FILL);
	
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPushMatrix ();
	glLoadIdentity(); // On la reset
	glTranslatef(0.5f, 0.5f, 0.); // Et on decale la texture pour un affiche propre
	glRotatef (180, 1, 0, 0);  // sinon les icones sont a l'envers.
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	// bon la je commente pas on fait juste une demi sphere applatie
	double a = .5/c;  // applatissement;
	double b = 1./nb_iter;
	double xab, yab, zab, xac, yac, zac, nx, ny, nz, n;
	
	glBegin(GL_QUADS);
	for (iter = 0;iter < nb_iter-1;iter ++)
	{
		for (deg = 0;deg < 360;deg += 10)
		{
			xab = b * cos(deg*RADIAN);
			yab = b * sin(deg*RADIAN);
			zab = a * sin(deg2*RADIAN) - a * sin((deg2+amp)*RADIAN);
			//zab = a*cos (deg2*RADIAN) * amp*RADIAN;
			xac = rayon * cos((deg+10)*RADIAN) - (rayon-b) * cos(deg*RADIAN);
			yac = rayon * sin((deg+10)*RADIAN) - (rayon-b) * sin(deg*RADIAN);
			zac = a * sin(deg2*RADIAN) - a * sin((deg2+amp)*RADIAN);
			//zac = a * sin((deg2+amp)*RADIAN) - a * sin(deg2*RADIAN);
			nx = yab*zac - zab*yac;
			ny = zab*xac - xab*zac;
			nz = xab*yac - yab*xac;
			n = sqrt (nx*nx + ny*ny + nz*nz);
			
			glNormal3f (nx/n, ny/n, nz/n);
			
			glVertex3f((rayon-b) * cos(deg*RADIAN),
				(rayon-b) * sin(deg*RADIAN),
				a * sin((deg2+amp)*RADIAN) + 0.1f/c);
			glVertex3f(rayon * cos(deg*RADIAN),
				rayon * sin(deg*RADIAN),
				a * sin(deg2*RADIAN) + 0.1f/c);
			glVertex3f(rayon * cos((deg+10)*RADIAN),
				rayon * sin((deg+10)*RADIAN),
				a * sin(deg2*RADIAN) + 0.1f/c);
			glVertex3f((rayon-b) * cos((deg+10)*RADIAN),
				(rayon-b) * sin((deg+10)*RADIAN),
				a * sin((deg2+amp)*RADIAN) + 0.1f/c);
			
			//nx = - nx;
			//ny = - ny;
			nz = - nz;
			
			glNormal3f (nx/n, ny/n, nz/n);
			glVertex3f((rayon-b) * cos(deg*RADIAN),
				 (rayon-b) * sin(deg*RADIAN),
				-a * sin((deg2+amp)*RADIAN) - 0.1f/c);
			glVertex3f(rayon * cos(deg*RADIAN),
				 rayon * sin(deg*RADIAN),
				-a * sin(deg2*RADIAN) - 0.1f/c);
			glVertex3f(rayon * cos((deg+10)*RADIAN),    
				 rayon * sin((deg+10)*RADIAN),
				-a * sin(deg2*RADIAN) - 0.1f/c);
			glVertex3f((rayon-b) * cos((deg+10)*RADIAN),
				 (rayon-b) * sin((deg+10)*RADIAN),
				-a * sin((deg2+amp)*RADIAN) - 0.1f/c);
		}
		rayon    -= b/c;
		deg2    += amp;
	}
	glEnd();
	
	glMatrixMode(GL_TEXTURE); // On selectionne la matrice des textures
	glPopMatrix ();
	glMatrixMode(GL_MODELVIEW); // On revient sur la matrice d'affichage
	
	glEndList(); // Fini la display list
	
	return iCallList;
}

static void _load_theme (CairoDockModuleInstance *myApplet, GError **erreur)
{
	//\_______________ On charge le theme si necessaire, avec en priorite les images utilisateur.
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
			cd_message ("  Mail theme item: %s\n", cElementPath);
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

	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
	{
		if (myConfig.cNoMailUserImage != NULL)
			myData.iNoMailTexture = cairo_dock_create_texture_from_image(myConfig.cNoMailUserImage);
		if (myConfig.cHasMailUserImage != NULL)
			myData.iHasMailTexture = cairo_dock_create_texture_from_image(myConfig.cHasMailUserImage);

		myData.iCubeCallList = cd_mail_load_cube_calllist();
		myData.iCapsuleCallList = 0;//cd_mail_load_capsule_calllist();
	}
	else
	{
		myData.iNoMailTexture = 0;
		myData.iHasMailTexture = 0;
		myData.iCubeCallList = 0;
		myData.iCapsuleCallList = 0;
	}

  cd_debug( "cd_mail : myData.iNoMailTexture = %d", myData.iNoMailTexture );
  cd_debug( "cd_mail : myData.iHasMailTexture = %d", myData.iHasMailTexture );

}

CD_APPLET_INIT_BEGIN
	
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}

	GError *erreur = NULL;
	_load_theme (myApplet, &erreur);
	if (erreur != NULL)
	{
		cd_warning ("mail : %s", erreur->message);
		g_error_free (erreur);
		return;
	}
	
	/*if (myIcon->acName == NULL && myDock)
	{
		CD_APPLET_SET_NAME_FOR_MY_ICON (MAIL_DEFAULT_NAME);
	}*/
	
	//CD_APPLET_REGISTER_FOR_CLICK_EVENT;
	cairo_dock_register_notification (CAIRO_DOCK_CLICK_ICON, (CairoDockNotificationFunc) CD_APPLET_ON_CLICK_FUNC, CAIRO_DOCK_RUN_FIRST, myApplet);  // on se met en premier pour pas que le dock essaye de lancer nos icones, car ce ne sont pas toutes des lanceurs, donc on va le faire nous-memes.
	CD_APPLET_REGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_REGISTER_FOR_MIDDLE_CLICK_EVENT;

	cd_mail_update_status( myApplet );

	if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
	{
		///CD_APPLET_REGISTER_FOR_UPDATE_ICON_EVENT;
		cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_mail_update_icon , CAIRO_DOCK_RUN_FIRST, myApplet);
	}

CD_APPLET_INIT_END


CD_APPLET_STOP_BEGIN
	//\_______________ On se desabonne de nos notifications.
	CD_APPLET_UNREGISTER_FOR_CLICK_EVENT;
	CD_APPLET_UNREGISTER_FOR_BUILD_MENU_EVENT;
	CD_APPLET_UNREGISTER_FOR_MIDDLE_CLICK_EVENT;

	///CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_EVENT;
	cairo_dock_remove_notification_func(CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_mail_update_icon , myApplet);
CD_APPLET_STOP_END


CD_APPLET_RELOAD_BEGIN
	if (myDesklet)
	{
		CD_APPLET_SET_DESKLET_RENDERER ("Simple");
	}

	//\_______________ On recharge les donnees qui ont pu changer.
	if (CD_APPLET_MY_CONFIG_CHANGED )
	{
		///CD_APPLET_UNREGISTER_FOR_UPDATE_ICON_EVENT;
		cairo_dock_remove_notification_func(CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_mail_update_icon , myApplet);
	
		GError *erreur = NULL;
		_load_theme (myApplet, &erreur);
		if (erreur != NULL)
		{
			cd_warning ("mail : when trying to load theme : %s", erreur->message);
			g_error_free (erreur);
			erreur = NULL;
		}
		
		/*if (myIcon->acName == NULL && myDock)
		{
			CD_APPLET_SET_NAME_FOR_MY_ICON (MAIL_DEFAULT_NAME);
		}*/
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL && myDesklet)
		{
			///CD_APPLET_REGISTER_FOR_UPDATE_ICON_EVENT;
			cairo_dock_register_notification (CAIRO_DOCK_UPDATE_ICON, (CairoDockNotificationFunc) cd_mail_update_icon , CAIRO_DOCK_RUN_FIRST, myApplet);
		}
	}

	cd_mail_update_status( myApplet );
	
CD_APPLET_RELOAD_END
