#include "DownloadPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"
#include <QTimer>

DownloadPage::DownloadPage(MainWindow* mainWindow, QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_mainWindow = (MainWindow*)mainWindow;
    QTimer* timer = new QTimer(this);
    m_handle = m_mainWindow->getHandle();

    ui.tableView->verticalHeader()->setVisible(false);
    m_model = new QStandardItemModel(ui.tableView);
    ui.tableView->setModel(m_model);

    qRegisterMetaType<IRC_NET_FILE_DOWNLOAD_INFO>("IRC_NET_FILE_DOWNLOAD_INFO");

    InitForm();

    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(m_mainWindow, SIGNAL(SendDownload(QMap<int, IRC_NET_FILE_DOWNLOAD_INFO>)), this, SLOT(OnFile(QMap<int, IRC_NET_FILE_DOWNLOAD_INFO>)));
    connect(timer, &QTimer::timeout, this, &DownloadPage::OnTimedout);

    timer->start(1000);

}

DownloadPage::~DownloadPage()
{
}

void DownloadPage::InitForm()
{
    m_model->setColumnCount(3);
    QStringList header;
    header << tr("Index") << tr("Name") << tr("Progress");
    m_model->setHorizontalHeaderLabels(header);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);
}

void DownloadPage::Refresh()
{
    m_model->removeRows(0, m_model->rowCount());
    for (auto itor = m_fileDownloadMap.begin(); itor != m_fileDownloadMap.end(); ++itor)
    {
        IRC_NET_FILE_DOWNLOAD_PROGRESS downloadProgress;
        int ret = IRC_NET_GetDownloadProgress(m_handle, itor.key(), &downloadProgress);
        int percent = 0;
        if (downloadProgress.totalSize != 0)
        {
            percent = (double)downloadProgress.downloadSize * 100 / downloadProgress.totalSize;
        }

        QList<QStandardItem*> itemList;
        QString str = QString::fromLocal8Bit(itor.value().downloadPath);
        QStringList list = str.split("/");
        itemList << new QStandardItem(QString::number(itor.key())) <<
            new QStandardItem(list.last()) <<
            new QStandardItem(QString::number(percent) + "%");
        m_model->appendRow(itemList);
    }
}

void DownloadPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}

void DownloadPage::OnTimedout()
{
    Refresh();
}

void DownloadPage::OnFile(QMap<int, IRC_NET_FILE_DOWNLOAD_INFO> filemap)
{
    m_fileDownloadMap = filemap;
    Refresh();
}