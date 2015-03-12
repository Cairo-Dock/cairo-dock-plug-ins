using System;
using System.Collections.Generic;  // Dictionnary
using DBus;

namespace CairoDock.Applet
{
    [Interface("org.cairodock.CairoDock.subapplet")]
    public interface ISubApplet
    {
        void SetQuickInfo(string cQuickInfo, string cIconID);
        void SetLabel(string cLabel, string cIconID);
        void SetIcon(string cImage, string cIconID);
        void SetEmblem(string cImage, int iPosition, string cIconID);
        void Animate(string cAnimation, int iNbRounds, string cIconID);
        void ShowDialog(string message, int iDuration, string cIconID);
        void AskQuestion(string cMessage, string cIconID);
        void AskValue(string cMessage, double fInitialValue, double fMaxlValue, string cIconID);
        void AskText(string cMessage, string cInitialText, string cIconID);
        void AddSubIcons(string[] pIconFields);
        void RemoveSubIcon(string cIconID);
        event OnClickSubIconEvent on_click_sub_icon;
        event OnMiddleClickSubIconEvent on_middle_click_sub_icon;
        event OnScrollSubIconEvent on_scroll_sub_icon;
        event OnBuildMenuSubIconEvent on_build_menu_sub_icon;
        event OnMenuSelectSubIconEvent on_menu_select_sub_icon;
        event OnDropDataSubIconEvent on_drop_data_sub_icon;
        event OnAnswerSubIconEvent on_answer_sub_icon;
    }

    public delegate void OnClickSubIconEvent(int iButtonState, string cIconID);

    public delegate void OnMiddleClickSubIconEvent(string cIconID);

    public delegate void OnScrollSubIconEvent(bool bDirectionUp, string cIconID);

    public delegate void OnBuildMenuSubIconEvent(string cIconID);

    public delegate void OnMenuSelectSubIconEvent(int iNumEntry, string cIconID);

    public delegate void OnDropDataSubIconEvent(string cReceivedData, string cIconID);

    public delegate void OnAnswerSubIconEvent(object answer, string cIconID);
}
