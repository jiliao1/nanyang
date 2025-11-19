#include "PTZPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"
#include <QMessageBox>
#include <QButtonGroup>
#include <QThread>

PTZPage::PTZPage(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    m_mainWindow = (MainWindow*)parent;

    InitForm();
    ConnectSignalSlot();
}

PTZPage::~PTZPage()
{}

void PTZPage::InitForm()
{
    QStringList items;
    items << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    ui.speedCombo->addItems(items);

    ui.tableWidget->setColumnCount(2);
    QStringList verHeader; //行表头
    verHeader << "Preset" << "residenceTime(s)";
    ui.tableWidget->setHorizontalHeaderLabels(verHeader);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    RefreshRegionScan();
    ui.bootLineScanRadioBtn->setEnabled(false);
    ui.parkLineScanRadioBtn->setEnabled(false);
}
void PTZPage::ConnectSignalSlot()
{
    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(ui.upBtn, SIGNAL(pressed()), this, SLOT(OnUpBtnPressed()));
    connect(ui.upBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.downBtn, SIGNAL(pressed()), this, SLOT(OnDownBtnPressed()));
    connect(ui.downBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.leftBtn, SIGNAL(pressed()), this, SLOT(OnLeftBtnPressed()));
    connect(ui.leftBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.rightBtn, SIGNAL(pressed()), this, SLOT(OnRightBtnPressed()));
    connect(ui.rightBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.upLeftBtn, SIGNAL(pressed()), this, SLOT(OnUpLeftBtnPressed()));
    connect(ui.upLeftBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.upRightBtn, SIGNAL(pressed()), this, SLOT(OnUpRightBtnPressed()));
    connect(ui.upRightBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.downLeftBtn, SIGNAL(pressed()), this, SLOT(OnDownLeftBtnPressed()));
    connect(ui.downLeftBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.downRightBtn, SIGNAL(pressed()), this, SLOT(OnDownRightBtnPressed()));
    connect(ui.downRightBtn, SIGNAL(released()), this, SLOT(OnBtnReleased()));
    connect(ui.zoomInBtn, SIGNAL(pressed()), this, SLOT(OnZoomInBtnPressed()));
    connect(ui.zoomInBtn, SIGNAL(released()), this, SLOT(OnZoomInBtnReleased()));
    connect(ui.zoomOutBtn, SIGNAL(pressed()), this, SLOT(OnZoomOutBtnPressed()));
    connect(ui.zoomOutBtn, SIGNAL(released()), this, SLOT(OnZoomOutBtnReleased()));
    connect(ui.focusNearBtn, SIGNAL(pressed()), this, SLOT(OnFocusNearBtnPressed()));
    connect(ui.focusNearBtn, SIGNAL(released()), this, SLOT(OnFocusNearBtnReleased()));
    connect(ui.focusFarBtn, SIGNAL(pressed()), this, SLOT(OnFocusFarBtnPressed()));
    connect(ui.focusFarBtn, SIGNAL(released()), this, SLOT(OnFocusFarBtnReleased()));
    connect(ui.irisCloseBtn, SIGNAL(pressed()), this, SLOT(OnIrisCloseBtnPressed()));
    connect(ui.irisCloseBtn, SIGNAL(released()), this, SLOT(OnIrisCloseBtnRealeased()));
    connect(ui.irisOpenBtn, SIGNAL(pressed()), this, SLOT(OnIrisOpenBtnPressed()));
    connect(ui.irisOpenBtn, SIGNAL(released()), this, SLOT(OnIrisOpenBtnRealeased()));
    connect(ui.wiperBtn, SIGNAL(clicked()), this, SLOT(OnWiperBtnClicked()));
    connect(ui.lightBtn, SIGNAL(clicked()), this, SLOT(OnLightBtnClicked()));
    connect(ui.defrostBtn, SIGNAL(clicked()), this, SLOT(OnDefrostBtnClicked()));
    connect(ui.fogLampBtn, SIGNAL(clicked()), this, SLOT(OnFogLampBtnClicked()));
    connect(ui.heaterBtn, SIGNAL(clicked()), this, SLOT(OnHearterBtnClicked()));
    connect(ui.fanBtn, SIGNAL(clicked()), this, SLOT(OnFanBtnClicked()));
    connect(ui.threeDimensionalBtn, SIGNAL(clicked()), this, SLOT(OnThreeDimensionalBtnClicked()));
    connect(ui.regionFocusBtn, SIGNAL(clicked()), this, SLOT(OnRegionFocusBtnClicked()));
    connect(ui.manualTrackBtn, SIGNAL(clicked()), this, SLOT(OnManualTrackBtnClicked()));
    connect(ui.arrowsBtn, SIGNAL(clicked()), this, SLOT(OnArrowsBtnClicked()));
    connect(ui.autoMaticBtn, SIGNAL(clicked()), this, SLOT(OnAutoMaticBtnClicked()));
    connect(ui.presetGetAllBtn, SIGNAL(clicked()), this, SLOT(OnPresetGetAllButtonClicked()));
    connect(ui.presetDeleteAllBtn, SIGNAL(clicked()), this, SLOT(OnPresetDeleteAllButtonClicked()));
    connect(ui.saveBtn, SIGNAL(clicked()), this, SLOT(OnSaveButtonClicked()));
    connect(ui.moveToBtn, SIGNAL(clicked()), this, SLOT(OnMoveToButtonClicked()));
    connect(ui.presetGetCurrentBtn, SIGNAL(clicked()), this, SLOT(OnGetCurrentPresetBtnClicked()));
    connect(ui.deleteBtn, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));
    connect(ui.cruiseGetAllBtn, SIGNAL(clicked()), this, SLOT(OnCruiseGetAllButtonClicked()));
    connect(ui.cruiseDeleteAllBtn, SIGNAL(clicked()), this, SLOT(OnCruiseDeleteAllButtonClicked()));
    connect(ui.cruiseSaveBtn, SIGNAL(clicked()), this, SLOT(OnCruiseSaveButtonClicked()));
    connect(ui.cruiseDeleteBtn, SIGNAL(clicked()), this, SLOT(OnCruiseDeleteButtonClicked()));
    connect(ui.cruiseStartBtn, SIGNAL(clicked()), this, SLOT(OnCruiseStartButtonClicked()));
    connect(ui.addPresetBtn, SIGNAL(clicked()), this, SLOT(OnAddPresetButtonClicked()));
    connect(ui.deletePresetBtn, SIGNAL(clicked()), this, SLOT(OnDeletePresetButtonClicked()));
    connect(ui.patrolGetAllBtn, SIGNAL(clicked()), this, SLOT(OnPatrolGetAllButtonClicked()));
    connect(ui.patrolDeleteAllBtn, SIGNAL(clicked()), this, SLOT(OnPatrolDeleteAllButtonClicked()));
    connect(ui.startPatrolBtn, SIGNAL(clicked()), this, SLOT(OnStartPatrolButtonClicked()));
    connect(ui.startRecordBtn, SIGNAL(clicked()), this, SLOT(OnStartRecordButtonClicked()));
    connect(ui.setBtn, SIGNAL(clicked()), this, SLOT(OnSetButtonClicked()));
    connect(ui.presetCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPresetComboChanged(int)));
    connect(ui.cruiseCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCruiseComboChanged(int)));
    connect(ui.patrolCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnPatrolComboChanged(int)));
    connect(ui.resetBtn, SIGNAL(clicked()), this, SLOT(OnResetBtnClicked()));
    connect(ui.lensInitBtn, SIGNAL(clicked()), this, SLOT(OnLensInitBtnClicked()));
    connect(ui.laserDistanceBtn, SIGNAL(clicked()), this, SLOT(OnLaserDistanceBtnClicked()));
    connect(ui.GetAllAreaScanBtn, SIGNAL(clicked()), this, SLOT(OnGetAllAreaScanInfoBtnClicked()));
    connect(ui.GetOneAreaScanBtn, SIGNAL(clicked()), this, SLOT(OnGetOneAreaScanBtnClicked()));
    connect(ui.SetAreaScanInfoBtn, SIGNAL(clicked()), this, SLOT(OnSetAreaScanInfoBtnClicked()));
    connect(ui.areaScanStartBtn, SIGNAL(clicked()), this, SLOT(OnAreaScanStartBtnClicked()));
    connect(ui.areaScanStopBtn, SIGNAL(clicked()), this, SLOT(OnAreaScanStopBtnClicked()));
    connect(ui.DeleteOneAreaScanBtn, SIGNAL(clicked()), this, SLOT(OnDeleteOneAreaScanBtnClicked()));
    connect(ui.DeleteAllAreaScanBtn, SIGNAL(clicked()), this, SLOT(OnDeleteAllAreaScanBtnClicked()));
    connect(ui.bootRefreshBtn, SIGNAL(clicked()), this, SLOT(OnBootRefreshBtnClicked()));
    connect(ui.bootConfirmBtn, SIGNAL(clicked()), this, SLOT(OnBootConfirmBtnClicked()));
    connect(ui.bootPresetRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootPresetRadioBtnClicked()));
    connect(ui.bootTourRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootTourRadioBtnClicked()));
    connect(ui.bootPatternRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootPatternRadioBtnClicked()));
    connect(ui.bootAreaScanRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootAreaScanRadioBtnClicked()));
    connect(ui.bootLineScanRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootLineScanRadioBtnClicked()));
    connect(ui.bootAutoResetRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootAutoResetRadioBtnClicked()));
    
    connect(ui.parkRefreshBtn, SIGNAL(clicked()), this, SLOT(OnParkRefreshBtnClicked()));
    connect(ui.parkConfirmBtn, SIGNAL(clicked()), this, SLOT(OnParkConfirmBtnClicked()));
    connect(ui.getCoordinatesBtn, SIGNAL(clicked()), this, SLOT(OnGetCoordinatesBtnClicked()));

    connect(ui.parkPresetRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootPresetRadioBtnClicked()));
    connect(ui.parkTourRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootTourRadioBtnClicked()));
    connect(ui.parkPatternRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootPatternRadioBtnClicked()));
    connect(ui.parkAreaScanRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootAreaScanRadioBtnClicked()));
    connect(ui.parkLineScanRadioBtn, SIGNAL(clicked()), this, SLOT(OnBootLineScanRadioBtnClicked()));
    connect(ui.setATBtn, SIGNAL(clicked()), this, SLOT(OnSetATBtnClicked()));
    connect(ui.getZoomBtn, SIGNAL(clicked()), this, SLOT(OnGetZoomBtnClicked()));
    connect(ui.setZoomBtn, SIGNAL(clicked()), this, SLOT(OnSetZoomBtnClicked()));
    connect(ui.targetConfigGetBtn, SIGNAL(clicked()), this, SLOT(OnTargetConfigGetBtnClicked()));
    connect(ui.targetConfigSetBtn, SIGNAL(clicked()), this, SLOT(OnTargetConfigSetBtnClicked()));
    connect(ui.getBearingBtn, SIGNAL(clicked()), this, SLOT(OnGetBearingBtnClicked()));   
}

