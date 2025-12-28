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

/******************************************************************************
exemples : 
----------

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/my_applet org.cairodock.CairoDock.applet.SetLabel string:new_label

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/demo  org.cairodock.CairoDock.applet.AddDataRenderer string:gauge int32:2 string:Turbo-night-fuel

dbus-send --session --dest=org.cairodock.CairoDock /org/cairodock/CairoDock/demo  org.cairodock.CairoDock.applet.RenderValues array:double:.7,.2

******************************************************************************/

#include <math.h>
#include <glib.h>
#include <implementations/cairo-dock-wayland-manager.h> // gldi_wayland_manager_have_layer_shell

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"


static inline gboolean _get_icon_and_container_from_id (DBusAppletData *pDbusApplet, const gchar *cIconID, Icon **pIcon, GldiContainer **pContainer)
{
	GldiModuleInstance *pInstance = pDbusApplet->pModuleInstance;
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	if (cIconID == NULL)
	{
		*pIcon = pInstance->pIcon;
		*pContainer = pInstance->pContainer;
	}
	else
	{
		GList *piconsList = (pInstance->pDock ? (pInstance->pIcon->pSubDock ? pInstance->pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		*pIcon = cairo_dock_get_icon_with_command (piconsList, cIconID);
		*pContainer = (pInstance->pDesklet ? CAIRO_CONTAINER (pInstance->pDesklet) : CAIRO_CONTAINER (pInstance->pIcon->pSubDock));
	}
	g_return_val_if_fail (pIcon != NULL && pContainer != NULL, FALSE);
	return TRUE;
}

static inline int _get_container_type (GldiContainer *pContainer)
{
	int iType = -1;
	
	if (CAIRO_DOCK_IS_DOCK (GLDI_OBJECT(pContainer)))
		return 0;
	if (CAIRO_DOCK_IS_DESKLET (pContainer))
		return 1;
	if (CAIRO_DOCK_IS_DIALOG (pContainer))
		return 2;
	if (CAIRO_DOCK_IS_FLYING_CONTAINER (pContainer))
		return 3;
	
	return iType;
}


static gboolean _applet_set_quick_info (DBusAppletData *pDbusApplet, const gchar *cQuickInfo, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	gldi_icon_set_quick_info (pIcon, cQuickInfo && *cQuickInfo != '\0' ? cQuickInfo : NULL);
	cairo_dock_redraw_icon (pIcon);
	return TRUE;
}

static gboolean _applet_set_label (DBusAppletData *pDbusApplet, const gchar *cLabel, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	gldi_icon_set_name (pIcon, cLabel);
	cairo_dock_redraw_icon (pIcon);  /// needs a function to redraw the label...
	return TRUE;
}

static gboolean _applet_set_icon (DBusAppletData *pDbusApplet, const gchar *cImage, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->image.pSurface != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->image.pSurface);
	cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon);
	return TRUE;
}

/* Not used
static gboolean _applet_set_icon_with_default (dbusApplet *pDbusApplet, const gchar **cImages, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->image.pSurface != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->image.pSurface);
	int i;
	for (i = 0; cImages[i] != NULL; i ++)
	{
		
		///cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	}
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon);
	return TRUE;
}
*/
static gboolean _applet_set_emblem (DBusAppletData *pDbusApplet, const gchar *cImage, gint iPosition, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (cImage == NULL || *cImage == '\0' || strcmp (cImage, "none") == 0)
	{
		cairo_dock_remove_overlay_at_position (pIcon, iPosition < CAIRO_OVERLAY_NB_POSITIONS ? iPosition : iPosition - CAIRO_OVERLAY_NB_POSITIONS, myApplet);  // for ease of use, handle both case similarily.
	}
	else
	{
		if (iPosition >= CAIRO_OVERLAY_NB_POSITIONS)  // [N; 2N-1] => print the overlay
			cairo_dock_print_overlay_on_icon_from_image (pIcon, cImage, iPosition - CAIRO_OVERLAY_NB_POSITIONS);
		else  // [0, N-1] => add it
			cairo_dock_add_overlay_from_image (pIcon, cImage, iPosition, myApplet);  // use 'myApplet' to identify the overlays set by the Dbus plug-in (since the plug-in can't be deactivated, 'myApplet' is constant).
	}
	
	cairo_dock_redraw_icon (pIcon);
	return TRUE;
}

static gboolean _applet_animate (DBusAppletData *pDbusApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (CAIRO_DOCK_IS_DOCK (pContainer) && cAnimation != NULL)
	{
		gldi_icon_request_animation (pIcon, cAnimation, iNbRounds);
		return TRUE;
	}
	return FALSE;
}

static gboolean _applet_show_dialog (DBusAppletData *pDbusApplet, const gchar *message, gint iDuration, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	// On empeche l'accumulation de dialogues informatifs.
	gldi_dialogs_remove_on_icon (pIcon);
	
	gldi_dialog_show_temporary_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
	return TRUE;
}

static void _on_dialog_destroyed (DBusAppletData *pDbusApplet)
{
	CD_APPLET_ENTER;
	pDbusApplet->pDialog = NULL;
	CD_APPLET_LEAVE();
}

