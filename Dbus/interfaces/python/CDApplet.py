# This is a part of the external applets for Cairo-Dock
# Copyright : (C) 2010-2011 by Fabounet
#             2026 Daniel Kondor
# E-mail : kondor.dani@gmail.com
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
import sys
import os.path
import gettext
import configparser

import gi
from gi.repository import GLib, Gio


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


#########################
### Interface helpers ###
#########################
class CDAppletIconIface:
	def __init__(self, proxy):
		self.proxy = proxy
	
	def Get(self, cProperty : str):
		return self.proxy.Get('(s)', cProperty).unpack()
	
	def GetAll(self):
		return self.proxy.GetAll().unpack()
	
	def SetQuickInfo(self, cQuickInfo : str):
		self.proxy.SetQuickInfo('(s)', cQuickInfo)
	
	def SetLabel(self, cLabel : str):
		self.proxy.SetLabel('(s)', cLabel)
	
	def SetIcon(self, cImage : str):
		self.proxy.SetIcon('(s)', cImage)
	
	def SetEmblem(self, cImage : str, iPosition : int):
		self.proxy.SetEmblem('(si)', cImage, iPosition)
	
	def Animate(self, cAnimation : str, iNbRounds : int):
		self.proxy.Animate('(si)', cAnimation, iNbRounds)
	
	def DemandsAttention(self, bStart : bool, cAnimation : str):
		self.proxy.DemandsAttention('(bs)', bStart, cAnimation)
	
	def ShowDialog(self, cMessage : str, iDuration : int):
		self.proxy.ShowDialog('(si)', cMessage, iDuration)
	
	def PopupDialog(self, hDialogAttributes : dict, hWidgetAttributes : dict):
		self.proxy.PopupDialog('(a{sv}a{sv})', hDialogAttributes, hWidgetAttributes)
	
	def AddDataRenderer(self, cType : str, iNbValues : int, cTheme : int):
		self.proxy.AddDataRenderer('(cii)', cType, iNbValues, cTheme)
	
	def RenderValues(self, pValues : list): # note: list of doubles
		self.proxy.RenderValues('(ad)', pValues)
	
	def ControlAppli(self, cApplicationClass : str):
		self.proxy.ControlAppli('(s)', cApplicationClass)
	
	def ActOnAppli(self, cAction : str):
		self.proxy.ActOnAppli('(s)', cAction)
	
	def AddMenuItems(self, pItems : list):
		self.proxy.AddMenuItems('(aa{sv})', pItems)
	
	# deprecated, not used: AskQuestion, AskValue, ShowAppli, PopulateMenu
	
	# deprecated, still used
	def AskText(self, cMessage : str, cInitialText : str):
		self.proxy.AskText('(ss)', cMessage, cInitialText)


