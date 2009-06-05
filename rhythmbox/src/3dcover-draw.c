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

#define _make_texture_at_size(texture, cGroupName, cKeyName, w, h)\
	cImageName = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);\
	_check_error(erreur);\
	if (cImageName != NULL) {\
		g_string_printf (sImagePath, "%s/%s", cThemePath, cImageName);\
		pSurface = cairo_dock_create_surface_for_icon (sImagePath->str, myDrawContext, w, h);\
		texture = cairo_dock_create_texture_from_surface (pSurface);\
		cairo_surface_destroy (pSurface);\
		g_free (cImageName); }

gboolean cd_opengl_load_3D_theme (CairoDockModuleInstance *myApplet, gchar *cThemePath)
{
	gchar *cImageName;
	cairo_surface_t *pSurface;
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", cThemePath, "theme.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_free (cConfFilePath);
	if (pKeyFile == NULL)
		return FALSE;
	
	GError *erreur = NULL;
	GString *sImagePath = g_string_new ("");	
	
	//\_______________ les images d'avant et arriere plan.
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	double fImageWidth, fImageHeight;  // dimensions de la surface generee.
	double fZoomX=1, fZoomY=1;  // facteur de zoom qui lui a ete applique.
	
	cImageName = g_key_file_get_string (pKeyFile, "Pictures", "frame", &erreur);
	_check_error(erreur);
	if (cImageName != NULL)
	{
		g_string_printf (sImagePath, "%s/%s", cThemePath, cImageName);
		pSurface = cairo_dock_create_surface_from_image (sImagePath->str,
			myDrawContext,
			1.,
			iWidth, iHeight,
			CAIRO_DOCK_FILL_SPACE,
			&fImageWidth, &fImageHeight,
			&fZoomX, &fZoomY);
		
		myData.TextureFrame = cairo_dock_create_texture_from_surface (pSurface);
		cairo_surface_destroy (pSurface);
		g_free (cImageName);
	}
	
	cImageName = g_key_file_get_string (pKeyFile, "Pictures", "reflect", &erreur);
	_check_error(erreur);
	if (cImageName != NULL)
	{
		g_string_printf (sImagePath, "%s/%s", cThemePath, cImageName);
		pSurface = cairo_dock_create_surface_from_image (sImagePath->str,
			myDrawContext,
			1.,
			iWidth, iHeight,
			CAIRO_DOCK_FILL_SPACE,
			&fImageWidth, &fImageHeight,
			NULL, NULL);
		
		myData.TextureReflect = cairo_dock_create_texture_from_surface (pSurface);
		cairo_surface_destroy (pSurface);
		g_free (cImageName);
	}
	
	//\_______________ les coordonnees des 4 coins de la pochette
	// dans le referentiel du cadre (directement obtenues avec Gimp) => dans le referentiel de l'icone iWidth x iHeight.
	myData.itopleftX = g_key_file_get_integer (pKeyFile, "Configuration", "topleftX", &erreur) * fZoomX;
	_check_error(erreur);
	myData.itopleftY = g_key_file_get_integer (pKeyFile, "Configuration", "topleftY", &erreur) * fZoomY;
	_check_error(erreur);
	
	myData.ibottomleftX = g_key_file_get_integer (pKeyFile, "Configuration", "bottomleftX", &erreur) * fZoomX;
	_check_error(erreur);
	myData.ibottomleftY = g_key_file_get_integer (pKeyFile, "Configuration", "bottomleftY", &erreur) * fZoomY;
	_check_error(erreur);
	
	myData.ibottomrightX = g_key_file_get_integer (pKeyFile, "Configuration", "bottomrightX", &erreur) * fZoomX;
	_check_error(erreur);
	myData.ibottomrightY = g_key_file_get_integer (pKeyFile, "Configuration", "bottomrightY", &erreur) * fZoomY;
	_check_error(erreur);
	
	myData.itoprightX = g_key_file_get_integer (pKeyFile, "Configuration", "toprightX", &erreur) * fZoomX;
	_check_error(erreur);
	myData.itoprightY = g_key_file_get_integer (pKeyFile, "Configuration", "toprightY", &erreur) * fZoomY;
	_check_error(erreur);			
	
	//\_______________ On definit la calllist qui déforme la pochette.
	myData.draw_cover = glGenLists(1);
	glNewList(myData.draw_cover , GL_COMPILE);
	glBegin(GL_QUADS);
	glTexCoord2d (0,0);
	glVertex3f (-.5 + myData.itopleftX / iWidth,		+.5 - myData.itopleftY / iHeight,		0);
	glTexCoord2d (0,1);
	glVertex3f (-.5 + myData.ibottomleftX / iWidth,		+.5 - myData.ibottomleftY /iHeight,		0.);
	glTexCoord2d (1,1);
	glVertex3f (-.5 + myData.ibottomrightX / iWidth,	+.5 - myData.ibottomrightY / iHeight,	0.);
	glTexCoord2d (1,0);
	glVertex3f (-.5 + myData.itoprightX / iWidth,		+.5 - myData.itoprightY / iHeight,		0.);
	glEnd();
	glEndList();
	
	//\_______________ les zones cliquables et l'OSD.
	myData.numberButtons = g_key_file_get_integer (pKeyFile, "Buttons", "number", NULL);
	
	myData.mouseOnButton1 = myData.mouseOnButton2 = myData.mouseOnButton3 = myData.mouseOnButton4 = 0;
	if (myData.numberButtons != 0)
	{
		myData.osd = g_key_file_get_boolean (pKeyFile, "Buttons", "osd", &erreur);
		_check_error(erreur);
		myData.b3dThemesDebugMode = g_key_file_get_boolean (pKeyFile, "Buttons", "debug", &erreur);
		_check_error(erreur);
		
		// Bouton 1
		myData.button1coordX = g_key_file_get_integer (pKeyFile, "Button1", "X", &erreur) * fZoomX;
		_check_error(erreur);
		myData.button1coordY = g_key_file_get_integer (pKeyFile, "Button1", "Y", &erreur) * fZoomY;
		_check_error(erreur);
		myData.button1sizeX = g_key_file_get_integer (pKeyFile, "Button1", "sizeX", &erreur) * fZoomX;
		_check_error(erreur);
		myData.button1sizeY = g_key_file_get_integer (pKeyFile, "Button1", "sizeY", &erreur) * fZoomY;
		_check_error(erreur);
		
		_make_texture_at_size (myData.TextureButton1, "Button1", "picture", myData.button1sizeX, myData.button1sizeY);
		
		if (myData.osd)
		{
			//_make_texture (myData.TextureOsdPlay, "Button1", "osd_play");
			//_make_texture (myData.TextureOsdPause, "Button1", "osd_pause");
		}
		
		// Bouton 4
		if (myData.numberButtons > 3)
		{
			myData.button4coordX = g_key_file_get_integer (pKeyFile, "Button4", "X", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button4coordY = g_key_file_get_integer (pKeyFile, "Button4", "Y", &erreur) * fZoomY;
			_check_error(erreur);
			myData.button4sizeX = g_key_file_get_integer (pKeyFile, "Button4", "sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button4sizeY = g_key_file_get_integer (pKeyFile, "Button4", "sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			
			_make_texture_at_size (myData.TextureButton4, "Button4", "picture", myData.button4sizeX, myData.button4sizeY);
			
			if (myData.osd)
			{
				//_make_texture (myData.TextureOsdHome, "Button4", "osd");
			}
		}
		
		// Bouton 3
		if (myData.numberButtons > 2)
		{
			myData.button3coordX = g_key_file_get_integer (pKeyFile, "Button3", "X", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button3coordY = g_key_file_get_integer (pKeyFile, "Button3", "Y", &erreur) * fZoomY;
			_check_error(erreur);
			myData.button3sizeX = g_key_file_get_integer (pKeyFile, "Button3", "sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button3sizeY = g_key_file_get_integer (pKeyFile, "Button3", "sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			
			_make_texture_at_size (myData.TextureButton3, "Button3", "picture", myData.button3sizeX, myData.button3sizeY);
			
			if (myData.osd)
			{
				//_make_texture (myData.TextureOsdNext, "Button3", "osd");
			}
		}
		
		// Bouton 2
		if (myData.numberButtons > 1)
		{
			myData.button2coordX = g_key_file_get_integer (pKeyFile, "Button2", "X", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button2coordY = g_key_file_get_integer (pKeyFile, "Button2", "Y", &erreur) * fZoomY;
			_check_error(erreur);
			myData.button2sizeX = g_key_file_get_integer (pKeyFile, "Button2", "sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button2sizeY = g_key_file_get_integer (pKeyFile, "Button2", "sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			
			_make_texture_at_size (myData.TextureButton2, "Button2", "picture", myData.button2sizeX, myData.button2sizeY);
			
			if (myData.osd)
			{
				//_make_texture (myData.TextureOsdPrev, "Button2", "osd");
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


#define _transform_coords(X, Y) \
	dx1 = 1. - fabs (X - X1) / W;\
	dy1 = 1. - fabs (Y - Y1) / H;\
	dx2 = 1. - fabs (X - X2) / W;\
	dy2 = 1. - fabs (Y - Y2) / H;\
	dx3 = 1. - fabs (X - X3) / W;\
	dy3 = 1. - fabs (Y - Y3) / H;\
	dx4 = 1. - fabs (X - X4) / W;\
	dy4 = 1. - fabs (Y - Y4) / H;\
	x = X + dx1 * dy1 * t1x + dx2 * dy2 * t2x + dx3 * dy3 * t3x + dx4 * dy4 * t4x;\
	y = Y + dx1 * dy1 * t1y + dx2 * dy2 * t2y + dx3 * dy3 * t3y + dx4 * dy4 * t4y;

void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	g_print ("%s ()\n", __func__);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_pbuffer ();
	
	// on dessine le cadre.
	_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureFrame, iWidth, iHeight, 1.);
	
	// on dessine la couverture.
	if (myData.TextureCover != 0)
	{
		glPushMatrix ();
		glScalef (iWidth, iHeight, 1.);
		glBindTexture(GL_TEXTURE_2D, myData.TextureCover);
		glCallList(myData.draw_cover);
		glPopMatrix ();
	}
	
	
	// on dessine les boutons qui sont allumes.
	if (myData.mouseOnButton1)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button1coordX, +iHeight/2 - myData.button1coordY, 0.);
		_cairo_dock_apply_texture_at_size (myData.TextureButton1, myData.button1sizeX, myData.button1sizeY);
		glPopMatrix ();
	}
	if (myData.mouseOnButton2)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button2coordX, +iHeight/2 - myData.button2coordY, 0.);
		_cairo_dock_apply_texture_at_size (myData.TextureButton2, myData.button2sizeX, myData.button2sizeY);
		glPopMatrix ();
	}
	if (myData.mouseOnButton3)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button3coordX, +iHeight/2 - myData.button3coordY, 0.);
		_cairo_dock_apply_texture_at_size (myData.TextureButton3, myData.button3sizeX, myData.button3sizeY);
		glPopMatrix ();
	}
	if (myData.mouseOnButton4)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button4coordX, +iHeight/2 - myData.button4coordY, 0.);
		_cairo_dock_apply_texture_at_size (myData.TextureButton4, myData.button4sizeX, myData.button4sizeY);
		glPopMatrix ();
	}
	
	// on determine la transformation pour les OSD.
	// en majuscule : coordonees initiales.
	double W = iWidth, H = iHeight;
	double X1 = 0, Y1 = 0;
	double X2 = W, Y2 = 0;
	double X3 = W, Y3 = H;
	double X4 = 0, Y4 = H;
	// translation du point P1(0;0)
	double x1 = myData.ibottomleftX, y1 = myData.ibottomleftY;
	double t1x = x1 - X1;
	double t1y = y1 - X1;
	// translation du point P2(500;0)
	double x2 = myData.ibottomrightX, y2 = myData.ibottomrightY;
	double t2x = x2 - X2;
	double t2y = y2 - Y2;
	// translation du point P2(500;500)
	double x3 = myData.itoprightX, y3 = myData.itoprightY;
	double t3x = x3 - X3;
	double t3y = y3 - Y3;
	// translation du point P2(0;500)
	double x4 = myData.itopleftX, y4 = myData.itopleftY;
	double t4x = x4 - X4;
	double t4y = y4 - Y4;
	
	double dx1, dy1, dx2, dy2, dx3, dy3, dx4, dy4;  // ponderations.
	double u1, v1, u2, v2, u3, v3, u4, v4;  // coordonnees finales des coins de l'OSD.
	
	// on dessine les OSD.
	
	
	// on dessine les reflets.
	if (myData.TextureReflect != 0)
		_cairo_dock_apply_texture_at_size (myData.TextureReflect, iWidth, iHeight);
	
	_cairo_dock_disable_texture ();
}

/*void cd_opengl_scene (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
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
}*/


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
	
	//\_________________ On convertit les coordonnees du pointeur dans le referentiel de l'icone.
	myData.iMouseX = myDesklet->iMouseX - myDesklet->iLeftSurfaceOffset;
	myData.iMouseY = myDesklet->iMouseY - myDesklet->iTopSurfaceOffset;
	
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
		g_print ("on est sur un bouton\n");
		myData.iState = (myData.mouseOnButton1 << 0) |
			(myData.mouseOnButton2 << 1) |
			(myData.mouseOnButton3 << 2) |
			(myData.mouseOnButton4 << 3);
		myData.NoOSD = FALSE;
		return TRUE;
	}
	else
	{
		myData.iState = 0;
		myData.NoOSD = TRUE;
		return FALSE;
	}
}
