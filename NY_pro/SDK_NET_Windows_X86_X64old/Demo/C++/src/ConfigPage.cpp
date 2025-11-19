#include "ConfigPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"
#include <QMessageBox>
#include <QDateTime>
#include <QButtonGroup>
#include <QRegExpValidator>

ConfigPage::ConfigPage(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_mainWindow = (MainWindow*)parent;
    
    InitForm();
    ConnectSignalSlot();
    
}

ConfigPage::~ConfigPage()
{}

void ConfigPage::ClearUi()
{
    ui.fameRateEdit->setText("");
    ui.emissivityEdit->setText("");
    ui.atmosTransEdit->setText("");
    ui.refTempEdit->setText("");
    ui.atmosTempEdit->setText("");
    ui.distanceEdit->setText("");
    ui.osdMasterCheck->setChecked(false);
    ui.pseudoColorCheck->setChecked(false);
    ui.paletteCombo->setCurrentIndex(-1);
    ui.gainSwitchCombo->setCurrentIndex(-1);
    ui.tempEnbleCheck->setChecked(false);
    ui.lowerThresholdEdit->setText("");
    ui.upperThresholdEdit->setText("");
    ui.dateTimeCheck->setChecked(false);
    ui.dateTimeLeftEdit->setText("");
    ui.dateTimeTopEdit->setText("");
    ui.dateTimeRightEdit->setText("");
    ui.dateTimeBottonEdit->setText("");
    ui.channelTitleCheck->setChecked(false);
    ui.channelTitleNameEdit->setText("");
    ui.channelTitleLeftEdit->setText("");
    ui.channelTitleTopEdit->setText("");
    ui.channelTitleRightEdit->setText("");
    ui.channelTitleBottonEdit->setText("");
    ui.unitCombo->setCurrentIndex(-1);
    ui.flipcomboBox->setCurrentIndex(-1);
    ui.brightnesslineEdit->setText("");
    ui.contrastlineEdit->setText("");
    ui.denoise2DCheckBox->setChecked(false);
    ui.denoise2DLineEdit->setText("");
    ui.denoise3DCheckBox->setChecked(false);
    ui.denoise3DLineEdit->setText("");
    ui.detailEnhanceCheckBox->setChecked(false);
    ui.detailEnhanceLineEdit->setText("");
}

void ConfigPage::InitForm()
{
    ui.ipEdit->setValidator(new QRegExpValidator(QRegExp("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")));
    ui.subnetMaskEdit->setValidator(new QRegExpValidator(QRegExp("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")));
    ui.getwayEdit->setValidator(new QRegExpValidator(QRegExp("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")));
    ui.standbuDnsEdit->setValidator(new QRegExpValidator(QRegExp("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")));
    ui.defaultDnsEdit->setValidator(new QRegExpValidator(QRegExp("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")));
}

