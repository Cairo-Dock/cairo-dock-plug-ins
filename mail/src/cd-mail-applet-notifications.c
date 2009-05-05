/******************************************************************************

This file is a part of the cairo-dock program,
released under the terms of the GNU General Public License.

Written by Fabrice Rey (for any bug report, please mail me to fabounet@users.berlios.de)

******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <glib/gi18n.h>
#include <time.h>
#include <math.h>

#include "cd-mail-applet-struct.h"
#include "cd-mail-applet-notifications.h"


static void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet);


CD_APPLET_ON_CLICK_BEGIN

    // spawn the selected program
	if( myConfig.cMailApplication )
	{
		gboolean r = cairo_dock_launch_command (myConfig.cMailApplication);
		
		if (!r)
		{
			cd_warning ("when couldn't execute '%s'", myConfig.cMailApplication);
			cairo_dock_show_temporary_dialog (D_("A problem occured\nIf '%s' is not your usual mail application,\nyou can change it in the conf panel of this module"), myIcon, myContainer, 5000, myConfig.cMailApplication);
		}
	}

CD_APPLET_ON_CLICK_END


static void _cd_mail_force_update(CairoDockModuleInstance *myApplet)
{
	guint i;
	if (myData.pMailAccounts != NULL)
	{
		for (i = 0; i < myData.pMailAccounts->len; i ++)
		{
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			if( pMailAccount )
			{
				cairo_dock_launch_measure(pMailAccount->pAccountMailTimer);
			}
		}
	}
}
CD_APPLET_ON_MIDDLE_CLICK_BEGIN

    _cd_mail_force_update(myApplet);

CD_APPLET_ON_MIDDLE_CLICK_END


static void _cd_mail_update_account (GtkMenuItem *menu_item, CDMailAccount *pMailAccount)
{
	if( pMailAccount )
	{
		cairo_dock_launch_measure(pMailAccount->pAccountMailTimer);
	}
}
static void _cd_mail_launch_mail_appli (GtkMenuItem *menu_item, CairoDockModuleInstance *myApplet)
{
	cairo_dock_launch_command (myConfig.cMailApplication);
}
CD_APPLET_ON_BUILD_MENU_BEGIN
	
	GtkWidget *pSubMenu = CD_APPLET_CREATE_MY_SUB_MENU ();
    if(myData.pMailAccounts && myData.pMailAccounts->len > 0)
    {
        /* add a "update account" item for each mailbox */
        GtkWidget *pRefreshAccountSubMenu = CD_APPLET_ADD_SUB_MENU (D_("Refresh a mail account"), pSubMenu);
        
        guint i;
        for (i = 0; i < myData.pMailAccounts->len; i ++)
        {
			CDMailAccount *pMailAccount = g_ptr_array_index (myData.pMailAccounts, i);
			CD_APPLET_ADD_IN_MENU_WITH_DATA (pMailAccount->name, _cd_mail_update_account, pRefreshAccountSubMenu, pMailAccount);
        }
    }
	if (myConfig.cMailApplication)
	{
		gchar *cLabel = g_strdup_printf (D_("Launch %s"), myConfig.cMailApplication);
		CD_APPLET_ADD_IN_MENU (cLabel, _cd_mail_launch_mail_appli, pSubMenu);
		g_free (cLabel);
	}
	CD_APPLET_ADD_ABOUT_IN_MENU (pSubMenu);
	
CD_APPLET_ON_BUILD_MENU_END


CD_APPLET_ON_UPDATE_ICON_BEGIN
	double fSpeedX, fSpeedY;
	if (myData.iNbUnreadMails == 0)
	{
		fSpeedX = 2.;
		fSpeedY = 2.;
	}
	else
	{
		fSpeedX = 2 * MAX (10., sqrt (myData.iNbUnreadMails));
		fSpeedY = fSpeedX/2;
	}
	if( myData.iNbUnreadMails > 0 || myData.current_rotX != 0 )  // mails non lus ou on finit la rotation en cours.
	{
		myData.current_rotX += fSpeedX;
	}
	if( myData.iNbUnreadMails > 0 || myData.current_rotY != 0 )  // mails non lus ou on finit la rotation en cours.
	{
		myData.current_rotY += fSpeedY;
	}

	if( myData.current_rotX>=360.f )
	{
		if (myData.iNbUnreadMails > 0)
			myData.current_rotX -= 360.f;  // on se ramene juste dans [0;360[
		else
			myData.current_rotX = 0;  // on s'arrete la.
	}
	if( myData.current_rotY>=360.f )
	{
		if (myData.iNbUnreadMails > 0)
			myData.current_rotY -= 360.f;
		else
			myData.current_rotY = 0;
	}
	
	cd_mail_render_3D_to_texture (myApplet);

	if( myData.iNbUnreadMails <= 0 && myData.current_rotX == 0 && myData.current_rotY == 0 )
	{
		CD_APPLET_STOP_UPDATE_ICON;
	}
CD_APPLET_ON_UPDATE_ICON_END



static void cd_mail_render_3D_to_texture (CairoDockModuleInstance *myApplet)
{
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();

	double fMaxScale = cairo_dock_get_max_scale (myContainer);
	double fRatio = (myDock ? myDock->fRatio : 1);
	int iWidth = (int) myIcon->fWidth / fRatio * fMaxScale;
	int iHeight = (int) myIcon->fHeight / fRatio * fMaxScale;

  //cd_debug( "iWidth=%d iHeight=%d", iWidth, iHeight);
  
	glPushMatrix ();

  glScalef(0.8*iWidth, 0.8*iHeight, 1.0);
  glTranslatef(0., 0., -1.0);

  glRotatef(myData.current_rotX, 1.0f, 0.0f, 0.0f);  /* rotate on the X axis */
  glRotatef(myData.current_rotY, 0.0f, 1.0f, 0.0f);  /* rotate on the Y axis */
//  glRotatef(30.f, 0.0f, 0.0f, 1.0f);  /* rotate on the Z axis */

	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // rend le cube transparent.
  glAlphaFunc ( GL_GREATER, 0.1 ) ;
  glEnable ( GL_ALPHA_TEST ) ;

	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_TEXTURE_GEN_S);                                // oui je veux une generation en S
	//glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, myData.iNbUnreadMails > 0?myData.iHasMailTexture:myData.iNoMailTexture);
	//glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR); // type de generation des coordonnees de la texture
	//glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	//glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); // type de generation des coordonnees de la texture
	//glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // pour les bouts de textures qui depassent.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glCallList (myData.iCubeCallList);
//	glCallList (myData.iCapsuleCallList);

	glDisable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);

  glDisable ( GL_ALPHA_TEST ) ;
	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
	glPopMatrix ();

	CD_APPLET_FINISH_DRAWING_MY_ICON;
}
