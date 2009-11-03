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

//\________________ Add your name in the copyright file (and / or modify your name here)

#include <cairo-dock.h>
#include <math.h>
#include "applet-struct.h"
#include "applet-rss.h"
#include "applet-draw.h"


char* ltrim( char* str, const char* t )  // Couper tout depuis la gauche
{
	char* curStr = NULL;

	char look[ 256 ] = { 1, 0 };
	while( *t )
		look[ (unsigned char)*t++ ] = 1;

	curStr = str;
	while( *curStr && look[ *curStr ] )
		++curStr;
	strcpy( str, curStr );
	
	return str; 
} 

gchar *cd_rssreader_cut_feed_lines_with_return (CairoDockModuleInstance *myApplet, int iMaxWidth, gchar *cLongLine)   // Un simple essai
{
	cairo_text_extents_t textExtents;  // -> textExtents.height et textExtents.width
	double iRatioForRealFontSize = 1.5; // ratio pour avoir un texte à peu près à la bonne dimension		
	
	///myData.cCuttedLine = g_strdup_printf ("%s", cLongLine); // On stocke la ligne dans myData.cCuttedLine
	myData.cCuttedLine = g_strdup (cLongLine);  // On stocke la ligne dans myData.cCuttedLine	
	cairo_text_extents (myDrawContext, myData.cCuttedLine, &textExtents);  // on recupere les dimensions de la ligne de départ (pour info seulement)
	int iTextWidth = (int)(textExtents.width * iRatioForRealFontSize);  // On lui applique le ratio effectué sur le texte	
	
	// Calcul du nombre de ligne à obtenir:
	///int iNbLines = (iTextWidth/iMaxWidth) + 1;  // inutilise, et faux de toute maniere.
	
	// On compte le nombre de lettre 	
	gint iNbLetters = strlen(myData.cCuttedLine);
	
	// On compte le nombre de mot
 	gchar cCurrentLetter = '\0';
	gint iNbWords = 1;
	long i=0;	
	for (i=0 ; i < iNbLetters ; i++)
	{
		cCurrentLetter = myData.cCuttedLine[i];
		if (cCurrentLetter == ' ')
			iNbWords++;
	}
	
	i = 1;	
	myData.cTempText = g_strdup_printf ("");
	
	gchar **cWord = g_strsplit (myData.cCuttedLine," ",0);
	
	do
	{
		myData.cCuttedLine = g_strdup_printf ("%s", cLongLine); // On réinitilise myData.cCuttedLine (utile pour la boucle)
		myData.cTempText = g_strdup_printf (" %s%s", cWord[iNbWords-i], myData.cTempText); // On définit ce qu'il y a à retirer	
		myData.cCuttedLine = *g_strsplit (myData.cCuttedLine, myData.cTempText, -1); // On coupe le texte
		cairo_text_extents (myDrawContext, myData.cCuttedLine, &textExtents);  // on recupere les dimensions de la ligne
		iTextWidth = (int)(textExtents.width * iRatioForRealFontSize); // On applique le ratio pour le texte	
		i++;
	} while ( iTextWidth > iMaxWidth ); // Si le nouveau texte dépasse encore -> On boucle
	
	g_free (cWord);
	ltrim( myData.cTempText, " " ); // On supprime l'espace afin de renvoyer le reste de la ligne à la fin
	
	cairo_show_text (myDrawContext, myData.cCuttedLine); // On affiche la nouvelle ligne		
	g_free (myData.cCuttedLine);
	
	// On renvoit le reste du texte à afficher :
	cd_debug ("RSSreader-debug : ---------------- >   End of line = \"%s\"", myData.cTempText);	
	return myData.cTempText;	
}


