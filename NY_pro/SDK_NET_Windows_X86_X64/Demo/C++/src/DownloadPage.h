#pragma once

#include <QWidget>
#include "ui_DownloadPage.h"
#include "IRCNetSDK.h"
#include <QStandardItemModel>

class MainWindow;
class DownloadPage : public QWidget
{
    Q_OBJECT

public:
    DownloadPage(MainWindow* mainWindow, QWidget* parent = nullptr);
    ~DownloadPage();

protected:
    void InitForm();
    void Refresh();

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void OnFile(QMap<int, IRC_NET_FILE_DOWNLOAD_INFO> filemap);
    void OnTimedout();

private:
    Ui::DownloadPageClass ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;
    QMap<int, IRC_NET_FILE_DOWNLOAD_INFO> m_fileDownloadMap;
    QStandardItemModel* m_model = nullptr;
};
