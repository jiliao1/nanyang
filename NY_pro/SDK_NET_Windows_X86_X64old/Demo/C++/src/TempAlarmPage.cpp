#include "TempAlarmPage.h"
#include "MainWindow.h"
#include "IRCNetSDK.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <qdatetime.h>
#include <qdebug.h>

TempAlarmPage::TempAlarmPage(QWidget* parent /*= nullptr*/)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_mainWindow = (MainWindow*)parent;
    m_alarmPage = new AlarmPage();
    m_alarmPage->hide();
    qRegisterMetaType<IRC_NET_TEMP_ALARM_INFO>("IRC_NET_TEMP_ALARM_INFO");
    qRegisterMetaType<IRC_NET_FIRE_ALARM_INFO>("IRC_NET_FIRE_ALARM_INFO");
    qRegisterMetaType<IRC_NET_REGION_INTRUSION_ALARM_INFO>("IRC_NET_REGION_INTRUSION_ALARM_INFO");
    qRegisterMetaType<IRC_NET_LINE_INTRUSION_ALARM_INFO>("IRC_NET_LINE_INTRUSION_ALARM_INFO");
    qRegisterMetaType<IRC_NET_TEMP_DIFF_ALARM_INFO>("IRC_NET_TEMP_DIFF_ALARM_INFO");
    qRegisterMetaType<IRC_NET_SMOKE_DETECT_ALARM_INFO>("IRC_NET_SMOKE_DETECT_ALARM_INFO");
    qRegisterMetaType<IRC_NET_LOCAL_ALARM_INFO>("IRC_NET_LOCAL_ALARM_INFO");
    

    InitForm();
    ConnectSignalSlot();
}

TempAlarmPage::~TempAlarmPage()
{}

void TempAlarmPage::ClearUi()
{
    ui.frameMaxTempEdit->setText("");
    ui.frameMaxLineEdit_X->setText("");
    ui.frameMaxLineEdit_Y->setText("");
    ui.frameMinTempEdit->setText("");
    ui.frameMinLineEdit_X->setText("");
    ui.frameMinLineEdit_Y->setText("");
    ui.frameAvgTempEdit->setText("");
    ui.frameCenterTempEdit->setText("");
    ui.openTempStreamBtn->setText("Open");
    ui.saveOneFrameBtn->setEnabled(false);
    ui.frameSnapEnableCheckBox->setChecked(false);
    ui.frameSnapChannel1CheckBox->setChecked(false);
    ui.frameSnapChannel2CheckBox->setChecked(false);
    ui.frameSnapDeplayEdit->setText("");
    ui.frameRecordEnableCheckBox->setChecked(false);
    ui.frameRecordChannel1CheckBox->setChecked(false);
    ui.frameRecordChannel2CheckBox->setChecked(false);
    ui.frameRecordDeplayEdit->setText("");
    ui.greaterCheckBox->setChecked(false);
    ui.greaterThresholdTempEdit->setText("");
    ui.lessCheckBox->setChecked(false);
    ui.lessThresholdTempEdit->setText("");
    ui.debounceEdit->setText("");
    ui.toleranceTempEdit->setText("");
    ui.presetCombo->clear();
    m_presetIndexMap.clear();
    ui.presetCombo->addItem("Frame");
    ui.shieldedAreaCombo->clear();
    m_shieldedMap.clear();
    ui.ruleCombo->clear();
    ui.ruleMaxTempEdit->setText("");
    ui.ruleMaxLineEdit_X->setText("");
    ui.ruleMaxLineEdit_Y->setText("");
    ui.ruleMinTempEdit->setText("");
    ui.ruleMinLineEdit_X->setText("");
    ui.ruleMinLineEdit_Y->setText("");
    ui.ruleAvgTempEdit->setText("");
    ui.ruleCenterTempEdit->setText("");
    ui.envEnableCheck->setChecked(false);
    ui.emissivityEdit->setText("");
    ui.atmosTransEdit->setText("");
    ui.refTempEdit->setText("");
    ui.atmosTempEdit->setText("");
    ui.distanceEdit->setText("");
    ui.alarmRuleCombo->setCurrentIndex(0);
    ui.thresholdTempEdit->setText("");
    ui.tempDebounceEdit->setText("");
    ui.tempToleranceTempEdit->setText("");
    ui.ROISnapEnableCheckBox->setChecked(false);
    ui.ROISnapChannel1CheckBox->setChecked(false);
    ui.ROISnapChannel2CheckBox->setChecked(false);
    ui.ROISnapDeplayEdit->setText("");
    ui.ROIRecordEnableCheckBox->setChecked(false);
    ui.ROIRecordChannel1CheckBox->setChecked(false);
    ui.ROIRecordChannel2CheckBox->setChecked(false);
    ui.ROIRecordDeplayEdit->setText("");
    ui.openAlarmCallbackBtn->setText("Open");
    ui.openAtrAlarmCallbackBtn->setText("Open");
    ui.atrFrameMaxTempEdit->setText("");
    ui.atrFrameMinTempEdit->setText("");
    ui.atrFrameAvgTempEdit->setText("");
    ui.atrFrameCenterTempEdit->setText("");
    ui.lowTempEdit->setText("");
    ui.highTempEdit->setText("");
    ui.lowLevelOneTempEdit->setText("");
    ui.lowLevelTwoTempEdit->setText("");
    ui.lowLevelThreeTempEdit->setText("");
    ui.highLevelOneTempEdit->setText("");
    ui.highLevelTwoTempEdit->setText("");
    ui.highLevelThreeTempEdit->setText("");
    ui.atrDelayEdit->setText("");
    ui.atrRuleCombo->clear();
    ui.atrTypeCombo->clear();
    ui.atrSwitchCombo->setCurrentIndex(-1);
    ui.lowEnableCheck->setChecked(false);
    ui.highEnableCheck->setChecked(false);
    ui.pointOnceMeasureBtn->setChecked(false);
    m_alarmPage->ClearAlarm();
}

void TempAlarmPage::InitButton()
{
    ui.getAllPresetBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.moveToBtn->setEnabled(m_mainWindow->m_devAbility.ptz);
    ui.pointOnceMeasureBtn->setChecked(false);
}

void TempAlarmPage::SetOnceTempEdit(float temp)
{
    ui.pointOnceEdit->setText(QString::number(temp, 'f', 2));
    ui.pointOnceMeasureBtn->setChecked(false);
}

void TempAlarmPage::SetAlarmCallParamHide()
{
    ui.groupBox->setEnabled(false);
    ui.label_3->setEnabled(false);
    ui.widget_3->setEnabled(false);
    ui.widget_6->setEnabled(false);
    ui.widget_7->setEnabled(false);
    ui.tab_2->setEnabled(false);
}

void TempAlarmPage::SetAlarmCallParamShow()
{
    ui.groupBox->setEnabled(true);
    ui.label_3->setEnabled(true);
    ui.widget_3->setEnabled(true);
    ui.widget_6->setEnabled(true);
    ui.widget_7->setEnabled(true);
    ui.tab_2->setEnabled(true);
}

void TempAlarmPage::InitForm()
{
    ui.saveOneFrameBtn->setEnabled(false);
    ui.saveAtrOneFrameBtn->setEnabled(false);
    ui.tabWidget->setCurrentIndex(0);
    ClearUi();
}

