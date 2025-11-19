#pragma once

#include <QMainWindow>
#include "ui_PeripheralPage.h"
#include "IRCNetSDKDef.h"

class MainWindow;
class PeripheralPage : public QWidget
{
    Q_OBJECT

public:
    PeripheralPage(QWidget *parent = nullptr);
    ~PeripheralPage();
    void SetDayOrNightShow(bool isShow);
protected:
    void ConnectSignalSlot();
    void TansparentSecondsToTime(int time, int& hours, int& minites, int& seconds);
    void ConvertTimeToSeconds(bool isStart, int& totalSeconds);

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void OnGetWiperBtn();
    void OnSetWiperBtn();
    void OnGetFillLightBtn();
    void OnSetFillLightBtn();
    void OnComboxIndexChanged(int index);
    void OnGetDayNightParamBtn();
    void OnSetDayNightParamBtn();
    void OnGetLogoPictureBtn();
    void OnSetLogoPictureBtn();

private:
    Ui::PeripheralPageClass ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;
};
