#include <stdlib.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-draw.h"
#include "3dcover-draw.h"
#include "applet-dbus.h"

#define _check_error(erreur) \
	if (erreur != NULL) { \
		cd_warning (erreur->message);\
		g_error_free (erreur);\
		erreur = NULL; }

#define _make_texture_at_size(texture, cGroupName, cKeyName, w, h)\
	cImageName = g_key_file_get_string (pKeyFile, cGroupName, cKeyName, &erreur);\
	_check_error(erreur);\
	if (cImageName != NULL) {\
		g_string_printf (sImagePath, "%s/%s", cThemePath, cImageName);\
		pSurface = cairo_dock_create_surface_for_icon (sImagePath->str, myDrawContext, w, h);\
		texture = cairo_dock_create_texture_from_surface (pSurface);\
		cairo_surface_destroy (pSurface);\
		g_free (cImageName); }

#define _draw_osd(texture, x_, y_, w_, h_)\
	X = x_ - w_/2;\
	Y = y_ + h_/2;\
	_transform_coords(X, Y, u1, v1);\
	X = x_ + w_/2;\
	Y = y_ + h_/2;\
	_transform_coords(X, Y, u2, v2);\
	X = x_ + w_/2;\
	Y = y_ - h_/2;\
	_transform_coords(X, Y, u3, v3);\
	X = x_ - w_/2;\
	Y = y_ - h_/2;\
	_transform_coords(X, Y, u4, v4);\
	glBindTexture (GL_TEXTURE_2D, texture);\
	glBegin (GL_QUADS);\
	glTexCoord2d (0,0);\
	glVertex3f (- iWidth/2 + u1, + iHeight/2 - v1, 0);\
	glTexCoord2d (1,0);\
	glVertex3f (- iWidth/2 + u2, + iHeight/2 - v2, 0);\
	glTexCoord2d (1,1);\
	glVertex3f (- iWidth/2 + u3, + iHeight/2 - v3, 0);\
	glTexCoord2d (0,1);\
	glVertex3f (- iWidth/2 + u4, + iHeight/2 - v4, 0);\
	glEnd ();