void TempAlarmPage::ConnectSignalSlot()
{
    connect(m_mainWindow, SIGNAL(ChangeHandle(IRC_NET_HANDLE)), this, SLOT(OnChangeHandle(IRC_NET_HANDLE)));
    connect(ui.getFrameTempDataBtn, SIGNAL(clicked()), this, SLOT(OnGetFrameTempDataBtnClicked()));
    connect(ui.openTempStreamBtn, SIGNAL(clicked()), this, SLOT(OnOpenTempStreamBtnClicked()));
    connect(ui.saveOneFrameBtn, SIGNAL(clicked()), this, SLOT(OnSaveOneFrameBtnClicked()));
    connect(ui.getAllRulesBtn, SIGNAL(clicked()), this, SLOT(OnGetAllRulesBtnClicked()));
    connect(ui.deleteAllBtn, SIGNAL(clicked()), this, SLOT(OnDeleteAllRulesBtnClicked()));
    connect(ui.getOneRuleInfoBtn, SIGNAL(clicked()), this, SLOT(OnGetOneRuleInfoBtnClicked()));
    connect(ui.deleteOneRuleInfoBtn, SIGNAL(clicked()), this, SLOT(OnDeleteOneRuleInfoBtnClicked()));
    connect(ui.alarmSetBtn, SIGNAL(clicked()), this, SLOT(OnSetAlarmInfoBtnClicked()));
    connect(ui.openAlarmCallbackBtn, SIGNAL(clicked()), this, SLOT(OnOpenAlarmCallbackBtnClicked()));
    connect(ui.getAlarmDataBtn, SIGNAL(clicked()), this, SLOT(OnGetAlarmDataBtnClicked()));
    connect(this, SIGNAL(GetTempAlarm(int, IRC_NET_TEMP_ALARM_INFO)), this, SLOT(OnGetTempAlarm(int, IRC_NET_TEMP_ALARM_INFO)));
    connect(this, SIGNAL(GetFireAlarm(int, IRC_NET_FIRE_ALARM_INFO)), this, SLOT(OnGetFireAlarm(int, IRC_NET_FIRE_ALARM_INFO)));
    connect(this, SIGNAL(GetRegionIntrusionAlarm(int, IRC_NET_REGION_INTRUSION_ALARM_INFO)), this, SLOT(OnGetRegionIntrusionAlarm(int, IRC_NET_REGION_INTRUSION_ALARM_INFO)));
    connect(this, SIGNAL(GetLineIntrusionAlarm(int, IRC_NET_LINE_INTRUSION_ALARM_INFO)), this, SLOT(OnGetLineIntrusionAlarm(int, IRC_NET_LINE_INTRUSION_ALARM_INFO)));
    connect(this, SIGNAL(GetTempDiffAlarm(int, IRC_NET_TEMP_DIFF_ALARM_INFO )), this, SLOT(OnGetTempDiffAlarm(int, IRC_NET_TEMP_DIFF_ALARM_INFO)));
    connect(this, SIGNAL(GetSmokeDetectAlarm(int, IRC_NET_SMOKE_DETECT_ALARM_INFO )), this, SLOT(OnGetSmokeDetectAlarm(int, IRC_NET_SMOKE_DETECT_ALARM_INFO)));
    connect(this, SIGNAL(GetLocalAlarm(int, IRC_NET_LOCAL_ALARM_INFO)), this, SLOT(OnGetLocalAlarm(int, IRC_NET_LOCAL_ALARM_INFO)));
    

    connect(ui.pointOnceMeasureBtn, SIGNAL(clicked()), this, SLOT(OnpointOnceMeasureBtnClicked()));
    connect(ui.getAllPresetBtn, SIGNAL(clicked()), this, SLOT(OnGetAllPreSetBtnClicked()));
    connect(ui.moveToBtn, SIGNAL(clicked()), this, SLOT(OnMoveToBtnClicked()));
    connect(ui.shieldedGetAllBtn, SIGNAL(clicked()), this, SLOT(OnShieldedGetAllBtnClicked()));
    connect(ui.shieldedDeleteAllBtn, SIGNAL(clicked()), this, SLOT(OnShieldedDeleteAllBtnClicked()));
    connect(ui.shieldedDeleteBtn, SIGNAL(clicked()), this, SLOT(OnShieldedDeleteBtnClicked()));
    connect(ui.ROISetBtn, SIGNAL(clicked()), this, SLOT(OnROISetBtnClicked()));
    connect(m_mainWindow, SIGNAL(Shielded(int, QList<QPoint>)), this, SLOT(OnShielded(int, QList<QPoint>)));
    connect(ui.getAtrAllRulesBtn, SIGNAL(clicked()), this, SLOT(OnGetAtrAllRulesBtnClicked()));
    connect(ui.deleteAtrAllBtn, SIGNAL(clicked()), this, SLOT(OnDeleteAtrAllBtnClicked()));
    connect(ui.getAtrOneRuleInfoBtn, SIGNAL(clicked()), this, SLOT(OngetAtrOneRuleInfoBtnClicked()));
    connect(ui.deleteAtrOneRuleInfoBtn, SIGNAL(clicked()), this, SLOT(OnDeleteAtrOneRuleInfoBtnClicked()));
    connect(ui.getAtrOneAlarmInfoBtn, SIGNAL(clicked()), this, SLOT(OnGetAtrOneAlarmInfoBtnClicked()));
    connect(ui.setAtrOneAlarmInfoBtn, SIGNAL(clicked()), this, SLOT(OnSetAtrOneAlarmInfoBtnClicked()));
    connect(ui.openAtrAlarmCallbackBtn, SIGNAL(clicked()), this, SLOT(OnOpenAtrAlarmCallbackBtnClicked()));
    connect(ui.getAtrAlarmDataBtn, SIGNAL(clicked()), this, SLOT(OnGetAlarmDataBtnClicked()));
    connect(ui.openAtrTempStreamBtn, SIGNAL(clicked()), this, SLOT(OnOpenAtrTempStreamBtnClicked()));
    connect(ui.saveAtrOneFrameBtn, SIGNAL(clicked()), this, SLOT(OnSaveOneFrameBtnClicked()));
}

void TempAlarmPage::ClearRoiRuleInfo()
{
    ui.ruleMaxTempEdit->setText("");
    ui.ruleMaxLineEdit_X->setText("");
    ui.ruleMaxLineEdit_Y->setText("");
    ui.ruleMinTempEdit->setText("");
    ui.ruleMinLineEdit_X->setText("");
    ui.ruleMinLineEdit_Y->setText("");
    ui.ruleAvgTempEdit->setText("");
    ui.ruleCenterTempEdit->setText("");
    ui.envEnableCheck->setChecked(false);
    ui.emissivityEdit->setText("");
    ui.atmosTransEdit->setText("");
    ui.refTempEdit->setText("");
    ui.atmosTempEdit->setText("");
    ui.distanceEdit->setText("");
    ui.alarmRuleCombo->setCurrentIndex(-1);
    ui.tempDebounceEdit->setText("");
    ui.thresholdTempEdit->setText("");
    ui.tempToleranceTempEdit->setText("");
}

void TempAlarmPage::RefreshRoiRuleInfo(const IRC_NET_TEMP_RULE_INFO& ruleInfo, const IRC_NET_TEMP_INFO& tempInfo)
{
    ui.ruleMaxTempEdit->setText(QString::number(tempInfo.maxTemp, 'f', 2));
    ui.ruleMaxLineEdit_X->setText(QString::number(tempInfo.maxTempPoint.x));
    ui.ruleMaxLineEdit_Y->setText(QString::number(tempInfo.maxTempPoint.y));
    ui.ruleMinTempEdit->setText(QString::number(tempInfo.minTemp, 'f', 2));
    ui.ruleMinLineEdit_X->setText(QString::number(tempInfo.minTempPoint.x));
    ui.ruleMinLineEdit_Y->setText(QString::number(tempInfo.minTempPoint.y));
    ui.ruleAvgTempEdit->setText(QString::number(tempInfo.avgTemp, 'f', 2));
    ui.ruleCenterTempEdit->setText(QString::number(tempInfo.centerTemp, 'f', 2));
    ui.envEnableCheck->setChecked(ruleInfo.envParamEnable);
    ui.emissivityEdit->setText(QString::number(ruleInfo.envParam.emissivity));
    ui.atmosTransEdit->setText(QString::number(ruleInfo.envParam.transmittance));
    ui.refTempEdit->setText(QString::number(ruleInfo.envParam.reflectedTemp));
    ui.atmosTempEdit->setText(QString::number(ruleInfo.envParam.atmosphereTemp));
    ui.distanceEdit->setText(QString::number(ruleInfo.envParam.distance));
    ui.alarmRuleCombo->setCurrentIndex(ruleInfo.alarmRuleInfo.type);
    ui.tempDebounceEdit->setText(QString::number(ruleInfo.alarmRuleInfo.debounce));
    ui.thresholdTempEdit->setText(QString::number(ruleInfo.alarmRuleInfo.thresholdTemp));
    ui.tempToleranceTempEdit->setText(QString::number(ruleInfo.alarmRuleInfo.toleranceTemp));
    ui.ROISnapEnableCheckBox->setChecked(ruleInfo.alarmLinkageSet.snapshotLinkageInfo.enable);
    ui.ROISnapChannel1CheckBox->setChecked(ruleInfo.alarmLinkageSet.snapshotLinkageInfo.channel[0]);
    ui.ROISnapChannel2CheckBox->setChecked(ruleInfo.alarmLinkageSet.snapshotLinkageInfo.channel[1]);
    ui.ROISnapDeplayEdit->setText(QString::number(ruleInfo.alarmLinkageSet.snapshotLinkageInfo.delay));
    ui.ROIRecordEnableCheckBox->setChecked(ruleInfo.alarmLinkageSet.recordLinkageInfo.enable);
    ui.ROIRecordChannel1CheckBox->setChecked(ruleInfo.alarmLinkageSet.recordLinkageInfo.channel[0]);
    ui.ROIRecordChannel2CheckBox->setChecked(ruleInfo.alarmLinkageSet.recordLinkageInfo.channel[1]);
    ui.ROIRecordDeplayEdit->setText(QString::number(ruleInfo.alarmLinkageSet.recordLinkageInfo.delay));
}

