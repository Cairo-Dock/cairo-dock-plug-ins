#define _BSD_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "applet-amazon.h"
#include "applet-struct.h"

#define LICENCE_KEY "0C3430YZ2MVJKQ4JEKG2"
#define AMAZON_API_URL_1 "http://ecs.amazonaws.com/onca/xml?Service=AWSECommerceService&AWSAccessKeyId="
#define AMAZON_API_URL_2 "&AssociateTag=webservices-20&ResponseGroup=Images,ItemAttributes&Operation=ItemSearch&ItemSearch.Shared.SearchIndex=Music"

//static gchar *TAB_IMAGE_SIZES[2] = {"MediumImage", "LargeImage"};


/**
 * Parse le fichier XML passé en argument
 * à la recherche de l'URL de la pochette
 * @param filename URI du fichier à lire
 */
/*gchar *cd_extract_url_from_xml_file (const gchar *filename)
{
	const xmlChar *name, *value;
	int ret;
	xmlTextReaderPtr reader;
	gboolean flag = FALSE;
	gchar *cResult = NULL;

	reader = xmlReaderForFile(filename, NULL, 0);
	if (reader != NULL)
	{
		ret = xmlTextReaderRead(reader);
		while (ret == 1)  // on parcourt tous les noeuds.
		{
			// nom du noeud.
			name = xmlTextReaderConstName(reader);
			if (name == NULL)
				name = BAD_CAST "--";
			g_print (" node: %s\n", name);
			
			if (strcmp (name, TAB_IMAGE_SIZES[myConfig.iImagesSize]) == 0)
			{
				ret = xmlTextReaderRead(reader);
				name = xmlTextReaderConstName(reader);  // <URL>
				g_print (" final node: %s\n", name);
				
				// valeur associee.
				value = xmlTextReaderConstValue(reader);  // renvoit toujours NULL :-(
				g_print (" => value: %s\n", value);
				
				cResult = g_strdup (value);
				break ;
			}
			
			// on passe au suivant.
			ret = xmlTextReaderRead(reader);
		}
		xmlFreeTextReader(reader);
	}
	else
	{
		cd_warning ("Unable to open %s\n", filename);
	}
	xmlCleanupParser();
	return cResult;
}*/
gchar *cd_extract_url_from_xml_file (const gchar *filename, gchar **artist, gchar **album, gchar **title)
{
	gsize length = 0;
	gchar *cContent = NULL;
	g_file_get_contents (filename,
		&cContent,
		&length,
		NULL);
	g_return_val_if_fail (cContent != NULL, NULL);
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	g_print ("cover size : %d\n", iWidth);
	const gchar *cImageSize = (iWidth > 1 && iWidth < 64 ? "SmallImage" : (iWidth < 200 ? "MediumImage" : "LargeImage"));  // small size : 80; medium size : 160; large size : 320
	gchar *str = g_strstr_len (cContent, -1, cImageSize);
	gchar *cResult = NULL;
	if (str)
	{
		str = g_strstr_len (str, -1, "<URL>");
		if (str)
		{
			str += 5;
			gchar *str2 = g_strstr_len (str, -1, "</URL>");
			if (str2)
			{
				cResult = g_strndup (str, str2 - str);
			}
		}
	}
	if (artist != NULL && *artist == NULL)
	{
		str = g_strstr_len (cContent, -1, "<Artist>");
		if (str)
		{
			str += 8;
			gchar *str2 = g_strstr_len (str, -1, "</Artist>");
			if (str2)
			{
				*artist = g_strndup (str, str2 - str);
				g_print ("artist <- %s\n", *artist);
			}
		}
	}
	if (album != NULL && *album == NULL)
	{
		str = g_strstr_len (cContent, -1, "<Album>");
		if (str)
		{
			str += 7;
			gchar *str2 = g_strstr_len (str, -1, "</Album>");
			if (str2)
			{
				*album = g_strndup (str, str2 - str);
				g_print ("album <- %s\n", *album);
			}
		}
	}
	if ((album != NULL && *album == NULL) || (title != NULL && *title == NULL))
	{
		str = g_strstr_len (cContent, -1, "<Title>");
		if (str)
		{
			str += 7;
			gchar *str2 = g_strstr_len (str, -1, "</Title>");
			if (str2)
			{
				gchar *cTitle = g_strndup (str, str2 - str);
				if (album != NULL && *album == NULL)
				{
					str = strchr (cTitle, '/');
					if (str)
					{
						*album = g_strndup (cTitle, str - cTitle);
						g_print ("album <- %s\n", *album);
						if (title != NULL && *title == NULL)
							*title = g_strndup (str+1, str2 - str - 1);
						g_free (cTitle);
						cTitle = NULL;
					}
				}
				if (album != NULL && *album == NULL)
				{
					*album = cTitle;
					g_print ("album <- %s\n", *album);
				}
				else
					g_free (cTitle);
			}
		}
	}
	g_free (cContent);
	return cResult;
}

