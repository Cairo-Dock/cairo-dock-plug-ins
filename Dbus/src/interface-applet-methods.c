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
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <implementations/cairo-dock-wayland-manager.h> // gldi_wayland_manager_have_layer_shell

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"

static inline GldiModuleInstance *_get_module_instance_from_dbus_applet (dbusApplet *pDbusApplet)
{
	return pDbusApplet->pModuleInstance;
}

static inline gboolean _get_icon_and_container_from_id (dbusApplet *pDbusApplet, const gchar *cIconID, Icon **pIcon, GldiContainer **pContainer)
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
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


static gboolean _applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	gldi_icon_set_quick_info (pIcon, cQuickInfo && *cQuickInfo != '\0' ? cQuickInfo : NULL);
	cairo_dock_redraw_icon (pIcon);
	return TRUE;
}

static gboolean _applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	gldi_icon_set_name (pIcon, cLabel);
	cairo_dock_redraw_icon (pIcon);  /// needs a function to redraw the label...
	return TRUE;
}

static gboolean _applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, const gchar *cIconID, GError **error)
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
static gboolean _applet_set_emblem (dbusApplet *pDbusApplet, const gchar *cImage, gint iPosition, const gchar *cIconID, GError **error)
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

static gboolean _applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID, GError **error)
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

static gboolean _applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, const gchar *cIconID, GError **error)
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

static void _on_dialog_destroyed (dbusApplet *pDbusApplet)
{
	CD_APPLET_ENTER;
	pDbusApplet->pDialog = NULL;
	CD_APPLET_LEAVE();
}

// deprecated
static gboolean _applet_ask_question (dbusApplet *pDbusApplet, const gchar *cMessage, const gchar *cIconID, GError **error)
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

static gboolean _applet_ask_value (dbusApplet *pDbusApplet, const gchar *cMessage, gdouble fInitialValue, gdouble fMaxValue, const gchar *cIconID, GError **error)
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

static gboolean _applet_ask_text (dbusApplet *pDbusApplet, const gchar *cMessage, const gchar *cInitialText, const gchar *cIconID, GError **error)
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