void TempAlarmPage::RefreshAtrAlarmInfo(const IRC_NET_TEMP_RULE_INFO_G1& tempAlarmInfo)
{
    ui.lowTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.thresholdTemp,'f',1));
    ui.highTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.thresholdTemp,'f', 1));
    ui.lowLevelOneTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel1, 'f', 1));
    ui.lowLevelTwoTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel2, 'f', 1));
    ui.lowLevelThreeTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel3, 'f', 1));
    ui.highLevelOneTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.tempLevel1, 'f', 1));
    ui.highLevelTwoTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.tempLevel2, 'f', 1));
    ui.highLevelThreeTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.tempLevel3, 'f', 1));
    ui.atrDelayEdit->setText(QString::number(tempAlarmInfo.debounce));
    ui.lowEnableCheck->setChecked(tempAlarmInfo.lowTempAlarmRuleInfo.enable);
    ui.highEnableCheck->setChecked(tempAlarmInfo.highTempAlarmRuleInfo.enable);
    if (IRC_NET_TEMP_ALARM_RULE_G1_HIGH_LOW_TEMP == tempAlarmInfo.alarmType)
    {
        ui.atrSwitchCombo->setCurrentIndex(2);
    }
    else if(IRC_NET_TEMP_ALARM_RULE_G1_HIGH_TEMP == tempAlarmInfo.alarmType)
    {
        ui.atrSwitchCombo->setCurrentIndex(0);
    }
    else if (IRC_NET_TEMP_ALARM_RULE_G1_LOW_TEMP == tempAlarmInfo.alarmType)
    {
        ui.atrSwitchCombo->setCurrentIndex(1);
    }
    else
    {
        ui.atrSwitchCombo->setCurrentIndex(3);
    }
}

void TempAlarmPage::RefreshAtrFrameAlarmInfo(const IRC_NET_FRAME_TEMP_ALARM_CONFIG_G1& tempAlarmInfo)
{
    ui.lowTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.thresholdTemp,'f', 1));
    ui.highTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.thresholdTemp,'f', 1));
    ui.lowLevelOneTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel1, 'f', 1));
    ui.lowLevelTwoTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel2, 'f', 1));
    ui.lowLevelThreeTempEdit->setText(QString::number(tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel3, 'f', 1));
    ui.highLevelOneTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.tempLevel1, 'f', 1));
    ui.highLevelTwoTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.tempLevel2, 'f', 1));
    ui.highLevelThreeTempEdit->setText(QString::number(tempAlarmInfo.highTempAlarmRuleInfo.tempLevel3, 'f', 1));
    ui.atrDelayEdit->setText(QString::number(tempAlarmInfo.debounce));
    ui.lowEnableCheck->setChecked(tempAlarmInfo.lowTempAlarmRuleInfo.enable);
    ui.highEnableCheck->setChecked(tempAlarmInfo.highTempAlarmRuleInfo.enable);
    if (IRC_NET_TEMP_ALARM_RULE_G1_HIGH_LOW_TEMP == tempAlarmInfo.alarmType)
    {
        ui.atrSwitchCombo->setCurrentIndex(2);
    }
    else if (IRC_NET_TEMP_ALARM_RULE_G1_HIGH_TEMP == tempAlarmInfo.alarmType)
    {
        ui.atrSwitchCombo->setCurrentIndex(0);
    }
    else if (IRC_NET_TEMP_ALARM_RULE_G1_LOW_TEMP == tempAlarmInfo.alarmType)
    {
        ui.atrSwitchCombo->setCurrentIndex(1);
    }
    else
    {
        ui.atrSwitchCombo->setCurrentIndex(3);
    }
}

void TempAlarmPage::GetRoiRuleInfo(IRC_NET_TEMP_RULE_INFO& ruleInfo)
{
    ruleInfo.envParamEnable = ui.envEnableCheck->isChecked();
    ruleInfo.envParam.emissivity = ui.emissivityEdit->text().toDouble();
    ruleInfo.envParam.transmittance = ui.atmosTransEdit->text().toDouble();
    ruleInfo.envParam.reflectedTemp = ui.refTempEdit->text().toDouble();
    ruleInfo.envParam.atmosphereTemp = ui.atmosTempEdit->text().toDouble();
    ruleInfo.envParam.distance = ui.distanceEdit->text().toDouble();
    ruleInfo.alarmRuleInfo.type = ui.alarmRuleCombo->currentIndex();
    ruleInfo.alarmRuleInfo.debounce = ui.debounceEdit->text().toInt();
    ruleInfo.alarmRuleInfo.thresholdTemp = ui.thresholdTempEdit->text().toDouble();
    ruleInfo.alarmRuleInfo.toleranceTemp = ui.toleranceTempEdit->text().toDouble();
}

void TempAlarmPage::SetFrameTempAlarm(const IRC_NET_FRAME_TEMP_ALARM_CONFIG& alarmConfig)
{
    ui.frameSnapEnableCheckBox->setChecked(alarmConfig.alarmLinkageSet.snapshotLinkageInfo.enable);
    ui.frameSnapChannel1CheckBox->setChecked(alarmConfig.alarmLinkageSet.snapshotLinkageInfo.channel[0]);
    ui.frameSnapChannel2CheckBox->setChecked(alarmConfig.alarmLinkageSet.snapshotLinkageInfo.channel[1]);
    ui.frameSnapDeplayEdit->setText(QString::number(alarmConfig.alarmLinkageSet.snapshotLinkageInfo.delay));

    ui.frameRecordEnableCheckBox->setChecked(alarmConfig.alarmLinkageSet.recordLinkageInfo.enable);
    ui.frameRecordChannel1CheckBox->setChecked(alarmConfig.alarmLinkageSet.recordLinkageInfo.channel[0]);
    ui.frameRecordChannel2CheckBox->setChecked(alarmConfig.alarmLinkageSet.recordLinkageInfo.channel[1]);
    ui.frameRecordDeplayEdit->setText(QString::number(alarmConfig.alarmLinkageSet.recordLinkageInfo.delay));

    alarmConfig.alarmRuleInfo[0].type == 0 ? ui.greaterCheckBox->setChecked(false) : ui.greaterCheckBox->setChecked(true);
    ui.greaterThresholdTempEdit->setText(QString::number(alarmConfig.alarmRuleInfo[0].thresholdTemp));
    alarmConfig.alarmRuleInfo[1].type == 0 ? ui.lessCheckBox->setChecked(false) : ui.lessCheckBox->setChecked(true);
    ui.lessThresholdTempEdit->setText(QString::number(alarmConfig.alarmRuleInfo[1].thresholdTemp));
    ui.debounceEdit->setText(QString::number(alarmConfig.alarmRuleInfo[1].debounce));
    ui.toleranceTempEdit->setText(QString::number(alarmConfig.alarmRuleInfo[1].toleranceTemp));
}

int  TempAlarmPage::GetTempRuleType(QString& str)
{
    int ret;
    if (str.startsWith("P"))
    {
        ret = 0;
    }
    else if (str.startsWith("L"))
    {
        ret = 1;
    }
    else
    {
        ret = 2;
    }
    return ret;
}

QString TempAlarmPage::GetRuleName(int type, int id)
{
    QString name;
    switch (type)
    {
    case IRC_NET_TEMP_RULE_POINT:
        name = tr("P") + QString::number(id);
        break;
    case IRC_NET_TEMP_RULE_LINE:
        name = tr("L") + QString::number(id);
        break;
    case IRC_NET_TEMP_RULE_RECT:
    case IRC_NET_TEMP_RULE_CIRCLE:
    case IRC_NET_TEMP_RULE_POLYGON:
        name = tr("R") + QString::number(id);
        break;
    default:
        break;
    }
    return name;
}

