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

#ifdef XGAMMA_WAYLAND
#define _GNU_SOURCE // needed for memfd_create
#include <cairo-dock-wayland-manager.h>
#include "wayland-wlr-gamma-control-client-protocol.h"

#include <sys/mman.h> // memfd_create
#include <stdlib.h> // mkostemp
#include <unistd.h> // unlink
#include <fcntl.h>
#include <gdk/gdkwayland.h>

#endif

#ifdef XGAMMA_X11
#include <gdk/gdkx.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/xf86vmode.h>
#endif

#include <cairo-dock.h>
#include "applet-struct.h"
#include "applet-xgamma.h"

gboolean s_bX11 = FALSE;
gboolean s_bWayland = FALSE;

#ifdef XGAMMA_WAYLAND
struct zwlr_gamma_control_manager_v1 *s_pManager = NULL;
static CDGamma s_gamma_req = {1.0, 1.0, 1.0};

gboolean xgamma_setup_wayland (void)
{
	const GldiWaylandProtocolInfo *info = gldi_wayland_get_global (zwlr_gamma_control_manager_v1_interface.name);
	if (info) s_bWayland = TRUE;
	return s_bWayland;
}
#endif

#ifdef XGAMMA_X11
gboolean xgamma_setup_x11 (void)
{
	Display *dpy = gdk_x11_get_default_xdisplay ();
	if (dpy == NULL)
	{
		// should not happen, backend checked by caller
		cd_warning ("Xgamma : unable to get X display");
		return FALSE;
	}
	
	int MajorVersion, MinorVersion;
	if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion))
	{
		cd_warning ("Xgamma : unable to query video extension version");
		return FALSE;
	}
	
	int EventBase, ErrorBase;
	if (!XF86VidModeQueryExtension(dpy, &EventBase, &ErrorBase))
	{
		cd_warning ("Xgamma : unable to query video extension information");
		return FALSE;
	}
	
	s_bX11 = TRUE;
	return TRUE;
}
#endif


#ifdef XGAMMA_WAYLAND

typedef struct _XGammaMonitor {
	struct zwlr_gamma_control_v1 *pGammaControl;
	uint32_t uGammaSize;
} XGammaMonitor;

static GHashTable *s_pMonitors; // key: GdkMonitor*, value: XGammaMonitor* (owned)

#ifdef __linux__
#define HAVE_MEMFD
#endif

#ifdef __FreeBSD__
#include <sys/param.h>
#if __FreeBSD_version >= 1300000
#define HAVE_MEMFD
#endif
#endif

#ifdef __NetBSD__
#include <sys/param.h>
#if __NetBSD_Version__ >= 1100000000
#define HAVE_MEMFD
#endif
#endif

static void _set_gamma_wayland (const CDGamma *pGamma, XGammaMonitor *pMonitor)
{
	// we need to create an anonymous fd and fill it with the gamma table
	int fd = -1;
#ifdef HAVE_MEMFD
	// memfd_create () is supported on Linux 3.17 and newer; also FreeBSD 13
	// and NetBSD 11
	fd = memfd_create ("Cairo-Dock-XGamma", MFD_CLOEXEC | MFD_ALLOW_SEALING);
#else
	// mkostemp () is supported on at least FreeBSD 10.0 (released in 2014),
	// NetBSD 7.0 (2015), OpenBSD 5.7 (2015) and DragonflyBSD 4.8 (2017)
	char *tmpname = g_strdup_printf ("%s/cd-xgamma-XXXXXX", g_get_tmp_dir ());
	fd = mkostemp (tmpname, O_CLOEXEC);
	if (fd >= 0) unlink (tmpname); // delete the file, only keep the fd
	g_free (tmpname);
#endif
	if (fd == -1)
	{
		cd_warning ("Cannot create memfd");
		return;
	}
	
	const size_t len = pMonitor->uGammaSize;
	const off_t n_bytes = len * 3UL * sizeof (uint16_t);
	if (ftruncate (fd, n_bytes))
	{
		close (fd);
		cd_warning ("Cannot set file size");
		return;
	}
	
	void *data = mmap (NULL, n_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED)
	{
		close (fd);
		cd_warning ("Cannot mmap");
		return;
	}
	
	uint16_t *tbl = (uint16_t*)data;
	size_t i;
	for (i = 0; i < 3; i++)
	{
		const size_t base = i * len;
		float gamma;
		switch (i)
		{
			case 0: gamma = pGamma->red; break;
			case 1: gamma = pGamma->green; break;
			case 2: gamma = pGamma->blue; break;
		}
		
		gamma = 1.0f / gamma;
		size_t j;
		for (j = 0; j < len; j++)
		{
			float x = ((float)j) / ((float)len);
			x = powf (x, gamma);
			tbl[base + j] = (uint16_t)(UINT16_MAX * x);
		}
	}
	
	munmap (data, n_bytes);
#ifdef HAVE_MEMFD
	// seals available since Linux 3.17
	if (fcntl (fd, F_ADD_SEALS, F_SEAL_GROW | F_SEAL_SHRINK | F_SEAL_WRITE) == -1)
		cd_warning ("Cannot seal fd");
#endif
	
	zwlr_gamma_control_v1_set_gamma (pMonitor->pGammaControl, fd);
	close (fd);
}

