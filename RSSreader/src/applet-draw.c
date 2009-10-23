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

char* rtrim( char* str, const char* t )  // Couper tout depuis la droite
{
	char* curEnd = str, *end = str;

	char look[ 256 ] = { 1, 0 };
	while( *t )
		look[ (unsigned char)*t++ ] = 1;

	while( *end )
	{
		if ( !look[ *end ] )
			curEnd = end + 1;
		++end;
	}
	*curEnd = '\0';

	return str;
}


void cd_rssreader_upload_title_TASK (CairoDockModuleInstance *myApplet)
{
	
	if (myData.pTitleTask == NULL)
	{
		myData.pTitleTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) cd_rssreader_upload_title,
			(CairoDockUpdateSyncFunc) cd_rssreader_update_title,
			myApplet);
		cairo_dock_launch_task (myData.pTitleTask);
	}
	else
		cd_debug ("RSSreader-debug : -----------------------------------------> TITLE TASK ALREADY RUNNING");
}


void cd_rssreader_upload_title (CairoDockModuleInstance *myApplet)
{	
	GString *sCommand = g_string_new ("");		
	
	g_string_printf (sCommand, "/usr/share/cairo-dock/plug-ins/RSSreader/rss_reader.sh %s 1 0", myConfig.cUrl);
							
	cd_debug ("RSSreader-debug : TITLE TASK ---------------------->  %s",sCommand->str);
	
	g_spawn_command_line_sync (sCommand->str, &myData.cTitleTaskBridge, NULL, NULL, NULL);  // myData.cTitleTaskBridge = cairo_dock_launch_command_sync (sCommand->str);
	cd_debug ("RSSreader-debug : TITLE TASK ---------------> myData.cTitleTaskBridge = \"%s\"", myData.cTitleTaskBridge);
	g_string_free (sCommand, TRUE);		
	
	// On vérifie que la commande nous a renvoyé quelque chose de cohérent
	if (myData.cTitleTaskBridge == NULL || strcmp(myData.cTitleTaskBridge, "") == 0)
	{			
		myData.cTitleTaskBridge = g_strdup_printf ("%s\n", myConfig.cMessageNoTitle);		
	}
}

void cd_rssreader_update_title (CairoDockModuleInstance *myApplet)
{
	myConfig.cName = g_strdup_printf ("%s",*(g_strsplit(myData.cTitleTaskBridge, "\n", 0)) );   // On récupère le contenu de myData.cTaskBridge
	
	
	if (strcmp(myConfig.cName, "") == 0)
		myConfig.cName = g_strdup_printf ("%s", myConfig.cMessageNoTitle);
	
	
	cairo_dock_update_conf_file (CD_APPLET_MY_CONF_FILE,G_TYPE_STRING, "Configuration", "name_rss_feed", myConfig.cName,G_TYPE_INVALID);
		
	myData.pTitleTask = NULL;
}


void cd_rssreader_upload_feeds_TASK (CairoDockModuleInstance *myApplet)
{
	
	if (myData.pTask == NULL)
	{
		myData.pTask = cairo_dock_new_task (0,
			(CairoDockGetDataAsyncFunc) cd_rssreader_upload_feeds,
			(CairoDockUpdateSyncFunc) cd_rssreader_update_feeds,
			myApplet);
		cairo_dock_launch_task (myData.pTask);
	}
	else
		cd_debug ("RSSreader-debug : -----------------------------------------> TASK ALREADY RUNNING");
}


void cd_rssreader_automatic_refresh (CairoDockModuleInstance *myApplet)
{
	cd_debug ("RSSreader-debug : -----------------------------------------> AUTOMATIC REFRESH");
	myData.pAutomaticRefreshTask = cairo_dock_new_task (0,
		(CairoDockGetDataAsyncFunc) cd_rssreader_upload_feeds,
		(CairoDockUpdateSyncFunc) cd_rssreader_update_feeds,
		myApplet);
	cairo_dock_launch_task (myData.pAutomaticRefreshTask);
}