bool TempAlarmPage::CheckAtrAlarmParam()
{
    if (-1 == ui.atrTypeCombo->currentIndex() || -1 == ui.atrSwitchCombo->currentIndex()
        || "" == ui.lowTempEdit->text() || "" == ui.lowLevelOneTempEdit->text() || "" == ui.lowLevelThreeTempEdit->text()
        || "" == ui.lowLevelTwoTempEdit->text() || "" == ui.highTempEdit->text() || "" == ui.highLevelOneTempEdit->text()
        || "" == ui.highLevelThreeTempEdit->text() || "" == ui.highLevelTwoTempEdit->text() || "" == ui.atrDelayEdit->text())
    {
        return false;
    }
    if (ui.lowTempEdit->text().toFloat() >= ui.highTempEdit->text().toFloat()
        || ui.highLevelTwoTempEdit->text().toFloat() <= ui.highLevelOneTempEdit->text().toFloat()
        || ui.highLevelThreeTempEdit->text().toFloat() <= ui.highLevelTwoTempEdit->text().toFloat()
        || ui.highLevelOneTempEdit->text().toFloat() < ui.highTempEdit->text().toFloat()
        || ui.lowLevelOneTempEdit->text().toFloat() > ui.lowTempEdit->text().toFloat()
        || ui.lowLevelTwoTempEdit->text().toFloat() >= ui.lowLevelOneTempEdit->text().toFloat()
        || ui.lowLevelThreeTempEdit->text().toFloat() >= ui.lowLevelTwoTempEdit->text().toFloat()
        )
    {
        return false;
    }
    return true;
}

void TempAlarmPage::SavePictureInfoForFire(IRC_NET_FIRE_ALARM_INFO info)
{
    if (m_flagFire)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureFire_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagFire = false;
                file.close();
            }
        }
    }
}

void TempAlarmPage::SavePictureInfoForTemp(IRC_NET_TEMP_ALARM_INFO info)
{
    if (m_flagTemp)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureTemp_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagTemp = false;
                file.close();
            }
        }

    }
}

void TempAlarmPage::SavePictureInfoForRegionIntrusion(IRC_NET_REGION_INTRUSION_ALARM_INFO info)
{
    if (m_flagRegion)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureRegion_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagRegion = false;
                file.close();
            }
        }

    }
}

void TempAlarmPage::SavePictureInfoForLineIntrusion(IRC_NET_LINE_INTRUSION_ALARM_INFO info)
{
    if (m_flagLine)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureLine_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagLine = false;
                file.close();
            }
        }

    }
}

void TempAlarmPage::SavePictureInfoForTempDiff(IRC_NET_TEMP_DIFF_ALARM_INFO info)
{
    if (m_flagDiff)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureTempDiff_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagDiff = false;
                file.close();
            }
        }

    }
}

void TempAlarmPage::SavePictureInfoForLocal(IRC_NET_LOCAL_ALARM_INFO info)
{
    if (m_flagLocal)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureLocal_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagLocal = false;
                file.close();
            }
        }

    }
}

void TempAlarmPage::SavePictureInfoForSmokeDetect(IRC_NET_SMOKE_DETECT_ALARM_INFO info)
{
    if (m_flagSmoke)
    {
        for (int i = 0; i < 2; i++)
        {
            if (info.alarmBaseInfo.pictureInfos[i].pictureInfo != nullptr && info.alarmBaseInfo.pictureInfos[i].size > 0)
            {
                QString filePath = "testPictureSmokeDetect_" + QString::number(i) + ".jpg";
                QFile file(filePath);
                if (!file.open(QIODevice::WriteOnly))
                {
                    break;
                }
                QByteArray imageData(info.alarmBaseInfo.pictureInfos[i].pictureInfo, info.alarmBaseInfo.pictureInfos[i].size);
                file.write(imageData);
                m_flagSmoke = false;
                file.close();
            }
        }

    }
}

void TempAlarmPage::OnChangeHandle(IRC_NET_HANDLE handle)
{
    m_handle = handle;
}

void TempAlarmPage::OnGetFrameTempDataBtnClicked()
{
    IRC_NET_TEMP_INFO tempInfo;
    int err = IRC_NET_QueryFrameTemp(m_handle, &tempInfo);
    QString temp = tr("Query Frame Temp");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.frameMaxTempEdit->setText(QString::number(tempInfo.maxTemp, 'f', 2));
    ui.frameMinTempEdit->setText(QString::number(tempInfo.minTemp, 'f', 2));
    ui.frameAvgTempEdit->setText(QString::number(tempInfo.avgTemp, 'f', 2));
    ui.frameCenterTempEdit->setText(QString::number(tempInfo.centerTemp, 'f', 2));
    ui.frameMaxLineEdit_X->setText(QString::number(tempInfo.maxTempPoint.x));
    ui.frameMaxLineEdit_Y->setText(QString::number(tempInfo.maxTempPoint.y));
    ui.frameMinLineEdit_X->setText(QString::number(tempInfo.minTempPoint.x));
    ui.frameMinLineEdit_Y->setText(QString::number(tempInfo.minTempPoint.y));

    IRC_NET_FRAME_TEMP_ALARM_CONFIG alarmConfig;
    err = IRC_NET_QueryFrameTempAlarmConfig(m_handle, &alarmConfig);
    temp = tr("Query Frame Temp AlarmConfig");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    SetFrameTempAlarm(alarmConfig);
}

void TempAlarmPage::OnOpenTempStreamBtnClicked()
{
    if (tr("Open") == ui.openTempStreamBtn->text())
    {
        //int err = IRC_NET_StartPullTemp(m_handle, TempCallback, this);
        int err = IRC_NET_StartPullTemp_V2(m_handle, TempCallback_V2, this);
        QString temp = tr("Open temp stream");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.saveOneFrameBtn->setEnabled(true);
        ui.openTempStreamBtn->setText("Close");
    }
    else
    {
        IRC_NET_StopPullTemp(m_handle);
        ui.saveOneFrameBtn->setEnabled(false);
        ui.openTempStreamBtn->setText("Open");
    }
}

void TempAlarmPage::OnSaveOneFrameBtnClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath() + "/", "Data file(*.dat);;Text File(*.txt)");
    if (fileName.isEmpty())
    {
        return;
    }

    QMutexLocker locker(&m_tempMutex);
    m_saveOneFrame = true;
    m_oneFramePath = fileName;
}

void TempAlarmPage::OnGetAllRulesBtnClicked()
{
    IRC_NET_TEMP_RULE_INDEX ruleIndex;
    ruleIndex.presetId = ui.presetCombo->currentIndex();
    if (-1 == ruleIndex.presetId)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please select the preset!"));
        return;
    }
    ruleIndex.id = -1;
    ruleIndex.type = -1;
    int size = 0;
    int err = IRC_NET_QueryTempRuleSize(m_handle, &ruleIndex, &size);
    QString temp = tr("Get temp rule size");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }

    IRC_NET_TEMP_RULE_INFO* ruleInfo = new IRC_NET_TEMP_RULE_INFO[size];
    memset(ruleInfo, 0, size * sizeof(IRC_NET_TEMP_RULE_INFO));
    int outSize = 0;
    err = IRC_NET_QueryTempRule(m_handle, &ruleIndex, ruleInfo, size, &outSize);
    temp = tr("Get temp rule");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        delete[] ruleInfo;
        return;
    }
    m_ruleIndexMap.clear();
    m_ruleInfoMap.clear();
    ui.ruleCombo->clear();
    for (int i = 0; i < outSize; i++)
    {
        if ("" != GetRuleName(ruleInfo[i].type, ruleInfo[i].id))
        {
            ui.ruleCombo->addItem(GetRuleName(ruleInfo[i].type, ruleInfo[i].id));
                IRC_NET_TEMP_RULE_INDEX index;
                index.presetId = ruleInfo[i].presetId;
                index.id = ruleInfo[i].id;
                index.type = ruleInfo[i].type;
                m_ruleInfoMap[i] = ruleInfo[i];
                m_ruleIndexMap[i] = index;
        }
      
    }
    ui.ruleCombo->setCurrentIndex(-1);
    delete[] ruleInfo;
}

void TempAlarmPage::OnDeleteAllRulesBtnClicked()
{
    int preset = ui.presetCombo->currentIndex();
    if (-1 == preset)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please select the preset!"));
        return;
    }
    int err = IRC_NET_DeleteAllTempRule(m_handle, preset);
    QString temp = tr("Delete");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }

    OnGetAllRulesBtnClicked();
}