static void _set_gamma_wayland2 (G_GNUC_UNUSED gpointer key, gpointer value,
	G_GNUC_UNUSED gpointer user_data)
{
	XGammaMonitor *pMonitor = (XGammaMonitor*)value;
	if (pMonitor->pGammaControl && pMonitor->uGammaSize)
		_set_gamma_wayland (&s_gamma_req, pMonitor);
}

static void _got_gamma_size (void *data,
	G_GNUC_UNUSED struct zwlr_gamma_control_v1 *zwlr_gamma_control_v1, uint32_t size)
{
	XGammaMonitor *pMonitor = (XGammaMonitor*)data;
	pMonitor->uGammaSize = size;
	if (s_gamma_req.red != 1.0 || s_gamma_req.green != 1.0 || s_gamma_req.blue != 1.0)
		_set_gamma_wayland (&s_gamma_req, pMonitor);
}

static void _gamma_control_failed (void *data, struct zwlr_gamma_control_v1 *pGammaControl)
{
	cd_warning ("Gamma control is unavailable");
	zwlr_gamma_control_v1_destroy (pGammaControl);
	XGammaMonitor *pMonitor = (XGammaMonitor*)data;
	pMonitor->pGammaControl = NULL;
	pMonitor->uGammaSize = 0;
}

static struct zwlr_gamma_control_v1_listener s_gamma_control_listener = {
	.gamma_size = _got_gamma_size,
	.failed = _gamma_control_failed
};

void _delete_monitor (gpointer data)
{
	XGammaMonitor *pMonitor = (XGammaMonitor*)data;
	if (pMonitor->pGammaControl) zwlr_gamma_control_v1_destroy (pMonitor->pGammaControl);
	g_free (pMonitor);
}

gboolean _xgamma_wayland_monitor_added (G_GNUC_UNUSED gpointer user_data, GdkMonitor *monitor)
{
	if (!g_hash_table_contains (s_pMonitors, monitor))
	{
		XGammaMonitor *pMonitor = g_new0 (XGammaMonitor, 1);
		struct wl_output *output = gdk_wayland_monitor_get_wl_output (monitor);
		pMonitor->pGammaControl = zwlr_gamma_control_manager_v1_get_gamma_control (s_pManager, output);
		zwlr_gamma_control_v1_add_listener (pMonitor->pGammaControl, &s_gamma_control_listener, pMonitor);
		g_hash_table_insert (s_pMonitors, monitor, pMonitor);
	}
	return GLDI_NOTIFICATION_LET_PASS;
}

gboolean _xgamma_wayland_monitor_removed (G_GNUC_UNUSED gpointer user_data, GdkMonitor *monitor)
{
	g_hash_table_remove (s_pMonitors, monitor);
	return GLDI_NOTIFICATION_LET_PASS;
}

