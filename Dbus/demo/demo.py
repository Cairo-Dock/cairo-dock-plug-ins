### We assume the name of this applet is "demo"
### It is installed in a directory, like for instance ~/demo.
### This script should be placed in this folder, alongside with 2 files :
### "icon" (the default icon of the applet) and "preview" (a preview of this applet)

### import ###
import sys
import gobject
import glib
import gtk
import dbus
import os.path
from dbus.mainloop.glib import DBusGMainLoop


### callbacks ###
def action_on_click(cModuleName,iState):
	if cModuleName != "demo":
		return
	print "clic !"

def action_on_middle_click(cModuleName):
	if cModuleName != "demo":
		return
	print "middle clic !"

def action_on_build_menu(cModuleName):
	if cModuleName != "demo":
		return
	print "build menu !"
	bus = dbus.SessionBus()
	remote_object = bus.get_object("org.cairodock.CairoDock", "/org/cairodock/CairoDock")
	iface = dbus.Interface(remote_object, "org.cairodock.CairoDock")
	iface.PopulateMenu("demo", ["choice 1", "choice 2"])
	del iface
	del remote_object
	del bus
	
def action_on_menu_select(cModuleName,iNumEntry):
	if cModuleName != "demo":
		return
	print "choice",iNumEntry,"has been selected !"
	

def action_on_init(cModuleName):
	if cModuleName != "demo":
		return
	print "our module is started"
	
def action_on_stop(cModuleName):
	if cModuleName != "demo":
		return
	print "our module is stopped"
	
def action_on_reload(cModuleName, bConfigHasChanged):
	if cModuleName != "demo":
		return
	print "our module is reloaded"
	if bConfigHasChanged:
		print " and our config has changed"
	

### init ###
def init():
	# let's connect to the dock.
	DBusGMainLoop(set_as_default=True)
	bus = dbus.SessionBus()
	try:
		remote_object = bus.get_object("org.cairodock.CairoDock",
				"/org/cairodock/CairoDock")
	except dbus.DBusException:
		print "Cairo-Dock not found on bus (did you activate its 'DBus' plug-in ?)"
		return
	iface = dbus.Interface(remote_object, "org.cairodock.CairoDock")
	
	# let's register our applet !
	iface.RegisterNewModule("demo", 3, "This is a distant applet\nIt handles right and middle click\n  by Fabounet", os.path.abspath("."))
	
	# register to the notifications on our applet
	iface.connect_to_signal("on_click_icon", action_on_click)
	iface.connect_to_signal("on_middle_click_icon", action_on_middle_click)
	iface.connect_to_signal("on_build_menu", action_on_build_menu)
	iface.connect_to_signal("on_menu_select", action_on_menu_select)
	iface.connect_to_signal("on_init_module", action_on_init)
	iface.connect_to_signal("on_stop_module", action_on_stop)
	iface.connect_to_signal("on_reload_module", action_on_reload)
	
	# a little test
	iface.ShowDialog("I'm connected to Cairo-Dock !", 4, "None", "None", "demo")
	iface.SetQuickInfo("123", "None", "None", "demo")
	
	# clean up memory
	del iface
	del remote_object
	del bus


### stop ###
def stop():
	print "STOP"
	bus = dbus.SessionBus()
	remote_object = bus.get_object("org.cairodock.CairoDock", "/org/cairodock/CairoDock")
	iface = dbus.Interface(remote_object, "org.cairodock.CairoDock")
	iface.UnregisterModule("demo")
	del iface
	del remote_object
	del bus
	

### main ###
if __name__ == '__main__':
	init()
	gtk.main()
	stop()
	sys.exit(0)
