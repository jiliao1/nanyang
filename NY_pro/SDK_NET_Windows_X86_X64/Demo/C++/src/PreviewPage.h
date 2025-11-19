#pragma once

#include <QWidget>
#include "ui_PreviewPage.h"
#include "IRCNetSDKDef.h"
#include <QImage>
#include <QMutex>
#include "MsgBox.h"

class MainWindow;
class PreviewPage : public QWidget
{
    Q_OBJECT

public:
    PreviewPage(QWidget *parent = nullptr);
    ~PreviewPage();

    QImage GetImg();
    static void FrameCallback(IRC_NET_HANDLE handle, char* frame, int width, int height, void* userData);
    static void FrameCallback_V2(IRC_NET_HANDLE handle, IRC_NET_VIDEO_INFO_CB* videoInfo, IRC_NET_IVS_INFO_CB* ivsInfo, void* userData);
    static void FrameCallback_Private(IRC_NET_HANDLE handle, IRC_NET_VIDEO_INFO_CB* videoInfo, IRC_NET_IVS_INFO_CB* ivsInfo, void* userData);
    void StopPreview();

    void ClearUi();

signals:
    void UpdateFrame();
    void PreviewOpen(bool);

protected:
    void InitForm();
    void ConnectSignalSlot();

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void OnPreviewBtnClicked();
    void OnSnapNormalBtnClicked();
    void OnSnapIrgBtnClicked();
    void OnSnapDlt664BtnClicked();
    void OnRecordBtnClicked();

private:
    Ui::PreviewPage ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;
    QImage m_img;
    QMutex m_mutex;
};