#endif


void xgamma_init (void)
{
#ifdef XGAMMA_WAYLAND
	if (s_bWayland)
	{
		s_gamma_req.red = 1.0f;
		s_gamma_req.green = 1.0f;
		s_gamma_req.blue = 1.0f;
		
		const GldiWaylandProtocolInfo *info = gldi_wayland_get_global (zwlr_gamma_control_manager_v1_interface.name);
		struct wl_registry *registry = gldi_wayland_get_registry ();
		if (! (info && registry))
		{
			cd_warning ("wlr-gamma-control disappeared");
			return;
		}
		
		uint32_t version = info->version;
		if (version > (uint32_t)zwlr_gamma_control_manager_v1_interface.version)
			version = zwlr_gamma_control_manager_v1_interface.version;
		
		s_pManager = wl_registry_bind (registry, info->id, &zwlr_gamma_control_manager_v1_interface, version);
		if (!s_pManager)
		{
			cd_warning ("Cannot bind wlr-gamma-control manager");
			return;
		}
		
		s_pMonitors = g_hash_table_new_full (NULL, NULL, NULL, _delete_monitor);
		int n_outputs = 0, i;
		GdkMonitor *const *monitors = gldi_desktop_get_monitors (&n_outputs);
		for (i = 0; i < n_outputs; i++) _xgamma_wayland_monitor_added (NULL, monitors[i]);
		
		gldi_object_register_notification (&myDesktopMgr, NOTIFICATION_DESKTOP_MONITOR_ADDED,
			(GldiNotificationFunc) _xgamma_wayland_monitor_added, GLDI_RUN_FIRST, NULL);
		gldi_object_register_notification (&myDesktopMgr, NOTIFICATION_DESKTOP_MONITOR_REMOVED,
			(GldiNotificationFunc) _xgamma_wayland_monitor_removed, GLDI_RUN_FIRST, NULL);
		
		return;
	}
#endif

#ifdef XGAMMA_X11
	if (s_bX11)
	{
		// nothing to do
		return;
	}
#endif
	
	cd_warning ("no supported method for setting gamma");
}

void xgamma_stop (void)
{
#ifdef XGAMMA_WAYLAND
	if (s_bWayland)
	{
		gldi_object_remove_notification (&myDesktopMgr, NOTIFICATION_DESKTOP_MONITOR_ADDED,
			(GldiNotificationFunc) _xgamma_wayland_monitor_added, NULL);
		gldi_object_remove_notification (&myDesktopMgr, NOTIFICATION_DESKTOP_MONITOR_REMOVED,
			(GldiNotificationFunc) _xgamma_wayland_monitor_removed, NULL);
		
		if (s_pMonitors)
		{
			g_hash_table_destroy (s_pMonitors);
			s_pMonitors = NULL;
		}
		if (s_pManager)
		{
			zwlr_gamma_control_manager_v1_destroy (s_pManager);
			s_pManager = NULL;
		}
	}
#endif
}


static void _clamp_gamma (float *x)
{
	if (*x < GAMMA_MIN) *x = GAMMA_MIN;
	else if (*x > GAMMA_MAX) *x = GAMMA_MAX;
}

static inline double _gamma_to_percent (double fGamma)
{
	if (fGamma < GAMMA_MIN)
		fGamma = GAMMA_MIN;
	if (fGamma > GAMMA_MAX)
		fGamma = GAMMA_MAX;
	return 100. * (fGamma - GAMMA_MIN) / (GAMMA_MAX - GAMMA_MIN);
}

static inline double _percent_to_gamma (double fGammaPercent)
{
	if (fGammaPercent < 0)
		fGammaPercent = 0;
	if (fGammaPercent > 100)
		fGammaPercent = 100;
	return GAMMA_MIN + fGammaPercent / 100. * (GAMMA_MAX - GAMMA_MIN);
}

