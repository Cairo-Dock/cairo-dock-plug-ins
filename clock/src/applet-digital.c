/**********************************************************************************

This file is a part of the cairo-dock clock applet, 
released under the terms of the GNU General Public License.

Written by Rémy Robertson (for any bug report, please mail me to changfu@cairo-dock.org)

**********************************************************************************/
#include <stdlib.h>
#include <math.h>

#include "applet-struct.h"
#include "applet-config.h"
#include "applet-digital.h"

CD_APPLET_INCLUDE_MY_VARS

#define CD_CLOCK_DATE_BUFFER_LENGTH 50
static char s_cDateBuffer[CD_CLOCK_DATE_BUFFER_LENGTH+1];

void cd_clock_configure_digital (CairoDockModuleInstance *myApplet) {
	cd_debug ("%s", __func__);
	
	GError *erreur = NULL;
	GKeyFile *pKeyFile = g_key_file_new (); //On ouvre le fichier de conf
	if (myConfig.cDigital == NULL)
		myConfig.cDigital = g_strdup ("default");
	
	gchar *cConfPath = g_strdup_printf ("%s/digital/%s/config", MY_APPLET_SHARE_DATA_DIR, myConfig.cDigital);
	cd_debug ("Clock: Using %s digital theme", cConfPath);
	g_key_file_load_from_file (pKeyFile, cConfPath, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &erreur);
	
	if (erreur != NULL) {
		cd_warning ("Attention : %s", erreur->message);
		g_error_free (erreur);
		erreur = NULL;
		return;
	}
	
	myData.pDigitalClock.bSecondCapable = cairo_dock_get_boolean_key_value (pKeyFile, "configuration", "seconds", FALSE, FALSE, NULL, NULL);
	myData.pDigitalClock.iFrameSpacing = cairo_dock_get_integer_key_value (pKeyFile, "configuration", "framespacing", FALSE, 2, NULL, NULL);
	
	myData.pDigitalClock.i12modeWidth = cairo_dock_get_integer_key_value (pKeyFile, "configuration", "12width", FALSE, 6, NULL, NULL);
	myData.pDigitalClock.i12modeHeight = cairo_dock_get_integer_key_value (pKeyFile, "configuration", "12height", FALSE, 6, NULL, NULL);
	myData.pDigitalClock.i12modeXOffset = cairo_dock_get_integer_key_value (pKeyFile, "configuration", "12offsetX", FALSE, 4, NULL, NULL);
	myData.pDigitalClock.i12modeYOffset = cairo_dock_get_integer_key_value (pKeyFile, "configuration", "12offsetY", FALSE, 1, NULL, NULL);
	myData.pDigitalClock.i12modeFrame = cairo_dock_get_integer_key_value (pKeyFile, "configuration", "12frame", FALSE, 4, NULL, NULL);
	
	int i = 0, j = (myData.pDigitalClock.bSecondCapable == TRUE ? 3 : 4);
	//Avec secondes 3 frames, une par unités (hh : mm : ss)
	//Sans secondes 4 frames, une par nombre (1|2:4|5)
	for (i = 0; i < j; i++) {
		gchar *cGroupName = g_strdup_printf ("frame_%d", i);
		myData.pDigitalClock.pFrame[i].iWidth = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, "width", FALSE, 4, NULL, NULL);
		myData.pDigitalClock.pFrame[i].iHeight = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, "height", FALSE, 4, NULL, NULL);
		myData.pDigitalClock.pFrame[i].iXOffset = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, "offsetX", FALSE, 0, NULL, NULL);
		myData.pDigitalClock.pFrame[i].iYOffset = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, "offsetY", FALSE, 0, NULL, NULL);
		g_free (cGroupName);
		
		cGroupName = g_strdup_printf ("text_%d", i);
		myData.pDigitalClock.pText[i].iXOffset = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, "offsetX", FALSE, -1, NULL, NULL);
		myData.pDigitalClock.pText[i].iYOffset = cairo_dock_get_integer_key_value (pKeyFile, cGroupName, "offsetY", FALSE, -1, NULL, NULL);
		g_free (cGroupName);
	}
	
	g_key_file_free (pKeyFile);
	g_free (cConfPath);
	
	cd_clock_digital_load_frames (myApplet);
}