void TempAlarmPage::OnGetOneRuleInfoBtnClicked()
{
    int index = ui.ruleCombo->currentIndex();
    if (!m_ruleIndexMap.contains(index))
    {
        return;
    }
    IRC_NET_TEMP_RULE_INDEX ruleIndex = m_ruleIndexMap[index];
    IRC_NET_TEMP_RULE_INFO ruleInfo;
    int outSize = 0;
    int err = IRC_NET_QueryTempRule(m_handle, &ruleIndex, &ruleInfo, 1, &outSize);
    QString temp = tr("Get temp rule");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    IRC_NET_RULE_TEMP_INFO tempInfo;
    err = IRC_NET_QueryRuleTemp(m_handle, &ruleIndex, &tempInfo, 1, &outSize);
    temp = tr("Get rule temp");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
    }

    RefreshRoiRuleInfo(ruleInfo, tempInfo.tempInfo);
}

void TempAlarmPage::OnDeleteOneRuleInfoBtnClicked()
{
    int index = ui.ruleCombo->currentIndex();
    if (!m_ruleIndexMap.contains(index))
    {
        return;
    }

    IRC_NET_TEMP_RULE_INDEX ruleIndex = m_ruleIndexMap[index];
    int err = IRC_NET_DeleteTempRule(m_handle, &ruleIndex);
    QString temp = tr("delete");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }

    OnGetAllRulesBtnClicked();
}

void TempAlarmPage::OnSetAlarmInfoBtnClicked()
{
    IRC_NET_FRAME_TEMP_ALARM_CONFIG alarmConfig;
    alarmConfig.enable = true;
    ui.greaterCheckBox->isChecked() ? alarmConfig.alarmRuleInfo[0].type = 3 : alarmConfig.alarmRuleInfo[0].type = 0;
    alarmConfig.alarmRuleInfo[0].thresholdTemp = ui.greaterThresholdTempEdit->text().toFloat();
    alarmConfig.alarmRuleInfo[0].toleranceTemp = ui.toleranceTempEdit->text().toFloat();
    alarmConfig.alarmRuleInfo[0].debounce = ui.debounceEdit->text().toInt();
    ui.lessCheckBox->isChecked() ? alarmConfig.alarmRuleInfo[1].type = 6 : alarmConfig.alarmRuleInfo[1].type = 0;
    alarmConfig.alarmRuleInfo[1].thresholdTemp = ui.lessThresholdTempEdit->text().toFloat();
    alarmConfig.alarmRuleInfo[1].toleranceTemp = ui.toleranceTempEdit->text().toFloat();
    alarmConfig.alarmRuleInfo[1].debounce = ui.debounceEdit->text().toInt();

    ui.frameSnapEnableCheckBox->isChecked() ? alarmConfig.alarmLinkageSet.snapshotLinkageInfo.enable = true : alarmConfig.alarmLinkageSet.snapshotLinkageInfo.enable = false;
    ui.frameSnapChannel1CheckBox->isChecked() ? alarmConfig.alarmLinkageSet.snapshotLinkageInfo.channel[0] = true : alarmConfig.alarmLinkageSet.snapshotLinkageInfo.channel[0] = false;
    ui.frameSnapChannel2CheckBox->isChecked() ? alarmConfig.alarmLinkageSet.snapshotLinkageInfo.channel[1] = true : alarmConfig.alarmLinkageSet.snapshotLinkageInfo.channel[1] = false;
    alarmConfig.alarmLinkageSet.snapshotLinkageInfo.delay = ui.frameSnapDeplayEdit->text().toInt();

    ui.frameRecordEnableCheckBox->isChecked() ? alarmConfig.alarmLinkageSet.recordLinkageInfo.enable = true : alarmConfig.alarmLinkageSet.recordLinkageInfo.enable = false;
    ui.frameRecordChannel1CheckBox->isChecked() ? alarmConfig.alarmLinkageSet.recordLinkageInfo.channel[0] = true : alarmConfig.alarmLinkageSet.recordLinkageInfo.channel[0] = false;
    ui.frameRecordChannel2CheckBox->isChecked() ? alarmConfig.alarmLinkageSet.recordLinkageInfo.channel[1] = true : alarmConfig.alarmLinkageSet.recordLinkageInfo.channel[1] = false;
    alarmConfig.alarmLinkageSet.recordLinkageInfo.delay = ui.frameRecordDeplayEdit->text().toInt();
    int err = IRC_NET_UpdateFrameTempAlarmConfig(m_handle, &alarmConfig);
    QString temp = tr("set alarm info");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void TempAlarmPage::OnOpenAlarmCallbackBtnClicked()
{
    if (tr("Open") == ui.openAlarmCallbackBtn->text())
    {
        int err = IRC_NET_SubscribeAlarm(m_handle, AlarmCallback, this);
        QString temp = tr("Subscribe alarm");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }

        ui.openAlarmCallbackBtn->setText("Close");
    }
    else
    {
        IRC_NET_UnsubscribeAlarm(m_handle);
        ui.openAlarmCallbackBtn->setText("Open");
    }
}

void TempAlarmPage::OnGetAlarmDataBtnClicked()
{
    m_alarmPage->show();
}

void TempAlarmPage::OnGetTempAlarm(int alarmType, IRC_NET_TEMP_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddTempAlarm(alarmType, alarmInfo);
}

void TempAlarmPage::OnGetFireAlarm(int alarmType, IRC_NET_FIRE_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddFireAlarm(alarmType, alarmInfo);
}

void TempAlarmPage::OnGetRegionIntrusionAlarm(int alarmType, IRC_NET_REGION_INTRUSION_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddRegionIntrusionAlarm(alarmType, alarmInfo);
}

void TempAlarmPage::OnGetLineIntrusionAlarm(int alarmType, IRC_NET_LINE_INTRUSION_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddLineIntrusionAlarm(alarmType, alarmInfo);
}

void TempAlarmPage::OnGetTempDiffAlarm(int alarmType, IRC_NET_TEMP_DIFF_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddTempDiffAlarm(alarmType, alarmInfo);
}

void TempAlarmPage::OnGetSmokeDetectAlarm(int alarmType, IRC_NET_SMOKE_DETECT_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddSmokeDetectAlarm(alarmType, alarmInfo);
}
void TempAlarmPage::OnGetLocalAlarm(int alarmType, IRC_NET_LOCAL_ALARM_INFO alarmInfo)
{
    m_alarmPage->AddLocalAlarm(alarmType, alarmInfo);
}

void TempAlarmPage::OnpointOnceMeasureBtnClicked()
{
    ui.pointOnceEdit->setText("");
    //m_mainWindow->SetOncePointTempParamCheck();
    if (ui.pointOnceMeasureBtn->isChecked())
    {
        m_mainWindow->SetOncePointTempParamCheck();
    }
    else
    {
        m_mainWindow->SetOncePointTempParamUnCheck();
    }
    
}

void TempAlarmPage::OnGetAllPreSetBtnClicked()
{
    ui.presetCombo->clear();
    m_presetIndexMap.clear();
    ui.presetCombo->addItem("Frame");
    int insize = 0;
    int outsize = 0;
    int err = IRC_NET_QueryPtzPresetSize(m_handle, &insize);
    QString temp = tr("QueryPtzPresetSize");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    IRC_NET_PTZ_PRESET_INFO* presetInfo = new IRC_NET_PTZ_PRESET_INFO[insize];
    memset(presetInfo, 0, insize * sizeof(IRC_NET_PTZ_PRESET_INFO));
    err = IRC_NET_QueryPtzPreset(m_handle, presetInfo, insize, &outsize);
    temp = tr("QueryPtzPreset");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    for (int i = 0; i < outsize; i++)
    {
        QString s = "";
        presetInfo[i].enable == true ? s = "*" : s = "";
        ui.presetCombo->addItem(QString::number(presetInfo[i].id) + s);
        m_presetIndexMap[i + 1] = presetInfo[i];
    }

    ui.presetCombo->setCurrentIndex(0);
    delete[]presetInfo;
}

