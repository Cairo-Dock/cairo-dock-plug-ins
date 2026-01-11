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
# Developped as a part of Cairo-Dock, but usable as a stand-alone global menu registrar.

from gi.repository import GLib, Gio

class Registrar():
	bus_name_str = 'com.canonical.AppMenu.Registrar'
	bus_obj_str  = '/com/canonical/AppMenu/Registrar'
	bus_iface_str = 'com.canonical.AppMenu.Registrar'
	windows = {}  # table of couple (X id, menu object path)
	bus_names = {} # match DBus names to currently known Xids
	
	interface_xml = """
	<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN" "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
	<node xmlns:dox="http://www.ayatana.org/dbus/dox.dtd">
		<dox:d><![CDATA[
		  @mainpage
		 
		  An interface to register menus that are associated with a window in an application.  The
		  main interface is docuemented here: @ref com::canonical::AppMenu::Registrar.
		    
		  The actual menus are transported using the dbusmenu protocol which is available
		  here: @ref com::canonical::dbusmenu.
		]]></dox:d>
		<interface name="com.canonical.AppMenu.Registrar" xmlns:dox="http://www.ayatana.org/dbus/dox.dtd">
			<dox:d>
			  An interface to register a menu from an application's window to be displayed in another
			  window.  This manages that association between XWindow Window IDs and the dbus
			  address and object that provides the menu using the dbusmenu dbus interface.
			</dox:d>
			<method name="RegisterWindow">
				<dox:d><![CDATA[
				  Associates a dbusmenu with a window
		     
				  /note this method assumes that the connection from the caller is the DBus connection
					to use for the object.  Applications that use multiple DBus connections will need to
					ensure this method is called with the same connection that implmenets the object.
				]]></dox:d>
				<arg name="windowId" type="u" direction="in">
					<dox:d>The XWindow ID of the window</dox:d>
				</arg>
				<arg name="menuObjectPath" type="o" direction="in">
					<dox:d>The object on the dbus interface implementing the dbusmenu interface</dox:d>
				</arg>
			</method>
			<method name="UnregisterWindow">
				<dox:d>
				  A method to allow removing a window from the database.  Windows will also be removed
				  when the client drops off DBus so this is not required.  It is polite though.  And
				  important for testing.
				</dox:d>
				<arg name="windowId" type="u" direction="in">
					<dox:d>The XWindow ID of the window</dox:d>
				</arg>
			</method>
			<method name="GetMenuForWindow">
				<dox:d>Gets the registered menu for a given window ID.</dox:d>
				<arg name="windowId" type="u" direction="in">
					<dox:d>The XWindow ID of the window to get</dox:d>
				</arg>
				<arg name="service" type="s" direction="out">
					<dox:d>The address of the connection on DBus (e.g. :1.23 or org.example.service)</dox:d>
				</arg>
				<arg name="menuObjectPath" type="o" direction="out">
					<dox:d>The path to the object which implements the com.canonical.dbusmenu interface.</dox:d>
				</arg>
			</method>
			<method name="GetMenus">
				<dox:d>
					Gets the information on all menus that the registrar knows about.  This
					is useful for debugging or bringing up a new renderer.
				</dox:d>
				<arg name="menus" type="a(uso)" direction="out">
					<dox:d>An array of structures containing the same parameters as @GetMenuForWindow.  Window ID, Service and ObjectPath.</dox:d>
				</arg>
			</method>
			<signal name="WindowRegistered">
				<dox:d>Signals when the registrar gets a new menu registered</dox:d>
				<arg name="windowId" type="u" direction="out">
					<dox:d>The XWindow ID of the window</dox:d>
				</arg>
				<arg name="service" type="s" direction="out">
					<dox:d>The address of the connection on DBus (e.g. :1.23 or org.example.service)</dox:d>
				</arg>
				<arg name="menuObjectPath" type="o" direction="out">
					<dox:d>The path to the object which implements the com.canonical.dbusmenu interface.</dox:d>
				</arg>
			</signal>
			<signal name="WindowUnregistered">
				<dox:d>Signals when the registrar removes a menu registration</dox:d>
				<arg name="windowId" type="u" direction="out">
					<dox:d>The XWindow ID of the window</dox:d>
				</arg>
			</signal>
		</interface>
	</node>
	"""
	
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
		conn.register_object(self.bus_obj_str, self.interface, self.handle_method_call, None, None) # note: no properties
	
	def on_name_lost(self, conn, name):
		self.loop.quit() # not much to do, better to leave restarting to the Global-Menu plugin if needed
	
	### methods ###
	def handle_method_call(self,
							conn: Gio.DBusConnection,
							sender: str,
							object_path: str,
							interface_name: str,
							method_name: str,
							par: GLib.Variant,
							invocation: Gio.DBusMethodInvocation):
		if method_name == 'RegisterWindow':
			Xid, menu = par.unpack() # par type already checked by Gio
			self.windows[Xid] = (sender, menu)
			if sender not in self.bus_names:
				self.bus_names[sender] = set()
			self.bus_names[sender].add(Xid)
			conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'WindowRegistered', GLib.Variant.new_tuple(
				GLib.Variant.new_uint32(Xid),
				GLib.Variant.new_string(sender),
				GLib.Variant.new_object_path(menu)
			))
			invocation.return_value(None)
		elif method_name == 'UnregisterWindow':
			Xid = par.unpack()[0]
			del self.windows[Xid]
			if sender in self.bus_names:
				self.bus_names[sender].remove(Xid)
				if len(self.bus_names[sender]) == 0:
					del self.bus_names[sender]
			conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'WindowUnregistered',
				GLib.Variant.new_tuple(GLib.Variant.new_uint32(Xid)))
			invocation.return_value(None)
		elif method_name == 'GetMenuForWindow':
			Xid = par.unpack()[0]
			if Xid in self.windows:
				name, menupath = self.windows[Xid]
				invocation.return_value(GLib.Variant.new_tuple(
					GLib.Variant.new_string(name), GLib.Variant.new_object_path(menupath)))
			else:
				# maybe better to return an error
				invocation.return_error_literal(Gio.DBusError.quark(), Gio.DBusError.INVALID_ARGS,
					"Window ID not known")
		elif method_name == 'GetMenus':
			res = GLib.VariantBuilder.new(GLib.VariantType('(a(uso))'))
			res.open(GLib.VariantType('a(uso)'))
			for Xid, v1 in self.windows.items():
				res.add_value(GLib.Variant.new_tuple(
					GLib.Variant.new_uint32(Xid),
					GLib.Variant.new_string(v1[0]),
					GLib.Variant.new_object_path(v1[1])
				))
			res.close()
			invocation.return_value(res.end())
		else:
			# should not happen, interface should be checked already in Gio
			invocation.return_error_literal(Gio.DBusError.quark(), Gio.DBusError.UNKNOWN_METHOD, "Method does not exist")
	
	def on_name_owner_changed(self, conn, sender, obj, iface, signal_name, pars):
		service, prev_owner, new_owner = pars.unpack()
		print("name_owner_changed: %s; %s; %s" % (service, prev_owner, new_owner))
		if (new_owner == ''):  # a service has disappear from the bus, check if it was an app with registered windows
			if service in self.bus_names:
				for Xid in self.bus_names[service]:
					del self.windows[Xid]
					conn.emit_signal(None, self.bus_obj_str, self.bus_iface_str, 'WindowUnregistered',
						GLib.Variant.new_tuple(GLib.Variant.new_uint32(Xid)))
				del self.bus_names[service]
		
			
if __name__ == '__main__':
	Registrar()

