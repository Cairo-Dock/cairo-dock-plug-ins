# This is a part of the Cairo-Dock plug-ins.
# Copyright : (C) 2010-2011 by Fabounet
# E-mail : fabounet@glx-dock.org
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# http://www.gnu.org/licenses/licenses.html#GPL

# Base class for Cairo-Dock's main interface.

####################
### dependancies ###
####################
import os.path
import dbus
import re

USER_DIR = os.path.abspath("~/.config")


##################
### Main class ###
##################
class CairoDock:
	
	def __init__(self, app_name="cairo-dock"):
		""" initialize the interface.
		It defines the following:
		 - cDataDir: main dir
		 - cCurrentThemeDir: current theme dir
		 - cConfFile : path to the global config file
		 """
		self.dock = None
		self.cAppName = app_name
		self.cDataDir = USER_DIR + '/' + app_name
		self.cCurrentThemeDir = self.cDataDir + '/current_theme'
		self.cConfFile = self.cCurrentThemeDir + '/' + app_name + '.conf'
		
		self._connect()
	
	def _connect(self):
		# get gldi on the bus.
		bus = dbus.SessionBus()
		
		name1 = self.cAppName.replace('-','')  # -> cairodock
		name2 = re.sub('-[a-z]', lambda x: x.group(0).upper(), self.cAppName)
		name2 = re.sub('^[a-z]', lambda x: x.group(0).upper(), name2)  # -> CairoDock
		name2 = name2.replace('-','')  # -> CairoDock
		cBusPath = '/org/'+name1+'/'+name2
		try:
			dbus_object = bus.get_object("org.cairodock.CairoDock", cBusPath)
		except:
			print ">>> object '"+cBusPath+"' can't be found on the bus, exit.\nMake sure that Cairo-Dock is running"
			return
		self.dock = dbus.Interface(dbus_object, "org.cairodock.CairoDock")  # this object represents gldi.
		
