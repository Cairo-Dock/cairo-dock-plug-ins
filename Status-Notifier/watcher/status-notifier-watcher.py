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
import gi

from gi.repository import GLib, Gio, GObject

class SNWatcher():
	bus_name_str = 'org.kde.StatusNotifierWatcher'
	bus_obj_str  = '/StatusNotifierWatcher'
	bus_iface_str = 'org.kde.StatusNotifierWatcher'
	items = {}  # dict of DBus service name to set of items
	hosts = set()  # status notifier hosts registered
	
	interface_xml = """
		<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN" "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
		<node>
		  <interface name="org.kde.StatusNotifierWatcher">

			<!-- methods -->
			<method name="RegisterStatusNotifierItem">
			   <arg name="service" type="s" direction="in"/>
			</method>

			<method name="RegisterStatusNotifierHost">
			   <arg name="service" type="s" direction="in"/>
			</method>


			<!-- properties -->

			<property name="RegisteredStatusNotifierItems" type="as" access="read">
			   <annotation name="com.trolltech.QtDBus.QtTypeName.Out0" value="QStringList"/>
			</property>

			<property name="IsStatusNotifierHostRegistered" type="b" access="read"/>

			<property name="ProtocolVersion" type="i" access="read"/>


			<!-- signals -->

			<signal name="StatusNotifierItemRegistered">
				<arg type="s"/>
			</signal>

			<signal name="StatusNotifierItemUnregistered">
				<arg type="s"/>
			</signal>

			<signal name="StatusNotifierHostRegistered">
			</signal>

			<signal name="StatusNotifierHostUnregistered">
			</signal>
		  </interface>
		</node>
	"""
	
	def _emit_host_registered (self):
		self.StatusNotifierHostRegistered()
		return False
	
	def __init__(self):
		self.interface = Gio.DBusNodeInfo.new_for_xml(self.interface_xml).interfaces[0]
		self.bus_id = Gio.bus_own_name(Gio.BusType.SESSION, self.bus_name_str, Gio.BusNameOwnerFlags.NONE,
			self.on_bus_acquired,
			None, # name acquired handler -- not used, we should export our interface already when the bus has been acquired
			self.on_name_lost)
		
		self.loop = GLib.MainLoop()
		self.loop.run()
	
	def on_bus_acquired(self, conn, name):
		conn.signal_subscribe('org.freedesktop.DBus', 'org.freedesktop.DBus', 'NameOwnerChanged', '/org/freedesktop/DBus',
			None, Gio.DBusSignalFlags.NONE, self.on_name_owner_changed)
		conn.register_object(self.bus_obj_str, self.interface, self.handle_method_call, self.get_prop, None)
	
	def on_name_lost(self, conn, name):
		self.loop.quit() # not much to do, better to leave restarting to the Status-Notifier plugin
	
	def on_name_owner_changed(self, conn, sender, obj, iface, signal_name, pars):
		service, prev_owner, new_owner = pars.unpack()
		# print("name_owner_changed: %s; %s; %s" % (service, prev_owner, new_owner))
		if (new_owner == ''):  # a service has disappear from the bus, check if it was an item or a host.
			if service in self.hosts:
				self.hosts.remove(service)
				if len(self.hosts) == 0:
					conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'StatusNotifierHostUnregistered', None)
			
			if service in self.items:
				for itemId in self.items[service]:
					conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'StatusNotifierItemUnregistered',
						GLib.Variant.new_tuple(GLib.Variant.new_string(itemId)))
				del self.items[service]
		
	### methods ###
	def handle_method_call(self,
							conn: Gio.DBusConnection,
							sender: str,
							object_path: str,
							interface_name: str,
							method_name: str,
							par: GLib.Variant,
							invocation: Gio.DBusMethodInvocation):
		
		if method_name == 'RegisterStatusNotifierHost':
			host_addr = par.unpack()[0] # should be string
			self.hosts.add(host_addr)
			conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'StatusNotifierHostRegistered', None)
			invocation.return_value(None)
		elif method_name == 'RegisterStatusNotifierItem':
			serviceOrPath = par.unpack()[0]
			service = None
			path = None
			# build the item id: service + path
			if (serviceOrPath[0] == '/'):
				service = sender
				path = serviceOrPath
			else:
				service = serviceOrPath
				path = '/StatusNotifierItem'
			itemId = service + path
			
			if sender not in self.items:
				self.items[sender] = set()
			if not itemId in self.items[sender]:
				self.items[sender].add(itemId)
				conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'StatusNotifierItemRegistered',
					GLib.Variant.new_tuple(GLib.Variant.new_string(itemId)))
			invocation.return_value(None)
		# note: unknown methods are already handled by Gio, no need to worry about them
	
	def get_prop(self,
				conn: Gio.DBusConnection,
				sender: str,
				obj: str,
				iface: str,
				name: str):
		if iface == 'org.kde.StatusNotifierWatcher':
			if name == 'RegisteredStatusNotifierItems':
				res = list()
				for x in self.items.values():
					res += x
				return GLib.Variant.new_strv(res)
			elif name == 'IsStatusNotifierHostRegistered':
				return GLib.Variant.new_boolean(len (self.hosts) != 0)
			elif name == 'ProtocolVersion':
				return GLib.Variant.new_int32(0)
	
if __name__ == '__main__':
	SNWatcher()