bool PTZPage::GetPreset()
{
    bool ret = false;
    do
    {
        int insize = 0;
        int outsize = 0;
        int err = IRC_NET_QueryPtzPresetSize(m_handle, &insize);
        QString temp = tr("QueryPtzPresetSize");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            break;
        }

        IRC_NET_PTZ_PRESET_INFO* presetInfo = new IRC_NET_PTZ_PRESET_INFO[insize];
        memset(presetInfo, 0, insize * sizeof(IRC_NET_PTZ_PRESET_INFO));
        err = IRC_NET_QueryPtzPreset(m_handle, presetInfo, insize, &outsize);
        temp = tr("QueryPtzPreset");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            delete[] presetInfo;
            break;
        }

        for (int i = 0; i < outsize; i++)
        {
            m_presetIndexMap[i] = presetInfo[i];
        }

        delete[] presetInfo;
        ret = true;
    } while (0);

    return ret;
}

bool PTZPage::GetCruise()
{
    bool ret = false;
    do
    {
        int insize = 0;
        int outsize = 0;
        int err = IRC_NET_QueryPtzTourSize(m_handle, &insize);
        QString temp = tr("QueryPtzTourSize");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            break;
        }

        IRC_NET_PTZ_TOUR_INFO* tourInfo = new IRC_NET_PTZ_TOUR_INFO[insize];
        memset(tourInfo, 0, insize * sizeof(IRC_NET_PTZ_TOUR_INFO));
        err = IRC_NET_QueryPtzTour(m_handle, 1, tourInfo, insize, &outsize);
        temp = tr("QueryPtzTour");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            delete[] tourInfo;
            break;
        }

        for (int i = 0; i < outsize; i++)
        {
            m_cruiseIndexMap[i] = tourInfo[i];
        }

        delete[] tourInfo;
        ret = true;
    } while (0);

    return ret;
}

void PTZPage::RefreshCruiseTable(int cruiseId)
{
    ui.tableWidget->setRowCount(m_cruiseIndexMap[cruiseId].presetNum);
    for (int i = 0; i < m_cruiseIndexMap[cruiseId].presetNum; i++)
    {
        ui.tableWidget->setItem(i, 0, new QTableWidgetItem());
        QComboBox* comboBox = new QComboBox();
        int index = 0;
        for (const auto& preset : m_presetIndexMap)
        {
            if (preset.enable)
            {
                comboBox->addItem(preset.name);
                comboBox->setItemData(index, preset.id);
                index++;
            }

            if (preset.id == m_cruiseIndexMap[cruiseId].presetInfos[i].presetInfo.id)
            {
                comboBox->setCurrentIndex(index - 1);
            }
        }

        ui.tableWidget->setCellWidget(i, 0, comboBox);
        ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(m_cruiseIndexMap[cruiseId].presetInfos[i].residenceTime)));
    }
}

void PTZPage::RefreshRegionScan()
{
    ui.clockwiseBtn->setChecked(true);
    ui.areaScanSpeedEdit->setText(0);
    ui.areaScanGradientEdit->setText(0);
    //ui.StartPresetIdEdit->setText(0);
    //ui.StopPresetIdEdit->setText(0);
}

