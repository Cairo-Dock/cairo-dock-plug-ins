#include <stdlib.h>
#include <math.h>

#include "rhythmbox-struct.h"
#include "rhythmbox-config.h"
#include "rhythmbox-draw.h"
#include "3dcover-draw.h"

#define _check_error(erreur) \
	if (erreur != NULL) { \
		cd_warning (erreur->message);\
		g_error_free (erreur);\
		erreur = NULL; }
#define _make_texture(texture, cGroupName, cKeyName)\
	cImageName = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);\
	_check_error(erreur);\
	if (cImageName != NULL) {\
		g_string_printf (sImagePath, "%s/%s", cThemePath, cImageName);\
		texture = cairo_dock_create_texture_from_image (sImagePath->str);\
		g_free (cImageName); }

gboolean cd_opengl_load_3D_theme (CairoDockModuleInstance *myApplet, gchar *cThemePath)
{
	gchar *cImageName;
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", cThemePath, "theme.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_free (cConfFilePath);
	if (pKeyFile == NULL)
		return FALSE;
	
	GError *erreur = NULL;
	GString *sImagePath = g_string_new ("");
	
	// on chope les coordonnees des 4 coins dans l'espace.
	myData.itopleftX = g_key_file_get_integer (pKeyFile, "Configuration", "topleftX", &erreur);
	_check_error(erreur);
	myData.itopleftY = g_key_file_get_integer (pKeyFile, "Configuration", "topleftY", &erreur);
	_check_error(erreur);
	
	myData.ibottomleftX = g_key_file_get_integer (pKeyFile, "Configuration", "bottomleftX", &erreur);
	_check_error(erreur);
	myData.ibottomleftY = g_key_file_get_integer (pKeyFile, "Configuration", "bottomleftY", &erreur);
	_check_error(erreur);
	
	myData.ibottomrightX = g_key_file_get_integer (pKeyFile, "Configuration", "bottomrightX", &erreur);
	_check_error(erreur);
	myData.ibottomrightY = g_key_file_get_integer (pKeyFile, "Configuration", "bottomrightY", &erreur);
	_check_error(erreur);
	
	myData.itoprightX = g_key_file_get_integer (pKeyFile, "Configuration", "toprightX", &erreur);
	_check_error(erreur);
	myData.itoprightY = g_key_file_get_integer (pKeyFile, "Configuration", "toprightY", &erreur);
	_check_error(erreur);				
	
	//\_______________ On definit la calllist qui déforme la pochette.
	myData.draw_cover = glGenLists(1);
	glNewList(myData.draw_cover , GL_COMPILE);
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
	
	//\_______________ les images d'avant et arriere plan.
	_make_texture (myData.TextureFrame, "Pictures", "frame");
	
	_make_texture (myData.TextureReflect, "Pictures", "reflect");
	
	//\_______________ les zones cliquables et l'OSD.
	myData.numberButtons = g_key_file_get_integer (pKeyFile, "Buttons", "number", &erreur);
	_check_error(erreur);
	myData.osd = g_key_file_get_boolean (pKeyFile, "Buttons", "osd", &erreur);
	_check_error(erreur);
	myData.b3dThemesDebugMode = g_key_file_get_boolean (pKeyFile, "Buttons", "debug", &erreur);
	_check_error(erreur);
				
	if (myData.numberButtons != 0)
	{
		// Bouton 1
		_make_texture (myData.TextureButton1, "Button1", "picture");
		
		myData.button1coordX = g_key_file_get_integer (pKeyFile, "Button1", "X", &erreur);
		_check_error(erreur);
		myData.button1coordY = g_key_file_get_integer (pKeyFile, "Button1", "Y", &erreur);
		_check_error(erreur);
		myData.button1sizeX = g_key_file_get_integer (pKeyFile, "Button1", "sizeX", &erreur);
		_check_error(erreur);
		myData.button1sizeY = g_key_file_get_integer (pKeyFile, "Button1", "sizeY", &erreur);
		_check_error(erreur);
		if (myData.osd)
		{
			_make_texture (myData.TextureOsdPlay, "Button1", "osd_play");
			_make_texture (myData.TextureOsdPause, "Button1", "osd_pause");
		}
		
		// Bouton 4
		if (myData.numberButtons > 3)
		{
			_make_texture (myData.TextureButton4, "Button4", "picture");
			
			myData.button4coordX = g_key_file_get_integer (pKeyFile, "Button4", "X", &erreur);
			_check_error(erreur);
			myData.button4coordY = g_key_file_get_integer (pKeyFile, "Button4", "Y", &erreur);
			_check_error(erreur);
			myData.button4sizeX = g_key_file_get_integer (pKeyFile, "Button4", "sizeX", &erreur);
			_check_error(erreur);
			myData.button4sizeY = g_key_file_get_integer (pKeyFile, "Button4", "sizeY", &erreur);
			_check_error(erreur);
			
			if (myData.osd)
			{
				_make_texture (myData.TextureOsdHome, "Button4", "osd");
			}
		}
		
		// Bouton 3
		if (myData.numberButtons > 2)
		{
			_make_texture (myData.TextureButton3, "Button3", "picture");
			
			myData.button3coordX = g_key_file_get_integer (pKeyFile, "Button3", "X", &erreur);
			_check_error(erreur);
			myData.button3coordY = g_key_file_get_integer (pKeyFile, "Button3", "Y", &erreur);
			_check_error(erreur);
			myData.button3sizeX = g_key_file_get_integer (pKeyFile, "Button3", "sizeX", &erreur);
			_check_error(erreur);
			myData.button3sizeY = g_key_file_get_integer (pKeyFile, "Button3", "sizeY", &erreur);
			_check_error(erreur);
			
			if (myData.osd)
			{
				_make_texture (myData.TextureOsdNext, "Button3", "osd");
			}
		}
		
		// Bouton 2
		if (myData.numberButtons > 1)
		{
			_make_texture (myData.TextureButton2, "Button2", "picture");
			
			myData.button2coordX = g_key_file_get_integer (pKeyFile, "Button2", "X", &erreur);
			_check_error(erreur);
			myData.button2coordY = g_key_file_get_integer (pKeyFile, "Button2", "Y", &erreur);
			_check_error(erreur);
			myData.button2sizeX = g_key_file_get_integer (pKeyFile, "Button2", "sizeX", &erreur);
			_check_error(erreur);
			myData.button2sizeY = g_key_file_get_integer (pKeyFile, "Button2", "sizeY", &erreur);
			_check_error(erreur);
			
			if (myData.osd)
			{
				_make_texture (myData.TextureOsdPrev, "Button2", "osd");
			}
		}
	}
	g_key_file_free (pKeyFile);
	return TRUE;
}

