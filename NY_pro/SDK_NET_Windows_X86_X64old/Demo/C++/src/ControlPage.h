#pragma once

#include <QWidget>
#include "ui_ControlPage.h"
#include "IRCNetSDKDef.h"

class MainWindow;
class ControlPage : public QWidget
{
    Q_OBJECT

public:
    ControlPage(QWidget *parent = nullptr);
    ~ControlPage();


protected:
    void InitForm();
    void ConnectSignalSlot();

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void onUpBtnClicked();
    void onDownBtnClicked();

private:
    Ui::ControlPageClass ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;
};