void cd_clock_digital_load_frames (CairoDockModuleInstance *myApplet) {
	cd_debug ("%s", __func__);
	
	int i = 0, j = (myData.pDigitalClock.bSecondCapable == TRUE ? 3 : 4);
	double fFrameWidth = 1, fFrameHeight = 1;
	//Avec secondes 3 frames, une par unités (hh : mm : ss)
	//Sans secondes 4 frames, une par nombre (1|2:4|5)
	for (i = 0; i < j; i++) {
		fFrameWidth = myIcon->fWidth / (double) myData.pDigitalClock.pFrame[i].iWidth;
		cd_debug ("Clock: frame %d width %.02f (%.02f %d)", i+1, fFrameWidth, myIcon->fWidth, myData.pDigitalClock.pFrame[i].iWidth);
		fFrameWidth = fFrameWidth - myData.pDigitalClock.iFrameSpacing;
		fFrameHeight = myIcon->fHeight; /// myData.pDigitalClock.pFrame[i].iHeight;
		double fImgW=0, fImgH=0;
		
		if (myConfig.cDigital == NULL)
			myConfig.cDigital = g_strdup ("default");
			
		gchar *cImagePath = g_strdup_printf ("%s/digital/%s/frame_%d.svg", MY_APPLET_SHARE_DATA_DIR, myConfig.cDigital, i);
		cd_debug ("Clock: Loading %s frame (%.02fx%.02f)", cImagePath, fFrameWidth, fFrameHeight);
		cairo_t *pCairoContext = cairo_dock_create_context_from_window (myContainer);
		myData.pDigitalClock.pFrame[i].pFrameSurface = cairo_dock_create_surface_from_image (cImagePath,
			pCairoContext,  // myDrawContext
			1.,
			fFrameWidth, fFrameHeight,
			FALSE,
			&fImgW, &fImgH,
			NULL, NULL);
		cairo_destroy (pCairoContext);
		g_free (cImagePath);
	}
}

void cd_clock_draw_frames (CairoDockModuleInstance *myApplet) {
	cd_debug ("%s", __func__);
	
	cairo_set_source_rgba (myDrawContext, 0., 0., 0., 0.);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_SOURCE);
	cairo_paint (myDrawContext);
	cairo_set_operator (myDrawContext, CAIRO_OPERATOR_OVER);
	
	int i = 0, j = (myData.pDigitalClock.bSecondCapable == TRUE ? 3 : 4);
	double fX, fY;
	//Avec secondes 3 frames, une par unité (hh | mm | ss)
	//Sans secondes 4 frames, une par nombre (1|2 | 4|5)
	for (i = 0; i < j; i++) {
		fX = (myIcon->fWidth / j) * i + myData.pDigitalClock.pFrame[i].iXOffset;
		fY = myData.pDigitalClock.pFrame[i].iYOffset;
		cd_debug ("Clock: frame:%d ; fX:%.02f ; fY:%.02f", i+1, fX, fY);
		cairo_set_source_surface (myDrawContext, myData.pDigitalClock.pFrame[i].pFrameSurface, fX, fY);
		cairo_paint (myDrawContext);
	}
	
	CD_APPLET_REDRAW_MY_ICON;
}

