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

#include "interface-applet-signals.h"
#include "interface-applet-methods.h"

static inline CairoDockModuleInstance *_get_module_instance_from_dbus_applet (dbusApplet *pDbusApplet)
{
	return pDbusApplet->pModuleInstance;
}

static inline gboolean _get_icon_and_container_from_id (dbusApplet *pDbusApplet, const gchar *cIconID, Icon **pIcon, CairoContainer **pContainer)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
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


static gboolean _applet_set_quick_info (dbusApplet *pDbusApplet, const gchar *cQuickInfo, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	cairo_dock_set_quick_info (pIcon, pContainer, cQuickInfo && *cQuickInfo != '\0' ? cQuickInfo : NULL);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_set_label (dbusApplet *pDbusApplet, const gchar *cLabel, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	cairo_dock_set_icon_name (cLabel, pIcon, pContainer);
	cairo_dock_redraw_icon (pIcon, pContainer);  /// needs a function to redraw the label...
	return TRUE;
}

static gboolean _applet_set_icon (dbusApplet *pDbusApplet, const gchar *cImage, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_set_icon_with_default (dbusApplet *pDbusApplet, const gchar **cImages, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pIconContext = cairo_create (pIcon->pIconBuffer);
	int i;
	for (i = 0; cImages[i] != NULL; i ++)
	{
		
		///cairo_dock_set_image_on_icon (pIconContext, cImage, pIcon, pContainer);
	}
	cairo_destroy (pIconContext);
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_set_emblem (dbusApplet *pDbusApplet, const gchar *cImage, gint iPosition, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	
	if (cImage == NULL || *cImage == '\0' || strcmp (cImage, "none") == 0)
	{
		cairo_dock_remove_overlay_at_position (pIcon, iPosition);
	}
	else
	{
		cairo_dock_add_overlay_from_image (pIcon, cImage, iPosition);
	}
	/**CairoEmblem *pEmblem = cairo_dock_make_emblem (cImage, pIcon);
	pEmblem->iPosition = iPosition;
	cairo_dock_draw_emblem_on_icon (pEmblem, pIcon, pContainer);
	cairo_dock_free_emblem (pEmblem);*/
	
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

static gboolean _applet_animate (dbusApplet *pDbusApplet, const gchar *cAnimation, gint iNbRounds, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (CAIRO_DOCK_IS_DOCK (pContainer) && cAnimation != NULL)
	{
		cairo_dock_request_icon_animation (pIcon, pContainer, cAnimation, iNbRounds);
		return TRUE;
	}
	return FALSE;
}

static gboolean _applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	// On empeche l'accumulation de dialogues informatifs.
	cairo_dock_remove_dialog_if_any_full (pIcon, FALSE);  // hors dialogues interactifs.
	
	cairo_dock_show_temporary_dialog_with_icon (message, pIcon, pContainer, 1000 * iDuration, "same icon");
	return TRUE;
}

// deprecated
static gboolean _applet_ask_question (dbusApplet *pDbusApplet, const gchar *cMessage, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	pDbusApplet->pDialog = cairo_dock_show_dialog_with_question (cMessage, pIcon, pContainer, "same icon", (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_question, pDbusApplet, NULL);
	return TRUE;
}

static gboolean _applet_ask_value (dbusApplet *pDbusApplet, const gchar *cMessage, gdouble fInitialValue, gdouble fMaxValue, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	pDbusApplet->pDialog = cairo_dock_show_dialog_with_value (cMessage, pIcon, pContainer, "same icon", fInitialValue, fMaxValue, (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_value, pDbusApplet, NULL);
	return TRUE;
}

static gboolean _applet_ask_text (dbusApplet *pDbusApplet, const gchar *cMessage, const gchar *cInitialText, const gchar *cIconID, GError **error)
{
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	pDbusApplet->pDialog = cairo_dock_show_dialog_with_entry (cMessage, pIcon, pContainer, "same icon", cInitialText, (CairoDockActionOnAnswerFunc) cd_dbus_applet_emit_on_answer_text, pDbusApplet, NULL);
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
	cairo_dock_set_dialog_widget_text_color (pLabel);
	g_free (cLabel);
}
static void _on_dialog_destroyed (dbusApplet *pDbusApplet)
{
	CD_APPLET_ENTER;
	pDbusApplet->pDialog = NULL;
	CD_APPLET_LEAVE();
}
static gboolean _applet_popup_dialog (dbusApplet *pDbusApplet, GHashTable *hDialogAttributes, GHashTable *hWidgetAttributes, const gchar *cIconID, GError **error)
{
	g_return_val_if_fail (hDialogAttributes != NULL, FALSE);
	Icon *pIcon;
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, cIconID, &pIcon, &pContainer))
		return FALSE;
	
	if (pDbusApplet->pDialog)  // on n'autorise qu'un seul dialogue interactif a la fois.
		cairo_dock_dialog_unreference (pDbusApplet->pDialog);
	
	CairoDialogAttribute attr;
	memset (&attr, 0, sizeof (CairoDialogAttribute));
	GValue *v;
	
	// attributs du dialogue.
	gchar *cImageFilePath = NULL;
	v = g_hash_table_lookup (hDialogAttributes, "icon");
	if (v && G_VALUE_HOLDS_STRING (v))
	{
		cImageFilePath = cairo_dock_search_icon_s_path (g_value_get_string (v));
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
	
	gboolean bUseMarkup = FALSE;
	v = g_hash_table_lookup (hDialogAttributes, "use-markup");
	if (v && G_VALUE_HOLDS_BOOLEAN (v))
		bUseMarkup = g_value_get_boolean (v);
	
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
						gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (pScrolledWindow), pOneWidget);
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
						GtkWidget *pBox = _gtk_hbox_new (3);
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

					#if (GTK_MAJOR_VERSION < 3)
					pScale = gtk_hscale_new_with_range (fMinValue, fMaxValue, (fMaxValue - fMinValue) / 100.);
					#else
					pScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, fMinValue, fMaxValue, (fMaxValue - fMinValue) / 100.);
					#endif
					pOneWidget = pScale;
					gtk_scale_set_digits (GTK_SCALE (pScale), iNbDigit);
					gtk_range_set_value (GTK_RANGE (pScale), fInitialValue);
					
					g_object_set (pScale, "width-request", 150, NULL);
					cairo_dock_set_dialog_widget_text_color (pScale);
					
					if (cMinLabel || cMaxLabel)
					{
						GtkWidget *pExtendedWidget = _gtk_hbox_new (0);
						GtkWidget *label = gtk_label_new (cMinLabel);
						GtkWidget *pAlign = gtk_alignment_new (1., 1., 0., 0.);
						gtk_container_add (GTK_CONTAINER (pAlign), label);
						gtk_box_pack_start (GTK_BOX (pExtendedWidget), pAlign, FALSE, FALSE, 0);
						gtk_box_pack_start (GTK_BOX (pExtendedWidget), pScale, FALSE, FALSE, 0);
						label = gtk_label_new (cMaxLabel);
						pAlign = gtk_alignment_new (1., 1., 0., 0.);
						gtk_container_add (GTK_CONTAINER (pAlign), label);
						gtk_box_pack_start (GTK_BOX (pExtendedWidget), pAlign, FALSE, FALSE, 0);
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
					
					#if (GTK_MAJOR_VERSION < 3 && GTK_MINOR_VERSION < 24)
					if (bEditable)
						pOneWidget = gtk_combo_box_entry_new_text ();
					else
						pOneWidget = gtk_combo_box_new_text ();
					#else
					if (bEditable)
						pOneWidget = gtk_combo_box_text_new_with_entry ();
					else
						pOneWidget = gtk_combo_box_text_new ();
					#endif
					pInteractiveWidget = pOneWidget;
					
					if (cValuesList != NULL)
					{
						int i;
						for (i = 0; cValuesList[i] != NULL; i ++)
						{
							#if (GTK_MAJOR_VERSION < 3 && GTK_MINOR_VERSION < 24)
							gtk_combo_box_append_text (GTK_COMBO_BOX (pInteractiveWidget), cValuesList[i]);
							#else
							gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (pInteractiveWidget), cValuesList[i]);
							#endif
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
	
	if (bUseMarkup)
		myDialogsParam.dialogTextDescription.bUseMarkup = TRUE;
	pDbusApplet->pDialog = cairo_dock_build_dialog (&attr, pIcon, pContainer);
	if (pOneWidget)
		gtk_widget_grab_focus (pOneWidget);
	if (bUseMarkup)
		myDialogsParam.dialogTextDescription.bUseMarkup = FALSE;
	
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
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusSubApplet->pApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
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
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusSubApplet->pApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	if (cIconID == NULL || strcmp (cIconID, "any") == 0)  // remove all
	{
		cairo_dock_remove_all_icons_from_applet (pInstance);
	}
	else
	{
		GList *pIconsList = (pInstance->pDock ? (pIcon->pSubDock ? pIcon->pSubDock->icons : NULL) : pInstance->pDesklet->icons);
		Icon *pOneIcon = cairo_dock_get_icon_with_command (pIconsList, cIconID);
		cairo_dock_remove_icon_from_applet (pInstance, pOneIcon);
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
	CairoContainer *pContainer;
	if (! _get_icon_and_container_from_id (pDbusApplet, NULL, &pIcon, &pContainer))
		return FALSE;
	
	if (bStart)
	{
		if (CAIRO_DOCK_IS_DOCK (pContainer))
		{
			cairo_dock_request_icon_attention (pIcon, CAIRO_DOCK (pContainer), cAnimation, 0);  // 0 <=> sans arret.
		}
	}
	else if (pIcon->bIsDemandingAttention)
	{
		cairo_dock_stop_icon_attention (pIcon, CAIRO_DOCK (pContainer));
	}
	return TRUE;
}

gboolean cd_dbus_applet_show_dialog (dbusApplet *pDbusApplet, const gchar *message, gint iDuration, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_show_dialog (pDbusApplet, message, iDuration, NULL, error);
}

// deprecated
gboolean cd_dbus_applet_ask_question (dbusApplet *pDbusApplet, const gchar *message, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_ask_question (pDbusApplet, message, NULL, error);
}

gboolean cd_dbus_applet_ask_value (dbusApplet *pDbusApplet, const gchar *message, gdouble fInitialValue, gdouble fMaxValue, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_ask_value (pDbusApplet, message, fInitialValue, fMaxValue, NULL, error);
}

gboolean cd_dbus_applet_ask_text (dbusApplet *pDbusApplet, const gchar *message, const gchar *cInitialText, GError **error)
{
	cd_debug ("%s (%s)\n", __func__, message);
	return _applet_ask_text (pDbusApplet, message, cInitialText, NULL, error);
}
// end of deprecated

gboolean cd_dbus_applet_popup_dialog (dbusApplet *pDbusApplet, GHashTable *hDialogAttributes, GHashTable *hWidgetAttributes, GError **error)
{
	return _applet_popup_dialog (pDbusApplet, hDialogAttributes, hWidgetAttributes, NULL, error);
}


gboolean cd_dbus_applet_add_data_renderer (dbusApplet *pDbusApplet, const gchar *cType, gint iNbValues, const gchar *cTheme, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	CairoDataRendererAttribute *pRenderAttr = NULL;  // les attributs du data-renderer global.
	if (strcmp (cType, "gauge") == 0)
	{
		CairoGaugeAttribute attr;  // les attributs de la jauge.
		memset (&attr, 0, sizeof (CairoGaugeAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "gauge";
		///attr.cThemePath = cairo_dock_get_gauge_theme_path (cTheme, CAIRO_DOCK_ANY_THEME);
		attr.cThemePath = cairo_dock_get_data_renderer_theme_path ("gauge", cTheme, CAIRO_DOCK_ANY_PACKAGE);
	}
	else if (strcmp (cType, "gauge") == 0)
	{
		CairoGraphAttribute attr;  // les attributs du graphe.
		memset (&attr, 0, sizeof (CairoGraphAttribute));
		pRenderAttr = CAIRO_DATA_RENDERER_ATTRIBUTE (&attr);
		pRenderAttr->cModelName = "graph";
		int w, h;
		cairo_dock_get_icon_extent (pIcon, &w, &h);
		pRenderAttr->iMemorySize = (w > 1 ? w : 32);
		// Line;Plain;Bar;Circle;Plain Circle
		if (cTheme == NULL || strcmp (cTheme, "Line") == 0)
			attr.iType = CAIRO_DOCK_GRAPH_LINE;
		else if (strcmp (cTheme, "Plain") == 0)
			attr.iType = CAIRO_DOCK_GRAPH_PLAIN;
		else if (strcmp (cTheme, "Bar") == 0)
			attr.iType = CAIRO_DOCK_GRAPH_BAR;
		else if (strcmp (cTheme, "Circle") == 0)
			attr.iType = CAIRO_DOCK_GRAPH_CIRCLE;
		else if (strcmp (cTheme, "Plain Circle") == 0)
			attr.iType = CAIRO_DOCK_GRAPH_CIRCLE_PLAIN;
		attr.bMixGraphs = FALSE;
		double *fHighColor = g_new (double, iNbValues*3);
		double *fLowColor = g_new (double, iNbValues*3);
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
		attr.fHighColor = fHighColor;
		attr.fLowColor = fLowColor;
		attr.fBackGroundColor[0] = 0;
		attr.fBackGroundColor[0] = 0;
		attr.fBackGroundColor[0] = 1;
		attr.fBackGroundColor[0] = .4;
	}
	else if (strcmp (cType, "bar") == 0)
	{
		/// A FAIRE...
	}
	
	if (pRenderAttr == NULL)
		return FALSE;
	
	pRenderAttr->iLatencyTime = 500;  // 1/2s
	pRenderAttr->iNbValues = iNbValues;
	//pRenderAttr->bUpdateMinMax = TRUE;
	//pRenderAttr->bWriteValues = TRUE;
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	if (pIcon->pDataRenderer == NULL)
		cairo_dock_add_new_data_renderer_on_icon (pIcon, pContainer, pRenderAttr);
	else
		cairo_dock_reload_data_renderer_on_icon (pIcon, pContainer, pRenderAttr);
	
	return TRUE;
}

gboolean cd_dbus_applet_render_values (dbusApplet *pDbusApplet, GArray *pValues, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
	g_return_val_if_fail (pContainer != NULL, FALSE);
	
	g_return_val_if_fail (pIcon->pIconBuffer != NULL, FALSE);
	cairo_t *pDrawContext = cairo_create (pIcon->pIconBuffer);
	cairo_dock_render_new_data_on_icon (pIcon, pContainer, pDrawContext, (double *)pValues->data);
	cairo_destroy (pDrawContext);
	
	cairo_dock_redraw_icon (pIcon, pContainer);
	return TRUE;
}

gboolean cd_dbus_applet_control_appli (dbusApplet *pDbusApplet, const gchar *cApplicationClass, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
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
			CairoContainer *pContainer = pInstance->pContainer;
			if (pContainer != NULL)
				cairo_dock_redraw_icon (pIcon, pContainer);
		}
	}
	
	g_free (cClass);
	return TRUE;
}

gboolean cd_dbus_applet_show_appli (dbusApplet *pDbusApplet, gboolean bShow, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL && pIcon->Xid != 0, FALSE);
	
	if (bShow)
		cairo_dock_show_xwindow (pIcon->Xid);
	else
		cairo_dock_minimize_xwindow (pIcon->Xid);
	
	return TRUE;
}

gboolean cd_dbus_applet_act_on_appli (dbusApplet *pDbusApplet, const gchar *cAction, GError **error)
{
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL && pIcon->Xid != 0, FALSE);
	
	g_return_val_if_fail (cAction != NULL, FALSE);
	
	if (strcmp (cAction, "minimize") == 0)
		cairo_dock_minimize_xwindow (pIcon->Xid);
	else if (strcmp (cAction, "show") == 0)
		cairo_dock_show_xwindow (pIcon->Xid);
	else if (strcmp (cAction, "toggle-visibility") == 0)
	{
		if (pIcon->bIsHidden)
			cairo_dock_show_xwindow (pIcon->Xid);
		else
			cairo_dock_minimize_xwindow (pIcon->Xid);
	}	
	else if (strcmp (cAction, "maximize") == 0)
		cairo_dock_maximize_xwindow (pIcon->Xid, TRUE);
	else if (strcmp (cAction, "restaure") == 0)
		cairo_dock_maximize_xwindow (pIcon->Xid, FALSE);
	else if (strcmp (cAction, "toggle-size") == 0)
	{
		cairo_dock_maximize_xwindow (pIcon->Xid, ! pIcon->bIsMaximized);
	}
	else if (strcmp (cAction, "close") == 0)
		cairo_dock_close_xwindow (pIcon->Xid);
	else if (strcmp (cAction, "kill") == 0)
		cairo_dock_kill_xwindow (pIcon->Xid);
	else
	{
		cd_warning ("invalid action '%s' on window %s", cAction, pIcon->cName);
	}
	
	return TRUE;
}

gboolean cd_dbus_applet_populate_menu (dbusApplet *pDbusApplet, const gchar **pLabels, GError **error)
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

gboolean cd_dbus_applet_add_menu_items (dbusApplet *pDbusApplet, GPtrArray *pItems, GError **error)
{
	if (myData.pModuleMainMenu == NULL/** || myData.pModuleSubMenu == NULL*/ || pDbusApplet != myData.pCurrentMenuDbusApplet)
	{
		cd_warning ("the 'AddMenuItems' method can only be used to populate the menu that was summoned from a right-click on your applet !\nthat is to say, after you received a 'build-menu' event.");
		return FALSE;
	}
	
	// get the position of our items in the menu.
	int iPosition = myData.iMenuPosition;
	
	// insert a separator
	gtk_menu_shell_insert (GTK_MENU_SHELL (myData.pModuleMainMenu), gtk_separator_menu_item_new (), iPosition++);
	
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
	GtkWidget *pMenuItem, *pMenu;
	GSList *group = NULL;
	GValue *v;
	guint i;
	for (i = 0; i < pItems->len; i ++)
	{
		pItem = g_ptr_array_index (pItems, i);
		
		// on recupere ses proprietes.
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
		
		// on cree l'item suivant son type.
		switch (iType)
		{
			case 0 :  // normal entry
				pMenuItem = gtk_image_menu_item_new_with_label (cLabel);
				g_signal_connect (G_OBJECT (pMenuItem), "activate", G_CALLBACK (cd_dbus_emit_on_menu_select), data);
			break;
			case 1:  // sub-menu
				pMenuItem = gtk_image_menu_item_new_with_label (cLabel);
				GtkWidget *pSubMenu = gtk_menu_new ();
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (pMenuItem), pSubMenu);
				int *pID = g_new (int, 1);
				*pID = id;
				g_hash_table_insert (pSubMenus, pID, pSubMenu);
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
		
		// on lui rajoute son icone et son tooltip.
		if (iType == 0 || iType == 1)
		{
			v = g_hash_table_lookup (pItem, "icon");
			if (v && G_VALUE_HOLDS_STRING (v))
			{
				cIcon = g_value_get_string (v);
				if (cIcon)
				{
					GtkWidget *image = NULL;
					if (*cIcon == '/')
					{
						GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size (cIcon, 16, 16, NULL);
						if (pixbuf)
						{
							image = gtk_image_new_from_pixbuf (pixbuf);
							g_object_unref (pixbuf);
						}
					}
					else
					{
						image = gtk_image_new_from_stock (cIcon, GTK_ICON_SIZE_MENU);
					}
#if (GTK_MAJOR_VERSION > 2 || GTK_MINOR_VERSION >= 16)
					gtk_image_menu_item_set_always_show_image (GTK_IMAGE_MENU_ITEM (pMenuItem), TRUE);
#endif
					gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pMenuItem), image);
				}
			}
		}
		
		v = g_hash_table_lookup (pItem, "tooltip");
		if (v && G_VALUE_HOLDS_STRING (v))
		{
			cToolTip = g_value_get_string (v);
			gtk_widget_set_tooltip_text (pMenuItem, cToolTip);
		}
		
		// on l'insere dans son menu.
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
	}
	
	g_hash_table_destroy (pSubMenus);
	g_hash_table_destroy (pGroups);
	gtk_widget_show_all (myData.pModuleMainMenu);
	
	return TRUE;
}


gboolean cd_dbus_applet_bind_shortkey (dbusApplet *pDbusApplet, const gchar **cShortkeys, GError **error)
{
	cd_debug ("%s ()", __func__);
	g_return_val_if_fail (cShortkeys != NULL, FALSE);
	
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	const gchar *cShortkey, *cDescription = "Pouet", *cGroupName = "Configuration", *cKeyName = "shortkey";
	CairoKeyBinding *pKeyBinding;
	int i;
	GList *kb;
	
	if (pDbusApplet->pShortkeyList == NULL)
	{
		for (i = 0; cShortkeys[i] != NULL; i ++)
		{
			cShortkey = cShortkeys[i];
			pKeyBinding = cd_keybinder_bind (cShortkey,
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
		for (i = 0, kb = pDbusApplet->pShortkeyList; cShortkeys[i] != NULL, kb != NULL; i ++, kb = kb->next)
		{
			cShortkey = cShortkeys[i];
			pKeyBinding = kb->data;
			cd_keybinder_rebind (pKeyBinding, cShortkey, NULL);
		}
	}
	/** // on enleve les vieux raccourcis dont l'applet ne veut plus.
	sk = pDbusApplet->pShortkeyList;
	while (sk != NULL)
	{
		next_sk = sk->next;
		key = sk->data;
		
		// on cherche ce raccourci parmi la nouvelle liste.
		for (i = 0; cShortkeys[i] != NULL; i ++)
		{
			cShortkey = cShortkeys[i];
			if (strcmp (cShortkey, key) == 0)
				break;
		}
		if (! cShortkeys[i])  // raccourci non trouve dans la nouvelle liste => on l'enleve
		{
			//g_print (" shortkey '%s' not wanted anymore\n", key);
			cd_keybinder_unbind (key, (CDBindkeyHandler) cd_dbus_applet_emit_on_shortkey);
			pDbusApplet->pShortkeyList = g_list_delete_link (pDbusApplet->pShortkeyList, sk);
		}
		sk = next_sk;
	}
	
	// on lie les nouveaux raccourcis non encore lies.
	gboolean bCouldBind;
	for (i = 0; cShortkeys[i] != NULL; i ++)
	{
		cShortkey = cShortkeys[i];
		for (sk = pDbusApplet->pShortkeyList; sk != NULL; sk = sk->next)  // on regarde si ce nouveau raccourci est deja lie.
		{
			key = sk->data;
			if (strcmp (cShortkey, key) == 0)  // ce raccourci a deja ete lie avec succes precedemment.
				break;
		}
		if (! sk)  // raccourci non encore lie => on lie.
		{
			//g_print (" shortkey '%s' wanted\n", cShortkey);
			bCouldBind = cd_keybinder_bind (cShortkey, (CDBindkeyHandler) cd_dbus_applet_emit_on_shortkey, pDbusApplet);
			if (bCouldBind)
				pDbusApplet->pShortkeyList = g_list_prepend (pDbusApplet->pShortkeyList, g_strdup (cShortkey));
			cd_debug ("*** bind %s: %d", cShortkey, bCouldBind);
		}
	}*/
	return TRUE;
}


gboolean cd_dbus_applet_get (dbusApplet *pDbusApplet, const gchar *cProperty, GValue *v, GError **error)
{
	cd_debug ("%s (%s)", __func__, cProperty);
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
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
		g_value_set_uint (v, pContainer->iType);
	}
	else if (strcmp (cProperty, "width") == 0)
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		if (pInstance->pDock)
		{
			double a = cairo_dock_get_max_scale (pContainer);
			double s = pInstance->pDock->fMagnitudeMax;
			iWidth /= (1 + a) / (1 + s*a);
		}
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iWidth);
	}
	else if (strcmp (cProperty, "height") == 0)
	{
		int iWidth, iHeight;
		cairo_dock_get_icon_extent (pIcon, &iWidth, &iHeight);
		if (pInstance->pDock)
		{
			double a = cairo_dock_get_max_scale (pContainer);
			double s = pInstance->pDock->fMagnitudeMax;
			iHeight /= (1 + a) / (1 + s*a);
		}
		g_value_init (v, G_TYPE_INT);
		g_value_set_int (v, iHeight);
	}
	else if (strncmp (cProperty, "Xid", 3) == 0)
	{
		Window Xid = pIcon->Xid;
		g_value_init (v, G_TYPE_UINT64);
		g_value_set_uint64 (v, Xid);
	}
	else if (strcmp (cProperty, "has_focus") == 0)
	{
		gboolean bHasFocus = (pIcon->Xid != 0 && pIcon->Xid == cairo_dock_get_current_active_window ());
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
	CairoDockModuleInstance *pInstance = _get_module_instance_from_dbus_applet (pDbusApplet);
	g_return_val_if_fail (pInstance != NULL, FALSE);
	
	Icon *pIcon = pInstance->pIcon;
	g_return_val_if_fail (pIcon != NULL, FALSE);
	
	CairoContainer *pContainer = pInstance->pContainer;
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
	if (pInstance->pDock)
	{
		double a = cairo_dock_get_max_scale (pContainer);
		double s = pInstance->pDock->fMagnitudeMax;
		iWidth /= (1 + a) / (1 + s*a);
		iHeight /= (1 + a) / (1 + s*a);
	}
	
	Window Xid = pIcon->Xid;
	gboolean bHasFocus = (pIcon->Xid != 0 && pIcon->Xid == cairo_dock_get_current_active_window ());
	
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
	g_value_set_uint (v, pContainer->iType);
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
	g_value_set_uint64(v, Xid);
	g_hash_table_insert (h, g_strdup ("Xid"), v);
	
	v = g_new0 (GValue, 1);
	g_value_init (v, G_TYPE_BOOLEAN);
	g_value_set_boolean (v, bHasFocus);
	g_hash_table_insert (h, g_strdup ("has_focus"), v);
	
	return TRUE;
}
