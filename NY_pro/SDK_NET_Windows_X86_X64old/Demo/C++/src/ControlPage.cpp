#include "ControlPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"

ControlPage::ControlPage(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_mainWindow = (MainWindow*)parent;

    InitForm();
    ConnectSignalSlot();
}

ControlPage::~ControlPage()
{}

void ControlPage::InitForm()
{
}

void ControlPage::ConnectSignalSlot()
{
    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(ui.upBtn, SIGNAL(clicked()), this, SLOT(onUpBtnClicked()));
    connect(ui.downBtn, SIGNAL(clicked()), this, SLOT(onDownBtnClicked()));
}

void ControlPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}

void ControlPage::onUpBtnClicked()
{
    int err = IRC_NET_SwivelControl(m_handle, IRC_NET_SWIVEL_CMD_UP);
    QString temp = tr("Up Pitch control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ControlPage::onDownBtnClicked()
{
    int err = IRC_NET_SwivelControl(m_handle, IRC_NET_SWIVEL_CMD_DOWN);
    QString temp = tr("Down Pitch control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