void cd_rssreader_upload_feeds (CairoDockModuleInstance *myApplet)
{		
	if (myConfig.cUrl == NULL )
	{
		myData.cTaskBridge = g_strdup_printf ("%s\n",myConfig.cMessageNoUrl);  // Le \n est important pour la suite
		cd_debug ("RSSreader-debug : TASK ---------------> myData.cTaskBridge = \"%s\"", myData.cTaskBridge);	
	}
	else
	{
		GString *sCommand = g_string_new ("");		
		
		g_string_printf (sCommand, "/usr/share/cairo-dock/plug-ins/RSSreader/rss_reader.sh %s %i %i", myConfig.cUrl, myConfig.iLines, myConfig.iTitleNum);
								
		cd_debug ("RSSreader-debug : TASK ---------------------->  %s",sCommand->str);
		
		g_spawn_command_line_sync (sCommand->str, &myData.cTaskBridge, NULL, NULL, NULL);  // myData.cTaskBridge = cairo_dock_launch_command_sync (sCommand->str);
		cd_debug ("RSSreader-debug : TASK ---------------> myData.cTaskBridge = \"%s\"", myData.cTaskBridge);
		g_string_free (sCommand, TRUE);		
		
		// On vérifie le nombre de lignes reçues
	 	gchar cCurrentLetter = {0};
		gint iNbLines = 0;
		long i=0;
		for (i=0 ; i < strlen(myData.cTaskBridge) ; i++)
		{
			cCurrentLetter = myData.cTaskBridge[i];				
			if (cCurrentLetter == '\n')
				iNbLines++;
		}		
		if (iNbLines == myConfig.iLines)
			cd_debug ("RSSreader-debug : TASK ---------------> Number of lines received = OK :-)");
		else
			cd_debug ("RSSreader-debug : TASK ---------------> Number of lines received = KO :-(");
		
		// On vérifie que la commande nous a renvoyé quelque chose de cohérent
		if (myData.cTaskBridge == NULL || strcmp(myData.cTaskBridge, "") == 0 || iNbLines != myConfig.iLines)
		{			
			myData.cTaskBridge = g_strdup_printf ("%s\n", myConfig.cMessageFailedToConnect);  // Le \n est important pour la suite		
		}		
	}	
}

