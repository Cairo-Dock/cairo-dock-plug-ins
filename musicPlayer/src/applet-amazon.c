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

#define _BSD_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <glib/gstdio.h>
#include "applet-amazon.h"
#include "applet-struct.h"

/*
 * For the detailed example we'll use a typical ItemLookup request:

http://webservices.amazon.com/onca/xml?Service=AWSECommerceServic
e&AWSAccessKeyId=00000000000000000000&Operation=ItemLookup&ItemId
=0679722769&ResponseGroup=ItemAttributes,Offers,Images,Reviews&Ve
rsion=2009-01-06 

Steps to Sign the Example Request

   1.
      Enter the timestamp. For this example, we'll use GMT time of 2009-01-01T12:00:00Z (%F%T%z)
      This is a fixed -length subset of the format defined by ISO 8601, represented in Universal Time (GMT): YYYY-MM-DDThh:mm:ssZ (where T and Z are literals).

      http://webservices.amazon.com/onca/xml?Service=AWSECommerceServic
      e&AWSAccessKeyId=00000000000000000000&Operation=ItemLookup&ItemId
      =0679722769&ResponseGroup=ItemAttributes,Offers,Images,Reviews&Ve
      rsion=2009-01-06&Timestamp=2009-01-01T12:00:00Z

   2.
      URL encode the request's comma (,) and colon (;) characters, so that they don't get misinterpreted. For more information about converting to RFC 3986 specifications, see documentation and code samples for your programming language.

      http://webservices.amazon.com/onca/xml?Service=AWSECommerceServic
      e&AWSAccessKeyId=00000000000000000000&Operation=ItemLookup&ItemId
      =0679722769&ResponseGroup=ItemAttributes%2COffers%2CImages%2CRevi
      ews&Version=2009-01-06&Timestamp=2009-01-01T12%3A00%3A00Z

      [Important] Important
      Be sure that you do not double-escape any characters.

   3.
      Split the parameter/value pairs and delete the ampersand characters (&) so that the example looks like the following:

      Service=AWSECommerceService
      AWSAccessKeyId=00000000000000000000
      Operation=ItemLookup
      ItemId=0679722769
      ResponseGroup=ItemAttributes%2COffers%2CImages%2CReviews
      Version=2009-01-06
      Timestamp=2009-01-01T12%3A00%3A00Z

   4.
      Sort your parameter/value pairs by byte value (not alphabetically, lowercase parameters will be listed after uppercase ones).

      AWSAccessKeyId=00000000000000000000
      ItemId=0679722769
      Operation=ItemLookup
      ResponseGroup=ItemAttributes%2COffers%2CImages%2CReviews
      Service=AWSECommerceService
      Timestamp=2009-01-01T12%3A00%3A00Z
      Version=2009-01-06

   5.
      Rejoin the sorted parameter/value list with ampersands. The result is the canonical string that we'll sign:

      AWSAccessKeyId=00000000000000000000&ItemId=0679722769&Operation=I
      temLookup&ResponseGroup=ItemAttributes%2COffers%2CImages%2CReview
      s&Service=AWSECommerceService&Timestamp=2009-01-01T12%3A00%3A00Z&
      Version=2009-01-06

   6.
      Prepend the following three lines (with line breaks) before the canonical string:

      GET
      webservices.amazon.com
      /onca/xml

   7.
      The string to sign:

      GET
      webservices.amazon.com
      /onca/xml
      AWSAccessKeyId=00000000000000000000&ItemId=0679722769&Operation=I
      temLookup&ResponseGroup=ItemAttributes%2COffers%2CImages%2CReview
      s&Service=AWSECommerceService&Timestamp=2009-01-01T12%3A00%3A00Z&
      Version=2009-01-06

   8.
      Calculate an RFC 2104-compliant HMAC with the SHA256 hash algorithm using the string above with our "dummy" Secret Access Key: 1234567890. For more information about this step, see documentation and code samples for your programming language.

      Nace+U3Az4OhN7tISqgs1vdLBHBEijWcBeCqL5xN9xg=

   9.
      URL encode the plus (+) and equal (=) characters in the signature:

      Nace%2BU3Az4OhN7tISqgs1vdLBHBEijWcBeCqL5xN9xg%3D

  10.
      Add the URL encoded signature to your request and the result is a properly-formatted signed request:

      http://webservices.amazon.com/onca/xml?AWSAccessKeyId=00000000000
      000000000&ItemId=0679722769&Operation=ItemLookup&ResponseGroup=It
      emAttributes%2COffers%2CImages%2CReviews&Service=AWSECommerceServ
      ice&Timestamp=2009-01-01T12%3A00%3A00Z&Version=2009-01-06&Signatu
      re=Nace%2BU3Az4OhN7tISqgs1vdLBHBEijWcBeCqL5xN9xg%3D
*/
#define BASE_URL "http://webservices.amazon.com/onca/xml"
#define HEADER "GET\nwebservices.amazon.com\n/onca/xml\n"
#define REQUEST "AWSAccessKeyId=%s&AssociateTag=none&Keywords=%s&Operation=ItemSearch&ResponseGroup=ItemAttributes%%2CImages&SearchIndex=Music&Service=AWSECommerceService&Timestamp=%s&Version=2009-01-06"  // 'AssociateTag' is requested
#define LICENCE_KEY "AKIAIAW2QBGIHVG4UIKA"  // if you reuse the code below, please take the time to make your own key !
#define PRIVATE_KEY "j7cHTob2EJllKGDScXCvuzTB108WDPpIUnVQTC4P"  // please do not use this key ! It is reserved for Cairo-Dock.

