#pragma once

#include <QWidget>
#include "ui_TempAlarmPage.h"
#include "IRCNetSDKDef.h"
#include <QMutex>
#include "AlarmPage.h"

class MainWindow;
class TempAlarmPage : public QWidget
{
    Q_OBJECT

public:
    TempAlarmPage(QWidget *parent = nullptr);
    ~TempAlarmPage();

    void SetWidgetEnable(bool enable) { ui.scrollAreaWidgetContents->setEnabled(enable); }
    void CloseAlarmPage() { m_alarmPage->close(); }
    void ClearUi();
    void InitButton();
    int GetCurPresetIndex() { return m_presetIndexMap[ui.presetCombo->currentIndex()].id; }
    void SetOnceTempEdit(float temp);
    void SetAlarmCallParamHide();
    void SetAlarmCallParamShow();

signals:
    void GetTempAlarm(int alarmType, IRC_NET_TEMP_ALARM_INFO alarmInfo);
    void GetFireAlarm(int alarmType, IRC_NET_FIRE_ALARM_INFO alarmInfo);
    void GetRegionIntrusionAlarm(int alarmType, IRC_NET_REGION_INTRUSION_ALARM_INFO alarmInfo);
    void GetLineIntrusionAlarm(int alarmType, IRC_NET_LINE_INTRUSION_ALARM_INFO alarmInfo);
    void GetTempDiffAlarm(int alarmType, IRC_NET_TEMP_DIFF_ALARM_INFO alarmInfo);
    void GetSmokeDetectAlarm(int alarmType, IRC_NET_SMOKE_DETECT_ALARM_INFO alarmInfo);
    void GetLocalAlarm(int alarmType, IRC_NET_LOCAL_ALARM_INFO alarmInfo);

protected:
    void InitForm();
    void ConnectSignalSlot();
    void ClearRoiRuleInfo();
    void RefreshRoiRuleInfo(const IRC_NET_TEMP_RULE_INFO& ruleInfo, const IRC_NET_TEMP_INFO& tempInfo);
    void RefreshAtrAlarmInfo(const IRC_NET_TEMP_RULE_INFO_G1& tempAlarmInfo);
    void RefreshAtrFrameAlarmInfo(const IRC_NET_FRAME_TEMP_ALARM_CONFIG_G1& tempAlarmInfo);
    void GetRoiRuleInfo(IRC_NET_TEMP_RULE_INFO& ruleInfo);
    void SetFrameTempAlarm(const IRC_NET_FRAME_TEMP_ALARM_CONFIG& alarmConfig);
    int GetTempRuleType(QString& str);
    QString GetRuleName(int type, int id);
    bool CheckAtrAlarmParam();
    void SavePictureInfoForFire(IRC_NET_FIRE_ALARM_INFO info);
    void SavePictureInfoForTemp(IRC_NET_TEMP_ALARM_INFO info);
    void SavePictureInfoForRegionIntrusion(IRC_NET_REGION_INTRUSION_ALARM_INFO info);
    void SavePictureInfoForLineIntrusion(IRC_NET_LINE_INTRUSION_ALARM_INFO info);
    void SavePictureInfoForTempDiff(IRC_NET_TEMP_DIFF_ALARM_INFO info);
    void SavePictureInfoForLocal(IRC_NET_LOCAL_ALARM_INFO info);
    void SavePictureInfoForSmokeDetect(IRC_NET_SMOKE_DETECT_ALARM_INFO info);

protected slots:
    void OnChangeHandle(IRC_NET_HANDLE handle);
    void OnGetFrameTempDataBtnClicked();
    void OnOpenTempStreamBtnClicked();
    void OnSaveOneFrameBtnClicked();
    void OnGetAllRulesBtnClicked();
    void OnDeleteAllRulesBtnClicked();
    void OnGetOneRuleInfoBtnClicked();
    void OnDeleteOneRuleInfoBtnClicked();
    void OnSetAlarmInfoBtnClicked();
    void OnOpenAlarmCallbackBtnClicked();
    void OnGetAlarmDataBtnClicked();
    void OnGetTempAlarm(int alarmType, IRC_NET_TEMP_ALARM_INFO alarmInfo);
    void OnGetFireAlarm(int alarmType, IRC_NET_FIRE_ALARM_INFO alarmInfo);
    void OnGetRegionIntrusionAlarm(int alarmType, IRC_NET_REGION_INTRUSION_ALARM_INFO alarmInfo);
    void OnGetLineIntrusionAlarm(int alarmType, IRC_NET_LINE_INTRUSION_ALARM_INFO alarmInfo);
    void OnGetTempDiffAlarm(int alarmType, IRC_NET_TEMP_DIFF_ALARM_INFO alarmInfo);
    void OnGetSmokeDetectAlarm(int alarmType, IRC_NET_SMOKE_DETECT_ALARM_INFO alarmInfo);
    void OnGetLocalAlarm(int alarmType, IRC_NET_LOCAL_ALARM_INFO alarmInfo);
    void OnpointOnceMeasureBtnClicked();
    void OnGetAllPreSetBtnClicked();
    void OnMoveToBtnClicked();
    void OnShieldedGetAllBtnClicked();
    void OnShieldedDeleteAllBtnClicked();
    void OnShieldedDeleteBtnClicked();
    void OnROISetBtnClicked();
    void OnShielded(int ruleType, QList<QPoint> points);
    void OnGetAtrAllRulesBtnClicked();
    void OnDeleteAtrAllBtnClicked();
    void OngetAtrOneRuleInfoBtnClicked();
    void OnDeleteAtrOneRuleInfoBtnClicked();
    void OnGetAtrOneAlarmInfoBtnClicked();
    void OnSetAtrOneAlarmInfoBtnClicked();
    void OnOpenAtrAlarmCallbackBtnClicked();
    void OnOpenAtrTempStreamBtnClicked();



protected:
    static void TempCallback(IRC_NET_HANDLE handle, char* temp, int width, int height, void* userData);
    static void TempCallback_V2(IRC_NET_HANDLE handle, IRC_NET_TEMP_INFO_CB* tempInfo, IRC_NET_TEMP_EXT_INFO_CB* extInfo, void* userData);
    static void AlarmCallback(IRC_NET_HANDLE handle, int alarmType, void* alarmInfo, void* userData);

private:
    Ui::TempAlarmPage ui;
    MainWindow* m_mainWindow = nullptr;
    IRC_NET_HANDLE m_handle = 0;
    QMap<int, IRC_NET_TEMP_RULE_INDEX> m_ruleIndexMap;
    QMap<int, IRC_NET_TEMP_RULE_INFO> m_ruleInfoMap;
    QMap<int, IRC_NET_TEMP_RULE_INFO_G1> m_ruleInfoMapG1;
    QMutex m_tempMutex;
    bool m_saveOneFrame = false;
    QString m_oneFramePath;
    AlarmPage* m_alarmPage = nullptr;
    QMap<int, IRC_NET_PTZ_PRESET_INFO> m_presetIndexMap;
    QMap<QString, IRC_NET_TEMP_MASK_INFO> m_shieldedMap;
    bool m_flagFire = true;
    bool m_flagTemp = true;
    bool m_flagRegion = true;
    bool m_flagLine = true;
    bool m_flagDiff = true;
    bool m_flagLocal = true;
    bool m_flagSmoke = true;
};