void cd_rssreader_update_feeds (CairoDockModuleInstance *myApplet)
{
	myData.cFeedLine[0] = g_strdup_printf ("%s",myData.cTaskBridge);  // On récupère le contenu de myData.cTaskBridge -> myData.cFeedLine[0] nous servira pour les dialogues ;-)		
	myData.cFeedLine[1] = g_strdup_printf ("%s",*(g_strsplit(myData.cFeedLine[0], "\n", 0)) ); 
	
	if (strcmp(myData.cFeedLine[0], g_strdup_printf ("%s\n", myConfig.cMessageNoUrl) ) == 0) // Si strcmp renvoie 0 (chaînes identiques)
	{
		myData.cFeedLine[2] = g_strdup_printf (D_("Drag'n drop a valid RSS feed URL,"));
		myData.cFeedLine[3] = g_strdup_printf (D_("or use \"Paste a new RSS Url\" from menu"));
		myData.cFeedLine[4] = g_strdup_printf (D_("to add one."));
		int i;
		for (i = 5 ; i < (myConfig.iLines+1) ; i++)
		{
			myData.cFeedLine[i] = g_strdup_printf ("%s", " ");
		}
	}	
	else if (strcmp(myData.cFeedLine[0], g_strdup_printf ("%s\n", myConfig.cMessageFailedToConnect)) == 0 ) // Si strcmp renvoie 0 (chaînes identiques)
	{
		int i;
		for (i = 2 ; i < (myConfig.iLines+1) ; i++)
		{
			myData.cFeedLine[i] = g_strdup_printf ("%s", " ");
		}
	}
	else
	{
		int i;			
		for (i = 1 ; i < (myConfig.iLines+1) ; i++)
		{
			if (i == 1)
				cd_debug ("RSSreader-debug : UPDATE ---------------> myData.cFeedLine [ %i ] \"%s\"",i ,myData.cFeedLine[i]);
			else if (i == myConfig.iLines)
			{
				myData.cTempText = g_strdup_printf ("%s", myData.cFeedLine[0]);
				rtrim( myData.cTempText, "\n" );
				rtrim( myData.cTempText, " " );
				myData.cTempText = strrchr(myData.cTempText, '\n');
				ltrim( myData.cTempText, "\n" );
				ltrim( myData.cTempText, " " );
				myData.cFeedLine[i] = g_strdup_printf ("%s", myData.cTempText);
				cd_debug ("RSSreader-debug : UPDATE ---------------> myData.cFeedLine [ %i ] \"%s\"",i ,myData.cFeedLine[i]);				
			}
			else
			{
				myData.cTempText = g_strdup_printf ("%s", myData.cFeedLine[0]);
				int j;
				for (j = 0 ; j <  (i-1) ; j++)
				{
					myData.cTempText = strchr(myData.cTempText, '\n');
					ltrim( myData.cTempText, "\n" );
					ltrim( myData.cTempText, " " );
				}
				g_strreverse (myData.cTempText);
				myData.cFeedLine[i] = strrchr(myData.cTempText, '\n');
				ltrim( myData.cFeedLine[i], "\n" );
				ltrim( myData.cFeedLine[i], " " );
				g_strreverse (myData.cFeedLine[i]);
				cd_debug ("RSSreader-debug : UPDATE ---------------> myData.cFeedLine [ %i ] \"%s\"",i ,myData.cFeedLine[i]);
			}
		}		
	}
	
	cd_debug ("RSSreader-debug : UPDATE ---------------> Current first line = \"%s\"",myData.cFeedLine[1]);
	cd_debug ("RSSreader-debug : UPDATE ---------------> Last first line    = \"%s\"",myData.cLastFirstFeedLine);
	
	
	// On teste s'il y a eu une modification depuis le dernier update
	if (myData.cLastFirstFeedLine == NULL)
	{
		myData.cLastFirstFeedLine = g_strdup_printf ("%s", myData.cFeedLine[1]);
		cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  1st START !");
		cd_applet_update_my_icon (myApplet, myIcon, myContainer);	
	}
	else
	{
		if (strcmp(myData.cFeedLine[1], myData.cLastFirstFeedLine) != 0) // Si strcmp renvoie 0 (chaînes identiques)
		{
			cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  Feed has been modified !");	
			cairo_dock_remove_dialog_if_any (myIcon);
			
			
			myData.cDialogMessage = g_strdup_printf ("\"%s\"\n%s",myConfig.cName, D_("This RSS feed has been modified...") );
			// Si modif, on affiche une bulle de dialogue pour le signaler
			cairo_dock_show_temporary_dialog_with_icon (myData.cDialogMessage,
				myIcon,
				myContainer,
				myConfig.iDialogsDuration,
				MY_APPLET_SHARE_DATA_DIR"/"MY_APPLET_ICON_FILE);
				
			myData.cLastFirstFeedLine = g_strdup_printf ("%s", myData.cFeedLine[1]); // On mémorise pour le prochain update
			cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  Feed has been stored for next update !");
			cd_applet_update_my_icon (myApplet, myIcon, myContainer);
				
		}
		else
			cd_debug ("RSSreader-debug : CONTROL MODIFICATION --------------->  No modification.");
	}
	
	myData.pTask = NULL;
}