void cd_opengl_reset_opengl_datas (CairoDockModuleInstance *myApplet)
{
	if (myData.draw_cover != 0)
	{
		glDeleteLists (myData.draw_cover, 1);
		myData.draw_cover = 0;
	}
	if(myData.TextureFrame != 0)
	{
		_cairo_dock_delete_texture (myData.TextureFrame);
		myData.TextureFrame = 0;
	}
	if(myData.TextureCover != 0)
	{	
		_cairo_dock_delete_texture (myData.TextureCover);
		myData.TextureCover = 0;
	}
	if(myData.TextureReflect != 0)
	{
		_cairo_dock_delete_texture (myData.TextureReflect);
		myData.TextureReflect = 0;
	}
	if(myData.TextureName != 0)
	{
		_cairo_dock_delete_texture (myData.TextureName);
		myData.TextureName = 0;
	}
	if(myData.TextureButton1 != 0)
	{
		_cairo_dock_delete_texture (myData.TextureButton1);
		myData.TextureButton1 = 0;
	}
	if(myData.TextureButton2 != 0)
	{
		_cairo_dock_delete_texture (myData.TextureButton2);
		myData.TextureButton2 = 0;
	}
	if(myData.TextureButton3 != 0)
	{
		_cairo_dock_delete_texture (myData.TextureButton3);
		myData.TextureButton3 = 0;
	}
	if(myData.TextureButton4 != 0)
	{
		_cairo_dock_delete_texture (myData.TextureButton4);
		myData.TextureButton4 = 0;
	}
	if(myData.TextureOsdPlay != 0)
	{
		_cairo_dock_delete_texture (myData.TextureOsdPlay);
		myData.TextureOsdPlay = 0;
	}
	if(myData.TextureOsdPause != 0)
	{
		_cairo_dock_delete_texture (myData.TextureOsdPause);
		myData.TextureOsdPause = 0;
	}
	if(myData.TextureOsdPrev != 0)
	{
		_cairo_dock_delete_texture (myData.TextureOsdPrev);
		myData.TextureOsdPrev = 0;
	}
	if(myData.TextureOsdNext != 0)
	{
		_cairo_dock_delete_texture (myData.TextureOsdNext);
		myData.TextureOsdNext = 0;
	}
	if(myData.TextureOsdHome != 0)
	{
		_cairo_dock_delete_texture (myData.TextureOsdHome);
		myData.TextureOsdHome = 0;
	}
}