static gchar *_hmac_crypt (const gchar *text, const gchar* key, GChecksumType iType)
{
	cd_debug ("%s (%s)\n", __func__, text);
	unsigned char k_ipad[65];    // inner padding - key XORd with ipad
	unsigned char k_opad[65];    // outer padding - key XORd with opad
	
	// if key is longer than 64 bytes reset it to key=MD5(key)
	int key_len = strlen (key);
	gchar *tmp_key = NULL;
	if (key_len > 64)
	{
		tmp_key = g_compute_checksum_for_string (iType, key, -1);
		key = tmp_key;
		key_len = strlen (key);  // 16
	}
	cd_debug ("  key_len:%d bytes\n", key_len);

	/* the HMAC_MD5 transform looks like:
	*
	* MD5(K XOR opad, MD5(K XOR ipad, text))
	*
	* where K is an n byte key
	* ipad is the byte 0x36 repeated 64 times
	* opad is the byte 0x5c repeated 64 times
	* and text is the data being protected */

	// start out by storing key in pads
	memset ( k_ipad, 0, sizeof k_ipad);
	memset ( k_opad, 0, sizeof k_opad);
	memcpy ( k_ipad, key, key_len);
	memcpy ( k_opad, key, key_len);

	// XOR key with ipad and opad values
	int i;
	for (i=0; i<64; i++)
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}
	
	// perform inner MD5
	GChecksum *cs = g_checksum_new (iType);
	g_checksum_update (cs, k_ipad, 64);
	g_checksum_update (cs, text, -1);
	guint8 inner_digest[65];
	gsize digest_len=64;
	g_checksum_get_digest (cs, inner_digest, &digest_len);
	
	// perform outer MD5
#if (GLIB_MAJOR_VERSION > 2) || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 18)
	g_checksum_reset (cs);
#else
	g_checksum_free (cs);
	cs = g_checksum_new (iType);
#endif
	g_checksum_update (cs, k_opad, 64);
	g_checksum_update (cs, inner_digest, digest_len);  // 16
	
	//gchar *cDigest = g_strdup (g_checksum_get_string (cs));
	g_checksum_get_digest (cs, inner_digest, &digest_len);
	gchar *cDigest = g_base64_encode (inner_digest, digest_len);
	
	g_checksum_free (cs);
	g_free (tmp_key);
	return cDigest;
}

/* reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" | "$" | ","
 * unreserved  = alphanum | mark
      mark        = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
*/
static gchar *_url_encode (const gchar * str)
{
	g_return_val_if_fail (str != NULL, NULL);
	cd_debug ("%s (%s)\n", __func__, str);
	const gchar * s = str;
	gchar * t = NULL;
	gchar * ret;
	const gchar * validChars = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_.!~*'()";
	gchar * isValidChar;
	int lenght = 0;
	// calcul de la taille de la chaine urlEncodée
	do{
		isValidChar = strchr(validChars, *s); // caractère valide?
		if(!isValidChar)
			lenght+=3; // %xx : 3 caractères
		else
			lenght++;  // sinon un seul
	}while(*++s); // avance d'un cran dans la chaine. Si on est pas à la fin, on continue...
	s = str;
	/// la ligne suivane peut planter ...
	cd_debug ("allocation of %d bytes...\n", lenght + 1);
	t = g_new (gchar, 4*(lenght + 1)); // Allocation à la bonne taille
	ret = t;
	//encodage
	do{
		isValidChar = strchr(validChars, *s);
		if(!isValidChar)
			sprintf(t, "%%%2X", *s), t+=3;
		else
			sprintf(t, "%c", *s), t++;
	}while(*++s);
	*t = 0; // 0 final
	return ret;
}

