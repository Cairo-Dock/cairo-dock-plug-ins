# This is a part of the external applets for Cairo-Dock
# Copyright : (C) 2010-2011 by Nochka85, Fabounet and Matttbe
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

####################
### dependancies ###
####################
import os.path
import subprocess
from CDApplet import CDApplet

####################
### Applet class ###
####################
class CDBashApplet(CDApplet):
	def __init__(self):
		# call high-level init
		self.app_folder = os.path.abspath(".")
		CDApplet.__init__(self)
	
	##### private methods #####
	
	def call(self,action):
		subprocess.call("cd " + self.app_folder + " && ./" + self.cAppletName + ".sh " + self.cAppletName + " " + self.cBusPath + " " + self.cConfFile + " " + self.cParentAppName + " " + action, shell=True)
	
	##### applet definition #####
	
	def get_config(self,keyfile):
		self.call("get_config")
	
	def end(self):
		self.call("end")
	
	def begin(self):
		self.call("begin")
	
	def reload(self):
		self.call("reload")
	
	##### callbacks #####
	
	def on_click(self,iState):
		self.call("on_click "+str(iState))
	
	def on_middle_click(self):
		self.call("on_middle_click")
		
	def on_build_menu(self):
		self.call("on_build_menu")
		
	def on_menu_select(self,iNumEntry):
		self.call("on_menu_select "+str(iNumEntry))
	
	def on_scroll(self,bScrollUp):
		self.call("on_scroll "+str(bScrollUp))
	
	def on_drop_data(self,cReceivedData):
		self.call("on_drop_data '"+cReceivedData+"'")
	
	def on_answer_dialog(self, button, answer):
		self.call("on_answer_dialog "+str(button)+" '"+str(answer)+"'")
	
	def on_shortkey(self,key):
		self.call("on_shortkey '"+key+"'")
	
	def on_change_focus(self,bIsActive):
		self.call("on_change_focus '"+str(bIsActive)+"'")
	
	def on_click_sub_icon(self, iState, cIconID):
		self.call("on_click_sub_icon '"+str(iState)+"' '"+cIconID+"'")
	
