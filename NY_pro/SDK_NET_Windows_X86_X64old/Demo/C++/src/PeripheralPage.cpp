#include "PeripheralPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"
#include <QMessageBox>
#include <QFileDialog>

PeripheralPage::PeripheralPage(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_mainWindow = (MainWindow*)parent;
    ui.SensitivityEdit->setVisible(false);
    ui.label_4->setVisible(false);
    ui.label_5->setVisible(false);
    ui.startDateTimeEdit->setVisible(false);
    ui.label_6->setVisible(false);
    ui.endDateTimeEdit->setVisible(false);
    ui.startDateTimeEdit->setDisplayFormat("HH:mm:ss");
    ui.endDateTimeEdit->setDisplayFormat("HH:mm:ss");
    ConnectSignalSlot();
}

PeripheralPage::~PeripheralPage()
{}
void PeripheralPage::SetDayOrNightShow(bool isShow)
{
    if (isShow)
    {
        ui.groupBox_2->setEnabled(true);
    }
    else
    {
        ui.groupBox_2->setEnabled(false);
    }
}
void PeripheralPage::ConnectSignalSlot()
{
    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(ui.GetWiperBtn, SIGNAL(clicked()), this, SLOT(OnGetWiperBtn()));
    connect(ui.SetWiperBtn, SIGNAL(clicked()), this, SLOT(OnSetWiperBtn()));
    connect(ui.GetFillLightBtn, SIGNAL(clicked()), this, SLOT(OnGetFillLightBtn()));
    connect(ui.SetFillLightBtn, SIGNAL(clicked()), this, SLOT(OnSetFillLightBtn()));
    connect(ui.DayNightCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboxIndexChanged(int)));
    connect(ui.GetDayNightParamBtn, SIGNAL(clicked()), this, SLOT(OnGetDayNightParamBtn()));
    connect(ui.SetDayNightParamBtn, SIGNAL(clicked()), this, SLOT(OnSetDayNightParamBtn()));
    connect(ui.GetLogoBtn, SIGNAL(clicked()), this, SLOT(OnGetLogoPictureBtn()));
    connect(ui.SetLogoBtn, SIGNAL(clicked()), this, SLOT(OnSetLogoPictureBtn()));
}

void PeripheralPage::TansparentSecondsToTime(int time, int& hours, int& minites, int& seconds)
{
    if (time <= 0)
    {
        hours = 0;
        minites = 0;
        seconds = 0;
        return;
    }
    hours = time / 3600;
    minites = (time - 3600 * hours) / 60;
    seconds = time - 3600 * hours - 60 * minites;
}

void PeripheralPage::ConvertTimeToSeconds(bool isStart, int& totalSeconds)
{
    int hours,minites,seconds = 0;
    QTime time;
    if (isStart)
    {
        time = ui.startDateTimeEdit->time();
    }
    else
    {
        time = ui.endDateTimeEdit->time();
    }
    hours = time.hour();
    minites = time.minute();
    seconds = time.second();
    totalSeconds = hours * 3600 + minites * 60 + seconds;
}

