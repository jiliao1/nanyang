#pragma once

#include <QWidget>
#include "ui_ConfigPage.h"
#include "IRCNetSDKDef.h"
class MainWindow;
class ConfigPage : public QWidget
{
    Q_OBJECT

public:
    ConfigPage(QWidget *parent = nullptr);
    ~ConfigPage();

    void ClearUi();

protected:
    void InitForm();
    void ConnectSignalSlot();

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void onFameRateGetBtnClicked();
    void onFameRateSetBtnClicked();
    void onSynTimeBtnClicked();
    void onShutterCorrectBtnClicked();
    void onEnvParaGetBtnClicked();
    void onEnvParaSetBtnClicked();
    void onOsdMasterGetBtnClicked();
    void onOsdMasterSetBtnClicked();
    void onPseudoColorGetBtnClicked();
    void onPseudoColorSetBtnClicked();
    void onPaletteGetBtnClicked();
    void onPaletteSetBtnClicked();
    void onGainSwitchGetBtnClicked();
    void onGainSwitchSetBtnClicked();
    void onTempSpanGetBtnClicked();
    void onTempSpanSetBtnClicked();
    void onDateTitleGetBtnClicked();
    void onDateTitleSetBtnClicked();
    void onChannelTitleGetBtnClicked();
    void onChannelTitleSetBtnClicked();
    void onIPConfigGetBtnClicked();
    void onIPConfigSetBtnClicked();
    void OnCmdSendBtnClicked();
    void OnDevRebootBtnClicked();
    void OnDevRestartBtnClicked();
    void OnLaserOSDGetBtnClicked();
    void OnLaserOSDSetBtnClicked();
    void OnUnitGetBtnClicked();
    void OnUnitSetBtnClicked();
    void OnThermalImageGetBtnClicked();
    void OnThermalImageSetBtnClicked();
    void OnThermalVideoEnhanceGetBtnClicked();
    void OnThermalVideoEnhanceSetBtnClicked();
    void OnDevFactoryBtnClicked();
private:
    Ui::ConfigPage ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;

};