void cd_rssreader_cut_feed_lines (CairoDockModuleInstance *myApplet, int iMaxWidth, gchar *cLongLine)   // Un simple essai
{
	cairo_text_extents_t textExtents;  // -> textExtents.height et textExtents.width
	double iRatioForRealFontSize = 1.5; // ratio pour avoir un texte à peu près à la bonne dimension	
	
	myData.cCuttedLine = g_strdup_printf ("%s", cLongLine); // On stocke la ligne dans myData.cCuttedLine
	cairo_text_extents (myDrawContext, myData.cCuttedLine, &textExtents);  // on recupere les dimensions de la ligne de départ
	int iTextWidth = (int)(textExtents.width * iRatioForRealFontSize);  // On lui applique le ratio effectué sur le texte	
	
	// On compte le nombre de lettre 	
	gint iNbLetters = strlen(myData.cCuttedLine);

	// On enlève lettre par lettre jusqu'à être ok (et on utilise 'cairo_dock_cut_string' pour mettre '...' dans l'espace trouvé 
	do
	{
		myData.cCuttedLine = cairo_dock_cut_string (myData.cCuttedLine, iNbLetters-1);
		iNbLetters--;
		
		cairo_text_extents (myDrawContext, myData.cCuttedLine, &textExtents);  // on recupere les dimensions de la ligne
		iTextWidth = (int)(textExtents.width * iRatioForRealFontSize); // On applique le ratio pour le texte
	} while ( iTextWidth > iMaxWidth ); // Si le nouveau texte dépasse encore -> On boucle	
	
	cairo_show_text (myDrawContext, myData.cCuttedLine); // On affiche la nouvelle ligne		
	g_free (myData.cCuttedLine);
}


/**
static void _get_title (CairoDockModuleInstance *myApplet)
{	
	gchar *cCommand = g_strdup_printf ("%s/rss_reader.sh %s 1 0", MY_APPLET_SHARE_DATA_DIR, myConfig.cUrl);
	cd_debug ("RSSreader-debug : TITLE TASK ---------------------->  %s", cCommand);
	
	g_spawn_command_line_sync (cCommand, &myData.cTitleTaskBridge, NULL, NULL, NULL);  // myData.cTitleTaskBridge = cairo_dock_launch_command_sync (cCommand);
	cd_debug ("RSSreader-debug : TITLE TASK ---------------> myData.cTitleTaskBridge = \"%s\"", myData.cTitleTaskBridge);
	g_free (cCommand);		
	
	// On vérifie que la commande nous a renvoyé quelque chose de cohérent
	if (myData.cTitleTaskBridge == NULL || strcmp(myData.cTitleTaskBridge, "") == 0)
	{			
		myData.cTitleTaskBridge = g_strdup_printf ("%s\n", myConfig.cMessageNoTitle);		
	}
}

static gboolean _update_title (CairoDockModuleInstance *myApplet)
{
	gchar *str = strchr (myData.cTitleTaskBridge, '\n');
	if (str)
		*str = '\0';
	myConfig.cName = g_strdup (myData.cTitleTaskBridge);
	///myConfig.cName = g_strdup_printf ("%s",*(g_strsplit(myData.cTitleTaskBridge, "\n", 0)) );   // On récupère le contenu de myData.cTaskBridge	
	
	///if (strcmp(myConfig.cName, "") == 0)
	if (*myConfig.cName == '\0')
		myConfig.cName = g_strdup (myConfig.cMessageNoTitle);	
	
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,G_TYPE_STRING, "Configuration", "name_rss_feed", myConfig.cName,G_TYPE_INVALID);
		
	///myData.pTitleTask = NULL;  // on la garde, on peut la reutiliser quand on veut, on la detruira a la fin.
	return TRUE;
}

void cd_rssreader_upload_title_TASK (CairoDockModuleInstance *myApplet)
{	
	if (myData.pTitleTask == NULL)
	{		
		myData.pTitleTask = cairo_dock_new_task (0,  // One shot task
			(CairoDockGetDataAsyncFunc) _get_title,
			(CairoDockUpdateSyncFunc) _update_title,
			myApplet);
		cairo_dock_launch_task (myData.pTitleTask);
	}
	else
		cd_debug ("RSSreader-debug : -----------------------------------------> TITLE TASK ALREADY RUNNING");
}
*/