void cd_clock_put_text_on_frames (CairoDockModuleInstance *myApplet, int width, int height, double fMaxScale, struct tm *pTime) {
	cd_debug ("%s", __func__);
	
	cairo_t *pSourceContext = myDrawContext;
	GString *sFormat = g_string_new ("");
	
	if (myConfig.b24Mode) {
		if (myData.pDigitalClock.bSecondCapable)
			g_string_printf (sFormat, "%%T");
		else
			g_string_printf (sFormat, " %%R");
	}
	else {
		if (myData.pDigitalClock.bSecondCapable)
			g_string_printf (sFormat, "%%r%%s");
		else
			g_string_printf (sFormat, "%%I:%%M");
	}

	if (myConfig.iShowDate == CAIRO_DOCK_INFO_ON_ICON)
		cd_clock_draw_date_on_frame (myApplet);
		//Erf! Comment bien gérer ca ...
	
	strftime (s_cDateBuffer, CD_CLOCK_DATE_BUFFER_LENGTH, sFormat->str, pTime);
	g_string_free (sFormat, TRUE);
	
	/// Attention : soit on passe un char a cd_clock_fill_text_surface(), soit un char *, il faut choisir.
	/// Ne pas modifier le pointeur d'une chaine ! utiliser un pointeur secondaire qu'on balade dessus, sinon au free ca fait mal ;-)
	gchar *cTime = g_strdup (s_cDateBuffer), *cT1 = NULL;
	if (myData.pDigitalClock.bSecondCapable) { //On coupe aux ':' donc on arrive a 12|45|32
		cT1 = g_strdup (cTime);
		gchar *str = strchr (cT1, ':'); //On récupère [12]:45:32
		if (str != NULL)
			*str = '\0';
		cd_clock_fill_text_surface (myApplet, cT1, 0);
		
		cT1 = g_strdup (cTime);
		str = strrchr (cT1, ':'); //On récupère [12:45]:32
		if (str != NULL)
			*str = '\0';
		str = strchr (cT1, ':'); //On récupère 12:[45]:32
		str++;
		cd_clock_fill_text_surface (myApplet, str, 1);
		
		cT1 = g_strdup (cTime);
		str = strrchr (cT1, ':'); //On récupère 12:45:[32]
		str++;
		cd_clock_fill_text_surface (myApplet, str, 2);
	}
	else { //On coupe au ':' puis on sépare les chiffres donc on arrive a 1|2 | 4|5
		cT1 = g_strdup (cTime);
		gchar *str = strchr (cT1, ':'); //On récupère [12]:45
		if (str != NULL)
			*str = '\0';
		cd_clock_fill_text_surface (myApplet, *cT1, 0);
		cT1++; /// GLUPS !
		cd_clock_fill_text_surface (myApplet, *cT1, 1);
		
		cT1 = g_strdup (cTime);
		str = strrchr (cT1, ':'); //On récupère 12:[45]
		str++;
		cd_clock_fill_text_surface (myApplet, *str, 2);
		str++;
		cd_clock_fill_text_surface (myApplet, *str, 3);
	}
	g_free (cTime);
	g_free (cT1);
	
	int i = 0, j = (myData.pDigitalClock.bSecondCapable == TRUE ? 3 : 4);
	for (i = 0; i < j; i++) {
		//On dessine le texte dans les frames
		cd_clock_draw_text_from_surface (myApplet, i);
	}
	
	//On ajoute le am/pm s'il le faut
	if (! myConfig.b24Mode) {
		if (pTime->tm_hour > 12)
			cd_clock_draw_ampm (myApplet, "PM");
		else
			cd_clock_draw_ampm (myApplet, "AM");
	}
}

void cd_clock_draw_ampm (CairoDockModuleInstance *myApplet, gchar *cMark) {
	g_print ("Adding %s to the last frame\n", cMark);
	//Ca s'est gérer dans la config.
	//Vérifier les structures necessaires.
}

void cd_clock_draw_text_from_surface (CairoDockModuleInstance *myApplet, int iNumber) {
	g_print ("Printing text #%d on corresponding frame\n", iNumber);
	//Il faudra surment scale down la surface avant de cairo_print
	//TODO prendre le code sur slider.
}

void cd_clock_fill_text_surface (CairoDockModuleInstance *myApplet, gchar *cStr, int iNumber) {
	g_print ("Filling the #%d surface with %s\n", iNumber, cStr);
	//Aucune idée de comment faire!
	//TODO demander a fabounet des indices ici.
}

void cd_clock_draw_date_on_frame (CairoDockModuleInstance *myApplet) {
	g_print ("Add date on frame\n");
}