static gboolean _applet_popup_dialog (dbusApplet *pDbusApplet, GHashTable *hDialogAttributes, GHashTable *hWidgetAttributes, const gchar *cIconID, GError **error)
{
	g_return_val_if_fail (hDialogAttributes != NULL, FALSE);
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		gldi_object_unref (GLDI_OBJECT(pDbusApplet->pDialog));
	
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	GValue *v;
	
	// attributs du dialogue.
	gchar *cImageFilePath = NULL;
	v = g_hash_table_lookup (hDialogAttributes, "icon");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		int w, h;
		cairo_dock_get_icon_extent (pIcon, &w, &h);
		cImageFilePath = cairo_dock_search_icon_s_path (g_value_get_string (v), MAX (w, h));
		attr.cImageFilePath = cImageFilePath;
	}
	else
		attr.cImageFilePath = "same icon";
	
	v = g_hash_table_lookup (hDialogAttributes, "message");
	if (v && G_VALUE_HOLDS_STRING (v))
		attr.cText = g_value_get_string (v);
	
	v = g_hash_table_lookup (hDialogAttributes, "time-length");
	if (v && G_VALUE_HOLDS_INT (v))
		attr.iTimeLength = 1000 * g_value_get_int (v);
	
	gchar **cButtonsImage = NULL;
	v = g_hash_table_lookup (hDialogAttributes, "buttons");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cButtonsImage = g_strsplit (g_value_get_string (v), ";", -1);  // NULL-terminated
		attr.cButtonsImage = (const gchar **)cButtonsImage;
	}
	
	v = g_hash_table_lookup (hDialogAttributes, "force-above");
	if (v && G_VALUE_HOLDS_BOOLEAN (v))
		attr.bForceAbove = g_value_get_boolean (v);
	
	v = g_hash_table_lookup (hDialogAttributes, "use-markup");
	if (v && G_VALUE_HOLDS_BOOLEAN (v))
		attr.bUseMarkup = g_value_get_boolean (v);
	
	attr.pUserData = pDbusApplet;
	attr.pFreeDataFunc = (GFreeFunc)_on_dialog_destroyed;
	
	// attributs du widget interactif.
	GtkWidget *pInteractiveWidget = NULL, *pOneWidget = NULL;
	if (hWidgetAttributes != NULL)  // un widget d'interaction est defini.
	{
		v = g_hash_table_lookup (hWidgetAttributes, "widget-type");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			const gchar *cType = g_value_get_string (v);
			if (cType)
			{
				if (strcmp (cType, "text-entry") == 0)
				{
					gboolean bMultiLines = FALSE;
					gboolean bEditable = TRUE;
					gboolean bVisible = TRUE;
					int iNbCharsMax = 0;
					const gchar *cInitialText = NULL;
					
					v = g_hash_table_lookup (hWidgetAttributes, "multi-lines");
					if (v && G_VALUE_HOLDS_BOOLEAN (v))
						bMultiLines = g_value_get_boolean (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "editable");
					if (v && G_VALUE_HOLDS_BOOLEAN (v))
						bEditable = g_value_get_boolean (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "visible");
					if (v && G_VALUE_HOLDS_BOOLEAN (v))
						bVisible = g_value_get_boolean (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "nb-chars");
					if (v && G_VALUE_HOLDS_INT(v))
						iNbCharsMax = g_value_get_int(v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "initial-value");
					if (v && G_VALUE_HOLDS_STRING (v))
						cInitialText = g_value_get_string (v);
					
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
					
					v = g_hash_table_lookup (hWidgetAttributes, "min-value");
					if (v && G_VALUE_HOLDS_DOUBLE (v))
						fMinValue = g_value_get_double (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "max-value");
					if (v && G_VALUE_HOLDS_DOUBLE (v))
						fMaxValue = g_value_get_double (v);
					fMaxValue = MAX (fMaxValue, fMinValue+1);
					
					v = g_hash_table_lookup (hWidgetAttributes, "nb-digit");
					if (v && G_VALUE_HOLDS_INT (v))
						iNbDigit = g_value_get_int (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "initial-value");
					if (v && G_VALUE_HOLDS_DOUBLE (v))
						fInitialValue = g_value_get_double (v);
					fInitialValue = MAX (MIN (fInitialValue, fMaxValue), fMinValue);
					
					v = g_hash_table_lookup (hWidgetAttributes, "min-label");
					if (v && G_VALUE_HOLDS_STRING (v))
						cMinLabel = g_value_get_string (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "max-label");
					if (v && G_VALUE_HOLDS_STRING (v))
						cMaxLabel = g_value_get_string (v);

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

					v = g_hash_table_lookup (hWidgetAttributes, "editable");
					if (v && G_VALUE_HOLDS_BOOLEAN (v))
						bEditable = g_value_get_boolean (v);
					
					v = g_hash_table_lookup (hWidgetAttributes, "values");
					if (v && G_VALUE_HOLDS_STRING (v))
						cValues = g_value_get_string (v);

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
					
					v = g_hash_table_lookup (hWidgetAttributes, "initial-value");
					if (bEditable)
					{
						if (v && G_VALUE_HOLDS_STRING (v))
							cInitialText = g_value_get_string (v);
						if (cInitialText != NULL)
						{
							GtkWidget *pEntry = gtk_bin_get_child (GTK_BIN (pInteractiveWidget));
							gtk_entry_set_text (GTK_ENTRY (pEntry), cInitialText);
						}
					}
					else
					{
						if (v && G_VALUE_HOLDS_INT (v))
							iInitialValue = g_value_get_int (v);
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
			}  // fin du type de widget.
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
	
	g_free (cImageFilePath);
	if (cButtonsImage)
		g_strfreev (cButtonsImage);
	return TRUE;
}


  ///////////////////////////////////////////////////
 ////////// sub-applet interface methods ///////////
///////////////////////////////////////////////////

gboolean cd_dbus_sub_applet_set_quick_info (dbusSubApplet *pDbusSubApplet, const gchar *cQuickInfo, const gchar *cIconID, GError **error)
{
	return _applet_set_quick_info (pDbusSubApplet->pApplet, cQuickInfo, cIconID, error);
}

gboolean cd_dbus_sub_applet_set_label (dbusSubApplet *pDbusSubApplet, const gchar *cLabel, const gchar *cIconID, GError **error)
{
	return _applet_set_label (pDbusSubApplet->pApplet, cLabel, cIconID, error);
}

gboolean cd_dbus_sub_applet_set_icon (dbusSubApplet *pDbusSubApplet, const gchar *cImage, const gchar *cIconID, GError **error)
{
	return _applet_set_icon (pDbusSubApplet->pApplet, cImage, cIconID, error);
}

gboolean cd_dbus_sub_applet_set_emblem (dbusSubApplet *pDbusSubApplet, const gchar *cImage, gint iPosition, const gchar *cIconID, GError **error)
{
	return _applet_set_emblem (pDbusSubApplet->pApplet, cImage, iPosition, cIconID, error);
}

gboolean cd_dbus_sub_applet_animate (dbusSubApplet *pDbusSubApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID, GError **error)
{
	return _applet_animate (pDbusSubApplet->pApplet, cAnimation, iNbRounds, cIconID, error);
}

gboolean cd_dbus_sub_applet_show_dialog (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, gint iDuration, const gchar *cIconID, GError **error)
{
	return _applet_show_dialog (pDbusSubApplet->pApplet, cMessage, iDuration, cIconID, error);
}

// deprecated
gboolean cd_dbus_sub_applet_ask_question (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, const gchar *cIconID, GError **error)
{
	return _applet_ask_question (pDbusSubApplet->pApplet, cMessage, cIconID, error);
}

gboolean cd_dbus_sub_applet_ask_value (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, gdouble fInitialValue, gdouble fMaxValue, const gchar *cIconID, GError **error)
{
	return _applet_ask_value (pDbusSubApplet->pApplet, cMessage, fInitialValue, fMaxValue, cIconID, error);
}

gboolean cd_dbus_sub_applet_ask_text (dbusSubApplet *pDbusSubApplet, const gchar *cMessage, const gchar *cInitialText, const gchar *cIconID, GError **error)
{
	return _applet_ask_text (pDbusSubApplet->pApplet, cMessage, cInitialText, cIconID, error);
}
// end of deprecated

gboolean cd_dbus_sub_applet_popup_dialog (dbusSubApplet *pDbusSubApplet, GHashTable *hDialogAttributes, GHashTable *hWidgetAttributes, const gchar *cIconID, GError **error)
{
	return _applet_popup_dialog (pDbusSubApplet->pApplet, hDialogAttributes, hWidgetAttributes, cIconID, error);
}


gboolean cd_dbus_sub_applet_add_sub_icons (dbusSubApplet *pDbusSubApplet, const gchar **pIconFields, GError **error)
{
	//g_print ("%s ()\n", __func__);
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusSubApplet->pApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	GldiContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	GList *pCurrentIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
	Icon *pLastIcon = cairo_dock_get_last_icon (pCurrentIconsList);
	int n = (pLastIcon ? pLastIcon->fOrder + 1 : 0);
	
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
	
	return TRUE;
}

gboolean cd_dbus_sub_applet_remove_sub_icon (dbusSubApplet *pDbusSubApplet, const gchar *cIconID, GError **error)
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusSubApplet->pApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	GldiContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	if (cIconID == NULL || strcmp (cIconID, "any") == 0)  // remove all
	{
		cairo_dock_remove_all_icons_from_applet (pInstance);
	}
	else
	{
		GList *pIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		Icon *pOneIcon = cairo_dock_get_icon_with_command (pIconsList, cIconID);
		///cairo_dock_remove_icon_from_applet (pInstance, pOneIcon);
		gldi_object_unref (GLDI_OBJECT(pOneIcon));
	}
	
	return TRUE;
}


  ///////////////////////////////////////////////
 ////////// applet interface methods ///////////
///////////////////////////////////////////////

gboolean cd_dbus_applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, GError **error)
{
	return _applet_set_quick_info (pDbusApplet, cQuickInfo, NULL, error);
}

