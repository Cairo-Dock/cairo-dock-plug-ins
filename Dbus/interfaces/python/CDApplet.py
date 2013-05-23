# This is a part of the external applets for Cairo-Dock
# Copyright : (C) 2010-2011 by Fabounet
# E-mail : fabounet@glx-dock.org
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

# Base class for Cairo-Dock's applets.
# Make your own class derive from a CDApplet, and override the functions you need (the ones which don't start with an underscore).

####################
### dependancies ###
####################
from __future__ import print_function
import sys
import os.path

try:
	import glib
	import gobject
	g_bMainLoopInGObject = True
except:
	from gi.repository import GLib as glib
	from gi.repository import GObject as gobject
	g_bMainLoopInGObject = False

import gettext
import dbus
from dbus.mainloop.glib import DBusGMainLoop
try:
	import ConfigParser # python 2
except:
	import configparser # python 3

# Enabling threading support (only for GLib < 2.32)
if glib.glib_version[0] < 2 or (glib.glib_version[0] == 2 and glib.glib_version[1] < 32):
	gobject.threads_init()

# enable dbus thread support
dbus.mainloop.glib.threads_init()

DBusGMainLoop(set_as_default=True)

INSTALL_PREFIX = os.path.abspath("..")  # applets are launched from their install directory, so ".." is the folder containing all applets.

GETTEXT_NAME = 'cairo-dock-plugins-extra'
LOCALE_DIR = INSTALL_PREFIX + '/locale'  # user version of /usr/share/locale
try:
	translations = gettext.translation(GETTEXT_NAME, LOCALE_DIR)
	try:
		_ = translations.ugettext  # python 2 => force the use of UTF8
	except:
		_ = translations.gettext
except: # e.g. No translation file found for this domain
	_ = lambda x: x