void TempAlarmPage::OnMoveToBtnClicked()
{
    IRC_NET_PTZ_PRESET_INFO presetInfo;
    presetInfo.enable = true;
    presetInfo.id = m_presetIndexMap[ui.presetCombo->currentIndex()].id;
    strcpy(presetInfo.name, m_presetIndexMap[ui.presetCombo->currentIndex()].name);
    int err = IRC_NET_PtzPresetControl(m_handle, IRC_NET_PTZ_PRESET_CMD_TYPE::IRC_NET_PTZ_PRESET_CMD_GOTO, &presetInfo);
    QString temp = tr("PTZ control move to");
	if (IRC_NET_ERROR_OK != err)
	{
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
	}
}

void TempAlarmPage::OnShieldedGetAllBtnClicked()
{
    int insize = 0;
    int outsize = 0;
    IRC_NET_TEMP_MASK_INDEX tempMaskIndex;
    tempMaskIndex.presetId = ui.presetCombo->currentIndex();
    tempMaskIndex.id = -1;
    if (tempMaskIndex.presetId < 0) {
        QMessageBox::critical(this, tr("Error"), tr("choose preset id"));
        return;
    }
    int err = IRC_NET_QueryTempMaskSize(m_handle, &tempMaskIndex, &insize);
    QString temp = tr("QueryTempMaskSize");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }

    IRC_NET_TEMP_MASK_INFO* tempMaskInfos = new IRC_NET_TEMP_MASK_INFO[insize];
    memset(tempMaskInfos, 0, insize * sizeof(IRC_NET_TEMP_MASK_INFO));
    err = IRC_NET_QueryTempMask(m_handle, &tempMaskIndex, tempMaskInfos, insize, &outsize);
    temp = tr("QueryTempMask");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    ui.shieldedAreaCombo->clear();
    m_shieldedMap.clear();
    for (int i = 0; i < outsize; i++) 
    {
        QString name = tr("SR") + QString::number(tempMaskInfos[i].index.id);
        ui.shieldedAreaCombo->addItem(name);
        m_shieldedMap[name] = tempMaskInfos[i];
    }
}

void TempAlarmPage::OnShieldedDeleteAllBtnClicked()
{
    int presetid = ui.presetCombo->currentIndex();
    int err = IRC_NET_DeleteAllTempMask(m_handle, presetid);
    QString temp = tr("QueryTempMask delete all");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    OnShieldedGetAllBtnClicked();
}

void TempAlarmPage::OnShieldedDeleteBtnClicked()
{
    IRC_NET_TEMP_MASK_INDEX tempMaskIndex;
    tempMaskIndex.presetId = ui.presetCombo->currentIndex();
    tempMaskIndex.id = m_shieldedMap[ui.shieldedAreaCombo->currentText()].index.id;
    int err = IRC_NET_DeleteTempMask(m_handle, &tempMaskIndex);
    QString temp = tr("QueryTempMask delete");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    OnShieldedGetAllBtnClicked();
}

void TempAlarmPage::OnROISetBtnClicked()
{
    IRC_NET_TEMP_RULE_INFO tempRegionRuleInfo;
    tempRegionRuleInfo = m_ruleInfoMap[ui.ruleCombo->currentIndex()];
    tempRegionRuleInfo.enable = true;
    tempRegionRuleInfo.envParamEnable = ui.envEnableCheck->isChecked();
    tempRegionRuleInfo.envParam.emissivity = ui.emissivityEdit->text().toFloat();
    tempRegionRuleInfo.envParam.transmittance = ui.atmosTransEdit->text().toFloat();
    tempRegionRuleInfo.envParam.reflectedTemp = ui.refTempEdit->text().toFloat();
    tempRegionRuleInfo.envParam.atmosphereTemp = ui.atmosTempEdit->text().toFloat();
    tempRegionRuleInfo.envParam.distance = ui.distanceEdit->text().toFloat();

    tempRegionRuleInfo.alarmRuleInfo.type = ui.alarmRuleCombo->currentIndex();
    if (IRC_NET_TEMP_RULE_POINT == tempRegionRuleInfo.type)
    {
        tempRegionRuleInfo.alarmRuleInfo.type = tempRegionRuleInfo.alarmRuleInfo.type > 2 ? 2 : tempRegionRuleInfo.alarmRuleInfo.type;
    }
    /*if (ui.alarmRuleCombo->currentIndex() == 0) {
        tempRegionRuleInfo.alarmRuleInfo.type = IRC_NET_TEMP_ALARM_RULE_NONE;
    }
    else if (ui.alarmRuleCombo->currentIndex() == 1) {
        tempRegionRuleInfo.alarmRuleInfo.type = IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_GT;
    }
    else {
        tempRegionRuleInfo.alarmRuleInfo.type = IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_LT;
    }*/
    tempRegionRuleInfo.alarmRuleInfo.thresholdTemp = ui.thresholdTempEdit->text().toInt();
    tempRegionRuleInfo.alarmRuleInfo.debounce = ui.tempDebounceEdit->text().toInt();
    tempRegionRuleInfo.alarmRuleInfo.toleranceTemp = ui.tempToleranceTempEdit->text().toInt();

    tempRegionRuleInfo.alarmLinkageSet.snapshotLinkageInfo.enable = ui.ROISnapEnableCheckBox->isChecked();
    tempRegionRuleInfo.alarmLinkageSet.snapshotLinkageInfo.channel[0] = ui.ROISnapChannel1CheckBox->isChecked();
    tempRegionRuleInfo.alarmLinkageSet.snapshotLinkageInfo.channel[1] = ui.ROISnapChannel2CheckBox->isChecked();
    tempRegionRuleInfo.alarmLinkageSet.snapshotLinkageInfo.delay = ui.ROISnapDeplayEdit->text().toInt();
    tempRegionRuleInfo.alarmLinkageSet.recordLinkageInfo.enable = ui.ROIRecordEnableCheckBox->isChecked();
    tempRegionRuleInfo.alarmLinkageSet.recordLinkageInfo.channel[0] = ui.ROIRecordChannel1CheckBox->isChecked();
    tempRegionRuleInfo.alarmLinkageSet.recordLinkageInfo.channel[1] = ui.ROIRecordChannel2CheckBox->isChecked();
    tempRegionRuleInfo.alarmLinkageSet.recordLinkageInfo.delay = ui.ROIRecordDeplayEdit->text().toInt();
    int err = IRC_NET_UpdateTempRule(m_handle, &tempRegionRuleInfo);
    QString temp = tr("Temp rule set");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void TempAlarmPage::OnShielded(int ruleType, QList<QPoint> points)
{
    IRC_NET_TEMP_MASK_INFO tempMaskInfo;
    tempMaskInfo.enable = true;
    tempMaskInfo.index.presetId = m_presetIndexMap[ui.presetCombo->currentIndex()].id;
    tempMaskInfo.index.id = m_shieldedMap.size()+1;
    QString name;
    strcpy(tempMaskInfo.name, name.toStdString().c_str());
    tempMaskInfo.points[0].x = points[0].x();
    tempMaskInfo.points[0].y = points[0].y();
    tempMaskInfo.points[1].x = points[1].x();
    tempMaskInfo.points[1].y = points[0].y();
    tempMaskInfo.points[2].x = points[1].x();
    tempMaskInfo.points[2].y = points[1].y();
    tempMaskInfo.points[3].x = points[0].x();
    tempMaskInfo.points[3].y = points[1].y();
    int err = IRC_NET_AddTempMask(m_handle, &tempMaskInfo);
    QString temp = tr("Add Shielded");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
}

void TempAlarmPage::OnGetAtrAllRulesBtnClicked()
{
    IRC_NET_TEMP_RULE_INDEX ruleIndex;
    ruleIndex.presetId = ui.presetCombo->currentIndex();
    if (-1 == ruleIndex.presetId)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please select the preset!"));
        return;
    }
    ruleIndex.id = -1;
    ruleIndex.type = -1;
    int size = 0;
    QString temp = tr("Get temp rule size");
    int err = IRC_NET_QueryTempRuleSize(m_handle, &ruleIndex, &size);
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    IRC_NET_TEMP_RULE_INFO_G1* ruleInfo = new IRC_NET_TEMP_RULE_INFO_G1[size];
    memset(ruleInfo, 0, size * sizeof(IRC_NET_TEMP_RULE_INFO_G1));
    int outSize = 0;
    temp = tr("Get G1 temp rule");
    err = IRC_NET_QueryTempRule_G1(m_handle, &ruleIndex, ruleInfo, size, &outSize);
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        delete[] ruleInfo;
        return;
    }
    m_ruleIndexMap.clear();
    m_ruleInfoMap.clear();
    ui.atrRuleCombo->clear();
    ui.atrTypeCombo->clear();
    ui.atrRuleCombo->addItem("frame");
    ui.atrTypeCombo->addItem("frame");
    for (int i = 0; i < outSize; i++)
    {
        ui.atrRuleCombo->addItem(GetRuleName(ruleInfo[i].type, ruleInfo[i].id));
        ui.atrTypeCombo->addItem(GetRuleName(ruleInfo[i].type, ruleInfo[i].id));
        IRC_NET_TEMP_RULE_INDEX index;
        index.id = ruleInfo[i].id;
        index.type = ruleInfo[i].type;
        m_ruleInfoMapG1[i] = ruleInfo[i];
        m_ruleIndexMap[i] = index;
    }
    ui.atrRuleCombo->setCurrentIndex(-1);
    ui.atrTypeCombo->setCurrentIndex(0);
    delete[] ruleInfo;
}