void cd_applet_draw_my_desklet (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	g_print ("%s (%dx%d)\n", __func__, iWidth, iHeight);
	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	PangoRectangle ink, log;
	
	// On efface la surface cairo actuelle
	cairo_dock_erase_cairo_context (myDrawContext);	
	
	// dessin du fond (optionnel).
	if (myConfig.bDisplayBackground)
	{
		cairo_save (myDrawContext); // On sauvegarde la position #2
		cairo_translate (myDrawContext,
				.5*myConfig.iBorderThickness,
				.5*myConfig.iBorderThickness);		
		cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
			0.,
			0.,
			iHeight);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			0.,
			myConfig.fBackgroundColor1[0],
			myConfig.fBackgroundColor1[1],
			myConfig.fBackgroundColor1[2],
			myConfig.fBackgroundColor1[3]);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			1.,
			myConfig.fBackgroundColor2[0],
			myConfig.fBackgroundColor2[1],
			myConfig.fBackgroundColor2[2],
			myConfig.fBackgroundColor2[3]);
		cairo_set_source (myDrawContext, pGradationPattern);
		
		if (myConfig.iBackgroundRadius != 0)  // On a besoin d'un rayon
		{		
			cairo_dock_draw_rounded_rectangle (myDrawContext,
				myConfig.iBackgroundRadius,
				0.,
				iWidth - myConfig.iBorderThickness - 2 * myConfig.iBackgroundRadius,
				iHeight - myConfig.iBorderThickness);			
		}
		else  // Il ne faut pas de rayon
		{
			cairo_rectangle (myDrawContext,
				0., 0.,
				iWidth - myConfig.iBorderThickness,
				iHeight - myConfig.iBorderThickness);
		}
		cairo_fill (myDrawContext);
		cairo_pattern_destroy (pGradationPattern);
		cairo_restore (myDrawContext);  // On restaure la position #2
	}
	
	int iMargin = .5 * myConfig.iBorderThickness + (1. - sqrt (2) / 2) * myConfig.iBackgroundRadius;  // marge a gauche et au-dessus.
	double fCurrentY = 0.;  // position de la ligne courante.
	
	// On affiche le logo si besoin
	PangoFontDescription *fd = pango_font_description_from_string (myConfig.cTitleFont);
	pango_layout_set_font_description (pLayout, fd);
	gboolean bLogoIsOn = FALSE;  // utile pour décaler le texte du titre
	if (myConfig.bDisplayLogo)
	{
		// on recupere la taille du titre.
		int iSize = pango_font_description_get_size (fd);
		if (!pango_font_description_get_size_is_absolute (fd))
			iSize /= PANGO_SCALE;
		if (iSize == 0)
			iSize = 16;
		
		// on en deduit la taille du logo.
		double fLogoSize = myConfig.fLogoSize * iSize;
		
		// on cree la surface du logo si necessaire.
		if (myData.fLogoSize != iSize || myData.pLogoSurface == NULL)
		{
			cairo_surface_destroy (myData.pLogoSurface);
			myData.fLogoSize = fLogoSize;
			myData.pLogoSurface = cairo_dock_create_surface_for_icon (myConfig.cLogoPath,
				myDrawContext,
				fLogoSize,
				fLogoSize);
		}
		
		// on affiche le logo.
		if (myData.pLogoSurface != NULL)
		{
			bLogoIsOn = TRUE;
			cairo_set_source_surface (myDrawContext,
			myData.pLogoSurface,
				iMargin + myConfig.iTitleMargin,
				iMargin);
			cairo_paint (myDrawContext);
		}		
	}
	pango_font_description_free (fd);
	
	// dessin du nom du flux.
	if (myData.pItemList && myData.pItemList[0].cTitle)
	{
		pango_layout_set_text (pLayout, myData.pItemList[0].cTitle, -1);
		pango_layout_get_pixel_extents (pLayout, &ink, &log);
		
		cairo_set_source_rgba (myDrawContext, myConfig.fTitleTextColor[0], myConfig.fTitleTextColor[1], myConfig.fTitleTextColor[2], myConfig.fTitleTextColor[3]);	
		
		cairo_move_to (myDrawContext,
			iMargin + myConfig.iTitleMargin + (bLogoIsOn ? myData.fLogoSize + 5 : 0),  // 5 pixels d'ecart entre le logo et le texte.
			iMargin + (bLogoIsOn ? MAX (0, (myData.fLogoSize - log.height)/2) : 0));
		pango_cairo_show_layout (myDrawContext, pLayout);
		
		fCurrentY = iMargin + MAX (log.height, (bLogoIsOn ? myData.fLogoSize : 0)) + myConfig.iSpaceBetweenFeedLines;
		
		// dessin des lignes.
		fd = pango_font_description_from_string (myConfig.cFont);
		pango_layout_set_font_description (pLayout, fd);
		pango_font_description_free (fd);
		
		cairo_set_source_rgba (myDrawContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2], myConfig.fTextColor[3]);	
		
		CDRssItem *pItem;
		int i;
		for (i = 1; i < myData.iNbItems; i ++)
		{
			pItem = &myData.pItemList[i];
			if (pItem->cTitle == NULL)
				continue;
			
			gchar *cLine = g_strdup (pItem->cTitle);
			cd_rssreader_cut_line (cLine, pLayout, iWidth - myConfig.iBorderThickness - iMargin - myConfig.iTextMargin);
			
			pango_layout_set_text (pLayout, cLine, -1);
			pango_layout_get_pixel_extents (pLayout, &ink, &log);
			g_free (cLine);
			
			cairo_move_to (myDrawContext,
				iMargin + myConfig.iTextMargin,
				fCurrentY);
			pango_cairo_show_layout (myDrawContext, pLayout);
			
			fCurrentY += log.height + myConfig.iSpaceBetweenFeedLines;
			g_print ("fCurrentY <- %.2f\n", fCurrentY);
		}
	}
	
	// dessin du cadre (optionnel).
	if (myConfig.bDisplayBackground)
	{
		cairo_save (myDrawContext);  // On sauvegarde la position #5
		cairo_set_source_rgba (myDrawContext,
			myConfig.fBorderColor[0],
			myConfig.fBorderColor[1],
			myConfig.fBorderColor[2],
			myConfig.fBorderColor[3]);
		cairo_set_line_width (myDrawContext, myConfig.iBorderThickness);
		cairo_translate (myDrawContext,
			.5*myConfig.iBorderThickness,
			.5*myConfig.iBorderThickness);
		cairo_dock_draw_rounded_rectangle (myDrawContext,
			myConfig.iBackgroundRadius,
			0.,
			iWidth - 2*myConfig.iBackgroundRadius - myConfig.iBorderThickness,
			iHeight - myConfig.iBorderThickness);
		cairo_stroke (myDrawContext);
		cairo_restore (myDrawContext);  // On restaure la position #4
	}
	
	g_object_unref (pLayout);
	
	// on met a jour la texture OpenGL.
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		///CD_APPLET_FINISH_DRAWING_MY_ICON;  // aucune compmande opengl n'a ete utilisee, on se contente de transferer la surface cairo a OpenGL.
		cairo_dock_update_icon_texture (myIcon);		
	}
	
	
	/*int iOffset = 2;
	double iRatioForRealFontSize = 1.5;
	//myData.fLogoSize = myConfig.fLogoSize*atol(myData.cTitleFontSize);
	myData.fLogoSize = myConfig.fLogoSize*12;  /// en attendant.
	
	// On efface la surface cairo actuelle
	cairo_dock_erase_cairo_context (myDrawContext);	
	
	///if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	///	CD_APPLET_START_DRAWING_MY_ICON;  // ca c'est pour dessiner en opengl, ici on fait tout en cairo.
		
	//\______ On commence le dessin en cairo:
	
	// Background (optionnel)
	if (myConfig.bDisplayBackground)
	{
		cairo_save (myDrawContext); // On sauvegarde la position #2
		cairo_translate (myDrawContext,
				myConfig.iBorderThickness/1.7 + (myConfig.iBorderThickness/2)*.95,
				myConfig.iBorderThickness/1.7 + (myConfig.iBorderThickness/2)*.95);  // Ne me demandez pas pourquoi ce /1.7 et ce 0.95:/		
		cairo_pattern_t *pGradationPattern = cairo_pattern_create_linear (0.,
			0.,
			0.,
			iHeight);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			0.,
			myConfig.fBackgroundColor1[0],
			myConfig.fBackgroundColor1[1],
			myConfig.fBackgroundColor1[2],
			myConfig.fBackgroundColor1[3]);
		cairo_pattern_add_color_stop_rgba (pGradationPattern,
			1.,
			myConfig.fBackgroundColor2[0],
			myConfig.fBackgroundColor2[1],
			myConfig.fBackgroundColor2[2],
			myConfig.fBackgroundColor2[3]);
		cairo_set_source (myDrawContext, pGradationPattern);
		
		if ( (myConfig.iBackgroundRadius - (myConfig.iBorderThickness/2.)) > 0)  // On a besoin d'un rayon
		{		
			cairo_dock_draw_rounded_rectangle (myDrawContext,
				myConfig.iBackgroundRadius - (myConfig.iBorderThickness/2.), 0.,
				iWidth - 2*myConfig.iBackgroundRadius - myConfig.iBorderThickness + 0.9,
				iHeight- (myConfig.iBorderThickness*2) +0.9);   // Ne me demandez pas pourquoi ces 2 petits recalage avec 0.9 (sinon, la bordure ne touche pas à doite et en bas) :/			
		}
		else  // Il ne faut pas de rayon
		{
			cairo_rectangle (myDrawContext, 0, 0,
				iWidth - 2*myConfig.iBackgroundRadius - myConfig.iBorderThickness + 0.9 + (2*(myConfig.iBackgroundRadius - (myConfig.iBorderThickness/2.))),
				iHeight- (myConfig.iBorderThickness*2) +0.9);   // Ne me demandez pas pourquoi ces 2 petits recalage avec 0.9 (sinon, la bordure ne touche pas à doite et en bas) :/
		}
		cairo_fill (myDrawContext);
		cairo_pattern_destroy (pGradationPattern);
		cairo_restore (myDrawContext); // On restaure la position #2
	}
	
	// On scale le dessin pour respecter l'échelle du texte.... pourquoi ??? .....Bonne question :/
	cairo_scale (myDrawContext, iRatioForRealFontSize, iRatioForRealFontSize);
	
	cairo_save (myDrawContext); // On sauvegarde la position #3
	
	
	// On affiche le logo si besoin
	if (myConfig.bDisplayLogo)
	{
		myData.pLogoSurface = cairo_dock_create_surface_for_icon (myConfig.cLogoPath, myDrawContext, myData.fLogoSize, myData.fLogoSize);
		myData.iMyLogoIsOn = 1; // utile pour décaler le texte du titre
		if (myData.pLogoSurface != NULL)
		{
			cairo_set_source_surface (myDrawContext, myData.pLogoSurface,
				iOffset + myConfig.iTitlePositionX + (myConfig.iBorderThickness/iRatioForRealFontSize),
				iOffset + myConfig.iTitlePositionY + (myConfig.iBorderThickness/iRatioForRealFontSize));
			cairo_paint (myDrawContext);
			cairo_surface_destroy (myData.pLogoSurface);
		}		
	}
	else
		myData.iMyLogoIsOn = 0; // utile pour décaler le texte du titre
	
	
	// On écrit le titre	
	cairo_set_source_rgba (myDrawContext, myConfig.fTitleTextColor[0], myConfig.fTitleTextColor[1], myConfig.fTitleTextColor[2], myConfig.fTitleTextColor[3]);	
	
	if (myData.bTitleFontIsBold && myData.bTitleFontIsItalic)
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cTitleFont,
			CAIRO_FONT_SLANT_ITALIC,
			CAIRO_FONT_WEIGHT_BOLD);
	}
	else if (myData.bTitleFontIsBold && ! myData.bTitleFontIsItalic)
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cTitleFont,
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);
	}
	else if (! myData.bTitleFontIsBold && myData.bTitleFontIsItalic)
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cTitleFont,
			CAIRO_FONT_SLANT_ITALIC,
			CAIRO_FONT_WEIGHT_NORMAL);
	}
	else
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cTitleFont,
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);	
	}
	cairo_set_font_size (myDrawContext, atol(myData.cTitleFontSize));	
	
	if (myConfig.bDisplayLogo)
		cairo_translate (myDrawContext, myData.fLogoSize + iOffset,  myData.fLogoSize/2 - atol(myData.cTitleFontSize)/2 - iOffset);
		
	cairo_translate (myDrawContext,
			iOffset + myConfig.iTitlePositionX + (myConfig.iBorderThickness/iRatioForRealFontSize),
			atol(myData.cTitleFontSize) + iOffset + myConfig.iTitlePositionY + (myConfig.iBorderThickness/iRatioForRealFontSize));
	cairo_show_text (myDrawContext, myConfig.cName);
	cairo_fill (myDrawContext);
	cairo_restore (myDrawContext); // On restaure la position #3
	
	
	// On écrit les ligne du flux	
	cairo_translate (myDrawContext, iOffset + myConfig.iTextPositionX, iOffset + myConfig.iTextPositionY);
	if (myConfig.bDisplayLogo)
		cairo_translate (myDrawContext, 0. + (myConfig.iBorderThickness/iRatioForRealFontSize),  myData.fLogoSize + iOffset + (myConfig.iBorderThickness/iRatioForRealFontSize));
	else
		cairo_translate (myDrawContext, 0. + (myConfig.iBorderThickness/iRatioForRealFontSize),  atol(myData.cTitleFontSize) + iOffset + (myConfig.iBorderThickness/iRatioForRealFontSize));
	
	
	cairo_set_source_rgba (myDrawContext, myConfig.fTextColor[0], myConfig.fTextColor[1], myConfig.fTextColor[2], myConfig.fTextColor[3]);	
	
	if (myData.bFontIsBold && myData.bFontIsItalic)
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cFont,
			CAIRO_FONT_SLANT_ITALIC,
			CAIRO_FONT_WEIGHT_BOLD);
	}
	else if (myData.bFontIsBold && ! myData.bFontIsItalic)
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cFont,
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_BOLD);
	}
	else if (! myData.bFontIsBold && myData.bFontIsItalic)
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cFont,
			CAIRO_FONT_SLANT_ITALIC,
			CAIRO_FONT_WEIGHT_NORMAL);
	}
	else
	{
		cairo_select_font_face (myDrawContext,
			myConfig.cFont,
			CAIRO_FONT_SLANT_NORMAL,
			CAIRO_FONT_WEIGHT_NORMAL);	
	}		
	cairo_set_font_size (myDrawContext, atol(myData.cFontSize));  
	
	
	myData.bIsSameLine = FALSE;
	myData.iCurrentLineForMaxLine = 1;
		
	if (strcmp(myData.cAllFeedLines, g_strdup_printf ("%s\n%s", myConfig.cMessageNoUrl, myConfig.cMessageNoUrl2)) == 0)
		myData.bMessageNoUrl = TRUE; // On forcera alors le mode Multi-lignes
	else
		myData.bMessageNoUrl = FALSE;	
	
	
	int i = 0;	
	if (iWidth != 1) // nécessaire car la boucle peut attaquer avant que le desklet ne soit dessiné
	{
		if (myData.cSingleFeedLine[0] != NULL)
		{			
			for (i=0 ; myData.cSingleFeedLine[i] != NULL ; i++)
			{				
				cairo_translate (myDrawContext, 0, atol(myData.cFontSize));
				
				cairo_text_extents_t textExtents;  // -> textExtents.height et textExtents.width
				cairo_text_extents (myDrawContext, myData.cSingleFeedLine[i], &textExtents);  // on recupere les dimensions de la ligne
				
				if (i != 0 || myData.bIsSameLine)
				{
					if (myData.bIsSameLine)
						cairo_translate (myDrawContext, 0, myConfig.iSpaceBetweenLinesMulti);
					else
						cairo_translate (myDrawContext, 0, myConfig.iSpaceBetweenFeedLines + textExtents.height); // Par défaut, on saute une ligne entre 2 flux (réglable en conf)
				}
					
				
				if (textExtents.width == 0)
					break;
					
				cd_debug ("RSSreader-debug-text_extents:  --------------->  %s", myData.cSingleFeedLine[i]);			
				cd_debug ("RSSreader-debug-text_extents :          textExtents.width = \"%i\"", (int)(textExtents.width*iRatioForRealFontSize));	
								
				int iMaxTextWidth = iWidth - 2*myConfig.iBorderThickness - 3*iOffset - (iRatioForRealFontSize*myConfig.iTextPositionX); // On réduit la limite pour garder un peu d'espace à droite
				
				
				if ((int)(textExtents.width*iRatioForRealFontSize) <= iMaxTextWidth )
				{
					cairo_show_text (myDrawContext, myData.cSingleFeedLine[i]);
					myData.bIsSameLine = FALSE;
					myData.iCurrentLineForMaxLine = 1;					
				}
				else
				{
					if (  myConfig.iMaxLines == 0 || myData.iCurrentLineForMaxLine < myConfig.iMaxLines || myData.bMessageNoUrl)
					{
						myData.cSingleFeedLine[i] = cd_rssreader_cut_feed_lines_with_return (myApplet, iMaxTextWidth, myData.cSingleFeedLine[i]);
						cd_debug ("RSSreader-debug-text_cut:  --------------->  %s", myData.cSingleFeedLine[i]);
						myData.bIsSameLine = TRUE;
						myData.iCurrentLineForMaxLine++;
						i--;		
					}
					else // On doit afficher la dernière ligne autorisée pour la ligne de flux
					{
						cd_rssreader_cut_feed_lines (myApplet, iMaxTextWidth, myData.cSingleFeedLine[i]);
						myData.bIsSameLine = FALSE;
						myData.iCurrentLineForMaxLine = 1;
					}
				}
								
				cairo_fill (myDrawContext);	
			}
		}
	}
	cairo_restore (myDrawContext); // On restaure la position #1
	*/
}


void cd_applet_update_my_icon (CairoDockModuleInstance *myApplet/**, Icon *pIcon, CairoContainer *pContainer*/)  // l'icone et le container sont accessibles par myApplet.
{
	if (myDesklet)
	{
		// taille de la texture.
		/**double fMaxScale = cairo_dock_get_max_scale (pContainer);
		double fRatio = pContainer->fRatio;
		int iWidth = (int) pIcon->fWidth / fRatio * fMaxScale;
		int iHeight = (int) pIcon->fHeight / fRatio * fMaxScale;*/
		int iWidth, iHeight;
		CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
		
		/**if (CD_APPLET_MY_CONTAINER_IS_OPENGL)  // pas la peine.
		{
			if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
				return ;
		}*/
		cd_applet_draw_my_desklet (myApplet, iWidth, iHeight);		
		
		CD_APPLET_REDRAW_MY_ICON;
	}
}