gboolean cd_opengl_load_3D_theme (CairoDockModuleInstance *myApplet, gchar *cThemePath)
{
	gchar *cImageName;
	cairo_surface_t *pSurface;
	gchar *cConfFilePath = g_strdup_printf ("%s/%s", cThemePath, "theme.conf");
	GKeyFile *pKeyFile = cairo_dock_open_key_file (cConfFilePath);
	g_free (cConfFilePath);
	if (pKeyFile == NULL)
		return FALSE;
	
	gchar *cThemePathUpToDate = NULL;
	gint iVersion = g_key_file_get_integer (pKeyFile, "Description", "Version", NULL);
	if (iVersion != 2)
	{
		/// effacer le theme et le recuperer sur le serveur...
		g_print ("theme en version inferieure => sera mis a jour...\n");
		// on ferme la config de l'actuel theme.
		g_key_file_free (pKeyFile);
		pKeyFile = NULL;
		
		// on supprime le theme.
		g_return_val_if_fail (cThemePath && *cThemePath == '/', FALSE);
		gchar *cCommand = g_strdup_printf ("rm -rf '%s'", cThemePath);
		int r = system (cCommand);
		g_free (cCommand);
		
		// on recupere le theme distant.
		pKeyFile = cairo_dock_open_key_file (myApplet->cConfFilePath);
		if (pKeyFile != NULL)
		{
			gboolean bFlushConfFileNeeded = FALSE;
			cThemePathUpToDate = CD_CONFIG_GET_THEME_PATH ("Configuration", "theme", "themes", "cd_box_simple");
			cThemePath = cThemePathUpToDate;
			g_key_file_free (pKeyFile);
			pKeyFile = NULL;
			
			// on ouvre la config du nouveau theme.
			cConfFilePath = g_strdup_printf ("%s/%s", cThemePath, "theme.conf");
			pKeyFile = cairo_dock_open_key_file (cConfFilePath);
			g_free (cConfFilePath);
			if (pKeyFile == NULL)
				return FALSE;
		}
		g_return_val_if_fail (pKeyFile != NULL, FALSE);
	}
	
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
	if (myData.numberButtons != 0)
	{
		myData.osd = g_key_file_get_boolean (pKeyFile, "Buttons", "osd", &erreur);
		_check_error(erreur);
		
		// Bouton 1
		myData.button1sizeX = g_key_file_get_integer (pKeyFile, "Button1", "sizeX", &erreur) * fZoomX;
		_check_error(erreur);
		myData.button1sizeY = g_key_file_get_integer (pKeyFile, "Button1", "sizeY", &erreur) * fZoomY;
		_check_error(erreur);
		myData.button1coordX = g_key_file_get_integer (pKeyFile, "Button1", "X", &erreur) * fZoomX + myData.button1sizeX/2;  // on se ramene au centre.
		_check_error(erreur);
		myData.button1coordY = g_key_file_get_integer (pKeyFile, "Button1", "Y", &erreur) * fZoomY + myData.button1sizeY/2;  // on se ramene au centre.
		_check_error(erreur);
		
		_make_texture_at_size (myData.TextureButton1, "Button1", "picture", myData.button1sizeX, myData.button1sizeY);
		
		if (myData.osd)
		{
			myData.osdPlaysizeX = g_key_file_get_integer (pKeyFile, "Button1", "osd play sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.osdPlaysizeY = g_key_file_get_integer (pKeyFile, "Button1", "osd play sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			myData.osdPlaycoordX = g_key_file_get_integer (pKeyFile, "Button1", "osd play X", &erreur) * fZoomX + myData.osdPlaysizeX/2;
			_check_error(erreur);
			myData.osdPlaycoordY = g_key_file_get_integer (pKeyFile, "Button1", "osd play Y", &erreur) * fZoomY + myData.osdPlaysizeY/2;
			_check_error(erreur);
			_make_texture_at_size (myData.TextureOsdPlay, "Button1", "osd_play", myData.osdPlaysizeX, myData.osdPlaysizeY);

			myData.osdPausesizeX = g_key_file_get_integer (pKeyFile, "Button1", "osd pause sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.osdPausesizeY = g_key_file_get_integer (pKeyFile, "Button1", "osd pause sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			myData.osdPausecoordX = g_key_file_get_integer (pKeyFile, "Button1", "osd pause X", &erreur) * fZoomX + myData.osdPausesizeX/2;
			_check_error(erreur);
			myData.osdPausecoordY = g_key_file_get_integer (pKeyFile, "Button1", "osd pause Y", &erreur) * fZoomY + myData.osdPausesizeY/2;
			_check_error(erreur);
			_make_texture_at_size (myData.TextureOsdPause, "Button1", "osd_pause", myData.osdPausesizeX, myData.osdPausesizeY);			
		}
		
		// Bouton 4
		if (myData.numberButtons > 3)
		{
			_check_error(erreur);
			myData.button4sizeX = g_key_file_get_integer (pKeyFile, "Button4", "sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button4sizeY = g_key_file_get_integer (pKeyFile, "Button4", "sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			myData.button4coordX = g_key_file_get_integer (pKeyFile, "Button4", "X", &erreur) * fZoomX + myData.button4sizeX/2;
			_check_error(erreur);
			myData.button4coordY = g_key_file_get_integer (pKeyFile, "Button4", "Y", &erreur) * fZoomY + myData.button4sizeY/2;
			
			_make_texture_at_size (myData.TextureButton4, "Button4", "picture", myData.button4sizeX, myData.button4sizeY);
			
			if (myData.osd)
			{
				myData.osdHomesizeX = g_key_file_get_integer (pKeyFile, "Button4", "osd sizeX", &erreur) * fZoomX;
				_check_error(erreur);
				myData.osdHomesizeY = g_key_file_get_integer (pKeyFile, "Button4", "osd sizeY", &erreur) * fZoomY;
				_check_error(erreur);
				myData.osdHomecoordX = g_key_file_get_integer (pKeyFile, "Button4", "osd X", &erreur) * fZoomX + myData.osdHomesizeX/2;
				_check_error(erreur);
				myData.osdHomecoordY = g_key_file_get_integer (pKeyFile, "Button4", "osd Y", &erreur) * fZoomY + myData.osdHomesizeY/2;
				_check_error(erreur);
				_make_texture_at_size (myData.TextureOsdHome, "Button4", "osd", myData.osdHomesizeX, myData.osdHomesizeY);
			}
		}
		
		// Bouton 3
		if (myData.numberButtons > 2)
		{
			myData.button3sizeX = g_key_file_get_integer (pKeyFile, "Button3", "sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button3sizeY = g_key_file_get_integer (pKeyFile, "Button3", "sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			myData.button3coordX = g_key_file_get_integer (pKeyFile, "Button3", "X", &erreur) * fZoomX + myData.button3sizeX/2;
			_check_error(erreur);
			myData.button3coordY = g_key_file_get_integer (pKeyFile, "Button3", "Y", &erreur) * fZoomY + myData.button3sizeY/2;
			_check_error(erreur);
			
			_make_texture_at_size (myData.TextureButton3, "Button3", "picture", myData.button3sizeX, myData.button3sizeY);
			
			if (myData.osd)
			{
				myData.osdNextsizeX = g_key_file_get_integer (pKeyFile, "Button3", "osd sizeX", &erreur) * fZoomX;
				_check_error(erreur);
				myData.osdNextsizeY = g_key_file_get_integer (pKeyFile, "Button3", "osd sizeY", &erreur) * fZoomY;
				_check_error(erreur);
				myData.osdNextcoordX = g_key_file_get_integer (pKeyFile, "Button3", "osd X", &erreur) * fZoomX + myData.osdNextsizeX/2;
				_check_error(erreur);
				myData.osdNextcoordY = g_key_file_get_integer (pKeyFile, "Button3", "osd Y", &erreur) * fZoomY + myData.osdNextsizeY/2;
				_check_error(erreur);
				_make_texture_at_size (myData.TextureOsdNext, "Button3", "osd", myData.osdNextsizeX, myData.osdNextsizeY);
			}
		}
		
		// Bouton 2
		if (myData.numberButtons > 1)
		{
			myData.button2sizeX = g_key_file_get_integer (pKeyFile, "Button2", "sizeX", &erreur) * fZoomX;
			_check_error(erreur);
			myData.button2sizeY = g_key_file_get_integer (pKeyFile, "Button2", "sizeY", &erreur) * fZoomY;
			_check_error(erreur);
			myData.button2coordX = g_key_file_get_integer (pKeyFile, "Button2", "X", &erreur) * fZoomX + myData.button2sizeX/2;
			_check_error(erreur);
			myData.button2coordY = g_key_file_get_integer (pKeyFile, "Button2", "Y", &erreur) * fZoomY + myData.button2sizeY/2;
			_check_error(erreur);
			
			_make_texture_at_size (myData.TextureButton2, "Button2", "picture", myData.button2sizeX, myData.button2sizeY);
			
			if (myData.osd)
			{
				myData.osdPrevsizeX = g_key_file_get_integer (pKeyFile, "Button2", "osd sizeX", &erreur) * fZoomX;
				_check_error(erreur);
				myData.osdPrevsizeY = g_key_file_get_integer (pKeyFile, "Button2", "osd sizeY", &erreur) * fZoomY;
				_check_error(erreur);
				myData.osdPrevcoordX = g_key_file_get_integer (pKeyFile, "Button2", "osd X", &erreur) * fZoomX + myData.osdPrevsizeX/2;
				_check_error(erreur);
				myData.osdPrevcoordY = g_key_file_get_integer (pKeyFile, "Button2", "osd Y", &erreur) * fZoomY + myData.osdPrevsizeY/2;
				_check_error(erreur);
				_make_texture_at_size (myData.TextureOsdPrev, "Button2", "osd", myData.osdPrevsizeX, myData.osdPrevsizeY);
			}
		}
	}
	g_key_file_free (pKeyFile);
	g_free (cThemePathUpToDate);
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
	if(myData.iPrevTextureCover != 0)
	{	
		_cairo_dock_delete_texture (myData.iPrevTextureCover);
		myData.iPrevTextureCover = 0;
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
	myData.mouseOnButton1 = myData.mouseOnButton2 = myData.mouseOnButton3 = myData.mouseOnButton4 = 0;
	myData.iButton1Count = myData.iButton2Count = myData.iButton3Count = myData.iButton4Count = 0;
	myData.iButtonState = 0;
	myData.iCoverTransition = 0;
}


#define _transform_coords(X, Y, x, y) \
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
	//g_print ("%s (%d)\n", __func__, myData.iCoverTransition);
	_cairo_dock_enable_texture ();
	_cairo_dock_set_blend_source ();
	
	// on dessine le cadre.
	_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureFrame, iWidth, iHeight, 1.);
	
	// on dessine la couverture.
	glPushMatrix ();
	glScalef (iWidth, iHeight, 1.);
	if (myData.iPrevTextureCover != 0 && myData.iCoverTransition != 0)
	{
		_cairo_dock_set_blend_over ();
		_cairo_dock_set_alpha ((double)myData.iCoverTransition/NB_TRANSITION_STEP);
		glBindTexture(GL_TEXTURE_2D, myData.iPrevTextureCover);
		glCallList(myData.draw_cover);
	}
	if (myData.TextureCover != 0)
	{
		_cairo_dock_set_blend_over ();
		_cairo_dock_set_alpha (1.-(double)myData.iCoverTransition/NB_TRANSITION_STEP);
		glBindTexture(GL_TEXTURE_2D, myData.TextureCover);
		glCallList(myData.draw_cover);
	}
	glPopMatrix ();
	_cairo_dock_set_blend_over ();
	
	
	// on dessine les boutons qui sont allumes.
	if (myData.iButton1Count)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button1coordX, +iHeight/2 - myData.button1coordY, 0.);
		_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureButton1, myData.button1sizeX, myData.button1sizeY, (double)myData.iButton1Count/NB_TRANSITION_STEP);
		glPopMatrix ();
	}
	if (myData.iButton2Count)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button2coordX, +iHeight/2 - myData.button2coordY, 0.);
		_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureButton2, myData.button2sizeX, myData.button2sizeY, (double)myData.iButton2Count/NB_TRANSITION_STEP);
		glPopMatrix ();
	}
	if (myData.iButton3Count)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button3coordX, +iHeight/2 - myData.button3coordY, 0.);
		_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureButton3, myData.button3sizeX, myData.button3sizeY, (double)myData.iButton3Count/NB_TRANSITION_STEP);
		glPopMatrix ();
	}
	if (myData.iButton4Count)
	{
		glPushMatrix ();
		glTranslatef (-iWidth/2 + myData.button4coordX, +iHeight/2 - myData.button4coordY, 0.);
		_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureButton4, myData.button4sizeX, myData.button4sizeY, (double)myData.iButton4Count/NB_TRANSITION_STEP);
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
	double X, Y;  // coordonnees initiales d'un coin de l'OSD.
	double u1, v1, u2, v2, u3, v3, u4, v4;  // coordonnees finales des coins de l'OSD.
	
	// on dessine les OSD.
	//if(myData.bIsRunning && myData.iState != 0)
	{
		_cairo_dock_set_alpha (1.);
		if (myData.mouseOnButton1)
		{
			if (myData.pPlayingStatus == PLAYER_PLAYING)
			{
				_draw_osd (myData.TextureOsdPause, myData.osdPausecoordX, myData.osdPausecoordY, myData.osdPausesizeX, myData.osdPausesizeY);
				//g_print ("%.1f;%.1f ; %.1f;%.1f ;%.1f;%.1f ;%.1f;%.1f\n", u1, v1, u2, v2,u3,v3,u4,v4);
			}
			else
			{
				_draw_osd (myData.TextureOsdPlay, myData.osdPlaycoordX, myData.osdPlaycoordY, myData.osdPlaysizeX, myData.osdPlaysizeY);
				//g_print ("%.1f;%.1f ; %.1f;%.1f ;%.1f;%.1f ;%.1f;%.1f\n", u1, v1, u2, v2,u3,v3,u4,v4);
			}
		}
		else if (myData.mouseOnButton2)
		{
			_draw_osd (myData.TextureOsdPrev, myData.osdPrevcoordX, myData.osdPrevcoordY, myData.osdPrevsizeX, myData.osdPrevsizeY);
		}
		else if (myData.mouseOnButton3)
		{
			_draw_osd (myData.TextureOsdNext, myData.osdNextcoordX, myData.osdNextcoordY, myData.osdNextsizeX, myData.osdNextsizeY);
		}
		else if (myData.mouseOnButton4)
		{
			_draw_osd (myData.TextureOsdHome, myData.osdHomecoordX, myData.osdHomecoordY, myData.osdHomesizeX, myData.osdHomesizeY);
		}
		else if (! myData.pPlayingStatus == PLAYER_PLAYING)
		{
			if (myData.bIsRunning)  // on verifie que le lecteur est bien ouvert (il se peut qu'il ne nous previenne pas lorsqu'il quitte).
			{
				cd_musicplayer_dbus_detect_player ();
			}
			if (myData.bIsRunning)  // si rhythmbox n'est pas lancé, on n'affiche pas l'osd de pause ;-)
			{
				_draw_osd (myData.TextureOsdPause, myData.osdPausecoordX, myData.osdPausecoordY, myData.osdPausesizeX, myData.osdPausesizeY);
			}
		}
		else if (myData.pPlayingStatus == PLAYER_PLAYING && ! myData.cover_exist)
		{
			_draw_osd (myData.TextureOsdPlay, myData.osdPlaycoordX, myData.osdPlaycoordY, myData.osdPlaysizeX, myData.osdPlaysizeY);
		}
	}
	
	// on dessine les reflets.
	_cairo_dock_set_blend_pbuffer ();
	if (myData.TextureReflect != 0)
		_cairo_dock_apply_texture_at_size_with_alpha (myData.TextureReflect, iWidth, iHeight, 1.);
	
	_cairo_dock_disable_texture ();
}


void cd_opengl_render_to_texture (CairoDockModuleInstance *myApplet)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	
	CD_APPLET_START_DRAWING_MY_ICON_OR_RETURN ();
	
	cd_opengl_scene (myApplet, iWidth, iHeight);
	
	CD_APPLET_FINISH_DRAWING_MY_ICON;
}


int cd_opengl_check_buttons_state (CairoDockModuleInstance *myApplet)
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
	
	return (myData.mouseOnButton1 << 0) |
			(myData.mouseOnButton2 << 1) |
			(myData.mouseOnButton3 << 2) |
			(myData.mouseOnButton4 << 3);
}
