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

class SNWatcher(dbus.service.Object):
	bus_name_str = 'org.kde.StatusNotifierWatcher'
	bus_obj_str  = '/StatusNotifierWatcher'
	bus_iface_str = 'org.kde.StatusNotifierWatcher'
	items_list = []  # array of service+path
	hosts_list = []  # array of services
	
	def _emit_host_registered (self):
		self.StatusNotifierHostRegistered()
		return False
	
	def __init__(self):
		DBusGMainLoop(set_as_default=True)
		try: 
			self.bus = dbus.SessionBus()
			bus_name = dbus.service.BusName (self.bus_name_str, self.bus)
			print("[Cairo-Dock] Status-Notifier: registered a watcher:",bus_name)
			dbus.service.Object.__init__(self, bus_name, self.bus_obj_str)
		except dbus.DBusException:
			print('Could not open dbus. Uncaught exception.')
			return
		
		bus_object = self.bus.get_object(dbus.BUS_DAEMON_NAME, dbus.BUS_DAEMON_PATH)
		self.main = dbus.Interface(bus_object, dbus.BUS_DAEMON_IFACE)
		self.main.connect_to_signal("NameOwnerChanged", self.on_name_owner_changed)
		
		if g_bMainLoopInGObject:
			self.loop = gobject.MainLoop()
		else:
			self.loop = glib.MainLoop()
		self.loop.run()
	
	def on_name_owner_changed(self, service, prev_owner, new_owner):
		# print("name_owner_changed: %s; %s; %s" % (service, prev_owner, new_owner))
		if (new_owner == '' and not service.startswith(':')):  # a service has disappear from the bus, check if it was an item or a host.
			# search amongst the items.
			match = service+'/'
			# print("  search for ",match)
			to_be_removed=[]
			for it in self.items_list:
				# print("  check for ",it)
				if (it.startswith(match)):  # it[0:len(match)] == match
					# print("    match!")
					to_be_removed.append(it)
			for it in to_be_removed:
				self.items_list.remove (it)
				self.StatusNotifierItemUnregistered(it)

			# search amongst the hosts.
			to_be_removed=[]
			for it in self.hosts_list:
				if (it == service):
					to_be_removed.append(it)
			for it in to_be_removed:
				self.hosts_list.remove (it)
				self.StatusNotifierHostUnregistered()
		elif (service == 'com.Skype.API'):  # this stupid proprietary software only creates its item when the host appears !
			glib.timeout_add_seconds(2, self._emit_host_registered)
		
	### methods ###
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = 's', out_signature = None)
	def RegisterStatusNotifierHost(self, service):
		if (self.hosts_list.count (service) == 0):  # if not already listed
			self.hosts_list.append (service)
			self.StatusNotifierHostRegistered()
		# print('hosts:',self.hosts_list)
		# sys.stdout.flush()
	
	@dbus.service.method(dbus_interface = bus_iface_str, in_signature = 's', out_signature = None, sender_keyword='sender')
	def RegisterStatusNotifierItem(self, serviceOrPath, sender=None):
		# build the item id: service + path
		if (serviceOrPath[0] == '/'):
			service = sender
			path = serviceOrPath
		else:
			service = serviceOrPath
			path = "/StatusNotifierItem"
		
		itemId = service + path
		# keep track of this new item, and emit the 'new' signal.
		if (self.items_list.count (itemId) == 0):  # if not already listed
			self.items_list.append (itemId)
			self.StatusNotifierItemRegistered (itemId)
	
	### Properties ###
	
	@dbus.service.method(dbus_interface = dbus.PROPERTIES_IFACE, in_signature = 'ss', out_signature = 'v')
	def Get(self, interface, property):
		if interface == 'org.kde.StatusNotifierWatcher':
			if property == 'RegisteredStatusNotifierItems':
				# print("items: ",self.items_list)
				if (len (self.items_list) != 0):
					return self.items_list
				else:  # too bad! dbus-python can't encode the GValue if 'items_list' is None or [].
					return ['']  # so we return a empty string; hopefuly the host will skip this invalid value.
			elif property == 'IsStatusNotifierHostRegistered':
				return (len (self.hosts_list) != 0)
			elif property == 'HasStatusNotifierHostRegistered':  # deprecated
				return (len (self.hosts_list) != 0)
			elif property == 'ProtocolVersion':
				return 0
	
	### Signals ###
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature='s')
	def StatusNotifierItemRegistered(self, service):
		# print("%s registered" % (service))
		# sys.stdout.flush()
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature='s')
	def StatusNotifierItemUnregistered(self, service):
		# print("%s unregistered" % (service))
		# sys.stdout.flush()
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature=None)
	def StatusNotifierHostRegistered(self):
		# print("a host has been registered")
		# sys.stdout.flush()
	
	@dbus.service.signal(dbus_interface=bus_name_str, signature=None)
	def StatusNotifierHostUnregistered(self):
		# print("a host has been unregistered")
		# sys.stdout.flush()
	
	
if __name__ == '__main__':
	SNWatcher()
