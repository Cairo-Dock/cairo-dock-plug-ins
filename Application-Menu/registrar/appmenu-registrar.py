#!/usr/bin/python
#
# Author: Fabrice Rey
# License: GPL-3
#
# Developped as a part of Cairo-Dock, but usable as a stand-alone systray daemon.
# The code follows the same logic as the KDE watcher, to ensure a complete compatibility.

import sys
import glib
import gobject
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
			print "registered a registrar:",bus_name
			dbus.service.Object.__init__(self, bus_name, self.bus_obj_str)
		except Exception, exception:
			print 'Could not open dbus. Uncaught exception.'
			return
		
		bus_object = self.bus.get_object(dbus.BUS_DAEMON_NAME, dbus.BUS_DAEMON_PATH)
		self.main = dbus.Interface(bus_object, dbus.BUS_DAEMON_IFACE)
		#self.main.connect_to_signal("NameOwnerChanged", self.on_name_owner_changed)
		
		self.loop = gobject.MainLoop()
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
		print "GetMenuForWindow(%d)" % Xid
		try:
			service,menu = self.windows[Xid]
			return service,menu  # dbus.ObjectPath(menu)
		except:
			return None,None
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = None, out_signature = 'a(uso)')
	def GetMenus(self):
		menus=[]
		for xid in self.windows:
			service,menu = self.windows[xid]
			menus.append((xid, service, menu))
	
	### Signals ###
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature='uso')
	def WindowRegistered(self, Xid, service, menu):
		print "%d registered" % (Xid)
		sys.stdout.flush()
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature='u')
	def WindowUnregistered(self, Xid):
		print "%d unregistered" % (Xid)
		sys.stdout.flush()
	

if __name__ == '__main__':
	Registrar()
