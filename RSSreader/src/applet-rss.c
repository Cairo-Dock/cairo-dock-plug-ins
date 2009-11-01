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

#include <math.h>
#include "applet-struct.h"
#include "applet-rss.h"

/* Insere des retours chariots dans une chaine de caracteres de facon à la faire tenir dans un rectangle donne.
 */
static double _cut_line (gchar *cLine, PangoLayout *pLayout, int iMaxWidth, int iMaxHeight)
{
	g_print ("%s (%s)\n", __func__, cLine);
	// on convertit les caracteres internet.
	gchar *str=cLine, *amp;
	do
	{
		amp = strchr (str, '&');
		if (!amp)
			break;
		if (amp[1] == '#' && g_ascii_isdigit (amp[2]) && g_ascii_isdigit (amp[3]) && g_ascii_isdigit (amp[4]) && amp[5] == ';')  // &#039;
		{
			*amp = atoi (amp+2);
			sprintf (amp+1, amp+6);
		}
	} while (1);
	
	// on insere des retours chariot pour tenir dans la largeur donnee.
	PangoRectangle ink, log;
	gchar *sp, *last_sp=NULL;
	double w, htotal=0., last_h;
	
	str = cLine;
	while (*str == ' ')  // on saute les espaces en debut de ligne.
		str ++;
	
	sp = str;
	do
	{
		sp = strchr (sp+1, ' ');  // on trouve l'espace suivant.
		if (!sp)  // plus d'espace, on quitte.
			break ;
		
		*sp = '\0';  // on coupe a cet espace.
		pango_layout_set_text (pLayout, str, -1);  // on regarde la taille de str a sp.
		pango_layout_get_pixel_extents (pLayout, &ink, &log);
		w = ink.width;
		
		if (w > iMaxWidth)  // on deborde.
		{
			if (last_sp != NULL)  // on coupe au dernier espace connu.
			{
				*sp = ' ';  // on remet l'espace.
				*last_sp = '\n';  // on coupe.
				htotal += last_h;
				str = last_sp + 1;  // on place le debut de ligne apres la coupure.
			}
			else  // aucun espace, c'est un mot entier.
			{
				*sp = '\n';  // on coupe apres le mot.
				htotal += ink.height;
				str = sp + 1;  // on place le debut de ligne apres la coupure.
			}
			while (*str == ' ')  // on saute les espaces en debut de ligne.
				str ++;
			sp = str;
			last_sp = NULL;
			last_h = 0.;
		}
		else  // ca rentre.
		{
			*sp = ' ';  // on remet l'espace.
			last_sp = sp;  // on memorise la derniere cesure qui fait tenir la ligne en largeur.
			last_h = ink.height;
			sp ++;  // on se place apres.
			while (*sp == ' ')  // on saute tous les espaces.
				sp ++;
		}
	} while (1);
	
	return htotal;
}

/* Decoupe un texte selon les retours chariots, et fait en sorte qu'il tienne dans l'icone.
 */
static gchar **_cut_text (CairoDockModuleInstance *myApplet, const gchar *cText, int iMaxWidth, int iMaxHeight)
{
	g_print ("%s (%dx%d)\n", __func__, iMaxWidth, iMaxHeight);
	PangoLayout *pLayout = pango_cairo_create_layout (myDrawContext);
	
	PangoFontDescription *fd = pango_font_description_from_string (myConfig.cFont);
	pango_layout_set_font_description (pLayout, fd);
	pango_font_description_free (fd);
	
	gchar **lines = g_strsplit (cText, "\n", -1);
	gchar *cLine;
	double fTextHeight, fLeftHeight = iMaxHeight;
	int i, j;
	for (i = 0; lines[i] != NULL; i ++)
	{
		cLine = lines[i];
		fTextHeight = _cut_line (cLine, pLayout, iMaxWidth, fLeftHeight);
		g_print (" + %.2f\n", fTextHeight);
		
		fLeftHeight -= fTextHeight;
		if (fLeftHeight < 0)
		{
			g_print ("on deborde en hauteur\n");
			/// add ...
			
			/// free left lines...
			int j;
			for (j = i+1; lines[j] != NULL; j ++)
			{
				g_free (lines[j]);
				lines[j] = NULL;
			}
			break ;
		}
	}
	
	g_object_unref (pLayout);
	return lines;
}


gchar **cd_rssreader_cut_text_for_icon (CairoDockModuleInstance *myApplet, const gchar *cText)
{
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	return _cut_text (myApplet, cText, iWidth, iHeight);
}

gchar **cd_rssreader_cut_text_for_dialog (CairoDockModuleInstance *myApplet, const gchar *cText)
{
	int iWidth, iHeight;
	iWidth = g_iScreenWidth[CAIRO_DOCK_HORIZONTAL] / 2;
	iHeight = g_iScreenHeight[CAIRO_DOCK_HORIZONTAL] / 2;
	return _cut_text (myApplet, cText, iWidth, iHeight);
}