void cd_applet_draw_my_desklet (CairoDockModuleInstance *myApplet, int iWidth, int iHeight)
{
	cairo_save (myDrawContext); // On sauvegarde la position #1
	
	int iOffset = 2;
	double iRatioForRealFontSize = 1.5;
	myData.fLogoSize = myConfig.fLogoSize*atol(myData.cTitleFontSize);
	
	// On efface la surface cairo actuelle
	cairo_dock_erase_cairo_context (myDrawContext);	
	
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		CD_APPLET_START_DRAWING_MY_ICON;
		
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
			cairo_set_source_surface (myDrawContext, myData.pLogoSurface, iOffset + myConfig.iTitlePositionX + (myConfig.iBorderThickness/1.5), iOffset + myConfig.iTitlePositionY + (myConfig.iBorderThickness/1.5));
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
		
	cairo_translate (myDrawContext, iOffset + myConfig.iTitlePositionX + (myConfig.iBorderThickness/1.5), atol(myData.cTitleFontSize) + iOffset + myConfig.iTitlePositionY + (myConfig.iBorderThickness/1.5));
	cairo_show_text (myDrawContext, myConfig.cName);
	cairo_fill (myDrawContext);
	cairo_restore (myDrawContext); // On restaure la position #3
	
	
	// On écrit les ligne du flux	
	cairo_translate (myDrawContext, iOffset + myConfig.iTextPositionX, iOffset + myConfig.iTextPositionY);
	if (myConfig.bDisplayLogo)
		cairo_translate (myDrawContext, 0. + (myConfig.iBorderThickness/1.5),  myData.fLogoSize + iOffset + (myConfig.iBorderThickness/1.5));
	else
		cairo_translate (myDrawContext, 0. + (myConfig.iBorderThickness/1.5),  atol(myData.cTitleFontSize) + iOffset + (myConfig.iBorderThickness/1.5));
	
	
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
		
	int i;
	for (i = 1 ; i < (myConfig.iLines+1) ; i++)
	{
		cairo_translate (myDrawContext, 0, atol(myData.cFontSize));
		
		if (i != 1)
			cairo_translate (myDrawContext, 0, myConfig.iSpaceBetweenLines);
		
		cairo_show_text (myDrawContext, myData.cFeedLine[i]);
		cairo_fill (myDrawContext);
		
	}	
	cairo_restore (myDrawContext); // On restaure la position #1
	
	// Bordure pour le Background (optionnel)
	if (myConfig.bDisplayBackground)
	{
		cairo_save (myDrawContext); // On sauvegarde la position #5
		cairo_set_source_rgba (myDrawContext, myConfig.fBorderColor[0], myConfig.fBorderColor[1], myConfig.fBorderColor[2], myConfig.fBorderColor[3]);
		cairo_set_line_width(myDrawContext, myConfig.iBorderThickness);
		cairo_translate (myDrawContext, myConfig.iBorderThickness/1.7, myConfig.iBorderThickness/1.7);  // Ne me demandez pas pourquoi ce /1.7 :/
		cairo_dock_draw_rounded_rectangle (myDrawContext,
					myConfig.iBackgroundRadius, 0.,
					iWidth - 2*myConfig.iBackgroundRadius - myConfig.iBorderThickness,
					iHeight- myConfig.iBorderThickness);
		cairo_stroke (myDrawContext);
		cairo_restore (myDrawContext); // On restaure la position #4
	}
		
	if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
	{
		CD_APPLET_FINISH_DRAWING_MY_ICON;
		cairo_dock_update_icon_texture (myIcon);		
	}	
}


void cd_applet_update_my_icon (CairoDockModuleInstance *myApplet, Icon *pIcon, CairoContainer *pContainer)
{
	if (myDesklet)
	{
		// taille de la texture.
		double fMaxScale = cairo_dock_get_max_scale (pContainer);
		double fRatio = pContainer->fRatio;
		int iWidth = (int) pIcon->fWidth / fRatio * fMaxScale;
		int iHeight = (int) pIcon->fHeight / fRatio * fMaxScale;
		
		if (CD_APPLET_MY_CONTAINER_IS_OPENGL)
		{
			if (! cairo_dock_begin_draw_icon (myIcon, myContainer))
				return ;
		}
		cd_applet_draw_my_desklet (myApplet, iWidth, iHeight);		
		
		CD_APPLET_REDRAW_MY_ICON;
	}	
}
