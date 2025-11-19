#include "PreviewPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

PreviewPage::PreviewPage(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_mainWindow = (MainWindow*)parent;
    InitForm();
    ConnectSignalSlot();
}

PreviewPage::~PreviewPage()
{
    if (tr("Stop") != ui.recordBtn->text())
    {
        IRC_NET_StopPreviewRecord(m_handle);
    }

    if (tr("Close") != ui.previewBtn->text())
    {
        IRC_NET_StopPreview(m_handle);
    }
}

QImage PreviewPage::GetImg()
{
    QMutexLocker locker(&m_mutex);
    return m_img;
}

void PreviewPage::FrameCallback(IRC_NET_HANDLE handle, char* frame, int width, int height, void* userData)
{
    PreviewPage* page = static_cast<PreviewPage*>(userData);
    {
        QMutexLocker locker(&page->m_mutex);
        page->m_img = QImage((unsigned char*)frame, width, height, QImage::Format_RGBA8888).copy();
    }
    page->UpdateFrame();
}

void PreviewPage::FrameCallback_V2(IRC_NET_HANDLE handle, IRC_NET_VIDEO_INFO_CB* videoInfo, IRC_NET_IVS_INFO_CB* ivsInfo, void* userData)
{
    PreviewPage* page = static_cast<PreviewPage*>(userData);
    {
        QMutexLocker locker(&page->m_mutex);
        QImage img = QImage((unsigned char*)videoInfo->frame, videoInfo->width, videoInfo->height, QImage::Format_RGBA8888).copy();
        if (0 != videoInfo->validWidth && 0 != videoInfo->validHeight)
        {
            page->m_img = img.copy(0, 0, videoInfo->validWidth, videoInfo->validHeight);
        }
        else
        {
            page->m_img = img.copy();
        }
    }
    page->UpdateFrame();
}

void PreviewPage::FrameCallback_Private(IRC_NET_HANDLE handle, IRC_NET_VIDEO_INFO_CB* videoInfo, IRC_NET_IVS_INFO_CB* ivsInfo, void* userData)
{
    PreviewPage* page = static_cast<PreviewPage*>(userData);
    {
        QMutexLocker locker(&page->m_mutex);
        QImage img = QImage((unsigned char*)videoInfo->frame, videoInfo->width, videoInfo->height, QImage::Format_RGBA8888).copy();
        if (0 != videoInfo->validWidth && 0 != videoInfo->validHeight)
        {
            page->m_img = img.copy(0, 0, videoInfo->validWidth, videoInfo->validHeight);
        }
        else
        {
            page->m_img = img.copy();
        }
    }
    page->UpdateFrame();
}

void PreviewPage::StopPreview()
{
    if (tr("Open") != ui.previewBtn->text())
    {
        OnPreviewBtnClicked();
    }
}

void PreviewPage::ClearUi()
{
}

void PreviewPage::InitForm()
{

}

void PreviewPage::ConnectSignalSlot()
{
    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(ui.previewBtn, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
    connect(ui.snapNormalBtn, SIGNAL(clicked()), this, SLOT(OnSnapNormalBtnClicked()));
    connect(ui.snapIrgBtn, SIGNAL(clicked()), this, SLOT(OnSnapIrgBtnClicked()));
    connect(ui.snapDlt664Btn, SIGNAL(clicked()), this, SLOT(OnSnapDlt664BtnClicked()));
    connect(ui.recordBtn, SIGNAL(clicked()), this, SLOT(OnRecordBtnClicked()));
}

void PreviewPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}

void PreviewPage::OnPreviewBtnClicked()
{
    if (tr("Open") == ui.previewBtn->text())


    {
        m_mainWindow->ResetVideoLabel();
        IRC_NET_PREVIEW_INFO previewInfo{ m_mainWindow->getOnlineChannel(), IRC_NET_STREAM_MAIN ,IRC_NET_FRAME_FMT_RGBA };
        //IRC_NET_PREVIEW_INFO previewInfo{ m_mainWindow->getOnlineChannel(), IRC_NET_STREAM_SUN ,IRC_NET_FRAME_FMT_GRAY };
        int err = IRC_NET_StartPreview_V2(m_handle, &previewInfo, FrameCallback_V2, this);
        //int err = IRC_NET_StartPrivatePreview(m_handle, &previewInfo, FrameCallback_Private, this);
        QString temp = tr("StartPreview");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.previewBtn->setText(tr("Close"));
        emit PreviewOpen(true);
    }
    else
    {
        int err = IRC_NET_StopPreview(m_handle);
        QString temp = tr("StopPreview");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;

        }

        ui.previewBtn->setText(tr("Open"));
        emit PreviewOpen(false);
        if (tr("Stop") == ui.recordBtn->text())
        {
            IRC_NET_StopPreviewRecord(m_handle);
            ui.recordBtn->setText(tr("Start"));
        }
    }
}

void PreviewPage::OnSnapNormalBtnClicked()
{
    if (tr("Open") == ui.previewBtn->text())
    {
        QMessageBox::critical(this, tr("Info"), tr("Please open preview first."));
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Snapshot"), "", "*.jpg");
    if (fileName.isEmpty())
    {
        return;
    }
    int err = IRC_NET_PreviewSnapshot(m_handle, fileName.toUtf8());
    QString temp = tr("PreviewSnapshot");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PreviewPage::OnSnapIrgBtnClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Snapshot"));
    if (fileName.isEmpty())
    {
        return;
    }

    QString irgName(fileName + ".irg");
    QString jpgName(fileName + ".jpg");
    int err = IRC_NET_GetIRGImage(m_handle, irgName.toUtf8(), jpgName.toUtf8());
    QString temp = tr("GetIRGImage");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PreviewPage::OnSnapDlt664BtnClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Snapshot"), "", "*.jpg");
    if (fileName.isEmpty())
    {
        return;
    }
    int err = IRC_NET_GetDLT664Image(m_handle, fileName.toUtf8());
    QString temp = tr("GetDLT664Image");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PreviewPage::OnRecordBtnClicked()
{
    if (tr("Open") == ui.previewBtn->text())
    {
        QMessageBox::critical(this, tr("Info"), tr("Please open preview first."));
        return;
    }

    if (tr("Start") == ui.recordBtn->text())
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Record"), "", "*.mp4");
        if (fileName.isEmpty())
        {
            return;
        }
#ifdef SYS_LINUX
        fileName.append(".mp4");
#endif
        int err = IRC_NET_StartPreviewRecord(m_handle, fileName.toLocal8Bit());
        QString temp = tr("StartPreviewRecord");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }

        ui.recordBtn->setText(tr("Stop"));
    }
    else
    {
        int err = IRC_NET_StopPreviewRecord(m_handle);
        QString temp = tr("StopPreviewRecord");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }

        ui.recordBtn->setText(tr("Start"));
    }
}

