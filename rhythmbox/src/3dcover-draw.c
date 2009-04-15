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
			
			
			// Ajout récup zones cliquables et de l'OSD
			myData.numberButtons = g_key_file_get_integer (pKeyFile, "Buttons", "number", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
			myData.osd = g_key_file_get_boolean (pKeyFile, "Buttons", "osd", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}			
			myData.b3dThemesDebugMode = g_key_file_get_boolean (pKeyFile, "Buttons", "debug", &erreur);
			if (erreur != NULL)
			{
				cd_warning (erreur->message);
				g_error_free (erreur);
				erreur = NULL;
			}
				
			
			if (myData.numberButtons != 0)
			{
				gchar *sButtonPic = NULL;
												
				sButtonPic = g_key_file_get_string (pKeyFile, "Button1", "picture", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				myData.cThemeButton1 = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
				
				myData.button1coordX = g_key_file_get_integer (pKeyFile, "Button1", "X", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				myData.button1coordY = g_key_file_get_integer (pKeyFile, "Button1", "Y", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				myData.button1sizeX = g_key_file_get_integer (pKeyFile, "Button1", "sizeX", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				myData.button1sizeY = g_key_file_get_integer (pKeyFile, "Button1", "sizeY", &erreur);
				if (erreur != NULL)
				{
					cd_warning (erreur->message);
					g_error_free (erreur);
					erreur = NULL;
				}
				if (myData.osd)
				{
					sButtonPic = g_key_file_get_string (pKeyFile, "Button1", "osd_play", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.cOsdPlay = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					
					sButtonPic = g_key_file_get_string (pKeyFile, "Button1", "osd_pause", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.cOsdPause = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
				}
				
				
				if (myData.numberButtons > 3)
				{
					sButtonPic = g_key_file_get_string (pKeyFile, "Button4", "picture", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.cThemeButton4 = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					
					myData.button4coordX = g_key_file_get_integer (pKeyFile, "Button4", "X", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button4coordY = g_key_file_get_integer (pKeyFile, "Button4", "Y", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button4sizeX = g_key_file_get_integer (pKeyFile, "Button4", "sizeX", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button4sizeY = g_key_file_get_integer (pKeyFile, "Button4", "sizeY", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					
					if (myData.osd)
					{
						sButtonPic = g_key_file_get_string (pKeyFile, "Button4", "osd", &erreur);
						if (erreur != NULL)
						{
							cd_warning (erreur->message);
							g_error_free (erreur);
							erreur = NULL;
						}
						myData.cOsdHome = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					}
				}
				
				
				if (myData.numberButtons > 2)
				{
					sButtonPic = g_key_file_get_string (pKeyFile, "Button3", "picture", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.cThemeButton3 = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					
					myData.button3coordX = g_key_file_get_integer (pKeyFile, "Button3", "X", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button3coordY = g_key_file_get_integer (pKeyFile, "Button3", "Y", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button3sizeX = g_key_file_get_integer (pKeyFile, "Button3", "sizeX", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button3sizeY = g_key_file_get_integer (pKeyFile, "Button3", "sizeY", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					
					if (myData.osd)
					{
						sButtonPic = g_key_file_get_string (pKeyFile, "Button3", "osd", &erreur);
						if (erreur != NULL)
						{
							cd_warning (erreur->message);
							g_error_free (erreur);
							erreur = NULL;
						}
						myData.cOsdNext = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					}
				}
				
				
				if (myData.numberButtons > 1)
				{
					sButtonPic = g_key_file_get_string (pKeyFile, "Button2", "picture", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.cThemeButton2 = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					
					myData.button2coordX = g_key_file_get_integer (pKeyFile, "Button2", "X", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button2coordY = g_key_file_get_integer (pKeyFile, "Button2", "Y", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button2sizeX = g_key_file_get_integer (pKeyFile, "Button2", "sizeX", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					myData.button2sizeY = g_key_file_get_integer (pKeyFile, "Button2", "sizeY", &erreur);
					if (erreur != NULL)
					{
						cd_warning (erreur->message);
						g_error_free (erreur);
						erreur = NULL;
					}
					
					if (myData.osd)
					{
						sButtonPic = g_key_file_get_string (pKeyFile, "Button2", "osd", &erreur);
						if (erreur != NULL)
						{
							cd_warning (erreur->message);
							g_error_free (erreur);
							erreur = NULL;
						}
						myData.cOsdPrev = g_strdup_printf ("%s/%s", myConfig.cThemePath, sButtonPic);
					}
				}
				
				g_free (sButtonPic);
			}
				
			else
			{
				// Si aucun paramètre de bouton n'est rentré OU si le nombre est à zéro -> On ne fait rien
			}
			
								
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
	
	if (myData.numberButtons != 0)
	{
		myData.TextureButton1 = cd_opengl_load_texture (myApplet, myData.cThemeButton1);
		if (myData.osd)
		{
			myData.TextureOsdPlay = cd_opengl_load_texture (myApplet, myData.cOsdPlay);
			myData.TextureOsdPause = cd_opengl_load_texture (myApplet, myData.cOsdPause);
		}		
		
		if (myData.numberButtons > 3)
		{
			myData.TextureButton4 = cd_opengl_load_texture (myApplet, myData.cThemeButton4);
			if (myData.osd)
				myData.TextureOsdHome = cd_opengl_load_texture (myApplet, myData.cOsdHome);
		}
		if (myData.numberButtons > 2)
		{
			myData.TextureButton3 = cd_opengl_load_texture (myApplet, myData.cThemeButton3);
			if (myData.osd)
				myData.TextureOsdNext = cd_opengl_load_texture (myApplet, myData.cOsdNext);
		}
		if (myData.numberButtons > 1)
		{
			myData.TextureButton2 = cd_opengl_load_texture (myApplet, myData.cThemeButton2);
			if (myData.osd)
				myData.TextureOsdPrev = cd_opengl_load_texture (myApplet, myData.cOsdPrev);
		}
	}
	

	//\_______________ On definie la calllist qui déforme la pochette.
	draw_cover = glGenLists(1);
	glNewList(draw_cover , GL_COMPILE);
	glBegin(GL_QUADS);
	glColor4ub(255,255,255,255); 
	glTexCoord2d(0,0);
	glVertex3d((myData.itopleftX-500.)/1000., (500.-myData.itopleftY)/1000., 0);
	glTexCoord2d(0,1);
	glVertex3d((myData.ibottomleftX-500.)/1000., (500.-myData.ibottomleftY)/1000., 0);
	glTexCoord2d(1,1);
	glVertex3d((myData.ibottomrightX-500.)/1000., (500.-myData.ibottomrightY)/1000., 0);
	glTexCoord2d(1,0);
	glVertex3d((myData.itoprightX-500.)/1000., (500.-myData.itoprightY)/1000., 0);
	glEnd();
	glEndList();
}

void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
	glColor4ub(255,255,255,255);	
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
	
	glDepthMask(GL_FALSE);		/* On passe le depth en lecture seul -> Tout s'affichera dans l'ordre
								de création sans tenir compte de la profondeur. */
			
	cairo_dock_apply_texture (myData.TextureFrame);
		
	glBindTexture(GL_TEXTURE_2D, myData.TextureCover);
	glCallList(draw_cover);
	
		
	// Affichage de l'osd PAUSE
	if (myConfig.bOverrideOsd && !myData.playing && myData.numberButtons != 0)
	{
		if(myData.opening)
		{
			glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPause);
			glCallList(draw_cover);
		}
	}
	else if (myData.osd && myData.cover_exist && myData.NoOSD)
	{
		if (myData.opening)
		{
			if (myData.playing)
			{
				// On n'affiche rien si on est en lecture
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPause);
				glCallList(draw_cover);
			}
		}
	}
				
	// On affiche les images des boutons survolés et de l'OSD ;)
	if (myData.numberButtons != 0)
	{
		if (myData.mouseOnButton1)
		{
			cairo_dock_apply_texture (myData.TextureButton1);
			if (myData.osd && !myConfig.bOverrideOsd)
			{
				if (myData.opening)
				{
					if (myData.playing)
					{
						glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPause);
						glCallList(draw_cover);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPlay);
						glCallList(draw_cover);
					}
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPlay);
					glCallList(draw_cover);
				}
			}
					
		}
		
		if (myData.numberButtons > 3)
		{
			if (myData.mouseOnButton4)
			{
				cairo_dock_apply_texture (myData.TextureButton4);
				if (myData.osd && !myConfig.bOverrideOsd)
				{
					glBindTexture(GL_TEXTURE_2D, myData.TextureOsdHome);
					glCallList(draw_cover);
				}
			}
		}
		if (myData.numberButtons > 2)
		{
			if (myData.mouseOnButton3)
			{
				cairo_dock_apply_texture (myData.TextureButton3);
				if (myData.osd && !myConfig.bOverrideOsd)
				{
					glBindTexture(GL_TEXTURE_2D, myData.TextureOsdNext);
					glCallList(draw_cover);
				}
			}
		}
		if (myData.numberButtons > 1)
		{
			if (myData.mouseOnButton2)
			{
				cairo_dock_apply_texture (myData.TextureButton2);
				if (myData.osd && !myConfig.bOverrideOsd)
				{
					glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPrev);
					glCallList(draw_cover);
				}
			}
		}
	}
	
	
	cairo_dock_apply_texture (myData.TextureReflect);
		
	
	// Debug mode pour l'aide à la création des thèmes:
	if(myData.b3dThemesDebugMode)
	{
		glDisable(GL_TEXTURE_2D);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE);
		
		if (myData.numberButtons != 0)
		{
			// Affichage button 1 :
			glBegin(GL_QUADS);
			glColor4ub(255,0,0,100);
			glVertex3d( ((myData.button1coordX - (myData.button1sizeX/2))-500.)/1000. , (500.- (myData.button1coordY - (myData.button1sizeY/2)))/1000., 0);
			glVertex3d( ((myData.button1coordX - (myData.button1sizeX/2))-500.)/1000. , (500.- (myData.button1coordY + (myData.button1sizeY/2)))/1000., 0);
			glVertex3d( ((myData.button1coordX + (myData.button1sizeX/2))-500.)/1000. , (500.- (myData.button1coordY + (myData.button1sizeY/2)))/1000., 0);
			glVertex3d( ((myData.button1coordX + (myData.button1sizeX/2))-500.)/1000. , (500.- (myData.button1coordY - (myData.button1sizeY/2)))/1000., 0);
			glEnd();
			}			
			if (myData.numberButtons > 3)
			{
				// Affichage button 4 :
				glBegin(GL_QUADS);
				glColor4ub(255,0,255,100);
				glVertex3d( ((myData.button4coordX - (myData.button4sizeX/2))-500.)/1000. , (500.- (myData.button4coordY - (myData.button4sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button4coordX - (myData.button4sizeX/2))-500.)/1000. , (500.- (myData.button4coordY + (myData.button4sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button4coordX + (myData.button4sizeX/2))-500.)/1000. , (500.- (myData.button4coordY + (myData.button4sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button4coordX + (myData.button4sizeX/2))-500.)/1000. , (500.- (myData.button4coordY - (myData.button4sizeY/2)))/1000., 0);
				glEnd();
			}
			if (myData.numberButtons > 2)
			{
				// Affichage button 3 :
				glBegin(GL_QUADS);
				glColor4ub(0,0,255,100);
				glVertex3d( ((myData.button3coordX - (myData.button3sizeX/2))-500.)/1000. , (500.- (myData.button3coordY - (myData.button3sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button3coordX - (myData.button3sizeX/2))-500.)/1000. , (500.- (myData.button3coordY + (myData.button3sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button3coordX + (myData.button3sizeX/2))-500.)/1000. , (500.- (myData.button3coordY + (myData.button3sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button3coordX + (myData.button3sizeX/2))-500.)/1000. , (500.- (myData.button3coordY - (myData.button3sizeY/2)))/1000., 0);
				glEnd();
			}
			if (myData.numberButtons > 1)
			{
				// Affichage button 2 :
				glBegin(GL_QUADS);
				glColor4ub(0,255,0,100);
				glVertex3d( ((myData.button2coordX - (myData.button2sizeX/2))-500.)/1000. , (500.- (myData.button2coordY - (myData.button2sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button2coordX - (myData.button2sizeX/2))-500.)/1000. , (500.- (myData.button2coordY + (myData.button2sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button2coordX + (myData.button2sizeX/2))-500.)/1000. , (500.- (myData.button2coordY + (myData.button2sizeY/2)))/1000., 0);
				glVertex3d( ((myData.button2coordX + (myData.button2sizeX/2))-500.)/1000. , (500.- (myData.button2coordY - (myData.button2sizeY/2)))/1000., 0);
				glEnd();
			}
			
			// Et on affiche les coordonnees survolées ;-)
			CD_APPLET_SET_QUICK_INFO_ON_MY_ICON_PRINTF ("Coord X = %i , Coord Y = %i", myData.iMouseX, myData.iMouseY);  
						
			glEnable(GL_TEXTURE_2D);
			glColor4ub(255,255,255,255);
		}
	
	
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

void cd_opengl_mouse_on_buttons (void)
{	
	//\_________________ On chope les coordonnees du curseur par rapport a notre container et on les limite à la taille
	// du desklet.
	int iMouseXglobal, iMouseYglobal;
	gdk_window_get_pointer (myContainer->pWidget->window, &iMouseXglobal, &iMouseYglobal, NULL);
	
	myData.iMyDeskletWidth = myDesklet->iWidth;
	myData.iMyDeskletHeight = myDesklet->iHeight;
			
	
	if (iMouseXglobal < 0 || iMouseYglobal < 0 || iMouseXglobal > myData.iMyDeskletWidth || iMouseYglobal > myData.iMyDeskletHeight)
	{
		iMouseXglobal = 0;
		iMouseYglobal = 0;
	}
	
	//\_________________ On recale l'origine au centre et se met à la même echelle que les thèmes : 0,0 à 1000,1000.
	
	if (myData.iMyDeskletWidth != 0 && myData.iMyDeskletHeight != 0)
	{
		myData.iMouseX =  (iMouseXglobal *1000 / myData.iMyDeskletWidth);
		myData.iMouseY =  (iMouseYglobal *1000 / myData.iMyDeskletHeight);
	}
	else
	{
		myData.iMouseX =  0; // Pour éviter la division par zéro lors de la première boucle
		myData.iMouseY =  0; 
	}
	
	
	//\_________________ On teste le survole :
	if (myData.numberButtons != 0)
	{
		// Test du survol button 1 :
		if ( myData.iMouseX > (myData.button1coordX - (myData.button1sizeX/2))  &&  myData.iMouseX < (myData.button1coordX + (myData.button1sizeX/2)))
		{
			if ( myData.iMouseY > (myData.button1coordY - (myData.button1sizeY/2))  &&  myData.iMouseY < (myData.button1coordY + (myData.button1sizeY/2)))
			{
				myData.mouseOnButton1 = TRUE;
			}
			else
				myData.mouseOnButton1 = FALSE;
		}
		else
			myData.mouseOnButton1 = FALSE;
				
		
		if (myData.numberButtons > 3)
		{
			// Test du survol button 4 :
			if ( myData.iMouseX > (myData.button4coordX - (myData.button4sizeX/2))  &&  myData.iMouseX < (myData.button4coordX + (myData.button4sizeX/2)))
			{
				if ( myData.iMouseY > (myData.button4coordY - (myData.button4sizeY/2))  &&  myData.iMouseY < (myData.button4coordY + (myData.button4sizeY/2)))
				{
					myData.mouseOnButton4 = TRUE;
				}
				else
					myData.mouseOnButton4 = FALSE;
			}
			else
				myData.mouseOnButton4 = FALSE;
		}
		if (myData.numberButtons > 2)
		{
			// Test du survol button 3 :
			if ( myData.iMouseX > (myData.button3coordX - (myData.button3sizeX/2))  &&  myData.iMouseX < (myData.button3coordX + (myData.button3sizeX/2)))
			{
				if ( myData.iMouseY > (myData.button3coordY - (myData.button3sizeY/2))  &&  myData.iMouseY < (myData.button3coordY + (myData.button3sizeY/2)))
				{
					myData.mouseOnButton3 = TRUE;
				}
				else
					myData.mouseOnButton3 = FALSE;
			}
			else
				myData.mouseOnButton3 = FALSE;
		}
		if (myData.numberButtons > 1)
		{
			// Test du survol button 2 :
			if ( myData.iMouseX > (myData.button2coordX - (myData.button2sizeX/2))  &&  myData.iMouseX < (myData.button2coordX + (myData.button2sizeX/2)))
			{
				if ( myData.iMouseY > (myData.button2coordY - (myData.button2sizeY/2))  &&  myData.iMouseY < (myData.button2coordY + (myData.button2sizeY/2)))
				{
					myData.mouseOnButton2 = TRUE;
				}
				else
					myData.mouseOnButton2 = FALSE;
			}
			else
				myData.mouseOnButton2 = FALSE;
		}
		
		if ( myData.mouseOnButton1 || myData.mouseOnButton2 || myData.mouseOnButton3 || myData.mouseOnButton4)
			myData.NoOSD = FALSE;
		else
			myData.NoOSD = TRUE;
	}
}