void TempAlarmPage::OnDeleteAtrAllBtnClicked()
{
    int preset = ui.presetCombo->currentIndex();
    if (-1 == preset)
    {
        QMessageBox::critical(this, tr("Error"), tr("Please select the preset!"));
        return;
    }
    int err = IRC_NET_DeleteAllTempRule(m_handle, preset);
    QString temp = tr("Delete temp rule");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    OnGetAtrAllRulesBtnClicked();
}

void TempAlarmPage::OngetAtrOneRuleInfoBtnClicked()
{
    int currentIndex = ui.atrRuleCombo->currentIndex();
    if (-1 == currentIndex)
    {
        return;
    }
    if (0 == currentIndex)
    {
        IRC_NET_TEMP_INFO tempInfo;
        int err = IRC_NET_QueryFrameTemp(m_handle, &tempInfo);
        QString temp = tr("Query Frame Temp");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.atrFrameMaxTempEdit->setText(QString::number(tempInfo.maxTemp));
        ui.atrFrameMinTempEdit->setText(QString::number(tempInfo.minTemp));
        ui.atrFrameAvgTempEdit->setText(QString::number(tempInfo.avgTemp));
        ui.atrFrameCenterTempEdit->setText(QString::number(tempInfo.centerTemp));
    }
    else
    {
        IRC_NET_TEMP_RULE_INDEX ruleIndex = m_ruleIndexMap[currentIndex-1];
        IRC_NET_TEMP_RULE_INFO_G1 ruleInfo;
        int outSize = 0;
        int err = IRC_NET_QueryTempRule_G1(m_handle, &ruleIndex, &ruleInfo, 1, &outSize);
        QString temp = tr("Get temp rule");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        IRC_NET_RULE_TEMP_INFO tempInfo;
        err = IRC_NET_QueryRuleTemp(m_handle, &ruleIndex, &tempInfo, 1, &outSize);
        temp = tr("Get rule temp");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.atrFrameMaxTempEdit->setText(QString::number(tempInfo.tempInfo.maxTemp,'f',1));
        ui.atrFrameMinTempEdit->setText(QString::number(tempInfo.tempInfo.minTemp, 'f', 1));
        ui.atrFrameAvgTempEdit->setText(QString::number(tempInfo.tempInfo.avgTemp, 'f', 1));
        ui.atrFrameCenterTempEdit->setText(QString::number(tempInfo.tempInfo.centerTemp, 'f', 1));
    }
}

void TempAlarmPage::OnDeleteAtrOneRuleInfoBtnClicked()
{
    int index = ui.atrRuleCombo->currentIndex()-1;
    if (!m_ruleIndexMap.contains(index))
    {
        return;
    }
    IRC_NET_TEMP_RULE_INDEX ruleIndex = m_ruleIndexMap[index];
    int err = IRC_NET_DeleteTempRule(m_handle, &ruleIndex);
    QString temp = tr("delete temp rule");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    OnGetAtrAllRulesBtnClicked();
    ui.atrFrameMaxTempEdit->setText("");
    ui.atrFrameMinTempEdit->setText("");
    ui.atrFrameAvgTempEdit->setText("");
    ui.atrFrameCenterTempEdit->setText("");

}

void TempAlarmPage::OnGetAtrOneAlarmInfoBtnClicked()
{
    int index = ui.atrTypeCombo->currentIndex()-1;
    if ("frame" == ui.atrTypeCombo->currentText())
    {
        IRC_NET_FRAME_TEMP_ALARM_CONFIG_G1 tempAlarmInfo;
        int err = IRC_NET_QueryFrameTempAlarmConfig_G1(m_handle, &tempAlarmInfo);
        QString temp = tr("Get atr frame rule temp");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        RefreshAtrFrameAlarmInfo(tempAlarmInfo);
    }
    if (!m_ruleIndexMap.contains(index))
    {
        return;
    }
    IRC_NET_TEMP_RULE_INDEX ruleIndex = m_ruleIndexMap[index];
    IRC_NET_TEMP_RULE_INFO ruleInfo;
    int outSize = 0;
    IRC_NET_TEMP_RULE_INFO_G1 tempAlarmInfo;
    int err = IRC_NET_QueryTempRule_G1(m_handle, &ruleIndex, &tempAlarmInfo, 1, &outSize);
    QString temp = tr("Get atr rule temp ");
    if (IRC_NET_ERROR_OK != err)
    {
        MSG_BOX.ShowErrorMessage(temp, err);
        return;
    }
    RefreshAtrAlarmInfo(tempAlarmInfo);
}

void TempAlarmPage::OnSetAtrOneAlarmInfoBtnClicked()
{
    IRC_NET_TEMP_RULE_INFO_G1 tempAlarmInfo;
    IRC_NET_FRAME_TEMP_ALARM_CONFIG_G1 tempFrameAlarmInfo;
    int switchIndex = ui.atrSwitchCombo->currentIndex();
    int typeIndex = ui.atrTypeCombo->currentIndex();
    if (-1 == switchIndex)
    {
        return;
    }
    if ("frame" == ui.atrTypeCombo->currentText())
    {
        if (0 == switchIndex)
        {
            tempFrameAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_HIGH_TEMP;
        }
        else if (1 == switchIndex)
        {
            tempFrameAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_LOW_TEMP;
        }
        else if (2 == switchIndex)
        {
            tempFrameAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_HIGH_LOW_TEMP;
        }
        else
        {
            tempFrameAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_NONE;
        }
        if (!CheckAtrAlarmParam())
        {
            QMessageBox::critical(this, tr("Error"), tr("Set temp alarm param wrong!"));
            return;
        }
        tempFrameAlarmInfo.alarmLinkageSnapshotEnable = false;
        tempFrameAlarmInfo.debounce = ui.atrDelayEdit->text().toInt();
        tempFrameAlarmInfo.lowTempAlarmRuleInfo.thresholdTemp = ui.lowTempEdit->text().toFloat();
        tempFrameAlarmInfo.highTempAlarmRuleInfo.thresholdTemp = ui.highTempEdit->text().toFloat();
        tempFrameAlarmInfo.highTempAlarmRuleInfo.tempLevel1 = ui.highLevelOneTempEdit->text().toFloat();
        tempFrameAlarmInfo.highTempAlarmRuleInfo.tempLevel2 = ui.highLevelTwoTempEdit->text().toFloat();
        tempFrameAlarmInfo.highTempAlarmRuleInfo.tempLevel3 = ui.highLevelThreeTempEdit->text().toFloat();
        tempFrameAlarmInfo.lowTempAlarmRuleInfo.tempLevel1 = ui.lowLevelOneTempEdit->text().toFloat();
        tempFrameAlarmInfo.lowTempAlarmRuleInfo.tempLevel2 = ui.lowLevelTwoTempEdit->text().toFloat();
        tempFrameAlarmInfo.lowTempAlarmRuleInfo.tempLevel3 = ui.lowLevelThreeTempEdit->text().toFloat();
        tempFrameAlarmInfo.highTempAlarmRuleInfo.enable = ui.highEnableCheck->isChecked();
        tempFrameAlarmInfo.lowTempAlarmRuleInfo.enable = ui.lowEnableCheck->isChecked();
        int err = IRC_NET_UpdateFrameTempAlarmConfig_G1(m_handle, &tempFrameAlarmInfo);
        QString temp = tr("Set frame temp alarm rule");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
    }
    else
    {
        if (0 == switchIndex)
        {
            tempAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_HIGH_TEMP;
        }
        else if (1 == switchIndex)
        {
            tempAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_LOW_TEMP;
        }
        else if (2 == switchIndex)
        {
            tempAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_HIGH_LOW_TEMP;
        }
        else
        {
            tempAlarmInfo.alarmType = IRC_NET_TEMP_ALARM_RULE_G1_NONE;
        }
        if (!CheckAtrAlarmParam())
        {
            QMessageBox::critical(this, tr("Error"), tr("Set temp alarm param wrong!"));
            return;
        }
        tempAlarmInfo.id = typeIndex - 1;
        QString strType = ui.atrTypeCombo->currentText();
        tempAlarmInfo.type = GetTempRuleType(strType);
        tempAlarmInfo.alarmLinkageSnapshotEnable = false;
        tempAlarmInfo.debounce = ui.atrDelayEdit->text().toInt();
        tempAlarmInfo.lowTempAlarmRuleInfo.thresholdTemp = ui.lowTempEdit->text().toFloat();
        tempAlarmInfo.highTempAlarmRuleInfo.thresholdTemp = ui.highTempEdit->text().toFloat();
        tempAlarmInfo.highTempAlarmRuleInfo.tempLevel1 = ui.highLevelOneTempEdit->text().toFloat();
        tempAlarmInfo.highTempAlarmRuleInfo.tempLevel2 = ui.highLevelTwoTempEdit->text().toFloat();
        tempAlarmInfo.highTempAlarmRuleInfo.tempLevel3 = ui.highLevelThreeTempEdit->text().toFloat();
        tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel1 = ui.lowLevelOneTempEdit->text().toFloat();
        tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel2 = ui.lowLevelTwoTempEdit->text().toFloat();
        tempAlarmInfo.lowTempAlarmRuleInfo.tempLevel3 = ui.lowLevelThreeTempEdit->text().toFloat();
        tempAlarmInfo.highTempAlarmRuleInfo.enable = ui.highEnableCheck->isChecked();
        tempAlarmInfo.lowTempAlarmRuleInfo.enable = ui.lowEnableCheck->isChecked();

        int err = IRC_NET_UpdateTempRule_G1(m_handle, &tempAlarmInfo);
        QString temp = tr("Set temp alarm rule");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
    }
   
}