gchar *cd_get_xml_file (const gchar *artist, const gchar *album, const gchar *cUri)
{
	g_return_val_if_fail ((artist != NULL && album != NULL) || (cUri != NULL), FALSE);
	
	gchar *cFileToDownload;
	
	if (artist != NULL && album != NULL)
	{
		gchar *cKeyWord = g_strdup (album);
		gchar *str = cKeyWord;
		for (str = cKeyWord; *str != '\0'; str ++)
		{
			if (*str == ' ')
			{
				*str = '|';
				while (*str == ' ')
					str ++;
			}
			if (*str == '.')
			{
				gchar *ptr;
				for (ptr = str; *ptr != '\0'; ptr ++)
					*ptr = *(ptr+1);
			}
		}
		cFileToDownload = g_strdup_printf ("%s%s%s&Artist=%s&Keywords=%s",
			AMAZON_API_URL_1,
			LICENCE_KEY,
			AMAZON_API_URL_2,
			artist,
			cKeyWord);
		g_free (cKeyWord);
	}
	else
	{
		g_print ("cUri : '%s'\n", cUri);
		gchar *cKeyWord;
		if (*cUri == '/')
		{
			cKeyWord = g_path_get_basename (cUri);
		}
		else
		{
			gchar *cPath = g_filename_from_uri (cUri, NULL, NULL);
			cKeyWord = g_path_get_basename (cPath);
			g_free (cPath);
		}
		g_return_val_if_fail (cKeyWord != NULL, NULL);
		gchar *str = strrchr (cKeyWord, '.');
		if (str)
			*str = '\0';
		g_strdelimit (cKeyWord, "-_~", '|');
		gchar **words = g_strsplit (cKeyWord, "|", -1);
		int i;
		GString *s = g_string_new ("");
		if (words)
		{
			for (i = 0; words[i] != NULL; i ++)
			{
				g_string_append_printf (s, "\"%s\"|", words[i]);
			}
			g_strfreev (words);
		}
		cFileToDownload = g_strdup_printf ("%s%s%s&Keywords=%s",
			AMAZON_API_URL_1,
			LICENCE_KEY,
			AMAZON_API_URL_2,
			s->str);
		g_free (cKeyWord);
		g_string_free (s, TRUE);
	}
	gchar *cTmpFilePath = g_strdup ("/tmp/amazon-cover.XXXXXX");
	int fds = mkstemp (cTmpFilePath);
	if (fds == -1)
	{
		g_free (cTmpFilePath);
		return NULL;
	}
	
	gchar *cCommand = g_strdup_printf ("wget \"%s\" -O \"%s\" -t 3 -T 4 > /dev/null 2>&1", cFileToDownload, cTmpFilePath);
	g_print ("%s\n",cCommand);
	cairo_dock_launch_command (cCommand);
	
	g_free (cCommand);
	g_free (cFileToDownload);
	close(fds);
	return cTmpFilePath;
}

void cd_download_missing_cover (const gchar *cURL)
{
	if (cURL == NULL)
		return ;
	g_return_if_fail (myData.cCoverPath != NULL);
	if (! g_file_test (myData.cCoverPath, G_FILE_TEST_EXISTS))
	{
		gchar *cCommand = g_strdup_printf ("wget \"%s\" -O \"%s\" -t 2 -T 5 > /dev/null 2>&1", cURL, myData.cCoverPath);
		g_print ("%s\n",cCommand);
		cairo_dock_launch_command (cCommand);
		g_free (cCommand);
	}
}
