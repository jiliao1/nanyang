#pragma once
#include <QWidget>
#include "ui_AlarmPage.h"
#include "IRCNetSDKDef.h"
#include <QStandardItemModel>

class AlarmPage : public QWidget
{
    Q_OBJECT
public:
    AlarmPage(QWidget* parent = nullptr);
    ~AlarmPage();

    void AddTempAlarm(int alarmType, IRC_NET_TEMP_ALARM_INFO info);
    void AddFireAlarm(int alarmType, IRC_NET_FIRE_ALARM_INFO alarmInfo);
    void AddRegionIntrusionAlarm(int alarmType, IRC_NET_REGION_INTRUSION_ALARM_INFO alarmInfo);
    void AddLineIntrusionAlarm(int alarmType, IRC_NET_LINE_INTRUSION_ALARM_INFO alarmInfo);
    void AddTempDiffAlarm(int alarmType, IRC_NET_TEMP_DIFF_ALARM_INFO alarmInfo);
    void AddSmokeDetectAlarm(int alarmType, IRC_NET_SMOKE_DETECT_ALARM_INFO alarmInfo);
    void AddLocalAlarm(int alarmType, IRC_NET_LOCAL_ALARM_INFO alarmInfo);
    void ClearAlarm();

protected:
    void InitForm();
    QString GetAlarmType(int type);
    QString GetAlarmArea(int ruleType, int id);
    QString GetAlarmTemp(int type, const IRC_NET_TEMP_INFO& temp, int tempUnit);
    QString GetAlarmState(int state);
    QString GetThrestemp(float temp);

private:
    Ui::AlarmPage ui;
    QStandardItemModel* m_model = nullptr;
};