void TempAlarmPage::OnOpenAtrAlarmCallbackBtnClicked()
{
    if (tr("Open") == ui.openAtrAlarmCallbackBtn->text())
    {
        int err = IRC_NET_SubscribeAlarm(m_handle, AlarmCallback, this);
        QString temp = tr("Subscribe alarm");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.openAtrAlarmCallbackBtn->setText("Close");
    }
    else
    {
        IRC_NET_UnsubscribeAlarm(m_handle);
        ui.openAtrAlarmCallbackBtn->setText("Open");
    }
}

void TempAlarmPage::OnOpenAtrTempStreamBtnClicked()
{
    if (tr("Open") == ui.openAtrTempStreamBtn->text())
    {
        //int err = IRC_NET_StartPullTemp(m_handle, TempCallback, this);
        int err = IRC_NET_StartPullTemp_V2(m_handle, TempCallback_V2, this);
        QString temp = tr("Open temp stream");
        if (IRC_NET_ERROR_OK != err)
        {
            MSG_BOX.ShowErrorMessage(temp, err);
            return;
        }
        ui.saveAtrOneFrameBtn->setEnabled(true);
        ui.openAtrTempStreamBtn->setText("Close");
    }
    else
    {
        IRC_NET_StopPullTemp(m_handle);
        ui.saveAtrOneFrameBtn->setEnabled(false);
        ui.openAtrTempStreamBtn->setText("Open");
    }
}

void TempAlarmPage::TempCallback(IRC_NET_HANDLE handle, char* temp, int width, int height, void* userData)
{
    TempAlarmPage* page = (TempAlarmPage*)userData;
    QMutexLocker locker(&page->m_tempMutex);
    if (page->m_saveOneFrame)
    {
        if (".dat" == page->m_oneFramePath.right(4))
        {
            QFile file(page->m_oneFramePath);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(temp, width * height * 2);
                file.close();
            }
        }
        else
        {
            QFile file(page->m_oneFramePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream textStream(&file);
                short tempValue = 0;
                for (int i = 0; i < width * height; i++)
                {
                    memcpy(&tempValue, &temp[2 * i], 2);
                    textStream << QString("%1").arg(QString::number(tempValue));
                    textStream << QString(", ");
                }
            }
        }
        page->m_saveOneFrame = false;
    }
}

void TempAlarmPage::TempCallback_V2(IRC_NET_HANDLE handle, IRC_NET_TEMP_INFO_CB* tempInfo, IRC_NET_TEMP_EXT_INFO_CB* extInfo, void* userData)
{
    TempAlarmPage* page = (TempAlarmPage*)userData;
    QMutexLocker locker(&page->m_tempMutex);
    /*static uint64_t time;
    qDebug() << (extInfo->utcTime - time);
    time = extInfo->utcTime;*/
   
    if (page->m_saveOneFrame)
    {
        if (".dat" == page->m_oneFramePath.right(4))
        {
            QFile file(page->m_oneFramePath);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(tempInfo->temp, tempInfo->width * tempInfo->height * 2);
                file.close();
            }
        }
        else
        {
            QFile file(page->m_oneFramePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream textStream(&file);
                short tempValue = 0;
                for (int i = 0; i < tempInfo->width * tempInfo->height; i++)
                {
                    memcpy(&tempValue, &tempInfo->temp[2 * i], 2);
                    textStream << QString("%1").arg(QString::number(tempValue));
                    textStream << QString(", ");
                }
            }
        }
        page->m_saveOneFrame = false;
    }
}

void TempAlarmPage::AlarmCallback(IRC_NET_HANDLE handle, int alarmType, void* alarmInfo, void* userData)
{
    TempAlarmPage* page = (TempAlarmPage*)userData;

    switch (alarmType)
    {
    case IRC_NET_ALARM_TEMP:
    case IRC_NET_ALARM_TEMP_RISE:
    {
        IRC_NET_TEMP_ALARM_INFO info;
        info = *(IRC_NET_TEMP_ALARM_INFO*)alarmInfo;
        page->GetTempAlarm(alarmType, info);
        page->SavePictureInfoForTemp(info);
    }
    break;
    case IRC_NET_ALARM_FIRE:
    case IRC_NET_ALARM_FIRE_PULSE:
    {
        IRC_NET_FIRE_ALARM_INFO info;
        info = *(IRC_NET_FIRE_ALARM_INFO*)alarmInfo;
        page->GetFireAlarm(alarmType, info);
        page->SavePictureInfoForFire(info);
    }
    break;
    case IRC_NET_ALARM_REGION_INTRUSION:
    {
        IRC_NET_REGION_INTRUSION_ALARM_INFO info;
        info = *(IRC_NET_REGION_INTRUSION_ALARM_INFO*)alarmInfo;
        page->GetRegionIntrusionAlarm(alarmType, info);
        page->SavePictureInfoForRegionIntrusion(info);
    }
    break;
    case IRC_NET_ALARM_LINE_INTRUSION:
    {
        IRC_NET_LINE_INTRUSION_ALARM_INFO info;
        info = *(IRC_NET_LINE_INTRUSION_ALARM_INFO*)alarmInfo;
        page->GetLineIntrusionAlarm(alarmType, info);
        page->SavePictureInfoForLineIntrusion(info);
    }
    break;
    case IRC_NET_ALARM_TEMP_DIFF:
    {
        IRC_NET_TEMP_DIFF_ALARM_INFO info;
        info = *(IRC_NET_TEMP_DIFF_ALARM_INFO*)alarmInfo;
        page->GetTempDiffAlarm(alarmType, info);
        page->SavePictureInfoForTempDiff(info);
    }
    break;
    case IRC_NET_ALARM_LOCAL:
    {
        IRC_NET_LOCAL_ALARM_INFO info;
        info = *(IRC_NET_LOCAL_ALARM_INFO*)alarmInfo;
        page->GetLocalAlarm(alarmType, info);
        page->SavePictureInfoForLocal(info);
    }
    break;
    case IRC_NET_ALARM_SMOKE_DETECT:
    {
        IRC_NET_SMOKE_DETECT_ALARM_INFO info;
        info = *(IRC_NET_SMOKE_DETECT_ALARM_INFO*)alarmInfo;
        page->GetSmokeDetectAlarm(alarmType, info);
        page->SavePictureInfoForSmokeDetect(info);
    }
    break;
    }
   
}