gboolean cd_dbus_applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, GError **error)
{
	return _applet_set_label (pDbusApplet, cLabel, NULL, error);
}

gboolean cd_dbus_applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, GError **error)
{
	return _applet_set_icon (pDbusApplet, cImage, NULL, error);
}

gboolean cd_dbus_applet_set_emblem (dbusApplet *pDbusApplet, const gchar *cImage, gint iPosition, GError **error)
{
	return _applet_set_emblem (pDbusApplet, cImage, iPosition, NULL, error);
}

gboolean cd_dbus_applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, GError **error)
{
	return _applet_animate (pDbusApplet, cAnimation, iNbRounds, NULL, error);
}

gboolean cd_dbus_applet_demands_attention (dbusApplet *pDbusApplet, gboolean bStart, const gchar *cAnimation, GError **error)
{
	Icon *pIcon;
	GldiContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, NULL, &pIcon, &pContainer))
		return FALSE;
	
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
	return TRUE;
}

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error)
{
	cd_debug ("%s (%s)", __func__, message);
	return _applet_show_dialog (pDbusApplet, message, iDuration, NULL, error);
}

// deprecated
gboolean cd_dbus_applet_ask_question (dbusApplet *pDbusApplet, const gchar *message, GError **error)
{
	cd_debug ("%s (%s)", __func__, message);
	return _applet_ask_question (pDbusApplet, message, NULL, error);
}