class CDAppletSubIconIface:
	def __init__(self, proxy):
		self.proxy = proxy
	
	def SetQuickInfo(self, cQuickInfo : str, cIconID : str):
		self.proxy.SetQuickInfo('(ss)', cQuickInfo, cIconID)
	
	def SetLabel(self, cLabel : str, cIconID : str):
		self.proxy.SetLabel('(ss)', cLabel, cIconID)
	
	def SetIcon(self, cImage : str, cIconID : str):
		self.proxy.SetIcon('(ss)', cImage, cIconID)
	
	def SetEmblem(self, cImage : str, iPosition : int, cIconID : str):
		self.proxy.SetEmblem('(sis)', cImage, iPosition, cIconID)
	
	def Animate(self, cAnimation : str, iNbRounds : int, cIconID : str):
		self.proxy.Animate('(sis)', cAnimation, iNbRounds, cIconID)
	
	def ShowDialog(self, cMessage : str, iDuration : int, cIconID : str):
		self.proxy.ShowDialog('(sis)', cMessage, iDuration, cIconID)
	
	# deprecated, not used: AskQuestion, AskValue, AskText
	
	def PopupDialog(self, hDialogAttributes : dict, hWidgetAttributes : dict, cIconID : str):
		self.proxy.PopupDialog('(a{sv}a{sv}s)', hDialogAttributes, hWidgetAttributes, cIconID)
	
	def AddSubIcons(self, pIconFields : list):
		self.proxy.AddSubIcons('(as)', pIconFields)
	
	def RemoveSubIcon(self, cIconID : str):
		self.proxy.RemoveSubIcon('(s)', cIconID)


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
		self.loop = GLib.MainLoop()
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
		CD_BUS_NAME = 'org.cairodock.CairoDock'
		CD_APPLET_IFACE_NAME = 'org.cairodock.CairoDock.applet'
		CD_SUBICONS_IFACE_NAME = 'org.cairodock.CairoDock.subapplet'
		
		self.icon_proxy = Gio.DBusProxy.new_for_bus_sync(Gio.BusType.SESSION, Gio.DBusProxyFlags.DO_NOT_AUTO_START,
			None, #!! TODO: load our interface info !!
			CD_BUS_NAME, self.cBusPath, CD_APPLET_IFACE_NAME, None)
		self.icon = CDAppletIconIface(self.icon_proxy)
		
		self.sub_icons_proxy = Gio.DBusProxy.new_for_bus_sync(Gio.BusType.SESSION, Gio.DBusProxyFlags.DO_NOT_AUTO_START,
			None, #!! TODO: load our interface info !!
			CD_BUS_NAME, self.cBusPath + '/sub_icons', CD_SUBICONS_IFACE_NAME, None)
		self.sub_icons = CDAppletSubIconIface(self.sub_icons_proxy)
		
		# connect to signals.
		self.icon_proxy.connect('g-signal', self.on_icon_signal)
		self.sub_icons_proxy.connect('g-signal', self.on_sub_icon_signal)
		
		Gio.bus_watch_name(Gio.BusType.SESSION, CD_BUS_NAME, Gio.BusNameWatcherFlags.NONE, None, self._on_dock_vanished)
		
	def run(self):
		""" start the applet and enter the main loop; we never get out of this function """
		self.begin()
		if not self._bEnded:  # _bEnded can be true if the applet runs its own main loop, in which case we stay stuck in the 'begin' function until the end of the applet; in this case, we don't want to run a main loop again!
			self.loop.run()
		print(">>> applet '"+self.cAppletName+"' terminated.")
		sys.exit(0)
	
	def _on_dock_vanished(self):
		self.loop.quit()
	
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
		if self._cMenuIconId is None:
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
	
	
	#######################
	### signal handlers ###
	#######################
	
	def on_icon_signal(self, proxy : Gio.DBusProxy, sender_name: str | None, signal_name: str, par: GLib.Variant):
		 # note: for all handlers, we just assume the pars are correct -- we will get an exception if not
		if signal_name == 'on_click':
			self.on_click(*par.unpack())  # when the user left-clicks on our icon.
		elif signal_name == 'on_middle_click':
			self.on_middle_click()  # when the user middle-clicks on our icon.
		elif signal_name == 'on_build_menu':
			self._on_build_menu()  # when the user right-clicks on our applet (which builds the menu)
		elif signal_name == 'on_menu_select':
			self._on_menu_select(*par.unpack())  # when the user selects an entry of this menu.
		elif signal_name == 'on_scroll':
			self.on_scroll(*par.unpack())  # when the user scroll up or down on our icon.
		elif signal_name == 'on_drop_data':
			self.on_drop_data(*par.unpack())  # when the user drops something on our icon.
		elif signal_name == 'on_answer_dialog':
			self.on_answer_dialog(*par.unpack())  # when the user answer a question.
		elif signal_name == 'on_shortkey':
			self.on_shortkey(*par.unpack())  # when the user press the shortkey.
		elif signal_name == 'on_change_focus':
			self.on_change_focus(*par.unpack())  # when the window's focus changes.
		elif signal_name == 'on_stop_module':
			self._on_stop()  # when the user deactivate our applet (or the DBus plug-in, or when the Cairo-Dock is stopped).
		elif signal_name == 'on_reload_module':
			self._on_reload(*par.unpack())  # when the user changes something in our config, or when the desklet is resized (with no change in the config).
	
	def on_sub_icon_signal(self, proxy : Gio.DBusProxy, sender_name: str | None, signal_name: str, par: GLib.Variant):
		if signal_name == 'on_click_sub_icon':
			self.on_click_sub_icon(*par.unpack())  # when the user left-clicks on a sub-icon.
		if signal_name == 'on_middle_click_sub_icon':
			self.on_middle_click_sub_icon(*par.unpack())
		if signal_name == 'on_scroll_sub_icon':
			self.on_scroll_sub_icon(*par.unpack())
		if signal_name == 'on_build_menu_sub_icon':
			self._on_build_menu_sub_icon(*par.unpack())
		if signal_name == 'on_drop_data_sub_icon':
			self.on_drop_data_sub_icon(*par.unpack())