void xgamma_add_gamma (CDGamma *pGamma, gint iNbSteps)
{
	if (iNbSteps == 0)
		return;
	double fGamma = xgamma_get_gamma (pGamma);
	double fGammaPercent = _gamma_to_percent (fGamma);
	fGammaPercent += iNbSteps * myConfig.iScrollVariation;
	double fNewGamma = _percent_to_gamma (fGammaPercent);
	double f = fNewGamma / fGamma;
	myData.Xgamma.red *= f;
	myData.Xgamma.green *= f;
	myData.Xgamma.blue *= f;
	
	xgamma_set_gamma (&myData.Xgamma);
}

double xgamma_get_gamma (CDGamma *pGamma)
{
	g_return_val_if_fail (pGamma != NULL, 1);
	
#ifdef XGAMMA_WAYLAND
	if (s_bWayland)
	{
		*pGamma = s_gamma_req;
	}
#endif
	
#ifdef XGAMMA_X11
	if (s_bX11)
	{
		Display *dpy = gdk_x11_get_default_xdisplay ();
		
		XF86VidModeGamma gamma1;
		if (!XF86VidModeGetGamma (dpy, DefaultScreen (dpy), &gamma1))
		{
			cd_warning ("Xgamma : unable to query gamma correction");
			return 1.;
		}
		pGamma->red = gamma1.red;
		pGamma->green = gamma1.green;
		pGamma->blue = gamma1.blue;
	}
#endif
	
	double fGamma = (pGamma->red + pGamma->blue + pGamma->green) / 3;
	cd_debug ("Gamma: %f, %f, %f, %f",
		pGamma->red, pGamma->blue, pGamma->green, fGamma);
	return fGamma;
}

void xgamma_set_gamma (CDGamma *pGamma)
{
	g_return_if_fail (pGamma != NULL);
	
	_clamp_gamma (&pGamma->red);
	_clamp_gamma (&pGamma->green);
	_clamp_gamma (&pGamma->blue);
	
#ifdef XGAMMA_WAYLAND
	if (s_bWayland)
	{
		g_return_if_fail (s_pMonitors != NULL);
		s_gamma_req = *pGamma;
		g_hash_table_foreach (s_pMonitors, _set_gamma_wayland2, NULL);
	}
#endif
	
#ifdef XGAMMA_X11
	if (s_bX11)
	{
		Display *dpy = gdk_x11_get_default_xdisplay ();
		
		XF86VidModeGamma gamma1;
		gamma1.red = pGamma->red;
		gamma1.green = pGamma->green;
		gamma1.blue = pGamma->blue;
		
		if (!XF86VidModeSetGamma(dpy, DefaultScreen (dpy), &gamma1))
		{
			cd_warning ("Xgamma : unable to set gamma correction");
			return;
		}
	}
#endif
	
	if (myConfig.cDefaultTitle == NULL)
	{
		double fGamma = (pGamma->red + pGamma->blue + pGamma->green) / 3;
		cd_gamma_display_gamma_on_label (fGamma);
	}
}