gboolean cd_dbus_applet_ask_value (dbusApplet *pDbusApplet, const gchar *message, gdouble fInitialValue, gdouble fMaxValue, GError **error)
{
	cd_debug ("%s (%s)", __func__, message);
	return _applet_ask_value (pDbusApplet, message, fInitialValue, fMaxValue, NULL, error);
}

gboolean cd_dbus_applet_ask_text (dbusApplet *pDbusApplet, const gchar *message, const gchar *cInitialText, GError **error)
{
	cd_debug ("%s (%s)", __func__, message);
	return _applet_ask_text (pDbusApplet, message, cInitialText, NULL, error);
}
// end of deprecated

gboolean cd_dbus_applet_popup_dialog (dbusApplet *pDbusApplet, GHashTable *hDialogAttributes, GHashTable *hWidgetAttributes, GError **error)
{
	return _applet_popup_dialog (pDbusApplet, hDialogAttributes, hWidgetAttributes, NULL, error);
}


gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error)
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	GldiContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
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
		return TRUE;
	}
	
	pRenderAttr->cModelName = cType;
	pRenderAttr->iLatencyTime = 500;  // 1/2s
	pRenderAttr->iNbValues = iNbValues;
	//pRenderAttr->bUpdateMinMax = TRUE;
	//pRenderAttr->bWriteValues = TRUE;
	g_return_val_if_fail (pIcon->image.pSurface != NULL, FALSE);
	cairo_dock_add_new_data_renderer_on_icon (pIcon, pContainer, pRenderAttr);

	g_free (fHighColor);
	g_free (fLowColor);

	return TRUE;
}

gboolean cd_dbus_applet_render_values (dbusApplet *pDbusApplet, GArray *pValues, GError **error)
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	GldiContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	g_return_val_if_fail (pIcon->image.pSurface != NULL, FALSE);
	cairo_t *pDrawContext = cairo_create (pIcon->image.pSurface);
	cairo_dock_render_new_data_on_icon (pIcon, pContainer, pDrawContext, (double *)pValues->data);
	cairo_destroy (pDrawContext);
	
	cairo_dock_redraw_icon (pIcon);
	return TRUE;
}

gboolean cd_dbus_applet_control_appli (dbusApplet *pDbusApplet, const gchar *cApplicationClass, GError **error)
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
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
	return TRUE;
}

gboolean cd_dbus_applet_show_appli (dbusApplet *pDbusApplet, gboolean bShow, GError **error)  // deprecated
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL && pIcon->pAppli != NULL, FALSE);
	
	if (bShow)
		gldi_window_show (pIcon->pAppli);
	else
		gldi_window_minimize (pIcon->pAppli);
	
	return TRUE;
}