####################
### Applet class ###
####################
class CDApplet:
	
	#############
	### Enums ###
	#############
	# orientation
	BOTTOM = 0
	TOP    = 1
	RIGHT  = 2
	LEFT   = 3
	# container type
	DOCK    = 0
	DESKLET = 1
	# emblem position
	EMBLEM_TOP_LEFT     = 0
	EMBLEM_BOTTOM_RIGHT = 1
	EMBLEM_BOTTOM_LEFT  = 2
	EMBLEM_TOP_RIGHT    = 3
	EMBLEM_MIDDLE       = 4
	EMBLEM_BOTTOM       = 5
	EMBLEM_TOP          = 6
	EMBLEM_RIGHT        = 7
	EMBLEM_LEFT         = 8
	EMBLEM_PERSISTENT   = 0
	EMBLEM_PRINT        = 9
	# menu item types
	MENU_ENTRY        = 0
	MENU_SUB_MENU     = 1
	MENU_SEPARATOR    = 2
	MENU_CHECKBOX     = 3
	MENU_RADIO_BUTTON = 4
	# main menu ID
	MAIN_MENU_ID = 0
	# dialog key pressed
	DIALOG_KEY_ENTER = -1
	DIALOG_KEY_ESCAPE = -2
	
	#####################
	### INIT AND DBUS ###
	#####################
	
	def __init__(self):
		""" initialize the applet. Must be called by any class that inheritates from it.
		It defines the following:
		 - icon : our main icon
		 - sub_icons: our sub-icons
		 - config : a dictionnary where our configuration parameters are stored
		 - cAppletName : name of our applet (the same as the folder where files are stored)
		 - cConfFile : path to our config file (you will rarely need it)
		 """
		self.icon = None
		self.sub_icons = None
		self.config = {}
		self.loop = None
		self._bEnded = False
		self._cMenuIconId = None
		self.cAppletName = sys.argv[0][2:]  # the command is ./applet_name
		self.cBusPath = sys.argv[2]
		self.cConfFile = sys.argv[3]
		self.cRootDataDir = sys.argv[4]
		self.cParentAppName = sys.argv[5]
		self.cShareDataDir = INSTALL_PREFIX + '/' + self.cAppletName
		
		self._get_config()
		self._connect_to_dock()
	
	def _connect_to_dock(self):
		# get our applet on the bus.
		bus = dbus.SessionBus()
		try:
			applet_object = bus.get_object("org.cairodock.CairoDock", self.cBusPath)
		except:
			print(">>> object '"+self.cBusPath+"' can't be found on the bus, exit.\nMake sure that Cairo-Dock is running")
			sys.exit(2)
		self.icon = dbus.Interface(applet_object, "org.cairodock.CairoDock.applet")  # this object represents our icon inside the dock or a desklet.
		sub_icons_object = bus.get_object("org.cairodock.CairoDock", self.cBusPath+"/sub_icons")
		self.sub_icons = dbus.Interface(sub_icons_object, "org.cairodock.CairoDock.subapplet")  # this object represents the list of icons contained in our sub-dock, or in our desklet. We'll add them one by one later, giving them a unique ID, which will be used to identify each of them.
		# connect to signals.
		self.icon.connect_to_signal("on_click", self.on_click)  # when the user left-clicks on our icon.
		self.icon.connect_to_signal("on_middle_click", self.on_middle_click)  # when the user middle-clicks on our icon.
		self.icon.connect_to_signal("on_build_menu", self._on_build_menu)  # when the user right-clicks on our applet (which builds the menu)
		self.icon.connect_to_signal("on_menu_select", self._on_menu_select)  # when the user selects an entry of this menu.
		self.icon.connect_to_signal("on_scroll", self.on_scroll)  # when the user scroll up or down on our icon.
		self.icon.connect_to_signal("on_drop_data", self.on_drop_data)  # when the user drops something on our icon.
		self.icon.connect_to_signal("on_answer_dialog", self.on_answer_dialog)  # when the user answer a question.
		self.icon.connect_to_signal("on_shortkey", self.on_shortkey)  # when the user press the shortkey.
		self.icon.connect_to_signal("on_change_focus", self.on_change_focus)  # when the window's focus changes.
		self.icon.connect_to_signal("on_stop_module", self._on_stop)  # when the user deactivate our applet (or the DBus plug-in, or when the Cairo-Dock is stopped).
		self.icon.connect_to_signal("on_reload_module", self._on_reload)  # when the user changes something in our config, or when the desklet is resized (with no change in the config).
		self.sub_icons.connect_to_signal("on_click_sub_icon", self.on_click_sub_icon)  # when the user left-clicks on a sub-icon.
		self.sub_icons.connect_to_signal("on_middle_click_sub_icon", self.on_middle_click_sub_icon)
		self.sub_icons.connect_to_signal("on_scroll_sub_icon", self.on_scroll_sub_icon)
		self.sub_icons.connect_to_signal("on_build_menu_sub_icon", self._on_build_menu_sub_icon)
		self.sub_icons.connect_to_signal("on_drop_data_sub_icon", self.on_drop_data_sub_icon)
	
	def run(self):
		""" start the applet and enter the main loop; we never get out of this function """
		self.begin()
		if not self._bEnded:  # _bEnded can be true if the applet runs its own main loop, in which case we stay stuck in the 'begin' function until the end of the applet; in this case, we don't want to run a main loop again!
			if g_bMainLoopInGObject:
				self.loop = gobject.MainLoop()
			else:
				self.loop = glib.MainLoop()
			self.loop.run()
		print(">>> applet '"+self.cAppletName+"' terminated.")
		sys.exit(0)
	
	##################################
	### callbacks on the main icon ###
	##################################
	
	def on_click(self,iState):
		""" action on click """
		pass
	
	def on_middle_click(self):
		""" action on middle-click """
		pass
	
	def _on_build_menu(self):
		self._cMenuIconId = None
		self.on_build_menu()
		pass
		
	def on_build_menu(self):
		""" build our menu """
		pass
	
	def _on_menu_select(self,iNumEntry):
		if self._cMenuIconId == None:
			self.on_menu_select(iNumEntry)
		else:
			self.on_menu_select_sub_icon(iNumEntry,self._cMenuIconId)
	
	def on_menu_select(self,iNumEntry):
		""" action on selecting an entry of our menu """
		pass
	
	def on_scroll(self,bScrollUp):
		""" action on scroll """
		pass
	
	def on_drop_data(self,cReceivedData):
		""" action on dropping something on our applet """
	
	def on_answer(self,answer):
		""" action on answering ok to a dialog with ok/cancel buttons (deprecated) """
		pass
	
	def on_answer_dialog(self, button, answer):
		""" action on answering a dialog """
		if button == -1 or button == 0:
			self.on_answer(answer)
	
	def on_shortkey(self,cKey):
		""" action on pressing one of the shortkeys we bound beforehand """
		pass
	
	def on_change_focus(self,bIsActive):
		""" action when the window controlled by the applet takes or looses the focus """
		pass
	
	##################################
	### callbacks on the sub-icons ###
	##################################
	
	def on_click_sub_icon(self, iState, cIconID):
		""" action on click on one of our sub-icons"""
		pass
	
	def on_middle_click_sub_icon(self, cIconID):
		""" action on middle-click on one of our sub-icons"""
		pass
	
	def on_scroll_sub_icon(self, bScrollUp, cIconID):
		""" action on scroll on one of our sub-icons"""
		pass
	
	def _on_build_menu_sub_icon(self, cIconID):
		self._cMenuIconId = cIconID
		self.on_build_menu_sub_icon(cIconID)
	
	def on_build_menu_sub_icon(self, cIconID):
		""" build our menu on one of our sub-icons"""
		pass
	
	def on_menu_select_sub_icon(self, iNumEntry, cIconID):
		""" action on selecting an entry of our menu on a sub-icon"""
		pass
	
	def on_drop_data_sub_icon(self, cReceivedData, cIconID):
		""" action on dropping something on one of our sub-icons"""
		pass
	
	def on_answer_dialog_sub_icon(self, button, answer, cIconID):
		""" action on answering a dialog about a sub-icon"""
		pass
	
	###############################
	### callbacks on the applet ###
	###############################
	
	def begin(self):
		""" action when the applet is started """
		pass
	
	def end(self):
		""" action when the applet is terminated """
		pass
	
	def _on_stop(self):
		self._bEnded = True
		self.end()
		if self.loop != None:
			self.loop.quit()
	
	def reload(self):
		""" called when our applet is reloaded (config parameters have changed) """
		pass
	
	def _on_reload(self,bConfigHasChanged):
		if bConfigHasChanged:
			self._get_config()
			self.reload()
	
	def get_config(self,keyfile):
		""" get our parameters from the key-file """
		pass
	
	def _get_config(self):
		try:
			keyfile = ConfigParser.RawConfigParser() # python 2
		except:
			keyfile = configparser.RawConfigParser() # python 3
		keyfile.read(self.cConfFile)
		self.get_config(keyfile)
	