static void on_scale_value_changed (GtkRange *range, gpointer data)
{
	int iChannelNumber = GPOINTER_TO_INT (data);
	cd_message ("%s (%d, %.2f)", __func__, iChannelNumber, gtk_range_get_value (GTK_RANGE (range)));
	
	switch (iChannelNumber)
	{
		case 0 :
		{
			double fOldGamma = (myData.Xgamma.red + myData.Xgamma.blue + myData.Xgamma.green) / 3;
			double fNewGamma = gtk_range_get_value (GTK_RANGE (range));
			double fDeltaGamma = fNewGamma - fOldGamma;
			
			myData.Xgamma.red += fDeltaGamma;
			myData.Xgamma.red = MAX (GAMMA_MIN, MIN (GAMMA_MAX, myData.Xgamma.red));
			myData.Xgamma.green += fDeltaGamma;
			myData.Xgamma.green = MAX (GAMMA_MIN, MIN (GAMMA_MAX, myData.Xgamma.green));
			myData.Xgamma.blue += fDeltaGamma;
			myData.Xgamma.blue = MAX (GAMMA_MIN, MIN (GAMMA_MAX, myData.Xgamma.blue));
			
			g_signal_handler_block (myData.pRedScale, myData.iRedScaleSignalID);
			g_signal_handler_block (myData.pGreenScale, myData.iGreenScaleSignalID);
			g_signal_handler_block (myData.pBlueScale, myData.iBlueScaleSignalID);
			
			gtk_range_set_value (GTK_RANGE (myData.pRedScale), myData.Xgamma.red);
			gtk_range_set_value (GTK_RANGE (myData.pGreenScale), myData.Xgamma.green);
			gtk_range_set_value (GTK_RANGE (myData.pBlueScale), myData.Xgamma.blue);
			
			g_signal_handler_unblock (myData.pRedScale, myData.iRedScaleSignalID);
			g_signal_handler_unblock (myData.pGreenScale, myData.iGreenScaleSignalID);
			g_signal_handler_unblock (myData.pBlueScale, myData.iBlueScaleSignalID);
		}
		break ;
		
		case 1 :
			myData.Xgamma.red = gtk_range_get_value (GTK_RANGE (range));
		break ;
		
		case 2 :
			myData.Xgamma.blue = gtk_range_get_value (GTK_RANGE (range));
		break ;
		
		case 3 :
			myData.Xgamma.green = gtk_range_get_value (GTK_RANGE (range));
		break ;
		
	}
	xgamma_set_gamma (&myData.Xgamma);
}
static GtkWidget *_xgamma_add_channel_widget (GtkWidget *pInteractiveWidget, const gchar *cLabel, const gchar *cColor, int iChannelNumber, guint *iSignalID, double fChannelGamma)
{
	GtkWidget *pLabel = gtk_label_new (NULL);
	if (cColor)
	{
		gchar *cText = g_strdup_printf ("<span color=\"%s\">%s</span>", cColor, cLabel);
		gtk_label_set_markup (GTK_LABEL (pLabel), cText);
		g_free (cText);
	}
	else
	{
		gtk_label_set_text (GTK_LABEL (pLabel), cLabel);
		gldi_dialog_set_widget_text_color (pLabel); // default colour
	}

	gtk_grid_attach (GTK_GRID (pInteractiveWidget),
		pLabel,
		1,
		iChannelNumber+1,
		1,
		1);

	GtkWidget *pHScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, GAMMA_MIN, GAMMA_MAX, .02);
	gtk_scale_set_digits (GTK_SCALE (pHScale), 2);
	gtk_range_set_value (GTK_RANGE (pHScale), fChannelGamma);
	g_object_set (pHScale, "width-request", 150, NULL);
	
	*iSignalID = g_signal_connect (G_OBJECT (pHScale),
		"value-changed",
		G_CALLBACK (on_scale_value_changed),
		GINT_TO_POINTER (iChannelNumber));
	gtk_grid_attach (GTK_GRID (pInteractiveWidget),
		pHScale,
		2,
		iChannelNumber+1,
		1,
		1);

	return pHScale;
}
void xgamma_create_scales_widget (double fGamma, CDGamma *pGamma)
{
	myData.pWidget = gtk_grid_new ();

	myData.pGlobalScale = _xgamma_add_channel_widget (myData.pWidget, D_("Gamma :"), NULL, 0, &myData.iGloalScaleSignalID, fGamma);

	myData.pRedScale = _xgamma_add_channel_widget (myData.pWidget, D_("Red :"), "red", 1, &myData.iRedScaleSignalID, pGamma->red);

	myData.pGreenScale = _xgamma_add_channel_widget (myData.pWidget, D_("Green :"), "green", 2, &myData.iGreenScaleSignalID, pGamma->green);

	myData.pBlueScale = _xgamma_add_channel_widget (myData.pWidget, D_("Blue :"), "blue", 3, &myData.iBlueScaleSignalID, pGamma->blue);

	gtk_widget_show_all (myData.pWidget);
}