// deprecated
static gboolean _applet_ask_question (DBusAppletData *pDbusApplet, const gchar *cMessage, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		gldi_object_unref (GLDI_OBJECT(pDbusApplet->pDialog));
	pDbusApplet->pDialog = gldi_dialog_show_with_question (cMessage, pIcon, pContainer, "same icon", (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_question, pDbusApplet, (GFreeFunc)_on_dialog_destroyed);
	return TRUE;
}

static gboolean _applet_ask_value (DBusAppletData *pDbusApplet, const gchar *cMessage, gdouble fInitialValue, gdouble fMaxValue, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		gldi_object_unref (GLDI_OBJECT(pDbusApplet->pDialog));
	pDbusApplet->pDialog = gldi_dialog_show_with_value (cMessage, pIcon, pContainer, "same icon", fInitialValue, fMaxValue, (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_value, pDbusApplet, (GFreeFunc)_on_dialog_destroyed);
	return TRUE;
}

static gboolean _applet_ask_text (DBusAppletData *pDbusApplet, const gchar *cMessage, const gchar *cInitialText, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		gldi_object_unref (GLDI_OBJECT(pDbusApplet->pDialog));
	pDbusApplet->pDialog = gldi_dialog_show_with_entry (cMessage, pIcon, pContainer, "same icon", cInitialText, (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_text, pDbusApplet, (GFreeFunc)_on_dialog_destroyed);
	return TRUE;
}
// end of deprecated

static void _on_text_changed (GtkWidget *pEntry, GtkWidget *pLabel)
{
	int iNbChars;
	if (GTK_IS_ENTRY (pEntry))
	{
		const gchar *cText = gtk_entry_get_text (GTK_ENTRY (pEntry));
		iNbChars = (cText ? strlen (cText) : 0);
	}
	else
	{
		GtkTextBuffer *pBuffer = GTK_TEXT_BUFFER (pEntry);
		iNbChars = gtk_text_buffer_get_char_count (pBuffer);
	}
	
	int iNbCharsMax = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (pEntry), "nb-chars-max"));
	
	gchar *cLabel;
	if (iNbChars < iNbCharsMax)
		cLabel = g_strdup_printf ("<b>%d</b>", iNbChars);
	else
		cLabel = g_strdup_printf ("<span color=\"red\"><b>%d</b></span>", iNbChars);
	gtk_label_set_markup (GTK_LABEL (pLabel), cLabel);
	gldi_dialog_set_widget_text_color (pLabel);
	g_free (cLabel);
}

static gboolean _applet_popup_dialog (DBusAppletData *pDbusApplet, GVariant *pDialogAttr, GVariant *pWidgetAttr, const gchar *cIconID)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		gldi_object_unref (GLDI_OBJECT(pDbusApplet->pDialog));
	
	
	GVariantDict *hDialogAttributes = g_variant_dict_new (pDialogAttr);
	GVariantDict *hWidgetAttributes = (g_variant_n_children (pWidgetAttr) > 0) ? g_variant_dict_new (pWidgetAttr) : NULL;
	
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	
	// attributs du dialogue.
	const gchar *cImageFilePath = NULL;
	if (g_variant_dict_lookup (hDialogAttributes, "icon", "&s", &cImageFilePath))
	{
		int w, h;
		cairo_dock_get_icon_extent (pIcon, &w, &h);
		cImageFilePath = cairo_dock_search_icon_s_path (cImageFilePath, MAX (w, h));
		attr.cImageFilePath = cImageFilePath;
	}
	else
		attr.cImageFilePath = "same icon";
	
	g_variant_dict_lookup (hDialogAttributes, "message", "&s", &attr.cText);
	if (g_variant_dict_lookup (hDialogAttributes, "time-length", "&i", &attr.iTimeLength))
		attr.iTimeLength *= 1000;
	
	gchar **cButtonsImage = NULL;
	const char *tmp = NULL;
	if (g_variant_dict_lookup (hDialogAttributes, "buttons", "&s", &tmp))
	{
		cButtonsImage = g_strsplit (tmp, ";", -1);  // NULL-terminated
		attr.cButtonsImage = (const gchar **)cButtonsImage;
	}
	
	g_variant_dict_lookup (hDialogAttributes, "force-above", "b", &attr.bForceAbove);
	g_variant_dict_lookup (hDialogAttributes, "use-markup", "b", &attr.bUseMarkup);
	
	attr.pUserData = pDbusApplet;
	attr.pFreeDataFunc = (GFreeFunc)_on_dialog_destroyed;
	
	// attributs du widget interactif.
	GtkWidget *pInteractiveWidget = NULL, *pOneWidget = NULL;
	if (hWidgetAttributes != NULL)  // un widget d'interaction est defini.
	{
		const gchar *cType;
		if (g_variant_dict_lookup (hWidgetAttributes, "widget-type", "&s", &cType))
		{
			if (strcmp (cType, "text-entry") == 0)
			{
				gboolean bMultiLines = FALSE;
				gboolean bEditable = TRUE;
				gboolean bVisible = TRUE;
				int iNbCharsMax = 0;
				const gchar *cInitialText = NULL;
				
				g_variant_dict_lookup (hDialogAttributes, "multi-lines", "b", &bMultiLines);
				g_variant_dict_lookup (hDialogAttributes, "editable", "b", &bEditable);
				g_variant_dict_lookup (hDialogAttributes, "visible", "b", &bVisible);
				g_variant_dict_lookup (hDialogAttributes, "nb-chars", "i", &iNbCharsMax);
				g_variant_dict_lookup (hDialogAttributes, "initial-value", "&s", &cInitialText);
				
				if (bMultiLines)
				{
					pOneWidget = gtk_text_view_new ();
					GtkWidget *pScrolledWindow = gtk_scrolled_window_new (NULL, NULL);
					gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (pScrolledWindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
					gtk_container_add (GTK_CONTAINER (pScrolledWindow), pOneWidget);
					g_object_set (pScrolledWindow, "width-request", 230, "height-request", 130, NULL);
					pInteractiveWidget = pScrolledWindow;
					
					if (! bEditable)
						gtk_text_view_set_editable (GTK_TEXT_VIEW (pOneWidget), FALSE);
					
					if (cInitialText != NULL)
					{
						GtkTextBuffer *pBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pOneWidget));
						gtk_text_buffer_set_text (pBuffer, cInitialText, -1);
					}
					
					if (attr.cButtonsImage != NULL)
						attr.pActionFunc = (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_text_view;
				}
				else
				{
					pOneWidget = gtk_entry_new ();
					pInteractiveWidget = pOneWidget;
					gtk_entry_set_has_frame (GTK_ENTRY (pOneWidget), FALSE);
					g_object_set (pOneWidget, "width-request", CAIRO_DIALOG_MIN_ENTRY_WIDTH, NULL);
					if (cInitialText != NULL)
						gtk_entry_set_text (GTK_ENTRY (pOneWidget), cInitialText);
					if (! bEditable)
						gtk_editable_set_editable (GTK_EDITABLE (pOneWidget), FALSE);
					if (! bVisible)
						gtk_entry_set_visibility (GTK_ENTRY (pOneWidget), FALSE);
					
					if (attr.cButtonsImage != NULL)
						attr.pActionFunc = (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_text_entry;
				}
				if (iNbCharsMax != 0)
				{
					gchar *cLabel = g_strdup_printf ("<b>%zd</b>", cInitialText ? strlen (cInitialText) : 0);
					GtkWidget *pLabel = gtk_label_new (cLabel);
					g_free (cLabel);
					gtk_label_set_use_markup (GTK_LABEL (pLabel), TRUE);
					GtkWidget *pBox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 3);
					gtk_box_pack_start (GTK_BOX (pBox), pInteractiveWidget, TRUE, TRUE, 0);
					gtk_box_pack_start (GTK_BOX (pBox), pLabel, FALSE, FALSE, 0);
					pInteractiveWidget = pBox;
					
					if (bMultiLines)
					{
						GtkTextBuffer *pBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pOneWidget));
						g_signal_connect (pBuffer, "changed", G_CALLBACK (_on_text_changed), pLabel);
						g_object_set_data (G_OBJECT (pBuffer), "nb-chars-max", GINT_TO_POINTER (iNbCharsMax));
					}
					else
					{
						g_signal_connect (pOneWidget, "changed", G_CALLBACK (_on_text_changed), pLabel);
						g_object_set_data (G_OBJECT (pOneWidget), "nb-chars-max", GINT_TO_POINTER (iNbCharsMax));
						gtk_entry_set_width_chars (GTK_ENTRY (pOneWidget), MIN (iNbCharsMax/2, 100));  // a rough estimate is: 140 chars ~ 1024 pixels
					}
				}
			}
			else if (strcmp (cType, "scale") == 0)
			{
				GtkWidget *pScale = NULL;
				double fMinValue = 0.;
				double fMaxValue = 100.;
				int iNbDigit = 2;
				double fInitialValue = 0.;
				const gchar *cMinLabel = NULL;
				const gchar *cMaxLabel = NULL;
				
				g_variant_dict_lookup (hWidgetAttributes, "min-value", "d", &fMinValue);
				g_variant_dict_lookup (hWidgetAttributes, "max-value", "d", &fMaxValue);
				fMaxValue = MAX (fMaxValue, fMinValue+1);
				g_variant_dict_lookup (hWidgetAttributes, "nb-digit", "i", &iNbDigit);
				g_variant_dict_lookup (hWidgetAttributes, "initial-value", "d", &fInitialValue);
				fInitialValue = MAX (MIN (fInitialValue, fMaxValue), fMinValue);
				g_variant_dict_lookup (hWidgetAttributes, "min-label", "&s", &cMinLabel);
				g_variant_dict_lookup (hWidgetAttributes, "max-label", "&s", &cMaxLabel);
				
				pScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, fMinValue, fMaxValue, (fMaxValue - fMinValue) / 100.);
				pOneWidget = pScale;
				gtk_scale_set_digits (GTK_SCALE (pScale), iNbDigit);
				gtk_range_set_value (GTK_RANGE (pScale), fInitialValue);
				
				g_object_set (pScale, "width-request", 150, NULL);
				gldi_dialog_set_widget_text_color (pScale);
				
				if (cMinLabel || cMaxLabel)
				{
					GtkWidget *pExtendedWidget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
					GtkWidget *label = gtk_label_new (cMinLabel);
					gtk_box_pack_start (GTK_BOX (pExtendedWidget), label, FALSE, FALSE, 0);
					gtk_box_pack_start (GTK_BOX (pExtendedWidget), pScale, FALSE, FALSE, 0);
					label = gtk_label_new (cMaxLabel);
					gtk_box_pack_start (GTK_BOX (pExtendedWidget), label, FALSE, FALSE, 0);
					pInteractiveWidget = pExtendedWidget;
				}
				else
					pInteractiveWidget = pScale;
				
				if (attr.cButtonsImage != NULL)
					attr.pActionFunc = (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_scale;
			}
			else if (strcmp (cType, "list") == 0)
			{
				gboolean bEditable = FALSE;
				const gchar *cValues = NULL;
				gchar **cValuesList = NULL;
				const gchar *cInitialText = NULL;
				int iInitialValue = 0;

				g_variant_dict_lookup (hWidgetAttributes, "editable", "b", &bEditable);
				g_variant_dict_lookup (hWidgetAttributes, "value", "&s", &cValues);
				if (cValues != NULL)
					cValuesList = g_strsplit (cValues, ";", -1);

				if (bEditable)
					pOneWidget = gtk_combo_box_text_new_with_entry ();
				else
					pOneWidget = gtk_combo_box_text_new ();
				pInteractiveWidget = pOneWidget;

				if (cValuesList != NULL)
				{
					int i;
					for (i = 0; cValuesList[i] != NULL; i ++)
					{
						gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pInteractiveWidget), cValuesList[i]);
					}
				}
				
				
				if (bEditable)
				{
					g_variant_dict_lookup (hWidgetAttributes, "initial-value", "&s", &cInitialText);
					if (cInitialText != NULL)
					{
						GtkWidget *pEntry = gtk_bin_get_child (GTK_BIN (pInteractiveWidget));
						gtk_entry_set_text (GTK_ENTRY (pEntry), cInitialText);
					}
				}
				else
				{
					g_variant_dict_lookup (hWidgetAttributes, "initial-value", "i", &iInitialValue);
					gtk_combo_box_set_active (GTK_COMBO_BOX (pInteractiveWidget), iInitialValue);
				}
				
				if (attr.cButtonsImage != NULL)
				{
					if (bEditable)
						attr.pActionFunc = (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_combo_entry;
					else
						attr.pActionFunc = (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_combo;
				}
			}
			else
				cd_warning ("unknown widget type '%s'", cType);
		}
	}
	attr.pInteractiveWidget = pInteractiveWidget;
	
	if (pInteractiveWidget == NULL)  // pas de widget, on renverra le numero du bouton appuye.
	{
		if (attr.cButtonsImage != NULL)
			attr.pActionFunc = (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_buttons;
	}
	else
		g_object_set_data (G_OBJECT (pInteractiveWidget), "cd-widget", pOneWidget);
	
	attr.pIcon = pIcon;
	attr.pContainer = pContainer;
	
	pDbusApplet->pDialog = gldi_dialog_new (&attr);
	if (pOneWidget)
		gtk_widget_grab_focus (pOneWidget);
	
	if (cButtonsImage)
		g_strfreev (cButtonsImage);
	
	g_variant_dict_unref (hDialogAttributes);
	if (hWidgetAttributes) g_variant_dict_unref (hWidgetAttributes);
	
	return TRUE;
}


  ///////////////////////////////////////////////////
 ////////// sub-applet interface methods ///////////
