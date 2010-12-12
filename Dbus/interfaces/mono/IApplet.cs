using System;
using System.Collections.Generic;  // Dictionnary
using NDesk.DBus;

namespace CairoDock.Applet
{
	public enum AppletEmblemPosition
	{
		UpperLeft,
		LowerRight,
		LowerLeft,
		UpperRight,
		Middle
	}
	
	public enum AppletContainerType
	{
		Dock,
		Desklet
	}
	
	public enum AppletOrientationType
	{
		Bottom,
		Top,
		Right,
		Left
	}

	[NDesk.DBus.Interface("org.cairodock.CairoDock.applet")]
	public interface IApplet
	{
		object Get(string cProperty);
		Dictionary<string, object> GetAll();
		void SetQuickInfo(string cQuickInfo);
		void SetLabel(string cLabel);
		void SetIcon(string cImage);
		void SetEmblem(string cImage, int iPosition);
		void Animate(string cAnimation, int iNbRounds);
		void DemandsAttention(bool bStart, string cAnimation);
		void ShowDialog(string cMessage, int iDuration);
		void AskQuestion(string cMessage);
		void AskValue(string cMessage, double fInitialValue, double fMaxlValue);
		void AskText(string cMessage, string cInitialText);
		void PopupDialog(Dictionary<string, object> hDialogAttributes, Dictionary<string, object> hWidgetAttributes);
		void AddDataRenderer(string cType, int iNbValues, string cTheme);
		void RenderValues(double[] pValues);
		void ControlAppli(string cApplicationClass);
		void ShowAppli(bool bShow);
		void PopulateMenu(string[] pLabels);
		void AddMenuItems(Dictionary<string, object>[] pItems);
		void BindShortkey(string[] cShortkeys);
		event OnClickEvent on_click;
		event OnMiddleClickEvent on_middle_click;
		event OnScrollEvent on_scroll;
		event OnBuildMenuEvent on_build_menu;
		event OnMenuSelectEvent on_menu_select;
		event OnDropDataEvent on_drop_data;
		event OnChangeFocusEvent on_change_focus;
		event OnAnswerEvent on_answer;
		event OnAnswerDialogEvent on_answer_dialog;
		event OnShortkeyEvent on_shortkey;
		event OnStopModuleEvent _on_stop;
		event OnReloadModuleEvent _on_reload;
	}

	public delegate void OnClickEvent(int iButtonState);

	public delegate void OnMiddleClickEvent();

	public delegate void OnScrollEvent(bool bDirectionUp);

	public delegate void OnBuildMenuEvent();

	public delegate void OnMenuSelectEvent(int iNumEntry);

	public delegate void OnDropDataEvent(string cReceivedData);

	public delegate void OnChangeFocusEvent(bool is_active);

	public delegate void OnAnswerEvent(object answer);

	public delegate void OnAnswerDialogEvent(int iClickedButton, object answer);

	public delegate void OnShortkeyEvent(string cShortkey);

	public delegate void OnStopModuleEvent();

	public delegate void OnReloadModuleEvent(bool bConfigHasChanged);

}
