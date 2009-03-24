#include <stdlib.h>
#include <math.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"
#include "rhythmbox-draw.h"
#include "3dcover-draw.h"

/* Declaration des Diplaylists */
GLuint draw_cover;


void cd_opengl_load_external_conf_theme_values (CairoDockModuleInstance *myApplet)
{
	GString *sElementPath = g_string_new ("");
	gchar *sFramePic = NULL;
	gchar *sReflectPic = NULL;
	
	g_string_printf (sElementPath, "%s/%s", myConfig.cThemePath, "theme.conf");
	
	if (g_file_test (sElementPath->str, G_FILE_TEST_EXISTS))
	{
		GError *erreur = NULL;
		GKeyFile *pKeyFile = cairo_dock_open_key_file (sElementPath->str);
			
		
		if (pKeyFile != NULL)
		{
			myData.itopleftX = g_key_file_get_integer (pKeyFile, "Configuration", "topleftX", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.itopleftY = g_key_file_get_integer (pKeyFile, "Configuration", "topleftY", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.ibottomleftX = g_key_file_get_integer (pKeyFile, "Configuration", "bottomleftX", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.ibottomleftY = g_key_file_get_integer (pKeyFile, "Configuration", "bottomleftY", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.ibottomrightX = g_key_file_get_integer (pKeyFile, "Configuration", "bottomrightX", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.ibottomrightY = g_key_file_get_integer (pKeyFile, "Configuration", "bottomrightY", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.itoprightX = g_key_file_get_integer (pKeyFile, "Configuration", "toprightX", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.itoprightY = g_key_file_get_integer (pKeyFile, "Configuration", "toprightY", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}						
			sFramePic = g_key_file_get_string (pKeyFile, "Pictures", "frame", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.cThemeFrame = g_strdup_printf ("%s/%s", myConfig.cThemePath, sFramePic);
						
			
			sReflectPic = g_key_file_get_string (pKeyFile, "Pictures", "reflect", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.cThemeReflect = g_strdup_printf ("%s/%s", myConfig.cThemePath, sReflectPic);
						
			g_key_file_free (pKeyFile);
		}
		else  // on prend des valeurs par defaut assez larges.
		{
			myData.itopleftX = -500;
			myData.itopleftY = 500;
			myData.ibottomleftX = -500;
			myData.ibottomleftY = -500;
			myData.ibottomrightX = 500;
			myData.ibottomrightY = -500;
			myData.itoprightX = 500;
			myData.itoprightY = 500;
		}
		
	}

	g_string_free (sElementPath, TRUE);
	
	g_free (sFramePic);
	g_free (sReflectPic);	
	
}


GLuint cd_opengl_load_texture (CairoDockModuleInstance *myApplet, gchar *texture)
{
	gchar *cImagePath;
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
			cImagePath = g_strdup_printf ("%s", texture);		
			myData.TextureName = cairo_dock_create_texture_from_image(cImagePath);
	}
	else
	{
		myData.TextureName = 0;
	}

  g_free( cImagePath );
  return myData.TextureName;
}

void cd_opengl_init_opengl_datas (void)
{
	//\_______________ On charge les textures.
	myData.TextureFrame = cd_opengl_load_texture (myApplet, myData.cThemeFrame);	
	myData.TextureReflect = cd_opengl_load_texture (myApplet, myData.cThemeReflect);

	//\_______________ On definie la calllist qui déforme la pochette.
	draw_cover = glGenLists(1);
	glNewList(draw_cover , GL_COMPILE);
	glBegin(GL_QUADS);
	glColor4ub(255,255,255,255); 
	glTexCoord2d(0,0);
	glVertex3d(myData.itopleftX/1000., myData.itopleftY/1000., 0);
	glTexCoord2d(0,1);
	glVertex3d(myData.ibottomleftX/1000., myData.ibottomleftY/1000., 0);
	glTexCoord2d(1,1);
	glVertex3d(myData.ibottomrightX/1000., myData.ibottomrightY/1000., 0);
	glTexCoord2d(1,0);
	glVertex3d(myData.itoprightX/1000., myData.itoprightY/1000., 0);
	glEnd();
	glEndList();
	
}

void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
		
	glEnable(GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc ( GL_GREATER, 0.1 ) ;
	glEnable ( GL_ALPHA_TEST ) ;
	
	glEnable(GL_TEXTURE_2D);
	
	glPushMatrix ();
			
	// On recentre la matrice et on la recule pour voir toute la scene.
	glTranslatef (iWidth/2, iHeight/2, -iWidth);
	glScalef(iWidth, -iHeight, iWidth);
	
	glDepthMask(GL_FALSE);		/* Je passe le depth en lecture seul -> Tout s'affichera dans l'ordre
								de création sans tenir compte de la profondeur. */
	
		
	cairo_dock_apply_texture (myData.TextureFrame);
	
	
	
	
	glBindTexture(GL_TEXTURE_2D, myData.TextureCover);
	//~ cd_message ("RB_YDU : la couverture '%s' est deja dispo", myData.playing_cover);
	glCallList(draw_cover);
	
	
	
	cairo_dock_apply_texture (myData.TextureReflect);
	
		
	glDepthMask(GL_TRUE);
	
		
	glDisable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 1.);
	glDisable ( GL_ALPHA_TEST ) ;
	glDisable (GL_BLEND);
	glDisable (GL_DEPTH_TEST);
		
	glPopMatrix ();
}

void cd_opengl_render_to_texture (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
		return ;

	cd_opengl_scene (myApplet, iWidth, iHeight);

	cairo_dock_end_draw_icon (myIcon, myContainer);
	CD_APPLET_REDRAW_MY_ICON;
}

gboolean cd_opengl_test_update_icon_slow (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer, gboolean *bContinueAnimation)
{	
	if (pContainer != myContainer)
		return CAIRO_DOCK_LET_PASS_NOTIFICATION;

		
	// taille de la texture.
	double fMaxScale = cairo_dock_get_max_scale (pContainer);
	double fRatio = pContainer->fRatio;
	int iWidth = (int) pIcon->fWidth / fRatio * fMaxScale;
	int iHeight = (int) pIcon->fHeight / fRatio * fMaxScale;

	cd_opengl_render_to_texture (myApplet, iWidth, iHeight);

	*bContinueAnimation = TRUE;

	return CAIRO_DOCK_LET_PASS_NOTIFICATION;
}