void PTZPage::RefreshBootAction(IRC_NET_BOOT_ACTION_INFO bootActionInfo)
{
    ui.bootEnableCheckBox->setChecked(bootActionInfo.enable);
    switch (bootActionInfo.actionType)
    {
    case IRC_NET_ACTION_AUTO_RESET:
        ui.bootAutoResetRadioBtn->setChecked(true);
        ui.BootComboBox->clear();
        break;
    case IRC_NET_ACTION_PRESET:
        ui.bootPresetRadioBtn->setChecked(true);
        OnBootPresetRadioBtnClicked();
        RefreshShowCombo(true,bootActionInfo.presetId);
        break;
    case IRC_NET_ACTION_TOUR:
        ui.bootTourRadioBtn->setChecked(true);
        OnBootTourRadioBtnClicked();
        RefreshShowCombo(true,bootActionInfo.tourId);
        break;
    case IRC_NET_ACTION_PATTERN:
        ui.bootPatternRadioBtn->setChecked(true);
        OnBootPatternRadioBtnClicked();
        RefreshShowCombo(true,bootActionInfo.patternId);
        break;
    case IRC_NET_ACTION_LINE_SCAN:
        ui.bootLineScanRadioBtn->setChecked(true);
        break;
    case IRC_NET_ACTION_AREA_SCAN:
        OnBootAreaScanRadioBtnClicked();
        ui.bootAreaScanRadioBtn->setChecked(true);
        RefreshShowCombo(true,bootActionInfo.regionScanId);
        break;
    default:
        break;
    }
}

void PTZPage::RefreshParkAction(IRC_NET_PARK_ACTION_INFO* parkActionInfo)
{
   ui.parkEnableCheckBox->setChecked(parkActionInfo->enable);
   ui.parkTimeEdit->setText(QString::number(parkActionInfo->second));
    switch (parkActionInfo->actionType)
    {
    case IRC_NET_ACTION_AUTO_RESET:
        break;
    case IRC_NET_ACTION_PRESET:
        ui.parkPresetRadioBtn->setChecked(true);
        OnBootPresetRadioBtnClicked();
        RefreshShowCombo(false, parkActionInfo->presetId);
        break;
    case IRC_NET_ACTION_TOUR:
        ui.parkTourRadioBtn->setChecked(true);
        OnBootTourRadioBtnClicked();
        RefreshShowCombo(false, parkActionInfo->tourId);
        break;
    case IRC_NET_ACTION_PATTERN:
        ui.parkPatternRadioBtn->setChecked(true);
        OnBootPatternRadioBtnClicked();
        RefreshShowCombo(false, parkActionInfo->patternId);
        break;
    case IRC_NET_ACTION_LINE_SCAN:
        ui.parkLineScanRadioBtn->setChecked(true);
        break;
    case IRC_NET_ACTION_AREA_SCAN:
        OnBootAreaScanRadioBtnClicked();
        ui.parkAreaScanRadioBtn->setChecked(true);
        RefreshShowCombo(false, parkActionInfo->regionScanId);
        break;
    default:
        break;
    }
}

void PTZPage::RefreshShowCombo(bool state,int id)
{
    if (id <= 0)
    {
        ui.BootComboBox->setCurrentIndex(-1);
        ui.parkComboBox->setCurrentIndex(-1);
        return;
    }
    if (state)
    {
        for (int i = 0; i < ui.BootComboBox->count(); i++)
        {
            if (ui.BootComboBox->itemText(i) == QString::number(id))
            {
                ui.BootComboBox->setCurrentIndex(i);
            }
        }
    }
    else
    {
        for (int i = 0; i < ui.parkComboBox->count(); i++)
        {
            if (ui.parkComboBox->itemText(i) == QString::number(id))
            {
                ui.parkComboBox->setCurrentIndex(i);
            }
        }
    }
    
}

void PTZPage::GetAuxiState()
{
    IRC_NET_PTZ_AUX_STATE ptzAuxState = m_mainWindow->m_ptzAuxState;
    ui.wiperBtn->setChecked(ptzAuxState.wiperState);
    ui.lightBtn->setChecked(ptzAuxState.lightState);
    ui.defrostBtn->setChecked(ptzAuxState.defrostState);
    ui.fogLampBtn->setChecked(ptzAuxState.defogState);
    ui.fanBtn->setChecked(ptzAuxState.fanState);
    ui.heaterBtn->setChecked(ptzAuxState.heaterState);
    ui.autoMaticBtn->setChecked(ptzAuxState.autoFocusState);
    ui.arrowsBtn->setChecked(ptzAuxState.syncViewState);
}

void PTZPage::ClearUi()
{
    ui.presetCombo->clear();
    ui.presetNameEdit->setText("");
    ui.cruiseCombo->clear();
    ui.cruiseNameEdit->setText("");
    while (ui.tableWidget->rowCount() > 0)
    {
        ui.tableWidget->removeRow(0);
    }
    ui.patrolCombo->clear();
    ui.pSpinBox->setValue(0);
    ui.tSpinBox->setValue(0);
    ui.zSpinBox->setValue(0);
    ui.panEdit->setText("");
    ui.tiltEdit->setText("");
    ui.visZoomEdit->setText("");
    ui.irZoomEdit->setText("");
    ui.visFovEdit->setText("");
    ui.irFovEdit->setText("");
    ui.zoomMultEdit->setText("");
    ui.angleXEdit->setText("");
    ui.angleYEdit->setText("");
    ui.angleZEdit->setText("");
    ui.anglePEdit->setText("");
    ui.angleTEdit->setText("");
    ui.angleRTEdit->setText("");
    ui.accelerationXEdit->setText("");
    ui.accelerationYEdit->setText("");
    ui.accelerationZEdit->setText("");
    ui.rotationXEdit->setText("");
    ui.rotationYEdit->setText("");
    ui.rotationZEdit->setText("");

}

void PTZPage::InitButton()
{
    ui.zoomInBtn->setEnabled(m_mainWindow->m_devAbility.zoom);
    ui.zoomOutBtn->setEnabled(m_mainWindow->m_devAbility.zoom);
    ui.focusFarBtn->setEnabled(m_mainWindow->m_devAbility.focus);
    ui.focusNearBtn->setEnabled(m_mainWindow->m_devAbility.focus);
    ui.irisCloseBtn->setEnabled(m_mainWindow->m_devAbility.iris);
    ui.irisOpenBtn->setEnabled(m_mainWindow->m_devAbility.iris);
    if (!m_mainWindow->m_devAbility.ptz)
    {
        QWidget* widget = this;
        for (QObject* obj : widget->children())
        {
            if (QPushButton* button = qobject_cast<QPushButton*>(obj))
            {
                button->setEnabled(false);
            }
        }
    }
    ui.wiperBtn->setEnabled(m_mainWindow->m_devAbility.wiper);
    ui.lightBtn->setEnabled(m_mainWindow->m_devAbility.light);
    ui.defrostBtn->setEnabled(m_mainWindow->m_devAbility.defrost);
    ui.fogLampBtn->setEnabled(m_mainWindow->m_devAbility.defog);
    ui.fanBtn->setEnabled(m_mainWindow->m_devAbility.fan);
    ui.heaterBtn->setEnabled(m_mainWindow->m_devAbility.heater);
    ui.autoMaticBtn->setEnabled(m_mainWindow->m_devAbility.autoFocus);
    ui.arrowsBtn->setEnabled(m_mainWindow->m_devAbility.syncView);
    ui.setBtn->setEnabled(m_mainWindow->m_devAbility.ptzPosition);
    ui.threeDimensionalBtn->setEnabled(m_mainWindow->m_devAbility.position3D);
    ui.regionFocusBtn->setEnabled(m_mainWindow->m_devAbility.regionFocus);
    ui.manualTrackBtn->setEnabled(m_mainWindow->m_devAbility.manualTrack);
    ui.lensInitBtn->setEnabled(m_mainWindow->m_devAbility.lensInit);
    ui.laserDistanceBtn->setEnabled(m_mainWindow->m_devAbility.laserDistance);
    ui.upLeftBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.upBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.upRightBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.leftBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.downLeftBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.downBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.downRightBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.rightBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.speedCombo->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.widget_2->setEnabled(m_mainWindow->m_devAbility.ptz);
}

void PTZPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}


void PTZPage::OnUpBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_UP;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnDownBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_DOWN;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnLeftBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_LEFT;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnRightBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_RIGHT;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnUpLeftBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_LEFT_TOP;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnUpRightBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_RIGHT_TOP;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnDownLeftBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_LEFT_DOWN;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnDownRightBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_RIGHT_DOWN;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnBtnReleased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_UP;
    ptzControlInfo.param1 = ui.speedCombo->currentText().toInt();
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control  ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnZoomInBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_ZOOM_IN;
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnZoomInBtnReleased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_ZOOM_IN;
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnZoomOutBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_ZOOM_OUT;
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnZoomOutBtnReleased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_ZOOM_OUT;
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnFocusNearBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_FOCUS_NEAR;
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnFocusNearBtnReleased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_FOCUS_NEAR;
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnFocusFarBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_FOCUS_FAR;
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnFocusFarBtnReleased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_FOCUS_FAR;
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnIrisCloseBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_IRIS_CLOSE;
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnIrisCloseBtnRealeased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_IRIS_CLOSE;
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnIrisOpenBtnPressed()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_IRIS_OPEN;
    ptzControlInfo.stop = 0;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}
void PTZPage::OnIrisOpenBtnRealeased()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_TYPE::IRC_NET_PTZ_CMD_IRIS_OPEN;
    ptzControlInfo.stop = 1;
    int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
    QString temp = tr("PTZ control");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnWiperBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_WIPER;
    if (ui.wiperBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Wiper control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Wiper control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnLightBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_LIGHT;
    if (ui.lightBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Light control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Light control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnDefrostBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_DEFROST;
    if (ui.defrostBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Deforst control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Deforst control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnFogLampBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_DEFOG;
    if (ui.fogLampBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Defog control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Defog control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnHearterBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_HEATER;
    if (ui.heaterBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Hearter control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Hearter control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnFanBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_FAN;
    if (ui.fanBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Fan control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ Fan control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnThreeDimensionalBtnClicked()
{
    if (ui.threeDimensionalBtn->isChecked())
    {
        emit Position3D(true);
    }
    else
    {
        emit Position3D(false);
    }
}

void PTZPage::OnRegionFocusBtnClicked()
{
  /*  if (ui.regionFocusBtn->isChecked())
    {
        emit PositionRegionFocus(true);
    }
    else
    {
        emit PositionRegionFocus(false);
    }*/
    emit PositionRegionFocus(true);
}

void PTZPage::OnManualTrackBtnClicked()
{
    if (ui.manualTrackBtn->isChecked())
    {
        emit PositionManualTrack(true);
    }
    else
    {
        emit PositionManualTrack(false);
    }
}



void PTZPage::OnArrowsBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_SYNC_VIEW;
    if (ui.arrowsBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ  arrow control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ arrow control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnAutoMaticBtnClicked()
{
    IRC_NET_PTZ_CONTROL_INFO ptzControlInfo;
    ptzControlInfo.channel = m_mainWindow->getOnlineChannel();
    ptzControlInfo.cmd = IRC_NET_PTZ_CMD_AUTO_FOCUS;
    if (ui.autoMaticBtn->isChecked())
    {
        ptzControlInfo.stop = 0;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ AutoFocus control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    else
    {
        ptzControlInfo.stop = 1;
        int err = IRC_NET_PtzControl(m_handle, &ptzControlInfo);
        QString temp = tr("PTZ AutoFocus control");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
}

void PTZPage::OnPresetGetAllButtonClicked()
{
    m_presetIndexMap.clear();
    if (!GetPreset())
    {
        return;
    }

    ui.presetCombo->clear();
    QString mark;
    for (const auto& preset : m_presetIndexMap)
    {
        mark = preset.enable ? "*" : "";
        ui.presetCombo->addItem(QString::number(preset.id) + mark);
    }

    ui.presetCombo->setCurrentIndex(0);
    //ui.presetNameEdit->setText(QString(presetInfo[0].name));
    //delete[] presetInfo;
}

void PTZPage::OnPresetDeleteAllButtonClicked()
{
    IRC_NET_PTZ_PRESET_INFO presetInfo;
    int err = IRC_NET_PtzPresetControl(m_handle, IRC_NET_PTZ_PRESET_CMD_TYPE::IRC_NET_PTZ_PRESET_CMD_DELETE_ALL, &presetInfo);
    QString temp = tr("PTZ control deleteallPtzPreset");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    ui.presetCombo->clear();
    OnPresetGetAllButtonClicked();
}

void PTZPage::OnSaveButtonClicked()
{
    IRC_NET_PTZ_PRESET_INFO presetInfo;
    presetInfo.enable = true;
    presetInfo.id = m_presetIndexMap[ui.presetCombo->currentIndex()].id;
    strcpy(presetInfo.name, ui.presetNameEdit->text().toStdString().c_str());

    int err = IRC_NET_PtzPresetControl(m_handle, IRC_NET_PTZ_PRESET_CMD_TYPE::IRC_NET_PTZ_PRESET_CMD_UPDATE_NAME, &presetInfo);
    QString temp = tr("PTZ control rename");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    QThread::msleep(200);
    err = IRC_NET_PtzPresetControl(m_handle, IRC_NET_PTZ_PRESET_CMD_TYPE::IRC_NET_PTZ_PRESET_CMD_ADD, &presetInfo);
    temp = tr("PTZ control add");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }

    OnPresetGetAllButtonClicked();
}

void PTZPage::OnMoveToButtonClicked()
{
    IRC_NET_PTZ_PRESET_INFO presetInfo;
    presetInfo.enable = true;
    presetInfo.id = m_presetIndexMap[ui.presetCombo->currentIndex()].id;
    strcpy(presetInfo.name, ui.presetNameEdit->text().toStdString().c_str());
    int err = IRC_NET_PtzPresetControl(m_handle, IRC_NET_PTZ_PRESET_CMD_TYPE::IRC_NET_PTZ_PRESET_CMD_GOTO, &presetInfo);
    QString temp = tr("PTZ control move to");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnGetCurrentPresetBtnClicked()
{
    int currentPresetId = -1;
    int err = IRC_NET_QueryPtzPresetId(m_handle, &currentPresetId);
    QString temp = tr("Query Current Preset");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.presetCombo->setCurrentIndex(currentPresetId-1);
}

void PTZPage::OnDeleteButtonClicked()
{
    IRC_NET_PTZ_PRESET_INFO presetInfo;
    presetInfo.enable = true;
    presetInfo.id = m_presetIndexMap[ui.presetCombo->currentIndex()].id;
    strcpy(presetInfo.name, ui.presetNameEdit->text().toStdString().c_str());
    int err = IRC_NET_PtzPresetControl(m_handle, IRC_NET_PTZ_PRESET_CMD_TYPE::IRC_NET_PTZ_PRESET_CMD_DELETE, &presetInfo);
    QString temp = tr("Delete preset");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    OnPresetGetAllButtonClicked();
}

void PTZPage::OnCruiseGetAllButtonClicked()
{
    m_cruiseIndexMap.clear();
    if (!GetCruise())
    {
        return;
    }

    if (!GetPreset())
    {
        return;
    }

    ui.cruiseCombo->clear();
    QString mark;
    for (const auto& cruise : m_cruiseIndexMap)
    {
        mark = cruise.enable ? "*" : "";
        ui.cruiseCombo->addItem(QString::number(cruise.id) + mark);
    }

    ui.cruiseCombo->setCurrentIndex(0);
}

void PTZPage::OnCruiseDeleteAllButtonClicked()
{
    IRC_NET_PTZ_TOUR_INFO ptzTourInfo;
    int err = IRC_NET_PtzTourControl(m_handle, IRC_NET_PTZ_TOUR_CMD_DELETE_ALL, &ptzTourInfo);
    QString temp = tr("Delete all PtzTour");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    OnCruiseGetAllButtonClicked();
}

void PTZPage::OnCruiseSaveButtonClicked()
{
    IRC_NET_PTZ_TOUR_INFO ptzTourInfo;
    ptzTourInfo.enable = true;
    ptzTourInfo.id = m_cruiseIndexMap[ui.cruiseCombo->currentIndex()].id;
    strcpy(ptzTourInfo.name, ui.cruiseNameEdit->text().toStdString().c_str());
    ptzTourInfo.running = false;
    IRC_NET_PTZ_TOUR_PRESET_INFO presetInfo;
    for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
        presetInfo.residenceTime = ui.tableWidget->item(i, 1)->text().toInt();
        QComboBox* comboBox = qobject_cast<QComboBox*>(ui.tableWidget->cellWidget(i, 0));
        presetInfo.presetInfo.id = comboBox->itemData(comboBox->currentIndex()).toInt();
        strcpy(presetInfo.presetInfo.name, comboBox->currentText().toStdString().c_str());
        ptzTourInfo.presetInfos[i] = presetInfo;
    }

    ptzTourInfo.presetNum = ui.tableWidget->rowCount();
    int err = IRC_NET_PtzTourControl(m_handle, IRC_NET_PTZ_TOUR_CMD_SAVE, &ptzTourInfo);
    QString temp = tr("PtzTour rename");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    OnCruiseGetAllButtonClicked();
}

void PTZPage::OnCruiseDeleteButtonClicked()
{
    IRC_NET_PTZ_TOUR_INFO ptzTourInfo;
    ptzTourInfo.id = m_cruiseIndexMap[ui.cruiseCombo->currentIndex()].id;
    int err = IRC_NET_PtzTourControl(m_handle, IRC_NET_PTZ_TOUR_CMD_DELETE, &ptzTourInfo);
    QString temp = tr("PtzTour delete");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    OnCruiseGetAllButtonClicked();
}

void PTZPage::OnCruiseStartButtonClicked()
{
    IRC_NET_PTZ_TOUR_INFO ptzTourInfo;
    ptzTourInfo.id = m_cruiseIndexMap[ui.cruiseCombo->currentIndex()].id;
    if (ui.cruiseStartBtn->text() == "Start") {
        int err = IRC_NET_PtzTourControl(m_handle, IRC_NET_PTZ_TOUR_CMD_START, &ptzTourInfo);
        QString temp = tr("PtzTour start");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.cruiseStartBtn->setText("Stop");
    }
    else if (ui.cruiseStartBtn->text() == "Stop") {
        int err = IRC_NET_PtzTourControl(m_handle, IRC_NET_PTZ_TOUR_CMD_STOP, &ptzTourInfo);
        QString temp = tr("PtzTour stop");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.cruiseStartBtn->setText("Start");
    }
}

void PTZPage::OnAddPresetButtonClicked()
{
    if (0 == m_presetIndexMap.size())
    {
        QMessageBox::critical(this, tr("Error"), tr("No preset."));
        return;
    }

    int count = ui.tableWidget->rowCount();
    ui.tableWidget->insertRow(count);
    ui.tableWidget->setItem(count, 0, new QTableWidgetItem());
    QComboBox* comboBox = new QComboBox();
    int index = 0;
    for (const auto& preset : m_presetIndexMap)
    {
        if (preset.enable)
        {
            comboBox->addItem(preset.name);
            comboBox->setItemData(index, preset.id);
            index++;
        }
    }
    ui.tableWidget->setCellWidget(count, 0, comboBox);
    ui.tableWidget->setItem(count, 1, new QTableWidgetItem(QString::number(10)));
}

void PTZPage::OnDeletePresetButtonClicked()
{
    QList<QTableWidgetItem*> selections = ui.tableWidget->selectedItems();
    int colCount = ui.tableWidget->columnCount();
    for (int i = 0; i < selections.size(); i += colCount)
    {
        QTableWidgetItem* item = selections[i];
        int row = item->row();
        ui.tableWidget->removeRow(row);
    }
}

void PTZPage::OnPatrolGetAllButtonClicked()
{
    int outSize = 0;
    int inSize = 0;
    int err = IRC_NET_QueryPtzPatternSize(m_handle, &inSize);
    QString temp = tr("QueryPtzPatternSize");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    IRC_NET_PTZ_PATTERN_INFO* patternInfo = new IRC_NET_PTZ_PATTERN_INFO[inSize];
    
    err = IRC_NET_QueryPtzPattern(m_handle, patternInfo, inSize, &outSize);
    temp = tr("QueryPtzPattern");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    ui.patrolCombo->clear();
    QString mark;
    for (int i = 0; i < outSize; i++)
    {
        mark = patternInfo[i].enable ? "*" : "";
        ui.patrolCombo->addItem(QString::number(patternInfo[i].id) + mark);
    }
    ui.patrolCombo->setCurrentIndex(0);
    delete[] patternInfo;
}

void PTZPage::OnPatrolDeleteAllButtonClicked()
{
    int err = IRC_NET_PtzPatternControl(m_handle, IRC_NET_PTZ_PATTERN_CMD_DELETE_ALL, 0);
    QString temp = tr("PtzPattern delete all");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    OnPatrolGetAllButtonClicked();
}

void PTZPage::OnStartPatrolButtonClicked()
{
    int patternId = ui.patrolCombo->currentIndex() + 1;
    if (ui.startPatrolBtn->text() == "StartPatrol")
    {
        int err = IRC_NET_PtzPatternControl(m_handle, IRC_NET_PTZ_PATTERN_CMD_START_REPLAY, patternId);
        QString temp = tr("PtzPattern start patrol");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.startPatrolBtn->setText("StopPatrol");
    }
    else if (ui.startPatrolBtn->text() == "StopPatrol")
    {
        int err = IRC_NET_PtzPatternControl(m_handle, IRC_NET_PTZ_PATTERN_CMD_STOP_REPLAY, patternId);
        QString temp = tr("PtzPattern stop patrol");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.startPatrolBtn->setText("StartPatrol");
    }
}

void PTZPage::OnStartRecordButtonClicked()
{
    int patternId = ui.patrolCombo->currentIndex() + 1;
    if (ui.startRecordBtn->text() == "StartRecord")
    {
        int err = IRC_NET_PtzPatternControl(m_handle, IRC_NET_PTZ_PATTERN_CMD_START_RECOED, patternId);
        QString temp = tr("PtzPattern start record");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.startRecordBtn->setText("StopRecord");
    }
    else if (ui.startRecordBtn->text() == "StopRecord")
    {
        int err = IRC_NET_PtzPatternControl(m_handle, IRC_NET_PTZ_PATTERN_CMD_STOP_RECOED, patternId);
        QString temp = tr("PtzPattern stop record");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        OnPatrolGetAllButtonClicked();
        ui.startRecordBtn->setText("StartRecord");
    }
}

void PTZPage::OnSetButtonClicked()
{
    IRC_NET_PTZ_POSITION_PARAM positionConfig;
    positionConfig.pan = ui.pSpinBox->value();
    positionConfig.tilt = ui.tSpinBox->value();
    positionConfig.zoom = ui.zSpinBox->value();
    int err = IRC_NET_PtzPosition(m_handle, &positionConfig);
    QString temp = tr("SetPotion");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnPresetComboChanged(int index)
{
    auto it = m_presetIndexMap.find(index);
    if (it != m_presetIndexMap.end())
    {
        ui.presetNameEdit->setText(QString(it->name));
    }
}

void PTZPage::OnCruiseComboChanged(int index)
{
    auto it = m_cruiseIndexMap.find(index);
    if (it != m_cruiseIndexMap.end())
    {
        ui.cruiseNameEdit->setText(QString(it->name));
        RefreshCruiseTable(index);
    }
}

void PTZPage::OnPatrolComboChanged(int index)
{
    QString id = ui.patrolCombo->currentText();
    if (id.contains("*"))
    {
        ui.startPatrolBtn->setEnabled(true);
    }
    else
    {
        ui.startPatrolBtn->setEnabled(false);
    }
}

void PTZPage::OnResetBtnClicked()
{
    int err = IRC_NET_ResetPtzConfig(m_handle);
    QString temp = tr("ResetPTZ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnLensInitBtnClicked()
{
    int err = IRC_NET_PtzLensInit(m_handle, m_mainWindow->getOnlineChannel());
    QString temp = tr("LensInit");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnLaserDistanceBtnClicked()
{
    int distance = 0;
    int err = IRC_NET_GetLaserDistance(m_handle,&distance);
    QString temp = tr("LaserDistance");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    temp = tr("Laser Distance is ") + QString::number(distance);
    QMessageBox::information(nullptr, "", temp);
}

void PTZPage::OnGetAllAreaScanInfoBtnClicked()
{
    m_regionScanInfoMap.clear();
    ui.startPresetCombo->clear();
    ui.stopPresetCombo->clear();
    ui.areaScanCombo->clear();
    int size = 0;
    int err = IRC_NET_QueryPtzRegionScanInfoSize(m_handle, &size);
    int outSize = 0;
    IRC_NET_REGION_SCAN_INFO* regionScanInfos = new IRC_NET_REGION_SCAN_INFO[size];
    err = IRC_NET_QueryPtzRegionScanInfo(m_handle, regionScanInfos, size, &outSize);
    for (int i = 0; i < outSize; i++)
    {
        ui.areaScanCombo->addItem(QString::number(regionScanInfos[i].id));
        m_regionScanInfoMap.insert(regionScanInfos[i].id, regionScanInfos[i]);
    }
    m_presetIndexMap.clear();
    if (!GetPreset())
    {
        return;
    }
    for (const auto& preset : m_presetIndexMap)
    {
        if (preset.enable)
        {
            ui.startPresetCombo->addItem(QString::number(preset.id));
            ui.stopPresetCombo->addItem(QString::number(preset.id));
        }
    }

}

void PTZPage::OnGetOneAreaScanBtnClicked()
{
    int id = ui.areaScanCombo->currentText().toInt();
    for (const auto& regionScanInfo : m_regionScanInfoMap)
    {
        if (id == regionScanInfo.id)
        {
            if (regionScanInfo.derection == 1)
            {
                ui.clockwiseBtn->setChecked(true);
                ui.counterClockWiseBtn->setChecked(false);
            }
            else
            {
                ui.clockwiseBtn->setChecked(false);
                ui.counterClockWiseBtn->setChecked(true);
            }
            ui.areaScanSpeedEdit->setText(QString::number(regionScanInfo.speed));
            ui.areaScanGradientEdit->setText(QString::number(regionScanInfo.tiltStepAngle));
            if (regionScanInfo.startPresetId == 0)
            {
                ui.startPresetCombo->setCurrentIndex(-1);
            }
            if (regionScanInfo.stopPresetId == 0)
            {
                ui.stopPresetCombo->setCurrentIndex(-1);
            }
            for (int i = 0; i < ui.startPresetCombo->count(); i++)
            {
                if (ui.startPresetCombo->itemText(i) == QString::number(regionScanInfo.startPresetId))
                {
                    ui.startPresetCombo->setCurrentIndex(i);
                }
            }
            for (int i = 0; i < ui.stopPresetCombo->count(); i++)
            {
                if (ui.stopPresetCombo->itemText(i) == QString::number(regionScanInfo.stopPresetId))
                {
                    ui.stopPresetCombo->setCurrentIndex(i);
                }
            }
        }
        
    }
}

void PTZPage::OnSetAreaScanInfoBtnClicked()
{
    IRC_NET_REGION_SCAN_INFO regionScanInfo;
    regionScanInfo.id = ui.areaScanCombo->currentText().toInt();
    regionScanInfo.derection = ui.clockwiseBtn->isChecked() ? 1 : -1;
    regionScanInfo.enable = true;
    regionScanInfo.speed = ui.areaScanSpeedEdit->text().toInt();
    regionScanInfo.tiltStepAngle = ui.areaScanGradientEdit->text().toFloat();
    regionScanInfo.startPresetId = ui.startPresetCombo->currentText().toInt();
    regionScanInfo.stopPresetId = ui.stopPresetCombo->currentText().toInt();
    int err = IRC_NET_SetPtzRegionScanInfo(m_handle, &regionScanInfo);
    QString temp = tr("SetPtzRegionScanInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnAreaScanStartBtnClicked()
{
    int id = ui.areaScanCombo->currentText().toInt();
    int err = IRC_NET_PtzRegionScanControl(m_handle, id, true);
    QString temp = tr("PtzRegionScanControl");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnAreaScanStopBtnClicked()
{
    int id = ui.areaScanCombo->currentText().toInt();
    int err = IRC_NET_PtzRegionScanControl(m_handle, id, false);
    QString temp = tr("PtzRegionScanControl");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnDeleteOneAreaScanBtnClicked()
{
    int id = ui.areaScanCombo->currentText().toInt();
    int err = IRC_NET_DeletePtzRegionScanInfo(m_handle, id);
    QString temp = tr("DeletePtzRegionScanInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    RefreshRegionScan();
    OnGetAllAreaScanInfoBtnClicked();
}

void PTZPage::OnDeleteAllAreaScanBtnClicked()
{
    int itemCount = ui.areaScanCombo->count();
    if (itemCount <= 0)
    {
        return;
    }
    for (int i = 0; i < itemCount; i++)
    {
        int id = ui.areaScanCombo->itemText(i).toInt();
        int err = IRC_NET_DeletePtzRegionScanInfo(m_handle, id);
        QString temp = tr("DeletePtzRegionScanInfo");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
        }
    }
    RefreshRegionScan();
    OnGetAllAreaScanInfoBtnClicked();
}

void PTZPage::OnBootRefreshBtnClicked()
{
    IRC_NET_BOOT_ACTION_INFO bootActionInfo;
    int err = IRC_NET_QueryPtzBootActionInfo(m_handle, &bootActionInfo);
    QString temp = tr("QueryPtzBootActionInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    RefreshBootAction(bootActionInfo);
}

void PTZPage::OnBootConfirmBtnClicked()
{
    IRC_NET_BOOT_ACTION_INFO bootActionInfo;
    memset(&bootActionInfo, 0, sizeof(bootActionInfo));
    if ("" == ui.BootComboBox->currentText() && ! ui.bootAutoResetRadioBtn->isChecked())
    {
        QString temp = tr("Number is empty!");
        MSG_BOX.ShowErrorMessage(temp, IRC_NET_ERROR_PARAM_WRONG);
        return;
    }
    if (ui.bootPresetRadioBtn->isChecked())
    {
        bootActionInfo.actionType = 1;
        bootActionInfo.presetId = ui.BootComboBox->currentText().toInt();
    }
    else if(ui.bootTourRadioBtn->isChecked())
    {
        bootActionInfo.actionType = 2;
        bootActionInfo.tourId = ui.BootComboBox->currentText().toInt();
    }
    else if (ui.bootPatternRadioBtn->isChecked())
    {
        bootActionInfo.actionType = 3;
        bootActionInfo.patternId = ui.BootComboBox->currentText().toInt();
    }
    else if (ui.bootAreaScanRadioBtn->isChecked())
    {
        bootActionInfo.actionType = 5;
        bootActionInfo.regionScanId = ui.BootComboBox->currentText().toInt();
    }
    else if (ui.bootLineScanRadioBtn->isChecked())
    {
        bootActionInfo.actionType = 4;
        bootActionInfo.lineScanId = ui.BootComboBox->currentText().toInt();
    }
    else if (ui.bootAutoResetRadioBtn->isChecked())
    {
        bootActionInfo.actionType = 0;
        bootActionInfo.presetId = 1;
    }
    bootActionInfo.enable = ui.bootEnableCheckBox->isChecked();
    int err = IRC_NET_SetPtzBootActionInfo(m_handle, &bootActionInfo);
    QString temp = tr("SetPtzBootActionInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnBootPresetRadioBtnClicked()
{
    //查询阈值点id
    m_presetIndexMap.clear();
    ui.BootComboBox->clear();
    ui.parkComboBox->clear();
    if (!GetPreset())
    {
        return;
    }
    for (const auto& preset : m_presetIndexMap)
    {
        if (preset.enable)
        {
            ui.BootComboBox->addItem(QString::number(preset.id));
            ui.parkComboBox->addItem(QString::number(preset.id));
        }
    }
}

void PTZPage::OnBootTourRadioBtnClicked()
{
    m_cruiseIndexMap.clear();
    ui.BootComboBox->clear();
    ui.parkComboBox->clear();
    if (!GetCruise())
    {
        return;
    }
    for (const auto& cruise : m_cruiseIndexMap)
    {
        if (cruise.enable)
        {
            ui.BootComboBox->addItem(QString::number(cruise.id));
            ui.parkComboBox->addItem(QString::number(cruise.id));
        }      
    }
}

void PTZPage::OnBootPatternRadioBtnClicked()
{
    ui.BootComboBox->clear();
    ui.parkComboBox->clear();
    int outSize = 0;
    int inSize = 0;
    int err = IRC_NET_QueryPtzPatternSize(m_handle, &inSize);
    QString temp = tr("QueryPtzPatternSize");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    IRC_NET_PTZ_PATTERN_INFO* patternInfo = new IRC_NET_PTZ_PATTERN_INFO[inSize];

    err = IRC_NET_QueryPtzPattern(m_handle, patternInfo, inSize, &outSize);
    temp = tr("QueryPtzPattern");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    for (int i = 0; i < outSize; i++)
    {
        if (patternInfo[i].enable)
        {
            ui.BootComboBox->addItem(QString::number(patternInfo[i].id));
            ui.parkComboBox->addItem(QString::number(patternInfo[i].id));
        }
    }
    for (int j = 0; j < outSize; j++)
    {
        m_patternInfoMap[j] = patternInfo[j];
    }
    delete[] patternInfo;
    patternInfo = nullptr;
}

void PTZPage::OnBootAreaScanRadioBtnClicked()
{
    m_regionScanInfoMap.clear();
    ui.BootComboBox->clear();
    ui.parkComboBox->clear();
    int size = 0;
    int err = IRC_NET_QueryPtzRegionScanInfoSize(m_handle, &size);
    int outSize = 0;
    IRC_NET_REGION_SCAN_INFO* regionScanInfos = new IRC_NET_REGION_SCAN_INFO[size];
    err = IRC_NET_QueryPtzRegionScanInfo(m_handle, regionScanInfos, size, &outSize);
    if (outSize > 16)
    {
        outSize = 16;
    }
    for (int i = 0; i < outSize; i++)
    {
        if (regionScanInfos[i].enable)
        {
            ui.BootComboBox->addItem(QString::number(regionScanInfos[i].id));
            ui.parkComboBox->addItem(QString::number(regionScanInfos[i].id));
        }
    }
}

void PTZPage::OnBootLineScanRadioBtnClicked()
{
    ui.BootComboBox->clear();
    ui.parkComboBox->clear();
    for (int i = 1; i <= 5; i++)
    {
        ui.BootComboBox->addItem(QString::number(i));
        ui.parkComboBox->addItem(QString::number(i));
    }
}

void PTZPage::OnBootAutoResetRadioBtnClicked()
{
    ui.BootComboBox->clear();
}

void PTZPage::OnParkRefreshBtnClicked()
{
    IRC_NET_PARK_ACTION_INFO parkActionInfo;
    memset(&parkActionInfo, 0, sizeof(parkActionInfo));
    int err = IRC_NET_QueryPtzParkActionInfo(m_handle, &parkActionInfo);
    QString temp = tr("QueryPtzParkActionInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
    RefreshParkAction(&parkActionInfo);
}

void PTZPage::OnParkConfirmBtnClicked()
{
    IRC_NET_PARK_ACTION_INFO parkActionInfo;
    memset(&parkActionInfo, 0, sizeof(parkActionInfo));
    if ("" == ui.parkComboBox->currentText())
    {
        QString temp = tr("Number is empty!");
        MSG_BOX.ShowErrorMessage(temp, IRC_NET_ERROR_PARAM_WRONG);
        return;
    }
    if (ui.parkPresetRadioBtn->isChecked())
    {
        parkActionInfo.actionType = 1;
        parkActionInfo.presetId = ui.parkComboBox->currentText().toInt();
    }
    else if (ui.parkTourRadioBtn->isChecked())
    {
        parkActionInfo.actionType = 2;
        parkActionInfo.tourId = ui.parkComboBox->currentText().toInt();
    }
    else if (ui.parkPatternRadioBtn->isChecked())
    {
        parkActionInfo.actionType = 3;
        parkActionInfo.patternId = ui.parkComboBox->currentText().toInt();
    }
    else if (ui.parkAreaScanRadioBtn->isChecked())
    {
        parkActionInfo.actionType = 5;
        parkActionInfo.regionScanId = ui.parkComboBox->currentText().toInt();
    }
    else if (ui.parkLineScanRadioBtn->isChecked())
    {
        parkActionInfo.actionType = 4;
        parkActionInfo.lineScanId = ui.parkComboBox->currentText().toInt();
    }
    parkActionInfo.second = ui.parkTimeEdit->text().toInt();
    parkActionInfo.enable = ui.parkEnableCheckBox->isChecked();
    int err = IRC_NET_SetPtzParkActionInfo(m_handle, &parkActionInfo);
    QString temp = tr("SetPtzParkActionInfo");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnGetCoordinatesBtnClicked()
{
    /*IRC_NET_PTZ_POSITION_PARAM ptzPosition;
    memset(&ptzPosition, 0, sizeof(ptzPosition));
    int err = IRC_NET_GetCurrentPtz(m_handle, &ptzPosition);
    QString temp = tr("GetCurrentPtz");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.panEdit->setText(QString::number(ptzPosition.pan,'f',2));
    ui.tiltEdit->setText(QString::number(ptzPosition.tilt,'f',2));
    ui.zoomEdit->setText(QString::number(ptzPosition.zoom,'f',2));*/
    IRC_NET_PTZ_POSITION_PARAM_V1 ptzPosition;
    memset(&ptzPosition, 0, sizeof(ptzPosition));
    int err = IRC_NET_GetCurrentPtz_V1(m_handle, &ptzPosition);
    QString temp = tr("GetCurrentPtz");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.panEdit->setText(QString::number(ptzPosition.pan, 'f', 2));
    ui.tiltEdit->setText(QString::number(ptzPosition.tilt, 'f', 2));
    ui.visZoomEdit->setText(QString::number(ptzPosition.visZoom, 'f', 2));
    ui.irZoomEdit->setText(QString::number(ptzPosition.irZoom, 'f', 2));
    ui.visFovEdit->setText("H "+QString::number(ptzPosition.visFovH, 'f', 2) +" V "+ QString::number(ptzPosition.visFovV,'f',2));
    ui.irFovEdit->setText("H " + QString::number(ptzPosition.irFovH, 'f', 2) + " V " + QString::number(ptzPosition.irFovV, 'f', 2));

}

void PTZPage::OnSetATBtnClicked()
{
    IRC_NET_PTZ_TRACKING_POSITION_PARAM positionConfig;
    positionConfig.panSpeed = ui.psSpinBox->value();
    positionConfig.tiltSpeed = ui.tsSpinBox->value();
    positionConfig.derection = ui.derectionSpinBox->value();
    int err = IRC_NET_PtzTrackingPosition(m_handle, &positionConfig);
    QString temp = tr("SetTrackingPosition");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }
}

void PTZPage::OnGetZoomBtnClicked()
{
    float multipler = 0;
    int err = IRC_NET_GetPtzZoomMultiplier(m_handle, m_mainWindow->getOnlineChannel(), &multipler);
    QString temp = tr("GetPtzZoomMultiplier");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.zoomMultEdit->setText(QString::number(multipler));
}

void PTZPage::OnSetZoomBtnClicked()
{
    float multipler = ui.zoomMultEdit->text().toFloat();
    int err = IRC_NET_SetPtzZoomMultiplier(m_handle, m_mainWindow->getOnlineChannel(), multipler);
    QString temp = tr("SetPtzZoomMultiplier");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PTZPage::OnTargetConfigGetBtnClicked()
{
    IRC_NET_TARGET_TRACK_DATA targetTrackData;
    memset(&targetTrackData, 0, sizeof(IRC_NET_TARGET_TRACK_DATA));
    int err = IRC_NET_GetTargetTrackConfig(m_handle, m_mainWindow->getOnlineChannel(), &targetTrackData);
    QString temp = tr("GetTargetTrackConfig");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.trackCheckBoxEnable->setChecked(targetTrackData.enable);
    ui.snapCheckBoxEnable->setChecked(targetTrackData.snapshotLinkageInfo.enable);
    ui.snapChn1Box->setChecked(targetTrackData.snapshotLinkageInfo.channel[0]);
    ui.snapChn2Box->setChecked(targetTrackData.snapshotLinkageInfo.channel[1]);
    ui.recordCheckBoxEnable->setChecked(targetTrackData.recordLinkageInfo.enable);
    ui.recordChn1Box->setChecked(targetTrackData.recordLinkageInfo.channel[0]);
    ui.recordChn2Box->setChecked(targetTrackData.recordLinkageInfo.channel[1]);
}

void PTZPage::OnTargetConfigSetBtnClicked()
{
    IRC_NET_TARGET_TRACK_DATA targetTrackData;
    memset(&targetTrackData, 0, sizeof(IRC_NET_TARGET_TRACK_DATA));
    targetTrackData.enable = ui.trackCheckBoxEnable->isChecked();
    targetTrackData.snapshotLinkageInfo.enable = ui.snapCheckBoxEnable->isChecked();
    targetTrackData.snapshotLinkageInfo.channel[0] = ui.snapChn1Box->isChecked();
    targetTrackData.snapshotLinkageInfo.channel[1] = ui.snapChn2Box->isChecked();
    targetTrackData.recordLinkageInfo.enable = ui.recordCheckBoxEnable->isChecked();
    targetTrackData.recordLinkageInfo.channel[0] = ui.recordChn1Box->isChecked();
    targetTrackData.recordLinkageInfo.channel[1] = ui.recordChn2Box->isChecked();
    int err = IRC_NET_SetTargetTrackConfig(m_handle, m_mainWindow->getOnlineChannel(), &targetTrackData);
    QString temp = tr("SetTargetTrackConfig");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void PTZPage::OnGetBearingBtnClicked()
{
    IRC_NET_ACCELERATION_DATA accelerationData;
    int err = IRC_NET_GetAccelerationData(m_handle, &accelerationData);
    QString temp = tr("GetAccelerationData");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.angleXEdit->setText(QString::number(accelerationData.angleData.coordinateData.x));
    ui.angleYEdit->setText(QString::number(accelerationData.angleData.coordinateData.y));
    ui.angleZEdit->setText(QString::number(accelerationData.angleData.coordinateData.z));
    ui.anglePEdit->setText(QString::number(accelerationData.angleData.pan));
    ui.angleTEdit->setText(QString::number(accelerationData.angleData.tilt));
    ui.angleRTEdit->setText(QString::number(accelerationData.angleData.rt));
    ui.accelerationXEdit->setText(QString::number(accelerationData.accelerationData.x));
    ui.accelerationYEdit->setText(QString::number(accelerationData.accelerationData.y));
    ui.accelerationZEdit->setText(QString::number(accelerationData.accelerationData.z));
    ui.rotationXEdit->setText(QString::number(accelerationData.rotationData.x));
    ui.rotationYEdit->setText(QString::number(accelerationData.rotationData.y));
    ui.rotationZEdit->setText(QString::number(accelerationData.rotationData.z));
}




