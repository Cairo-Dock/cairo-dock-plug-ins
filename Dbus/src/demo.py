### import ###
import sys
import gobject
import glib
import gtk
import dbus
from dbus.mainloop.glib import DBusGMainLoop


### callbacks ###
def action_on_click(cModuleName,iState):
	if cModuleName != "toto":
		return
	print "clic !"

def action_on_middle_click(cModuleName):
	if cModuleName != "toto":
		return
	print "middle clic !"

def action_on_build_menu(cModuleName):
	if cModuleName != "toto":
		return
	print "build menu !"
	bus = dbus.SessionBus()
	remote_object = bus.get_object("org.cairodock.CairoDock", "/org/cairodock/CairoDock")
	iface = dbus.Interface(remote_object, "org.cairodock.CairoDock")
	iface.PopulateMenu("toto", ["choice 1", "choice 2"])
	del iface
	del remote_object
	del bus
	
def action_on_menu_select(cModuleName,iNumEntry):
	if cModuleName != "toto":
		return
	print "choice",iNumEntry,"has been selected !"
	

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
	iface.RegisterNewModule("toto", 3, "This is a distant applet\nIt handles right and middle click\n by Fabounet", "~/toto")
	
	# register to the notifications on our applet
	iface.connect_to_signal("on_click_icon", action_on_click)
	iface.connect_to_signal("on_middle_click_icon", action_on_middle_click)
	iface.connect_to_signal("on_build_menu", action_on_build_menu)
	iface.connect_to_signal("on_menu_select", action_on_menu_select)
	
	# a little test
	iface.ShowDialog("I'm connected to Cairo-Dock !", 4, "None", "None", "toto")
	iface.SetQuickInfo("123", "None", "None", "toto")
	
	# clean up memory
	del iface
	del remote_object
	del bus


### main ###
if __name__ == '__main__':
	init()
	gtk.main()
	sys.exit(0)