static gchar *_compute_request_and_signature (const gchar *cKeyWords, gchar **cSignature)
{
	time_t t = time (NULL);  // %F%T%z
	struct tm currentTime;
	localtime_r (&t, &currentTime);
	gchar cTimeStamp[50+1];
	strftime (cTimeStamp, 50, "%FT%T%z", &currentTime);
	//cd_debug ("timestamp : %s\n", cTimeStamp);
	
	gchar *keywords = _url_encode(cKeyWords);
	cd_debug ("keywords : '%s'\n", keywords);
	gchar *time = _url_encode (cTimeStamp);
	gchar *cRequest = g_strdup_printf (REQUEST, LICENCE_KEY, keywords, time);
	g_free (keywords);  // plante avec (Renan Luce, On N'Est Pas À Une Bêtise Près)
	g_free (time);
	cd_debug ("cRequest : '%s'\n", cRequest);
	//gchar *cRequest = g_strdup_printf (REQUEST, (cArtist), LICENCE_KEY, (cKeyWords), _url_encode (cTimeStamp));
	
	gchar *cBuffer = g_strconcat (HEADER, cRequest, NULL);
	
	*cSignature = _hmac_crypt (cBuffer, PRIVATE_KEY, G_CHECKSUM_SHA256);
	cd_debug ("cSignature : '%s'", *cSignature);
	
	g_free (cBuffer);
	return cRequest;
}

static gchar *_make_keywords (const gchar *artist, const gchar *album, const gchar *cUri)
{
	gchar *cKeyWords = NULL;
	if (artist != NULL && album != NULL)
	{
		cKeyWords = g_strdup_printf ("%s,%s", artist, album);
		g_strdelimit (cKeyWords, "-_~", ' ');
	}
	else  // on essaie de se baser sur le nom du fichier.
	{
		if (*cUri == '/')
		{
			cKeyWords = g_path_get_basename (cUri);
		}
		else
		{
			gchar *cPath = g_filename_from_uri (cUri, NULL, NULL);
			cKeyWords = g_path_get_basename (cPath);
			g_free (cPath);
		}
		g_return_val_if_fail (cKeyWords != NULL, NULL);
		gchar *str = strrchr (cKeyWords, '.');
		if (str)
			*str = '\0';
		g_strdelimit (cKeyWords, "-_~", ' ');
	}
	cd_debug ("cKeyWords : '%s'\n", cKeyWords);
	return cKeyWords;
}

static gchar *_build_url (const gchar *cArtist, const gchar *cAlbum, const gchar *cUri)
{
	cd_debug ("%s (%s; %s; %s)\n", __func__, cArtist,cAlbum, cUri);
	gchar *cKeyWords = _make_keywords (cArtist, cAlbum, cUri);
	
	gchar *cSignature = NULL;
	gchar *cRequest = _compute_request_and_signature (cKeyWords, &cSignature);
	
	gchar *cUrl = g_strdup_printf ("%s?%s&Signature=%s", BASE_URL, cRequest, _url_encode (cSignature));
	cd_debug ("==> URL : %s\n", cUrl);
	
	g_free (cKeyWords);
	g_free (cSignature);
	g_free (cRequest);
	return cUrl;
}

gboolean cd_amazon_dl_cover (const gchar *artist, const gchar *album, const gchar *cUri, const gchar *cLocalPath)
{
	g_return_val_if_fail ((artist != NULL && album != NULL) || (cUri != NULL), FALSE);
	
	// build the request to the Amazon Web Service.
	gchar *cRequest = _build_url (artist, album, cUri);
	
	// make the request and wait for the answer.
	GError *error = NULL;
	gchar *cXmlData =  cairo_dock_get_url_data (cRequest, &error);
	g_free (cRequest);
	if (error)
	{
		cd_warning ("while trying to get data from Amazon about %s/%s/%s: %s", artist, album, cUri, error->message);
		g_error_free (error);
		return FALSE;
	}
	if (cXmlData == NULL)
	{
		cd_message ("no data from Amazon");
		return FALSE;
	}
	
	// extract the URL of the cover from the data we got.
	// Note: the XML data also contains various info, but since we only get it when we dl the cover, and since the cover will stay in the cache, we won't come here again, so there is no point in getting these info only once.
	const gchar *cCoverURL = NULL;
	int iWidth, iHeight;
	CD_APPLET_GET_MY_ICON_EXTENT (&iWidth, &iHeight);
	const gchar *cImageSize = (iWidth > 1 && iWidth <= 80 ? "SmallImage" : (iWidth <= 160 ? "MediumImage" : "LargeImage"));  // small size : 80; medium size : 160; large size : 320
	gchar *str = g_strstr_len (cXmlData, -1, cImageSize);
	if (str)
	{
		str = g_strstr_len (str, -1, "<URL>");
		if (str)
		{
			str += 5;
			gchar *str2 = g_strstr_len (str, -1, "</URL>");
			if (str2)
			{
				*str2 = '\0';
				cCoverURL = str;
			}
		}
	}
	
	if (cCoverURL == NULL)  // it's probably an answer stating that there is no result, ignore silently.
	{
		g_free (cXmlData);
		return FALSE;
	}
	
	// now download the image
	if (!cairo_dock_download_file (cCoverURL, cLocalPath))
	{
		cd_warning ("couldn't downoad the image from Amazon about %s/%s/%s", artist, album, cUri);
		g_free (cXmlData);
		return FALSE;
	}
	
	g_free (cXmlData);
	return TRUE;
}