void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	//g_print ("%s (%dx%d)\n", __func__, iWidth, iHeight);
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
	
	glDepthMask(GL_FALSE);  // On passe le depth en lecture seul -> Tout s'affichera dans l'ordre de création sans tenir compte de la profondeur.
			
	cairo_dock_apply_texture (myData.TextureFrame);
		
	glBindTexture(GL_TEXTURE_2D, myData.TextureCover);
	glCallList(myData.draw_cover);
	
		
	// Affichage de l'osd PAUSE
	if (myConfig.bOverrideOsd && !myData.playing && myData.numberButtons != 0)
	{
		if(myData.bIsRunning)
		{
			glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPause);
			glCallList(myData.draw_cover);
		}
	}
	else if (myData.osd && myData.cover_exist && myData.NoOSD)
	{
		if (myData.bIsRunning)
		{
			if (myData.playing)
			{
				// On n'affiche rien si on est en lecture
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPause);
				glCallList(myData.draw_cover);
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
				if (myData.bIsRunning)
				{
					if (myData.playing)
					{
						glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPause);
						glCallList(myData.draw_cover);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPlay);
						glCallList(myData.draw_cover);
					}
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPlay);
					glCallList(myData.draw_cover);
				}
			}
					
		}
		
		if (myData.numberButtons > 3 && myData.mouseOnButton4)
		{
			cairo_dock_apply_texture (myData.TextureButton4);
			if (myData.osd && !myConfig.bOverrideOsd)
			{
				glBindTexture(GL_TEXTURE_2D, myData.TextureOsdHome);
				glCallList(myData.draw_cover);
			}
		}
		if (myData.numberButtons > 2 && myData.mouseOnButton3)
		{
			cairo_dock_apply_texture (myData.TextureButton3);
			if (myData.osd && !myConfig.bOverrideOsd)
			{
				glBindTexture(GL_TEXTURE_2D, myData.TextureOsdNext);
				glCallList(myData.draw_cover);
			}
		}
		if (myData.numberButtons > 1 && myData.mouseOnButton2)
		{
			cairo_dock_apply_texture (myData.TextureButton2);
			if (myData.osd && !myConfig.bOverrideOsd)
			{
				glBindTexture(GL_TEXTURE_2D, myData.TextureOsdPrev);
				glCallList(myData.draw_cover);
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


void cd_opengl_render_to_texture (CairoDockModuleInstance *myApplet)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
	
	cd_opengl_scene (myApplet, iWidth, iHeight);
	
	CD_APPLET_FINISH_DRAWING_MY_ICON;
}


gboolean cd_opengl_mouse_is_over_buttons (CairoDockModuleInstance *myApplet)
{	
	if (myDesklet->iWidth == 0 || myDesklet->iHeight == 0 || myData.numberButtons == 0)  // precaution.
		return FALSE;
	
	//\_________________ On convertit les coordonnees du pointeur dans le repere du theme : 0,0 à 1000,1000.
	myData.iMouseX = myDesklet->iMouseX;
	myData.iMouseY = myDesklet->iMouseY;
	
	if (myData.iMouseX < 0 || myData.iMouseY < 0 || myData.iMouseX > myDesklet->iWidth || myData.iMouseY > myDesklet->iHeight)
	{
		myData.iMouseX = 0;
		myData.iMouseY = 0;
	}
	else
	{
		myData.iMouseX = (myData.iMouseX * 1000 / myDesklet->iWidth);
		myData.iMouseY = (myData.iMouseY * 1000 / myDesklet->iHeight);
	}
	
	//\_________________ On teste le survole des differents boutons :
	// Test du survol button 1 :
	myData.mouseOnButton1 =	(
		myData.iMouseX > (myData.button1coordX - (myData.button1sizeX/2)) &&
		myData.iMouseX < (myData.button1coordX + (myData.button1sizeX/2)) && 
		myData.iMouseY > (myData.button1coordY - (myData.button1sizeY/2)) &&
		myData.iMouseY < (myData.button1coordY + (myData.button1sizeY/2))
	);
	
	if (myData.numberButtons > 3)
	{
		// Test du survol button 4 :
		myData.mouseOnButton4 =	(
			myData.iMouseX > (myData.button4coordX - (myData.button4sizeX/2)) &&
			myData.iMouseX < (myData.button4coordX + (myData.button4sizeX/2)) && 
			myData.iMouseY > (myData.button4coordY - (myData.button4sizeY/2)) &&
			myData.iMouseY < (myData.button4coordY + (myData.button4sizeY/2))
		);
	}
	if (myData.numberButtons > 2)
	{
		// Test du survol button 3 :
		myData.mouseOnButton3 =	(
			myData.iMouseX > (myData.button3coordX - (myData.button3sizeX/2)) &&
			myData.iMouseX < (myData.button3coordX + (myData.button3sizeX/2)) && 
			myData.iMouseY > (myData.button3coordY - (myData.button3sizeY/2)) &&
			myData.iMouseY < (myData.button3coordY + (myData.button3sizeY/2))
		);
	}
	if (myData.numberButtons > 1)
	{
		// Test du survol button 2 :
		myData.mouseOnButton2 =	(
			myData.iMouseX > (myData.button2coordX - (myData.button2sizeX/2)) &&
			myData.iMouseX < (myData.button2coordX + (myData.button2sizeX/2)) && 
			myData.iMouseY > (myData.button2coordY - (myData.button2sizeY/2)) &&
			myData.iMouseY < (myData.button2coordY + (myData.button2sizeY/2))
		);
	}
	
	if ( myData.mouseOnButton1 || myData.mouseOnButton2 || myData.mouseOnButton3 || myData.mouseOnButton4)
	{
		myData.NoOSD = FALSE;
		return TRUE;
	}
	else
	{
		myData.NoOSD = TRUE;
		return FALSE;
	}
}