gboolean cd_dbus_applet_act_on_appli (dbusApplet *pDbusApplet, const gchar *cAction, GError **error)
{
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL && pIcon->pAppli != NULL, FALSE);
	
	g_return_val_if_fail (cAction != NULL, FALSE);
	
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
		cd_warning ("invalid action '%s' on window %s", cAction, pIcon->cName);
	}
	
	return TRUE;
}

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error)  // deprecated
{
	if (myData.pModuleMainMenu == NULL || pDbusApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'PopulateMenu' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return FALSE;
	}
	
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
	
	return TRUE;
}

static void _on_map_menuitem (GtkWidget *pMenuItem, gpointer data)
{
	if (data) gtk_widget_set_tooltip_text (pMenuItem, (const gchar*)data);
}

static void _weak_free_helper (gpointer ptr, GObject*)
{
	g_free (ptr);
}

gboolean cd_dbus_applet_add_menu_items (dbusApplet *pDbusApplet, GPtrArray *pItems, GError **error)
{
	if (myData.pModuleMainMenu == NULL/** || myData.pModuleSubMenu == NULL*/ || pDbusApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'AddMenuItems' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return FALSE;
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
	GHashTable *pItem;
	GtkWidget *pMenu;
	GSList *group = NULL;
	GValue *v;
	guint i;
	for (i = 0; i < pItems->len; i ++)
	{
		pItem = g_ptr_array_index (pItems, i);
		
		// get its properties.
		const gchar *cLabel = NULL, *cIcon = NULL, *cToolTip = NULL;
		int iType = 0, iMenuID = -1, id = i, iGroupID = 0;
		gboolean bState = FALSE;
		gpointer data;
		
		v = g_hash_table_lookup (pItem, "type");
		if (v && G_VALUE_HOLDS_INT (v))
			iType = g_value_get_int (v);
		
		v = g_hash_table_lookup (pItem, "label");
		if (v && G_VALUE_HOLDS_STRING (v))
			cLabel = g_value_get_string (v);
		
		v = g_hash_table_lookup (pItem, "id");
		if (v && G_VALUE_HOLDS_INT (v))
			id = g_value_get_int (v);
		data = GINT_TO_POINTER (id);
		
		if (iType == 0 || iType == 1)
		{
			v = g_hash_table_lookup (pItem, "icon");
			if (v && G_VALUE_HOLDS_STRING (v))
				cIcon = g_value_get_string (v);
		}
		
		v = g_hash_table_lookup (pItem, "state");
		if (v && G_VALUE_HOLDS_BOOLEAN (v))
			bState = g_value_get_boolean (v);
		
		v = g_hash_table_lookup (pItem, "group");
		if (v && G_VALUE_HOLDS_INT (v))
		{
			iGroupID = g_value_get_int (v);
			group = g_hash_table_lookup (pGroups, &iGroupID);  // si NULL, ca fera un nouveau groupe.
		}
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
		v = g_hash_table_lookup (pItem, "sensitive");
		if (v && G_VALUE_HOLDS_BOOLEAN (v))
			gtk_widget_set_sensitive (pMenuItem, g_value_get_boolean (v));
		
		// set the tooltip
		v = g_hash_table_lookup (pItem, "tooltip");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cToolTip = g_value_get_string (v);
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
		v = g_hash_table_lookup (pItem, "menu");
		if (v && G_VALUE_HOLDS_INT (v))
			iMenuID = g_value_get_int (v);
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
	}
	
	g_hash_table_destroy (pSubMenus);
	g_hash_table_destroy (pGroups);
	gtk_widget_show_all (myData.pModuleMainMenu);
	
	g_object_set (myData.pModuleMainMenu, "height-request", iMenuHeight + iItemHeight, NULL);  // GTK doesn't resize menus correctly, so we have to force it...
	gtk_menu_reposition (GTK_MENU (myData.pModuleMainMenu));
	
	return TRUE;
}


gboolean cd_dbus_applet_bind_shortkey (dbusApplet *pDbusApplet, const gchar **cShortkeys, GError **error)
{
	cd_debug ("%s ()", __func__);
	g_return_val_if_fail (cShortkeys != NULL, FALSE);
	
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	const gchar *cShortkey, *cDescription = "-", *cGroupName = "Configuration", *cKeyName = "shortkey";
	GldiShortkey *pKeyBinding;
	int i;
	GList *kb;
	
	if (pDbusApplet->pShortkeyList == NULL)
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
				(CDBindkeyHandler) cd_dbus_applet_emit_on_shortkey, pDbusApplet),
			pDbusApplet->pShortkeyList = g_list_append (pDbusApplet->pShortkeyList, pKeyBinding);
		}
	}
	else  // just rebind, we consider that the applet wants to rebind the same shortkeys.
	{
		for (i = 0, kb = pDbusApplet->pShortkeyList; cShortkeys[i] != NULL && kb != NULL; i ++, kb = kb->next)
		{
			cShortkey = cShortkeys[i];
			pKeyBinding = kb->data;
			gldi_shortkey_rebind (pKeyBinding, cShortkey, NULL);
		}
	}
	return TRUE;
}