void ConfigPage::ConnectSignalSlot()
{
    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(ui.fameRateGetBtn, SIGNAL(clicked()), this, SLOT(onFameRateGetBtnClicked()));
    connect(ui.fameRateSetBtn, SIGNAL(clicked()), this, SLOT(onFameRateSetBtnClicked()));
    connect(ui.synTimeBtn, SIGNAL(clicked()), this, SLOT(onSynTimeBtnClicked()));
    connect(ui.shutterCorrectBtn, SIGNAL(clicked()), this, SLOT(onShutterCorrectBtnClicked()));
    connect(ui.envParaGetBtn, SIGNAL(clicked()), this, SLOT(onEnvParaGetBtnClicked()));
    connect(ui.envParaSetBtn, SIGNAL(clicked()), this, SLOT(onEnvParaSetBtnClicked()));
    connect(ui.osdMasterGetBtn, SIGNAL(clicked()), this, SLOT(onOsdMasterGetBtnClicked()));
    connect(ui.osdMasterSetBtn, SIGNAL(clicked()), this, SLOT(onOsdMasterSetBtnClicked()));
    connect(ui.pseudoColorGetBtn, SIGNAL(clicked()), this, SLOT(onPseudoColorGetBtnClicked()));
    connect(ui.pseudoColorSetBtn, SIGNAL(clicked()), this, SLOT(onPseudoColorSetBtnClicked()));
    connect(ui.paletteGetBtn, SIGNAL(clicked()), this, SLOT(onPaletteGetBtnClicked()));
    connect(ui.paletteSetBtn, SIGNAL(clicked()), this, SLOT(onPaletteSetBtnClicked()));
    connect(ui.gainSwitchGetBtn, SIGNAL(clicked()), this, SLOT(onGainSwitchGetBtnClicked()));
    connect(ui.gainSwitchSetBtn, SIGNAL(clicked()), this, SLOT(onGainSwitchSetBtnClicked()));
    connect(ui.tempSpanGetBtn, SIGNAL(clicked()), this, SLOT(onTempSpanGetBtnClicked()));
    connect(ui.tempSpanSetBtn, SIGNAL(clicked()), this, SLOT(onTempSpanSetBtnClicked()));
    connect(ui.dateTitleGetBtn, SIGNAL(clicked()), this, SLOT(onDateTitleGetBtnClicked()));
    connect(ui.dateTitleSetBtn, SIGNAL(clicked()), this, SLOT(onDateTitleSetBtnClicked()));
    connect(ui.channelTitleGetBtn, SIGNAL(clicked()), this, SLOT(onChannelTitleGetBtnClicked()));
    connect(ui.channelTitleSetBtn, SIGNAL(clicked()), this, SLOT(onChannelTitleSetBtnClicked()));
    connect(ui.ipConfigGetBtn, SIGNAL(clicked()), this, SLOT(onIPConfigGetBtnClicked()));
    connect(ui.ipConfigSetBtn, SIGNAL(clicked()), this, SLOT(onIPConfigSetBtnClicked()));
    connect(ui.sendCmdBtn, SIGNAL(clicked()), this, SLOT(OnCmdSendBtnClicked()));
    connect(ui.devRebootBtn, SIGNAL(clicked()), this, SLOT(OnDevRebootBtnClicked()));
    connect(ui.devRestartBtn, SIGNAL(clicked()), this, SLOT(OnDevRestartBtnClicked()));
    connect(ui.laserOSDGetBtn, SIGNAL(clicked()), this, SLOT(OnLaserOSDGetBtnClicked()));
    connect(ui.laserOSDSetBtn, SIGNAL(clicked()), this, SLOT(OnLaserOSDSetBtnClicked()));
    connect(ui.unitGetBtn, SIGNAL(clicked()), this, SLOT(OnUnitGetBtnClicked()));
    connect(ui.unitSetBtn, SIGNAL(clicked()), this, SLOT(OnUnitSetBtnClicked()));
    connect(ui.thermalImageGetBtn, SIGNAL(clicked()), this, SLOT(OnThermalImageGetBtnClicked()));
    connect(ui.thermalImageSetBtn, SIGNAL(clicked()), this, SLOT(OnThermalImageSetBtnClicked()));
    connect(ui.thermalVideoEnhanceGetBtn, SIGNAL(clicked()), this, SLOT(OnThermalVideoEnhanceGetBtnClicked()));
    connect(ui.thermalVideoEnhanceSetBtn, SIGNAL(clicked()), this, SLOT(OnThermalVideoEnhanceSetBtnClicked()));
    connect(ui.devFactoryBtn, SIGNAL(clicked()), this, SLOT(OnDevFactoryBtnClicked()));
}

void ConfigPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}