void PeripheralPage::OnGetWiperBtn()
{
    int mode = -1;
    int err = IRC_NET_GetWiperConfigInfo(m_handle, &mode);
    QString temp = tr("GetWiperConfigInfo");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.WiperCombox->setCurrentIndex(mode);
    }
    else
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PeripheralPage::OnSetWiperBtn()
{
    int mode = -1;
    if ("Auto" == ui.WiperCombox->currentText())
    {
        mode = 0;
    }
    if ("Manual" == ui.WiperCombox->currentText())
    {
        mode = 1;
    }
    
    int err = IRC_NET_SetWiperConfigInfo(m_handle,mode);
    QString temp = tr("SetWiperConfigInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PeripheralPage::OnGetFillLightBtn()
{
    IRC_NET_FILL_LIGHT_CONFIG_INFO fillLightConfigInfo;
    int err = IRC_NET_GetFillLightConfigInfo(m_handle, &fillLightConfigInfo);
    QString temp = tr("GetFillLight");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.FillLightCombox->setCurrentIndex(fillLightConfigInfo.fillLightMode);
    }
    else
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PeripheralPage::OnSetFillLightBtn()
{
    int mode = -1;
    if ("Auto" == ui.FillLightCombox->currentText())
    {
        mode = 0;
    }
    if ("Manual" == ui.FillLightCombox->currentText())
    {
        mode = 1;
    }
    IRC_NET_FILL_LIGHT_CONFIG_INFO fillLightConfigInfo;
    fillLightConfigInfo.fillLightMode = mode;
    int err = IRC_NET_SetFillLightConfigInfo(m_handle, &fillLightConfigInfo);
    QString temp = tr("SetFillLight");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PeripheralPage::OnComboxIndexChanged(int index)
{
    if (0 == index)
    {
        ui.SensitivityEdit->setVisible(true);
        ui.label_4->setVisible(true);
        ui.label_5->setVisible(false);
        ui.startDateTimeEdit->setVisible(false);
        ui.label_6->setVisible(false);
        ui.endDateTimeEdit->setVisible(false);
    }
    else if (3 == index)
    {
        ui.SensitivityEdit->setVisible(false);
        ui.label_4->setVisible(false);
        ui.label_5->setVisible(true);
        ui.startDateTimeEdit->setVisible(true);
        ui.label_6->setVisible(true);
        ui.endDateTimeEdit->setVisible(true);
    }
    else
    {
        ui.SensitivityEdit->setVisible(false);
        ui.label_4->setVisible(false);
        ui.label_5->setVisible(false);
        ui.startDateTimeEdit->setVisible(false);
        ui.label_6->setVisible(false);
        ui.endDateTimeEdit->setVisible(false);
    }
}

void PeripheralPage::OnGetDayNightParamBtn()
{
    IRC_NET_DAY_NIGHT_MODE_PARAM param;
    int err = IRC_NET_GetDayNightModeParam(m_handle, &param);
    QString temp = tr("GetDayNightModeParam");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    ui.DayNightCombox->setCurrentIndex(param.dayNightMode);
    if (0 == param.dayNightMode)
    {
        ui.SensitivityEdit->setText(QString::number(param.sensitivity));
    }
    if (3 == param.dayNightMode)
    {
        int hours, minites, seconds = 0;
        TansparentSecondsToTime(param.start, hours, minites, seconds);
        QTime startTime(hours, minites, seconds);
        ui.startDateTimeEdit->setTime(startTime);
        ui.startDateTimeEdit->setDisplayFormat("HH:mm:ss");
        TansparentSecondsToTime(param.end, hours, minites, seconds);
        QTime endTime(hours, minites, seconds);
        ui.endDateTimeEdit->setTime(endTime);
        ui.endDateTimeEdit->setDisplayFormat("HH:mm:ss");
    }
}

void PeripheralPage::OnSetDayNightParamBtn()
{
    IRC_NET_DAY_NIGHT_MODE_PARAM param;
    int err = IRC_NET_GetDayNightModeParam(m_handle, &param);
    QString temp = tr("GetDayNightModeParam");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    if (0 == ui.DayNightCombox->currentIndex())
    {
        param.sensitivity = ui.SensitivityEdit->text().toInt();
    }
    if (3 == ui.DayNightCombox->currentIndex())
    {
        int startTotal, endTotal = 0;
        ConvertTimeToSeconds(true, startTotal);
        param.start = startTotal;
        ConvertTimeToSeconds(false, endTotal);
        param.start = startTotal;
        param.end = endTotal;
    }
    param.dayNightMode = ui.DayNightCombox->currentIndex();
    err = IRC_NET_SetDayNightModeParam(m_handle, &param);
    temp = tr("SetDayNightModeParam");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PeripheralPage::OnGetLogoPictureBtn()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("GetLogo"), "", "*.png");
    if (fileName.isEmpty())
    {
        return;
    }
    int id = ui.LogoCombox->currentIndex();
    int err = IRC_NET_GetLogoPicture(m_handle,id,fileName.toUtf8());
    QString temp = tr("GetLogoPicture");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PeripheralPage::OnSetLogoPictureBtn()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("SetLogo"), "", "*.png");
    if (fileName.isEmpty())
    {
        return;
    }
    int id = ui.LogoCombox->currentIndex();
    int err = IRC_NET_SetLogoPicture(m_handle, id, fileName.toUtf8());
    QString temp = tr("SetLogoPicture");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PeripheralPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}