///////////////////////////////////////////////////

static void _sub_applet_set_quick_info (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cQuickInfo, *cIconID;
	g_variant_get (pPar, "(&s&s)", &cQuickInfo, &cIconID);
	if (_applet_set_quick_info (pApplet, cQuickInfo, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_set_label (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cLabel, *cIconID;
	g_variant_get (pPar, "(&s&s)", &cLabel, &cIconID);
	if (_applet_set_label (pApplet, cLabel, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_set_icon (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cImage, *cIconID;
	g_variant_get (pPar, "(&s&s)", &cImage, &cIconID);
	if (_applet_set_icon (pApplet, cImage, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_set_emblem (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cImage, *cIconID;
	gint iPosition;
	g_variant_get (pPar, "(&si&s)", &cImage, &iPosition, &cIconID);
	if (_applet_set_emblem (pApplet, cImage, iPosition, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_animate (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cAnimation, *cIconID;
	gint iNbRounds;
	g_variant_get (pPar, "(&si&s)", &cAnimation, &iNbRounds, &cIconID);
	if (_applet_animate (pApplet, cAnimation, iNbRounds, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_show_dialog (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage, *cIconID;
	gint iDuration;
	g_variant_get (pPar, "(&si&s)", &cMessage, &iDuration, &cIconID);
	if (_applet_show_dialog (pApplet, cMessage, iDuration, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

// deprecated
static void _sub_applet_ask_question (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage, *cIconID;
	g_variant_get (pPar, "(&s&s)", &cMessage, &cIconID);
	if (_applet_ask_question (pApplet, cMessage, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_ask_value (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage, *cIconID;
	gdouble fInitialValue, fMaxValue;
	g_variant_get (pPar, "(&sdd&s)", &cMessage, &fInitialValue, &fMaxValue, &cIconID);
	if (_applet_ask_value (pApplet, cMessage, fInitialValue, fMaxValue, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}

static void _sub_applet_ask_text (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage, *cInitialText, *cIconID;
	g_variant_get (pPar, "(&s&s&s)", &cMessage, &cInitialText, &cIconID);
	if (_applet_ask_text (pApplet, cMessage, cInitialText, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
}
// end of deprecated

static void _sub_applet_popup_dialog (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GVariant *pDialogAttr, *pWidgetAttr;
	const gchar *cIconID;
	g_variant_get (pPar, "(@a{sv}@a{sv}&s)", &pDialogAttr, &pWidgetAttr, &cIconID);
	if (_applet_popup_dialog (pApplet, pDialogAttr, pWidgetAttr, cIconID))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_INVALID_ARGS, "Icon not found: '%s'", cIconID);
	g_variant_unref (pDialogAttr);
	g_variant_unref (pWidgetAttr);
}


static void _sub_applet_add_sub_icons (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	//g_print ("%s ()\n", __func__);
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	GldiContainer *pContainer = pInstance ? pInstance->pContainer : NULL;
	if (!(pInstance && pIcon && pContainer))
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return;
	}
	
	GList *pCurrentIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
	Icon *pLastIcon = cairo_dock_get_last_icon (pCurrentIconsList);
	int n = (pLastIcon ? pLastIcon->fOrder + 1 : 0);
	
	const gchar **pIconFields = NULL;
	g_variant_get (pPar, "(^a&s)", &pIconFields);
	
	if (!pIconFields) // happens for empty arrays, should be an error?
		g_dbus_method_invocation_return_value (pInv, NULL);
	
	GList *pIconsList = NULL;
	Icon *pOneIcon;
	int i;
	for (i = 0; pIconFields[3*i] && pIconFields[3*i+1] && pIconFields[3*i+2]; i ++)
	{
		pOneIcon = cairo_dock_create_dummy_launcher (g_strdup (pIconFields[3*i]),
			g_strdup (pIconFields[3*i+1]),
			g_strdup (pIconFields[3*i+2]),
			NULL,
			i + n);
		pIconsList = g_list_append (pIconsList, pOneIcon);
	}
	if (pIconFields[3*i] != NULL)
	{
		cd_warning ("the number of argument is incorrect\nThis may result in an incorrect number of loaded icons.");
	}
	
	gpointer data[3] = {GINT_TO_POINTER (0), GINT_TO_POINTER (TRUE), NULL};
	cairo_dock_insert_icons_in_applet (pInstance, pIconsList, NULL, "Panel", (CairoDeskletRendererConfigPtr) data);  // NULL <=> default sub-docks renderer
	
	g_free (pIconFields); // note: no need to free the elements, but need to free the array
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _sub_applet_remove_sub_icon (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	GldiContainer *pContainer = pInstance ? pInstance->pContainer : NULL;
	if (!(pInstance && pIcon && pContainer))
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return;
	}
	
	const gchar *cIconID;
	g_variant_get (pPar, "(&s)", &cIconID);
	
	// note: cIconID likely will not be NULL (there are no NULL values in DBus), but can be an empty string
	if (!cIconID || !*cIconID || !strcmp (cIconID, "any"))  // remove all
	{
		cairo_dock_remove_all_icons_from_applet (pInstance);
	}
	else
	{
		GList *pIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		Icon *pOneIcon = cairo_dock_get_icon_with_command (pIconsList, cIconID); //!! TODO: should we return an error if not found ?
		///cairo_dock_remove_icon_from_applet (pInstance, pOneIcon);
		gldi_object_unref (GLDI_OBJECT(pOneIcon));
	}
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}


  ///////////////////////////////////////////////
 ////////// applet interface methods ///////////
///////////////////////////////////////////////

static void _m_applet_set_quick_info (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cQuickInfo;
	g_variant_get (pPar, "(&s)", &cQuickInfo);
	if (_applet_set_quick_info (pApplet, cQuickInfo, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

static void _m_applet_set_label (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cLabel;
	g_variant_get (pPar, "(&s)", &cLabel);
	if (_applet_set_label (pApplet, cLabel, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

static void _m_applet_set_icon (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cImage;
	g_variant_get (pPar, "(&s)", &cImage);
	if (_applet_set_icon (pApplet, cImage, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

static void _m_applet_set_emblem (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cImage;
	gint iPosition;
	g_variant_get (pPar, "(&si)", &cImage, &iPosition);
	if (_applet_set_emblem (pApplet, cImage, iPosition, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

static void _m_applet_animate (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cAnimation;
	gint iNbRounds;
	g_variant_get (pPar, "(&si)", &cAnimation, &iNbRounds);
	if (_applet_animate (pApplet, cAnimation, iNbRounds, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}


static void _m_applet_demands_attention (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pApplet, NULL, &pIcon, &pContainer))
	{
		g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return;
	}
	
	gboolean bStart;
	const gchar *cAnimation;
	g_variant_get (pPar, "(b&s)", &bStart, &cAnimation);
	
	if (bStart)
	{
		if (CAIRO_DOCK_IS_DOCK (pContainer))
		{
			gldi_icon_request_attention (pIcon, cAnimation, 0);  // 0 <=> sans arret.
		}
	}
	else if (pIcon->bIsDemandingAttention)
	{
		gldi_icon_stop_attention (pIcon);
	}
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _m_applet_show_dialog (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage;
	gint iDuration;
	g_variant_get (pPar, "(&si)", &cMessage, &iDuration);
	if (_applet_show_dialog (pApplet, cMessage, iDuration, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

// deprecated
static void _m_applet_ask_question (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage;
	g_variant_get (pPar, "(&s)", &cMessage);
	if (_applet_ask_question (pApplet, cMessage, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

static void _m_applet_ask_value (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage;
	gdouble fInitialValue, fMaxValue;
	g_variant_get (pPar, "(&sdd)", &cMessage, &fInitialValue, &fMaxValue);
	if (_applet_ask_value (pApplet, cMessage, fInitialValue, fMaxValue, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}

static void _m_applet_ask_text (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	const gchar *cMessage, *cInitialText;
	g_variant_get (pPar, "(&s&s)", &cMessage, &cInitialText);
	if (_applet_ask_text (pApplet, cMessage, cInitialText, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
}
// end of deprecated

static void _m_applet_popup_dialog (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GVariant *pDialogAttr, *pWidgetAttr;
	g_variant_get (pPar, "(@a{sv}@a{sv})", &pDialogAttr, &pWidgetAttr);
	if (_applet_popup_dialog (pApplet, pDialogAttr, pWidgetAttr, NULL))
		g_dbus_method_invocation_return_value (pInv, NULL);
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
	g_variant_unref (pDialogAttr);
	g_variant_unref (pWidgetAttr);
}

static void _applet_add_data_renderer (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	GldiContainer *pContainer = pInstance ? pInstance->pContainer : NULL;
	if (!(pInstance && pIcon && pContainer))
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return;
	}
	
	const gchar *cType, *cTheme;
	gint iNbValues;
	
	g_variant_get (pPar, "(&si&s)", &cType, &iNbValues, &cTheme);
	
	CairoDataRendererAttribute *pRenderAttr = NULL;  // attributes for the global data-renderer.
	CairoGaugeAttribute aGaugeAttr;  // gauge attributes.
	CairoGraphAttribute aGraphAttr;  // graph attributes.
	CairoGraphAttribute aProgressBarAttr;  // progressbar attributes.
	double *fHighColor = NULL, *fLowColor = NULL;
	if (strcmp (cType, "gauge") == 0)
	{
		memset (&aGaugeAttr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGaugeAttr);
		aGaugeAttr.cThemePath = cairo_dock_get_data_renderer_theme_path (cType, cTheme, CAIRO_DOCK_ANY_PACKAGE);
	}
	else if (strcmp (cType, "graph") == 0)
	{
		memset (&aGraphAttr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aGraphAttr);
		int w, h;
		cairo_dock_get_icon_extent (pIcon, &w, &h);
		pRenderAttr->iMemorySize = (w > 1 ? w : 32);
		// Line;Plain;Bar;Circle;Plain Circle
		if (cTheme == NULL || strcmp (cTheme, "Line") == 0)
			aGraphAttr.iType = CAIRO_DOCK_GRAPH_LINE;
		else if (strcmp (cTheme, "Plain") == 0)
			aGraphAttr.iType = CAIRO_DOCK_GRAPH_PLAIN;
		else if (strcmp (cTheme, "Bar") == 0)
			aGraphAttr.iType = CAIRO_DOCK_GRAPH_BAR;
		else if (strcmp (cTheme, "Circle") == 0)
			aGraphAttr.iType = CAIRO_DOCK_GRAPH_CIRCLE;
		else if (strcmp (cTheme, "Plain Circle") == 0)
			aGraphAttr.iType = CAIRO_DOCK_GRAPH_CIRCLE_PLAIN;
		aGraphAttr.bMixGraphs = FALSE;
		if (iNbValues > 0)
		{
			fHighColor = g_new (double, iNbValues*3);
			fLowColor  = g_new (double, iNbValues*3);
			int i;
			for (i = 0; i < iNbValues; i ++)
			{
				fHighColor[3*i] = 1;
				fHighColor[3*i+1] = 0;
				fHighColor[3*i+2] = 0;
				fLowColor[3*i] = 0;
				fLowColor[3*i+1] = 1;
				fLowColor[3*i+2] = 1;
			}
		}
		aGraphAttr.fHighColor = fHighColor;
		aGraphAttr.fLowColor = fLowColor;
		aGraphAttr.fBackGroundColor[0] = 0;
		aGraphAttr.fBackGroundColor[0] = 0;
		aGraphAttr.fBackGroundColor[0] = 1;
		aGraphAttr.fBackGroundColor[0] = .4;
	}
	else if (strcmp (cType, "progressbar") == 0)
	{
		memset (&aProgressBarAttr, 0, sizeof (CairoProgressBarAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&aProgressBarAttr);
	}
	
	if (pRenderAttr == NULL || iNbValues <= 0)
	{
		cairo_dock_remove_data_renderer_on_icon (pIcon);
		g_dbus_method_invocation_return_value (pInv, NULL);
		return;
	}
	
	pRenderAttr->cModelName = cType;
	pRenderAttr->iLatencyTime = 500;  // 1/2s
	pRenderAttr->iNbValues = iNbValues;
	//pRenderAttr->bUpdateMinMax = TRUE;
	//pRenderAttr->bWriteValues = TRUE;
	if (pIcon->image.pSurface)
	{
		cairo_dock_add_new_data_renderer_on_icon (pIcon, pContainer, pRenderAttr);
		g_dbus_method_invocation_return_value (pInv, NULL);
	}
	else g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet icon does not have an image surface");

	g_free (fHighColor);
	g_free (fLowColor);
}

static void _applet_render_values (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	GldiContainer *pContainer = pInstance ? pInstance->pContainer : NULL;
	CairoDataRenderer *pRenderer = pIcon ? cairo_dock_get_icon_data_renderer (pIcon) : NULL;
	if (!(pInstance && pIcon && pContainer)) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
	if (!pIcon->image.pSurface) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet icon does not have an image surface");
	if (!pRenderer) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet icon does not have a data renderer");
	if (! (pInstance && pIcon && pContainer && pRenderer && pIcon->image.pSurface)) return;
	
	CairoDataToRenderer *pData = cairo_data_renderer_get_data (pRenderer);
	int iNbValues = pData->iNbValues;
	
	// note: we should make sure that we have at least the expected number of elements,
	// as cairo_dock_render_new_data_on_icon () will copy without checking the size
	double *values = g_new0 (double, iNbValues);
	GVariantIter *iter = NULL;
	g_variant_get (pPar, "(ad)", &iter);
	
	int i;
	for (i = 0; i < iNbValues; i++)
		if (!g_variant_iter_next (iter, "d", values + i)) break;
	g_variant_iter_free (iter);
	
	cairo_t *pDrawContext = cairo_create (pIcon->image.pSurface);
	cairo_dock_render_new_data_on_icon (pIcon, pContainer, pDrawContext, values);
	cairo_destroy (pDrawContext);
	g_free (values);
	
	cairo_dock_redraw_icon (pIcon);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _applet_control_appli (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	if (!(pInstance && pIcon))
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return;
	}
	
	const gchar *cApplicationClass;
	g_variant_get (pPar, "(&s)", &cApplicationClass);
	
	gchar *cClass = (cApplicationClass ? g_ascii_strdown (cApplicationClass, -1) : NULL);
	if (cairo_dock_strings_differ (pIcon->cClass, cClass))
	{
		if (pIcon->cClass != NULL)
			cairo_dock_deinhibite_class (pIcon->cClass, pIcon);
		if (cClass != NULL)
		{
			cairo_dock_inhibite_class (cClass, pIcon);  /// useful to use cairo_dock_register_class ?...
		}
		if (! cairo_dock_is_loading ())
		{
			GldiContainer *pContainer = pInstance->pContainer;
			if (pContainer != NULL)
				cairo_dock_redraw_icon (pIcon);
		}
	}
	
	g_free (cClass);
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _applet_show_appli (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	if (!(pInstance && pIcon)) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
	if (!pIcon->pAppli) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet is not controlling an application");
	if (!(pInstance && pIcon && pIcon->pAppli)) return;
	
	gboolean bShow;
	g_variant_get (pPar, "(b)", &bShow);
	
	if (bShow)
		gldi_window_show (pIcon->pAppli);
	else
		gldi_window_minimize (pIcon->pAppli);
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _applet_act_on_appli (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	if (!(pInstance && pIcon)) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
	if (!pIcon->pAppli) g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
		G_DBUS_ERROR_FAILED, "Applet is not controlling an application");
	if (!(pInstance && pIcon && pIcon->pAppli)) return;
	
	const gchar *cAction;
	g_variant_get (pPar, "(&s)", &cAction);
	if (!*cAction)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_INVALID_ARGS, "No action given");
		return;
	}
	
	if (strcmp (cAction, "minimize") == 0)
		gldi_window_minimize (pIcon->pAppli);
	else if (strcmp (cAction, "show") == 0)
		gldi_window_show (pIcon->pAppli);
	else if (strcmp (cAction, "toggle-visibility") == 0)
	{
		if (pIcon->pAppli->bIsHidden)
			gldi_window_show (pIcon->pAppli);
		else
			gldi_window_minimize (pIcon->pAppli);
	}	
	else if (strcmp (cAction, "maximize") == 0)
		 gldi_window_maximize(pIcon->pAppli, TRUE);
	else if (strcmp (cAction, "restore") == 0)
		gldi_window_maximize (pIcon->pAppli, FALSE);
	else if (strcmp (cAction, "toggle-size") == 0)
	{
		gldi_window_maximize (pIcon->pAppli, ! pIcon->pAppli->bIsMaximized);
	}
	else if (strcmp (cAction, "close") == 0)
		gldi_window_close (pIcon->pAppli);
	else if (strcmp (cAction, "kill") == 0)
		gldi_window_kill (pIcon->pAppli);
	else
	{
		g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_INVALID_ARGS, "Invalid action: '%s'", cAction);
		return;
	}
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _applet_populate_menu (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)  // deprecated
{
	if (myData.pModuleMainMenu == NULL || pApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'PopulateMenu' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
			"the 'PopulateMenu' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return;
	}
	
	const gchar **pLabels;
	g_variant_get (pPar, "(^a&s)", &pLabels);
	
	int i;
	for (i = 0; pLabels[i] != NULL; i ++)
	{
		if (*pLabels[i] == '\0')
		{
			gtk_menu_shell_append (GTK_MENU_SHELL (myData.pModuleMainMenu), gtk_separator_menu_item_new ());
		}
		else
		{
			cairo_dock_add_in_menu_with_stock_and_data (pLabels[i],
				NULL,
				G_CALLBACK (cd_dbus_emit_on_menu_select),
				myData.pModuleMainMenu,
				GINT_TO_POINTER (i));
		}
	}
	gtk_widget_show_all (myData.pModuleMainMenu);
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}

static void _on_map_menuitem (GtkWidget *pMenuItem, gpointer data)
{
	if (data) gtk_widget_set_tooltip_text (pMenuItem, (const gchar*)data);
}

static void _weak_free_helper (gpointer ptr, G_GNUC_UNUSED GObject* pObj)
{
	g_free (ptr);
}

static void _applet_add_menu_items (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	if (myData.pModuleMainMenu == NULL/** || myData.pModuleSubMenu == NULL*/ || pApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'AddMenuItems' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
			"the 'AddMenuItems' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return;
	}
	
	GtkRequisition natural_size;
	gtk_widget_get_preferred_size (myData.pModuleMainMenu, NULL, &natural_size);
	int iItemHeight = 0, iMenuHeight = natural_size.height;
	int iIconSize;
	gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &iIconSize, NULL);
	
	// get the position of our items in the menu.
	int iPosition = myData.iMenuPosition;
	
	// insert a separator
	GtkWidget *pMenuItem = gtk_separator_menu_item_new ();
	gtk_menu_shell_insert (GTK_MENU_SHELL (myData.pModuleMainMenu), pMenuItem, iPosition++);
	gtk_widget_get_preferred_size (pMenuItem, NULL, &natural_size);
	iItemHeight += natural_size.height;
	
	// table des menus et groupes de radio-boutons.
	GHashTable *pSubMenus = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	GHashTable *pGroups = g_hash_table_new_full (g_int_hash,
		g_int_equal,
		g_free,
		NULL);
	
	// on parcours la liste des items.
	GtkWidget *pMenu;
	GSList *group = NULL;
	// GValue *v;
	
	// pItems: "aa{sv}"
	GVariantIter *iter;
	g_variant_get (pPar, "(aa{sv})", &iter);
	GVariant *pItemVar;
	
	guint i;
	for (i = 0; g_variant_iter_next (iter, "@a{sv}", &pItemVar); i ++)
	{
		GVariantDict *pItem = g_variant_dict_new (pItemVar);
		
		// get its properties.
		const gchar *cLabel = NULL, *cIcon = NULL, *cToolTip = NULL;
		int iType = 0, iMenuID = -1, id = i, iGroupID = 0;
		gboolean bState = FALSE;
		gpointer data;
		
		g_variant_dict_lookup (pItem, "type", "i", &iType);
		g_variant_dict_lookup (pItem, "label", "&s", &cLabel);
		g_variant_dict_lookup (pItem, "id", "i", &id);
		data = GINT_TO_POINTER (id);
		
		if (iType == 0 || iType == 1)
			g_variant_dict_lookup (pItem, "icon", "&s", &cIcon);
		
		g_variant_dict_lookup (pItem, "state", "b", &bState);
		if (g_variant_dict_lookup (pItem, "group", "i", &iGroupID))
			group = g_hash_table_lookup (pGroups, &iGroupID);  // si NULL, ca fera un nouveau groupe.
		else  // si on ne definit pas le groupe, c'est donc le groupe en cours qui est utilise, ou un nouveau groupe si encore aucun n'est en cours.
			iGroupID = id;  // utilise seulement si le groupe est nouvellement cree, pour l'enregistrer.
		
		// create the item according to its type.
		switch (iType)
		{
			case 0 :  // normal entry
				pMenuItem = gldi_menu_item_new_with_action (cLabel, cIcon, G_CALLBACK (cd_dbus_emit_on_menu_select), data);
			break;
			case 1:  // sub-menu
			{
				GtkWidget *pSubMenu;
				pMenuItem = gldi_menu_item_new_with_submenu (cLabel, cIcon, &pSubMenu);
				int *pID = g_new (int, 1);
				*pID = id;
				g_hash_table_insert (pSubMenus, pID, pSubMenu);
			}
			break;
			case 2:  // separator
				pMenuItem = gtk_separator_menu_item_new ();
			break;
			case 3:  // check-box
				pMenuItem = gtk_check_menu_item_new_with_label (cLabel);
				if (bState)
					gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(pMenuItem), bState);
				g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(cd_dbus_emit_on_menu_select), data);
			break;
			case 4:  // group-box
				pMenuItem = gtk_radio_menu_item_new_with_label (group, cLabel);
				if (group == NULL)  // le groupe ne change plus par la suite (g_list_append).
				{
					group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM(pMenuItem));
					int *pID = g_new (int, 1);
					*pID = iGroupID;
					g_hash_table_insert (pGroups, pID, group);
				}
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(pMenuItem), bState);
				g_signal_connect(G_OBJECT(pMenuItem), "toggled", G_CALLBACK(cd_dbus_emit_on_menu_select), data);
			break;
			default:
				continue;
		}
		
		// set sensitivity
		gboolean tmp;
		if (g_variant_dict_lookup (pItem, "sensitive", "b", &tmp))
			gtk_widget_set_sensitive (pMenuItem, tmp);
		
		// set the tooltip
		if (g_variant_dict_lookup (pItem, "tooltip", "&s", &cToolTip))
		{
			if (gldi_wayland_manager_have_layer_shell ())
			{
				/** Need to manage the tooltip ourselves, see e.g.
				 * https://github.com/wmww/gtk-layer-shell/issues/207 */
				gchar *tmp = g_strdup (cToolTip);
				g_signal_connect (G_OBJECT (pMenuItem), "map", G_CALLBACK (_on_map_menuitem), tmp);
				g_object_weak_ref (G_OBJECT (pMenuItem), _weak_free_helper, tmp);
			}
			else gtk_widget_set_tooltip_text (pMenuItem, cToolTip);
		}
		
		// insert in its menu.
		g_variant_dict_lookup (pItem, "menu", "i", &iMenuID);
		if (iMenuID <= 0)
			pMenu = myData.pModuleMainMenu;
		else
		{
			pMenu = g_hash_table_lookup (pSubMenus, &iMenuID);
			if (pMenu == NULL)
				pMenu = myData.pModuleMainMenu;
		}
		
		gtk_menu_shell_insert (GTK_MENU_SHELL (pMenu), pMenuItem, iPosition++);
		if (pMenu == myData.pModuleMainMenu)
		{
			gtk_widget_show_all (pMenuItem);  // make it visible now so that its height is correctly calculated by GTK (else its child is ignored)
			gtk_widget_get_preferred_size (pMenuItem, NULL, &natural_size);
			iItemHeight += natural_size.height;
		}
		
		g_variant_dict_unref (pItem);
		g_variant_unref (pItemVar);
	}
	
	g_variant_iter_free (iter);
	g_hash_table_destroy (pSubMenus);
	g_hash_table_destroy (pGroups);
	gtk_widget_show_all (myData.pModuleMainMenu);
	
	g_object_set (myData.pModuleMainMenu, "height-request", iMenuHeight + iItemHeight, NULL);  // GTK doesn't resize menus correctly, so we have to force it...
	gtk_menu_reposition (GTK_MENU (myData.pModuleMainMenu));
	
	g_dbus_method_invocation_return_value (pInv, NULL);
}


static void _applet_bind_shortkey (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	if (!pInstance)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR,
			G_DBUS_ERROR_FAILED, "Applet does not exist");
		return;
	}
	
	const gchar **cShortkeys;
	g_variant_get (pPar, "(^a&s)", &cShortkeys);
	
	const gchar *cShortkey, *cDescription = "-", *cGroupName = "Configuration", *cKeyName = "shortkey";
	GldiShortkey *pKeyBinding;
	int i;
	GList *kb;
	
	if (pApplet->pShortkeyList == NULL)
	{
		for (i = 0; cShortkeys[i] != NULL; i ++)
		{
			cShortkey = cShortkeys[i];
			pKeyBinding = gldi_shortkey_new (cShortkey,
				pInstance->pModule->pVisitCard->cTitle,
				cDescription,
				pInstance->pModule->pVisitCard->cIconFilePath,
				pInstance->cConfFilePath,
				cGroupName, cKeyName,
				(CDBindkeyHandler) cd_dbus_applet_emit_on_shortkey, pApplet);
			pApplet->pShortkeyList = g_list_append (pApplet->pShortkeyList, pKeyBinding);
		}
	}
	else  // just rebind, we consider that the applet wants to rebind the same shortkeys.
	{
		for (i = 0, kb = pApplet->pShortkeyList; cShortkeys[i] != NULL && kb != NULL; i ++, kb = kb->next)
		{
			cShortkey = cShortkeys[i];
			pKeyBinding = kb->data;
			gldi_shortkey_rebind (pKeyBinding, cShortkey, NULL);
		}
	}
	g_dbus_method_invocation_return_value (pInv, NULL);
}


// new interface: use DBus properties -- TODO: we need to track changes and send out notifications about them !!
static GVariant *_applet_get_property (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cSender,
	G_GNUC_UNUSED const gchar *cObj, G_GNUC_UNUSED const gchar *cInterface, const gchar* cProperty,
	GError** error, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	GldiContainer *pContainer = pInstance ? pInstance->pContainer : NULL;
	if (!(pApplet && pInstance && pIcon && pContainer))
	{
		g_set_error_literal (error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return NULL;
	}
	
	GVariant *res = NULL;
	// x, y, orientation, type, width, height
	if (strcmp (cProperty, "x") == 0)
	{
		int x;
		if (pContainer->bIsHorizontal)
		{
			x = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		}
		else
		{
			x = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
		}
		res = g_variant_new_int32 (x);
	}
	else if (strcmp (cProperty, "y") == 0)
	{
		int y;
		if (pContainer->bIsHorizontal)
		{
			y = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
		}
		else
		{
			y = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		}
		res = g_variant_new_int32 (y);
	}
	else if (strcmp (cProperty, "orientation") == 0)
	{
		CairoDockPositionType iScreenBorder = ((! pContainer->bIsHorizontal) << 1) | (! pContainer->bDirectionUp);
		res = g_variant_new_uint32 (iScreenBorder);
	}
	else if (strcmp (cProperty, "container") == 0)
	{
		unsigned int iType = _get_container_type (pContainer);
		res = g_variant_new_uint32 (iType);
	}
	else if (strcmp (cProperty, "width") == 0)  // this is the dimension of the icon when it's hovered.
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		res = g_variant_new_int32 (iWidth);
	}
	else if (strcmp (cProperty, "height") == 0)
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		res = g_variant_new_int32 (iHeight);
	}
	else if (strncmp (cProperty, "Xid", 3) == 0)
	{
		// note: this is only used to indicate whether the app is open
		res = g_variant_new_uint64 (!!pIcon->pAppli);
	}
	else if (strcmp (cProperty, "has_focus") == 0)
	{
		gboolean bHasFocus = (pIcon->pAppli != NULL && pIcon->pAppli == gldi_windows_get_active ());
		res = g_variant_new_boolean (bHasFocus);
	}
	else
		g_set_error (error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY, "Unknown property (%s)", cProperty);
	return res;
}

GVariant *cd_dbus_applet_get_property (GDBusConnection *pConn, const gchar *cSender, const gchar *cObj,
	const gchar *cInterface, const gchar* cProperty, GError** error, gpointer data)
{
	CD_APPLET_ENTER;
	
	DBusAppletData *pApplet = (DBusAppletData*)data;
	
	if (!pApplet)
	{
		g_set_error_literal (error, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Applet does not exist");
		CD_APPLET_LEAVE (NULL);
	}
	
	GVariant *res = _applet_get_property (pConn, cSender, cObj, cInterface, cProperty, error, pApplet);
	
	CD_APPLET_LEAVE (res);
}

// old interface: our own functions for getting properties
static void _applet_get (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GError *err = NULL;
	const gchar *cProperty;
	g_variant_get (pPar, "(&s)", &cProperty);
	
	GVariant *res = cd_dbus_applet_get_property (NULL, NULL, NULL, NULL, cProperty, &err, pApplet);
	if (!res)
	{
		// error is set in this case
		g_dbus_method_invocation_return_gerror (pInv, err);
		g_error_free (err);
	}
	else // we need to box the result as "(v)"
		g_dbus_method_invocation_return_value (pInv, g_variant_new ("(v)", res)); // will take ownership of res (it is a floating reference)
}


static void _applet_get_all (GVariant *pPar, GDBusMethodInvocation *pInv, DBusAppletData *pApplet)
{
	GldiModuleInstance *pInstance = pApplet->pModuleInstance;
	Icon *pIcon = pInstance ? pInstance->pIcon : NULL;
	GldiContainer *pContainer = pInstance ? pInstance->pContainer : NULL;
	if (!(pInstance && pIcon && pContainer))
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Applet or its icon does not exist");
		return;
	}
	
	// return value should be a{sv}
	GVariantBuilder res;
	
	#if GLIB_CHECK_VERSION (2, 84, 0)
	g_variant_builder_init_static
	#else
	g_variant_builder_init
	#endif
	(&res, G_VARIANT_TYPE ("(a{sv})"));
	
	g_variant_builder_open (&res, G_VARIANT_TYPE ("a{sv}"));
	
	int x, y;
	if (pContainer->bIsHorizontal)
	{
		x = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		y = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
	}
	else
	{
		y = pContainer->iWindowPositionX + pIcon->fDrawX + pIcon->fWidth * pIcon->fScale/2;
		x = pContainer->iWindowPositionY + pIcon->fDrawY + pIcon->fHeight * pIcon->fScale/2;
	}
	CairoDockPositionType iScreenBorder = ((! pContainer->bIsHorizontal) << 1) | (! pContainer->bDirectionUp);
	int iWidth, iHeight;
	cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
	
	gboolean bHasFocus = (pIcon->pAppli != NULL && pIcon->pAppli == gldi_windows_get_active ());
	
	g_variant_builder_add (&res, "{sv}", "x", g_variant_new_int32 (x));
	g_variant_builder_add (&res, "{sv}", "y", g_variant_new_int32 (y));
	g_variant_builder_add (&res, "{sv}", "orientation", g_variant_new_uint32 (iScreenBorder));
	g_variant_builder_add (&res, "{sv}", "container", g_variant_new_uint32 (_get_container_type (pContainer)));
	g_variant_builder_add (&res, "{sv}", "width", g_variant_new_int32 (iWidth));
	g_variant_builder_add (&res, "{sv}", "height", g_variant_new_int32 (iHeight));
	g_variant_builder_add (&res, "{sv}", "Xid", g_variant_new_uint64 (!!pIcon->pAppli));
	g_variant_builder_add (&res, "{sv}", "has_focus", g_variant_new_boolean (bHasFocus));
	
	g_variant_builder_close (&res);
	g_dbus_method_invocation_return_value (pInv, g_variant_builder_end (&res));
}


void cd_dbus_applet_method_call (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cSender,
	G_GNUC_UNUSED const gchar *cObj, // object path -- should we re-check that it matches the applet?
	G_GNUC_UNUSED const gchar *cInterface, // interface -- will always be org.cairodock.CairoDock.applet
	const gchar *cMethod, GVariant *pPar, GDBusMethodInvocation* pInv, gpointer data)
{
	CD_APPLET_ENTER;
	
	DBusAppletData *pApplet = (DBusAppletData*)data;
	if (!pApplet)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Applet does not exist");
		CD_APPLET_LEAVE ();
	}
	
	     if (!strcmp (cMethod, "Get")) _applet_get (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "GetAll")) _applet_get_all (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetQuickInfo")) _m_applet_set_quick_info (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetLabel")) _m_applet_set_label (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetIcon")) _m_applet_set_icon (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetEmblem")) _m_applet_set_emblem (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "Animate")) _m_applet_animate (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "DemandsAttention")) _m_applet_demands_attention (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "ShowDialog")) _m_applet_show_dialog (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "PopupDialog")) _m_applet_popup_dialog (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AddDataRenderer")) _applet_add_data_renderer (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "RenderValues")) _applet_render_values (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "ControlAppli")) _applet_control_appli (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "ActOnAppli")) _applet_act_on_appli (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AddMenuItems")) _applet_add_menu_items (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "BindShortkey")) _applet_bind_shortkey (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AskQuestion")) _m_applet_ask_question (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AskText")) _m_applet_ask_text (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AskValue")) _m_applet_ask_value (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "ShowAppli")) _applet_show_appli (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "PopulateMenu")) _applet_populate_menu (pPar, pInv, pApplet);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD, "Unknown method: '%s'", cMethod);
	
	CD_APPLET_LEAVE ();
}

GVariant *cd_dbus_sub_applet_get_property (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cSender,
	G_GNUC_UNUSED const gchar *cObj, G_GNUC_UNUSED const gchar *cInterface, const gchar* cProp,
	GError** error, G_GNUC_UNUSED gpointer data)
{
	// sub-applets don't have any DBus properties
	g_set_error (error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY, "Unknown property (%s)", cProp);
	return NULL;
}


void cd_dbus_sub_applet_method_call (G_GNUC_UNUSED GDBusConnection *pConn, G_GNUC_UNUSED const gchar *cSender,
	G_GNUC_UNUSED const gchar *cObj, // object path -- should we re-check that it matches the applet?
	G_GNUC_UNUSED const gchar *cInterface, // interface -- will always be org.cairodock.CairoDock.subapplet
	const gchar *cMethod, GVariant *pPar, GDBusMethodInvocation* pInv, gpointer data)
{
	CD_APPLET_ENTER;
	
	DBusAppletData *pApplet = (DBusAppletData*)data;
	if (!pApplet)
	{
		g_dbus_method_invocation_return_error_literal (pInv, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, "Applet does not exist");
		CD_APPLET_LEAVE ();
	}
	
	     if (!strcmp (cMethod, "SetQuickInfo")) _sub_applet_set_quick_info (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetLabel")) _sub_applet_set_label (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetIcon")) _sub_applet_set_icon (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "SetEmblem")) _sub_applet_set_emblem (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "Animate")) _sub_applet_animate (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "ShowDialog")) _sub_applet_show_dialog (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "PopupDialog")) _sub_applet_popup_dialog (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AskQuestion")) _sub_applet_ask_question (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AskText")) _sub_applet_ask_text (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AskValue")) _sub_applet_ask_value (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "AddSubIcons")) _sub_applet_add_sub_icons (pPar, pInv, pApplet);
	else if (!strcmp (cMethod, "RemoveSubIcon")) _sub_applet_remove_sub_icon (pPar, pInv, pApplet);
	else g_dbus_method_invocation_return_error (pInv, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD, "Unknown method: '%s'", cMethod);
	
	CD_APPLET_LEAVE ();
}

