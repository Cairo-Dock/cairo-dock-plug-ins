#!/usr/bin/env python3
#
# This is a part of the Cairo-Dock plug-ins.
# Copyright : (C) 2011 by Fabrice Rey
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
#
# Developped as a part of Cairo-Dock, but usable as a stand-alone systray daemon.
# The code follows the same logic as the KDE watcher, to ensure a complete compatibility.

import sys

from gi.repository import GLib as glib
from gi.repository import GObject as gobject
g_bMainLoopInGObject = False

import dbus, dbus.service
from dbus.mainloop.glib import DBusGMainLoop

class Registrar(dbus.service.Object):
	bus_name_str = 'com.canonical.AppMenu.Registrar'
	bus_obj_str  = '/com/canonical/AppMenu/Registrar'
	bus_iface_str = 'com.canonical.AppMenu.Registrar'
	windows = {}  # table of couple (X id, menu object path)
	
	def __init__(self):
		DBusGMainLoop(set_as_default=True)
		try: 
			self.bus = dbus.SessionBus()
			bus_name = dbus.service.BusName (self.bus_name_str, self.bus)
			print("registered a registrar:",bus_name)
			dbus.service.Object.__init__(self, bus_name, self.bus_obj_str)
		except dbus.DBusException:
			print('Could not open dbus. Uncaught exception.')
			return
		
		bus_object = self.bus.get_object(dbus.BUS_DAEMON_NAME, dbus.BUS_DAEMON_PATH)
		self.main = dbus.Interface(bus_object, dbus.BUS_DAEMON_IFACE)
		#self.main.connect_to_signal("NameOwnerChanged", self.on_name_owner_changed)
		
		if g_bMainLoopInGObject:
			self.loop = gobject.MainLoop()
		else:
			self.loop = glib.MainLoop()
		self.loop.run()
	
	### methods ###
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = 'uo', out_signature = None, sender_keyword='sender')
	def RegisterWindow(self, Xid, menu, sender=None):
		self.windows[Xid] = (sender, menu)
		
		self.WindowRegistered (Xid, sender, menu)
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = 'u', out_signature = None)
	def UnregisterWindow(self, Xid):
		del self.windows[Xid]
		
		self.WindowUnregistered (Xid)
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = 'u', out_signature = 'so')
	def GetMenuForWindow(self, Xid):
		print("GetMenuForWindow(%d)..." % Xid)
		try:
			service,menu = self.windows[Xid]
			return service,menu  # dbus.ObjectPath(menu)
		except:
			return "","/"  # return an empty string to avoid ugly warnings; clients should be able to deal with it.
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = None, out_signature = 'a(uso)')
	def GetMenus(self):
		menus=[]
		for xid in self.windows:
			service,menu = self.windows[xid]
			menus.append((xid, service, menu))
	
	### Signals ###
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature='uso')
	def WindowRegistered(self, Xid, service, menu):
		print("%d registered" % (Xid))
		sys.stdout.flush()
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature='u')
	def WindowUnregistered(self, Xid):
		print("%d unregistered" % (Xid))
		sys.stdout.flush()
	

if __name__ == '__main__':
	Registrar()
