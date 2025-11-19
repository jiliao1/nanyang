#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "PreviewPage.h"
#include "TempAlarmPage.h"
#include "ConfigPage.h"
#include "PTZPage.h"
#include "DownloadPage.h"
#include "ControlPage.h"
#include "PeripheralPage.h"
#include "IRCNetSDKDef.h"
#include <QCalendarWidget>
#include "MsgBox.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getOnlineChannel() { return m_onlineChannel; };
    IRC_NET_HANDLE getHandle() { return m_handle; };
    IRC_NET_PTZ_ABILITY m_devAbility;
    IRC_NET_PTZ_AUX_STATE m_ptzAuxState;
    void closeEvent(QCloseEvent* event);
    void ResetVideoLabel();
    void SetOncePointTempParamCheck();
    void SetOncePointTempParamUnCheck();

signals:
    void ChangeHandle(IRC_NET_HANDLE handle);
    void Exception(int type);
    void SendDownload(QMap<int, IRC_NET_FILE_DOWNLOAD_INFO>);
    void Shielded(int, QList<QPoint>);
    void OnceGetPointTemp(QPoint);

protected:
    void InitForm();
    void InitPages();
    void RefreshPagesUi();
    void ConnectSignalSlot();
    void SearchFile();
    void GetAbility();
    void GetAuxState();
    void RefreshPages();
    void SetLoginInfoEnable(bool enable);
    void HandleDevSearchInfo(IRC_NET_DEV_SEARCH_INFO* searchInfo);
    void uncheckOthers(QAbstractButton* button, QButtonGroup* group);
    void uncheckAll(QButtonGroup* group);

    static void ExceptionCallback(IRC_NET_HANDLE handle, int exceptionType, void* userData);
    static void DevSearchCallback(IRC_NET_DEV_SEARCH_INFO* searchInfo, void* userData);

protected slots:
    void OnDevSearched(const QString& ip);
    void OnLoginBtnClicked();
    void OnDrawPointBtnClicked();
    void OnDrawLineBtnClicked();
    void OnDrawAreaBtnClicked();
    void OnDrawCircleBtnClicked();
    void OnDrawPolygonBtnClicked();
    void OnShieldedAreaBtnClicked();
    void OnUpdateFrame();
    void OnGetOncePointTemp(QPoint point);
    void OnAddRule(int ruleType, QList<QPoint> points);
    void OnAddShielded(int ruleType, QList<QPoint> points);
    void OnTabChanged(int index);
    void OnPreviewOpen(bool);
    void OnException(int type);
    void OnChannelButtonClicked();
    void OnFileTypeButtonClicked();
    void OnIPSearchButtonClicked();
    void OnSDcardSearchButtonClicked();
    void OnBackButtonClicked();
    void OnSellectAllButtonClicked();
    void OnNextButtonClicked();
    void OnDownloadButtonClicked();
    void OnPosition3D(bool state);
    void OnPositionRegionFocus(bool state);
    void OnPositionManualTrack(bool state);
    void OnPostion(int ruleType, QList<QPoint> points);
    void OnRegionFocus(QList<QPoint> points);
    void OnManualTrack(QList<QPoint> points);
    void OnDownloadProgressButtonClicked();

signals:
    void DevSearched(const QString& ip);

private:
    Ui::MainWindow *ui;
    PeripheralPage* m_peripheralPage = nullptr;
    PreviewPage* m_previewPage = nullptr;
    TempAlarmPage* m_tempAlarmPage = nullptr;
    ConfigPage* m_configPage = nullptr;
    PTZPage* m_ptzPage = nullptr;
    DownloadPage* m_downloadPage = nullptr;
    ControlPage* m_controlPage = nullptr;
    IRC_NET_HANDLE m_handle = 0;
    int m_onlineChannel = 0;
    int m_playbackChannel = 0;
    int m_filetype = 1;
    int m_page = 0;
    QMap<int, IRC_NET_FILE_INFO> m_fileInfoMap;
    QMap<int, IRC_NET_FILE_DOWNLOAD_INFO> m_fileDownloadMap;
    IRC_NET_DEV_INFO m_devInfo;
    QButtonGroup* m_group;
};
#endif // MAINWINDOW_H