gboolean cd_dbus_applet_get (dbusApplet *pDbusApplet, const gchar *cProperty, GValue *v, GError **error)
{
	cd_debug ("%s (%s)", __func__, cProperty);
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	GldiContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
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
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, x);
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
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, y);
	}
	else if (strcmp (cProperty, "orientation") == 0)
	{
		CairoDockPositionType iScreenBorder = ((! pContainer->bIsHorizontal) << 1) | (! pContainer->bDirectionUp);
		g_value_init (v, G_TYPE_UINT);
		g_value_set_uint (v, iScreenBorder);
	}
	else if (strcmp (cProperty, "container") == 0)
	{
		g_value_init (v, G_TYPE_UINT);
		int iType = _get_container_type (pContainer);
		g_value_set_uint (v, iType);
	}
	else if (strcmp (cProperty, "width") == 0)  // this is the dimension of the icon when it's hovered.
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iWidth);
	}
	else if (strcmp (cProperty, "height") == 0)
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iHeight);
	}
	else if (strncmp (cProperty, "Xid", 3) == 0)
	{
		g_value_init (v, G_TYPE_UINT64);
		g_value_set_uint64 (v, GPOINTER_TO_INT(pIcon->pAppli));
	}
	else if (strcmp (cProperty, "has_focus") == 0)
	{
		gboolean bHasFocus = (pIcon->pAppli != NULL && pIcon->pAppli == gldi_windows_get_active ());
		g_value_init (v, G_TYPE_BOOLEAN);
		g_value_set_boolean (v, bHasFocus);
	}
	else
	{
		g_set_error (error, 1, 1, "the property %s doesn't exist", cProperty);
		return FALSE;
	}
	return TRUE;
}

gboolean cd_dbus_applet_get_all (dbusApplet *pDbusApplet, GHashTable **hProperties, GError **error)
{
	cd_debug ("%s ()", __func__);
	GldiModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	GldiContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
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
	
	GHashTable *h = g_hash_table_new_full (g_str_hash,
		g_str_equal,
		g_free,
		g_free);
	*hProperties = h;
	GValue *v;
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, x);
	g_hash_table_insert (h, g_strdup ("x"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, y);
	g_hash_table_insert (h, g_strdup ("y"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	g_value_set_uint (v, iScreenBorder);
	g_hash_table_insert (h, g_strdup ("orientation"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT);
	int iType = _get_container_type (pContainer);
	g_value_set_uint (v, iType);
	g_hash_table_insert (h, g_strdup ("container"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, iWidth);
	g_hash_table_insert (h, g_strdup ("width"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_INT);
	g_value_set_int (v, iHeight);
	g_hash_table_insert (h, g_strdup ("height"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_UINT64);
	g_value_set_uint64(v, GPOINTER_TO_INT(pIcon->pAppli));
	g_hash_table_insert (h, g_strdup ("Xid"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_BOOLEAN);
	g_value_set_boolean (v, bHasFocus);
	g_hash_table_insert (h, g_strdup ("has_focus"), v);
	
	return TRUE;
}