void ConfigPage::onFameRateGetBtnClicked()
{
    int rate = 0;
    int err = IRC_NET_GetFrameRate(m_handle, &rate);
    QString temp = tr("GetFrameRate");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.fameRateEdit->setText(QString::number(rate));
    }
    else 
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onFameRateSetBtnClicked()
{
    int rate = ui.fameRateEdit->text().toInt();
    QString temp = "";
    int err = IRC_NET_SetFrameRate(m_handle, rate);
    temp = tr("SetFrameRate");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onSynTimeBtnClicked()
{
    std::string formattedDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
    const char* dateTime = formattedDateTime.c_str();
    int err = IRC_NET_SyncSystemTime(m_handle, dateTime);
    QString temp = tr("SyncSystemTime");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onShutterCorrectBtnClicked()
{
    int err = IRC_NET_CorrectShutter(m_handle);
    QString temp = tr("CorrectShutter");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onEnvParaGetBtnClicked()
{
    IRC_NET_ENV_PARAM envParam;
    int err = IRC_NET_GetEnvParam(m_handle, &envParam);
    QString temp = tr("GetEnvParam");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.emissivityEdit->setText(QString::number(envParam.emissivity, 'f', 2));
        ui.refTempEdit->setText(QString::number(envParam.reflectedTemp, 'f', 2));
        ui.distanceEdit->setText(QString::number(envParam.distance, 'f', 2));
        ui.atmosTransEdit->setText(QString::number(envParam.transmittance, 'f', 2));
        ui.atmosTempEdit->setText(QString::number(envParam.atmosphereTemp, 'f', 2));
    }
    else 
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onEnvParaSetBtnClicked()
{
    IRC_NET_ENV_PARAM envParam;
    envParam.emissivity = ui.emissivityEdit->text().toFloat();
    envParam.reflectedTemp = ui.refTempEdit->text().toFloat();
    envParam.distance = ui.distanceEdit->text().toFloat();
    envParam.transmittance = ui.atmosTransEdit->text().toFloat();
    envParam.atmosphereTemp = ui.atmosTempEdit->text().toFloat();
    int err = IRC_NET_SetEnvParam(m_handle, &envParam);
    QString temp = tr("SetEnvParam");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onOsdMasterGetBtnClicked()
{
    int osdMode = 0;
    int err = IRC_NET_GetOSDState(m_handle, &osdMode);
    QString temp = tr("GetOSDState");
    if (IRC_NET_ERROR_OK == err)
    {
        if (osdMode == 0)
        {
            ui.osdMasterCheck->setChecked(true);
        }
        else {
            ui.osdMasterCheck->setChecked(false);
        }
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onOsdMasterSetBtnClicked()
{
    bool state = ui.osdMasterCheck->isChecked();
    int osdMode = 2;
    if (ui.osdMasterCheck->isChecked())
    {
        osdMode = 0;
    }
    int err = IRC_NET_SetOSDState(m_handle, osdMode);
    QString temp = tr("SetOSDState");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onPseudoColorGetBtnClicked()
{
    int state = 0;
    int err = IRC_NET_GetTempBarState(m_handle, &state);
    QString temp = tr("GetTempBarState");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.pseudoColorCheck->setChecked(state);
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onPseudoColorSetBtnClicked()
{
    int state = ui.pseudoColorCheck->isChecked();
    int err = IRC_NET_SetTempBarState(m_handle, state);
    QString temp = tr("SetTempBarState");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onPaletteGetBtnClicked()
{
    int palleteType = 0;
    int err = IRC_NET_GetPalleteType(m_handle, &palleteType);
    QString temp = tr("GetPalleteType");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.paletteCombo->setCurrentIndex(palleteType);
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onPaletteSetBtnClicked()
{
    int palleteType = ui.paletteCombo->currentIndex();
    if (palleteType < 0)
    {
        return;
    }
    int err = IRC_NET_SetPalleteType(m_handle, palleteType);
    QString temp = tr("SetPalleteType");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onGainSwitchGetBtnClicked()
{
    int level = 0;
    int err = IRC_NET_GetTempLevel(m_handle, &level);
    QString temp = tr("GetTempLevel");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.gainSwitchCombo->setCurrentIndex(level);
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onGainSwitchSetBtnClicked()
{
    int level = ui.gainSwitchCombo->currentIndex();
    if (level < 0)
    {
        return;
    }
    int err = IRC_NET_SetTempLevel(m_handle, level);
    QString temp = tr("SetTempLevel");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onTempSpanGetBtnClicked()
{
    IRC_NET_TEMP_SPAN_INFO tempSpan;
    int err = IRC_NET_GetTempSpanInfo(m_handle, &tempSpan);
    QString temp = tr("GetTempSpanInfo");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.tempEnbleCheck->setChecked(tempSpan.enable);
        ui.lowerThresholdEdit->setText(QString::number(tempSpan.lowTemp, 'f', 2));
        ui.upperThresholdEdit->setText(QString::number(tempSpan.highTemp, 'f', 2));
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onTempSpanSetBtnClicked()
{
    IRC_NET_TEMP_SPAN_INFO tempSpan;
    tempSpan.enable = ui.tempEnbleCheck->isChecked();
    tempSpan.lowTemp = ui.lowerThresholdEdit->text().toFloat();
    tempSpan.highTemp = ui.upperThresholdEdit->text().toFloat();
    int err = IRC_NET_SetTempSpanInfo(m_handle, &tempSpan);
    QString temp = tr("SetTempSpanInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onDateTitleGetBtnClicked()
{
    IRC_NET_OSD_TIME_TITLE_INFO timeTitle;
	int channel = m_mainWindow->getOnlineChannel();
    int err = IRC_NET_GetOSDTimeTitleInfo(m_handle, channel, &timeTitle);
    QString temp = tr("GetOSDTimeTitleInfo");
	if (IRC_NET_ERROR_OK == err)
    {
        ui.dateTimeCheck->setChecked(timeTitle.enable);
        ui.dateTimeLeftEdit->setText(QString::number(timeTitle.rect.left));
        ui.dateTimeTopEdit->setText(QString::number(timeTitle.rect.top));
        ui.dateTimeRightEdit->setText(QString::number(timeTitle.rect.right));
        ui.dateTimeBottonEdit->setText(QString::number(timeTitle.rect.bottom));
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onDateTitleSetBtnClicked()
{
    IRC_NET_OSD_TIME_TITLE_INFO timeTitle;
    timeTitle.enable = ui.dateTimeCheck->isChecked();
    timeTitle.rect.left = ui.dateTimeLeftEdit->text().toInt();
    timeTitle.rect.top = ui.dateTimeTopEdit->text().toInt();
    timeTitle.rect.right = ui.dateTimeRightEdit->text().toInt();
    timeTitle.rect.bottom = ui.dateTimeBottonEdit->text().toInt();
	int channel = m_mainWindow->getOnlineChannel();
    int err = IRC_NET_SetOSDTimeTitleInfo(m_handle, channel, &timeTitle);
    QString temp = tr("SetOSDTimeTitleInfo");
	if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onChannelTitleGetBtnClicked()
{
    IRC_NET_OSD_CHANNEL_TITLE_INFO channelTitle{};
	int channel = m_mainWindow->getOnlineChannel();
    int err = IRC_NET_GetOSDChannelTitleInfo(m_handle, channel, &channelTitle);
    QString temp = tr("GetOSDChannelTitleInfo");
	if (IRC_NET_ERROR_OK == err)
    {
        ui.channelTitleCheck->setChecked(channelTitle.enable);
        ui.channelTitleNameEdit->setText(QString::fromStdString(channelTitle.name));
        ui.channelTitleLeftEdit->setText(QString::number(channelTitle.rect.left));
        ui.channelTitleTopEdit->setText(QString::number(channelTitle.rect.top));
        ui.channelTitleRightEdit->setText(QString::number(channelTitle.rect.right));
        ui.channelTitleBottonEdit->setText(QString::number(channelTitle.rect.bottom));
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onChannelTitleSetBtnClicked()
{
    IRC_NET_OSD_CHANNEL_TITLE_INFO channelTitle{};
    channelTitle.enable = ui.channelTitleCheck->isChecked();
    strcpy(channelTitle.name, ui.channelTitleNameEdit->text().toStdString().c_str());
    channelTitle.rect.left = ui.channelTitleLeftEdit->text().toInt();
    channelTitle.rect.top = ui.channelTitleTopEdit->text().toInt();
    channelTitle.rect.right = ui.channelTitleRightEdit->text().toInt();
    channelTitle.rect.bottom = ui.channelTitleBottonEdit->text().toInt();
	int channel = m_mainWindow->getOnlineChannel();
    int err = IRC_NET_SetOSDChannelTitleInfo(m_handle, channel, &channelTitle);
    QString temp = tr("SetOSDChannelTitleInfo");
	if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onIPConfigGetBtnClicked()
{
    IRC_NET_IP_CONFIG ipConfig;
    int err = IRC_NET_GetIpConfig(m_handle, &ipConfig);
    QString temp = tr("GetIPConfig");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.dhcpEnableCheck->setChecked(ipConfig.dhcpEnable);
        ui.ipNameEdit->setText(QString::fromStdString(ipConfig.name));
        ui.macEdit->setText(QString::fromStdString(ipConfig.mac));
        ui.subnetMaskEdit->setText(QString::fromStdString(ipConfig.subnetMask));
        ui.getwayEdit->setText(QString::fromStdString(ipConfig.gateway));
        ui.ipEdit->setText(QString::fromStdString(ipConfig.ip));
        ui.defaultDnsEdit->setText(QString::fromStdString(ipConfig.defaultDns));
        ui.standbuDnsEdit->setText(QString::fromStdString(ipConfig.standbyDns));
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::onIPConfigSetBtnClicked()
{
    IRC_NET_IP_CONFIG ipConfig;
    ipConfig.dhcpEnable = ui.dhcpEnableCheck->isChecked();
    strcpy(ipConfig.subnetMask, ui.subnetMaskEdit->text().toStdString().c_str());
    strcpy(ipConfig.gateway, ui.getwayEdit->text().toStdString().c_str());
    strcpy(ipConfig.ip, ui.ipEdit->text().toStdString().c_str());
    strcpy(ipConfig.defaultDns, ui.defaultDnsEdit->text().toStdString().c_str());
    strcpy(ipConfig.standbyDns, ui.standbuDnsEdit->text().toStdString().c_str());
    strcpy(ipConfig.mac, ui.macEdit->text().toStdString().c_str());
    int err = IRC_NET_SetIpConfig(m_handle, &ipConfig);
    QString temp = tr("SetIPConfig");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnCmdSendBtnClicked()
{
    int err = IRC_NET_SetTransparentState(m_handle, 1);
    QString temp = tr("SetTransparentState");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    QString input = ui.sendCmdEdit->text();
    if (input.isEmpty())
    {
        return;
    }
    QStringList hexStrings = input.split(" ");
    QByteArray byteArray;
    for (const QString& hexString : hexStrings) {
        bool ok;
        quint8 byteValue = hexString.toUInt(&ok, 16);
        if (ok) {
            byteArray.append(static_cast<char>(byteValue));
        }
        else {
            QString temp = tr("Invalid input");
            MSG_BOX.ShowErrorMessage(temp, 1);
            return;
        }
    }
    int len = byteArray.length();
    char* cmd = new char[len] { 0 };
    for (int i = 0; i < len; i++)
    {
        cmd[i] = byteArray[i];
    }
    int revSize = 0;
    int size = 256;
    char* revCmd = new char[size];
    memset(revCmd, 0, size);
    err = IRC_NET_TransparentData(m_handle, cmd, len, revCmd, 256, &revSize,2000);
    temp = tr("SerialTransparent");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        if (nullptr != cmd)
        {
            delete[] cmd;
            cmd = nullptr;
        }
        if (nullptr != revCmd)
        {
            delete[] revCmd;
            revCmd = nullptr;
        }
        return;
    }
    QString tmpStr;
    for (int i = 0; i < revSize; i++)
    {
        tmpStr += QString("%1").arg(static_cast<unsigned char>(revCmd[i]), 2, 16, QChar('0')).toUpper();
        tmpStr += " ";
    }
    ui.recvCmdEdit->setText(tmpStr);
    err = IRC_NET_SetTransparentState(m_handle, 0);
}

void ConfigPage::OnDevRebootBtnClicked()
{
    int err = IRC_NET_SystemReboot(m_handle);
    QString temp = tr("Dev Reboot");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnDevRestartBtnClicked()
{
    int err = IRC_NET_SystemRestart(m_handle);
    QString temp = tr("Dev Restart");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnLaserOSDGetBtnClicked()
{
    IRC_NET_LASER_DISTANCE_OSD_PARAM osdParam;
    int channel = m_mainWindow->getOnlineChannel();
    int err = IRC_NET_GetLaserDistanceOsdParam(m_handle, channel, &osdParam);
    QString temp = tr("Get Laser Distance Osd Param");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.laserEnbleCheck->setChecked(osdParam.enable);
    ui.fontSizeCombo->setCurrentIndex(osdParam.titleFormat.fontSize);
    ui.alligmentCombo->setCurrentIndex(osdParam.titleFormat.alignType);
    ui.laserOsdLeftEdit->setText(QString::number(osdParam.titleFormat.titlePosition.left));
    ui.laserOsdTopEdit->setText(QString::number(osdParam.titleFormat.titlePosition.top));
    ui.laserOsdRightEdit->setText(QString::number(osdParam.titleFormat.titlePosition.right));
    ui.laserOsdBottomEdit->setText(QString::number(osdParam.titleFormat.titlePosition.bottom));
}

void ConfigPage::OnLaserOSDSetBtnClicked()
{
    IRC_NET_LASER_DISTANCE_OSD_PARAM osdParam;
    int channel = m_mainWindow->getOnlineChannel();
    osdParam.enable = ui.laserEnbleCheck->isChecked();
    osdParam.titleFormat.alignType = ui.alligmentCombo->currentIndex();
    osdParam.titleFormat.fontSize = ui.fontSizeCombo->currentIndex();
    osdParam.titleFormat.titlePosition.left = ui.laserOsdLeftEdit->text().toInt();
    osdParam.titleFormat.titlePosition.top = ui.laserOsdTopEdit->text().toInt();
    osdParam.titleFormat.titlePosition.right = ui.laserOsdRightEdit->text().toInt();
    osdParam.titleFormat.titlePosition.bottom = ui.laserOsdBottomEdit->text().toInt();
    int err = IRC_NET_SetLaserDistanceOsdParam(m_handle, channel, &osdParam);
    QString temp = tr("Set Laser Distance Osd Param");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void ConfigPage::OnUnitGetBtnClicked()
{
    int unit = 0;
    int err = IRC_NET_GetTempUnit(m_handle, &unit);
    QString temp = tr("GetTempUnit");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.unitCombo->setCurrentIndex(unit);
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnUnitSetBtnClicked()
{
    int unit = ui.unitCombo->currentIndex();
    if (unit < 0)
    {
        return;
    }
    int err = IRC_NET_SetTempUnit(m_handle, unit);
    QString temp = tr("SetTempUnit");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnThermalImageGetBtnClicked()
{
    int luminance, contrast, flipMode = 0;
    int err = IRC_NET_GetThermalImageLuminance(m_handle, &luminance);
    QString temp = tr("GetThermalImageLuminance");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.brightnesslineEdit->setText(QString::number(luminance));
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    err = IRC_NET_GetThermalImageContrast(m_handle, &contrast);
    temp = tr("GetThermalImageContrast");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.contrastlineEdit->setText(QString::number(contrast));
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    err = IRC_NET_GetThermalImageFlipMode(m_handle, &flipMode);
    temp = tr("GetThermalImageFlipMode");
    if (IRC_NET_ERROR_OK == err)
    {
        ui.flipcomboBox->setCurrentIndex(flipMode);
    }
    else {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

}

void ConfigPage::OnThermalImageSetBtnClicked()
{
    int luminance, contrast, flipMode = 0;
    luminance = ui.brightnesslineEdit->text().toInt();
    int err = IRC_NET_SetThermalImageLuminance(m_handle, luminance);
    QString temp = tr("SetThermalImageLuminance");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    contrast = ui.contrastlineEdit->text().toInt();
    err = IRC_NET_SetThermalImageContrast(m_handle, contrast);
    temp = tr("IRC_NET_SetThermalImageContrast");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    flipMode = ui.flipcomboBox->currentIndex();
    err = IRC_NET_SetThermalImageFlipMode(m_handle, flipMode);
    temp = tr("IRC_NET_SetThermalImageFlipMode");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnThermalVideoEnhanceGetBtnClicked()
{
    IRC_NET_THERMAL_IMAGE_MODE_ENHANCE_INFO thermalImageEnhanceInfo;
    int err = IRC_NET_GetThermalImageEnhanceInfo(m_handle, &thermalImageEnhanceInfo);
    QString temp = tr("IRC_NET_GetThermalImageEnhanceInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    ui.denoise2DCheckBox->setChecked(thermalImageEnhanceInfo.denoise2D.enable);
    ui.denoise2DLineEdit->setText(QString::number(thermalImageEnhanceInfo.denoise2D.level));
    ui.denoise3DCheckBox->setChecked(thermalImageEnhanceInfo.denoise3D.enable);
    ui.denoise3DLineEdit->setText(QString::number(thermalImageEnhanceInfo.denoise3D.level));
    ui.detailEnhanceCheckBox->setChecked(thermalImageEnhanceInfo.detailEnhance.enable);
    ui.detailEnhanceLineEdit->setText(QString::number(thermalImageEnhanceInfo.detailEnhance.level));
}

void ConfigPage::OnThermalVideoEnhanceSetBtnClicked()
{
    IRC_NET_THERMAL_IMAGE_MODE_ENHANCE_INFO thermalImageEnhanceInfo;
    int err = IRC_NET_GetThermalImageEnhanceInfo(m_handle, &thermalImageEnhanceInfo);
    QString temp = tr("IRC_NET_GetThermalImageEnhanceInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    thermalImageEnhanceInfo.denoise2D.enable = ui.denoise2DCheckBox->isChecked();
    thermalImageEnhanceInfo.denoise2D.level = ui.denoise2DLineEdit->text().toInt();
    thermalImageEnhanceInfo.denoise3D.enable = ui.denoise3DCheckBox->isChecked();
    thermalImageEnhanceInfo.denoise3D.level = ui.denoise3DLineEdit->text().toInt();
    thermalImageEnhanceInfo.detailEnhance.enable = ui.detailEnhanceCheckBox->isChecked();
    thermalImageEnhanceInfo.detailEnhance.level = ui.detailEnhanceLineEdit->text().toInt();
    err = IRC_NET_SetThermalImageEnhanceInfo(m_handle, &thermalImageEnhanceInfo);
    temp = tr("IRC_NET_SetThermalImageEnhanceInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void ConfigPage::OnDevFactoryBtnClicked()
{
    int err = IRC_NET_RestoreFactoryDefaults(m_handle);
    QString temp = tr("Dev Restore Factory");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}