static void _xgamma_apply_values (int iClickedButton, GtkWidget *pWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cd_message ("%s (ok)", __func__);
	}
	else
	{
		cd_message ("%s (cancel)", __func__);
		myData.Xgamma = myData.XoldGamma;
		xgamma_set_gamma (&myData.Xgamma);
	}
	gldi_dialog_hide (myData.pDialog);
	gldi_object_ref (GLDI_OBJECT(myData.pDialog));  // pour garder notre dialogue en vie.
	
}
CairoDialog *xgamma_build_dialog (void)
{
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));
	attr.cText = D_("Set up gamma:");
	attr.pInteractiveWidget = myData.pWidget;
	const gchar *cButtons[3] = {"ok", "cancel", NULL};
	attr.cButtonsImage = cButtons;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) _xgamma_apply_values;
	attr.pUserData = myApplet;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	return gldi_dialog_new (&attr);
}

void xgamma_build_and_show_widget (void)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_if_fail (fGamma >= 0);
	myData.XoldGamma = myData.Xgamma;
	
	xgamma_create_scales_widget (fGamma, &myData.Xgamma);
	
	if (myDock)
	{
		myData.pDialog = xgamma_build_dialog ();
	}
	else
	{
		gldi_desklet_add_interactive_widget (myDesklet, myData.pWidget);
		CD_APPLET_SET_DESKLET_RENDERER (NULL);  // pour empecher le clignotement du au double-buffer.
		CD_APPLET_SET_STATIC_DESKLET;
	}
}

static void on_scale_value_changed_simple (GtkRange *range, gpointer data)
{
	double fGammaPercent = gtk_range_get_value (GTK_RANGE (range));
	double fGamma = _percent_to_gamma (fGammaPercent);
	
	myData.Xgamma.red = fGamma;
	myData.Xgamma.blue = fGamma;
	myData.Xgamma.green = fGamma;
	xgamma_set_gamma (&myData.Xgamma);
}
static void _xgamma_apply_value_simple (int iClickedButton, GtkWidget *pWidget, gpointer data, CairoDialog *pDialog)
{
	if (iClickedButton == 0 || iClickedButton == -1)  // ok button or Enter.
	{
		cd_message ("%s (ok)", __func__);
	}
	else
	{
		cd_message ("%s (cancel)", __func__);
		myData.Xgamma = myData.XoldGamma;
		xgamma_set_gamma (&myData.Xgamma);
	}
}
CairoDialog *xgamma_build_dialog_simple (void)
{
	double fGamma = xgamma_get_gamma (&myData.Xgamma);
	g_return_val_if_fail (fGamma >= 0, NULL);
	double fGammaPercent = _gamma_to_percent (fGamma);
	myData.XoldGamma = myData.Xgamma;
	
	CairoDialogAttr attr;
	memset (&attr, 0, sizeof (CairoDialogAttr));

	GtkWidget *pHScale = gtk_scale_new_with_range (GTK_ORIENTATION_HORIZONTAL, 0, 100., 1.);
	gtk_scale_set_digits (GTK_SCALE (pHScale), 0);
	gtk_range_set_value (GTK_RANGE (pHScale), fGammaPercent);
	g_object_set (pHScale, "width-request", 150, NULL);
	g_signal_connect (G_OBJECT (pHScale),
		"value-changed",
		G_CALLBACK (on_scale_value_changed_simple),
		NULL);
	gldi_dialog_set_widget_text_color (pHScale);
	
	attr.cText = D_("Set up gamma:");
	attr.pInteractiveWidget = pHScale;
	const gchar *cButtons[3] = {"ok", "cancel", NULL};
	attr.cButtonsImage = cButtons;
	attr.pActionFunc = (CairoDockActionOnAnswerFunc) _xgamma_apply_value_simple;
	attr.pUserData = myApplet;
	attr.pIcon = myIcon;
	attr.pContainer = myContainer;
	return gldi_dialog_new (&attr);
}


void cd_gamma_display_gamma_on_label (double fGamma)
{
	double fGammaPercent = _gamma_to_percent (fGamma);
	gchar *cLabel = g_strdup_printf ("%s: %d%%", D_("Luminosity"), (int)fGammaPercent);
	gldi_icon_set_name (myIcon, cLabel);
	g_free (cLabel);
}
