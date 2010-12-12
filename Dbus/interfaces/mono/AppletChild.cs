using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationsMenu.CairoDock
{
    public abstract class AppletChild
    {
		Applet _applet;
		
        public Applet Applet
		{
			get
			{
				return _applet;
			}
			internal set
			{
				if(_applet == value)
				{
					return;
				}
				
				if(_applet != null)
				{
					OnHandleDestroyed(EventArgs.Empty);
				}
   
				_applet = value;
				OnHandleCreated(EventArgs.Empty);
			}
		}
		
		public AppletChild()
			:this(null)
		{
		}
		
        public AppletChild(Applet parent)
        {
            this.Applet = parent;
        }

        protected IApplet DBusApplet { get { return _applet.DBusApplet; } }
        protected ISubApplet DBusSubApplet { get { return _applet.DBusSubApplet; } }
		
		public event EventHandler HandleCreated;
		public event EventHandler HandleDestroyed;
		
		protected virtual void OnHandleCreated(EventArgs e)
		{
			if(HandleCreated != null)
			{
				HandleCreated(this, e);
			}
		}
		
		protected virtual void OnHandleDestroyed(EventArgs e)
		{
			if(HandleDestroyed != null)
			{
				HandleDestroyed(this, e);
			}
		}
    }
}
