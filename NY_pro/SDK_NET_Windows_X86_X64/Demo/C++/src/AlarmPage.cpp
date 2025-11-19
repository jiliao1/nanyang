#include "AlarmPage.h"
#include <QDateTime>

AlarmPage::AlarmPage(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    m_model = new QStandardItemModel(ui.tableView);
    ui.tableView->setModel(m_model);
    InitForm();
}

AlarmPage::~AlarmPage()
{

}

void AlarmPage::AddTempAlarm(int alarmType, IRC_NET_TEMP_ALARM_INFO info)
{
    QList<QStandardItem*> itemList;
    QString channel = (alarmType == IRC_NET_ALARM_TEMP_RISE || alarmType == IRC_NET_ALARM_TEMP )? QString::number(info.alarmBaseInfo.channel) : "";
    itemList << new QStandardItem(channel) <<
        new QStandardItem(GetAlarmType(info.alarmRuleType)) <<
        new QStandardItem(QString::number(info.thresholdTemp)) <<
        new QStandardItem(QString::number(info.ruleTempInfo.ruleIndex.id)) <<
        new QStandardItem(QString::fromLocal8Bit(info.name) == "Frame" ? "Frame" : GetAlarmArea(info.ruleTempInfo.ruleIndex.type, info.ruleTempInfo.ruleIndex.id)) <<
        new QStandardItem(GetAlarmTemp(info.alarmRuleType,info.ruleTempInfo.tempInfo,info.tempUnit)) << 
        new QStandardItem(GetAlarmState(info.alarmBaseInfo.alarmAction)) << 
        new QStandardItem(QDateTime::fromSecsSinceEpoch(info.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::AddFireAlarm(int alarmType, IRC_NET_FIRE_ALARM_INFO alarmInfo)
{
    QList<QStandardItem*> itemList;
    itemList << new QStandardItem(QString::number(alarmInfo.alarmBaseInfo.channel)) <<
        new QStandardItem(GetAlarmType(alarmType)) <<
        new QStandardItem("") <<
        new QStandardItem(QString::number(alarmInfo.id)) <<
        new QStandardItem(QString::fromLocal8Bit(alarmInfo.name)) <<
        new QStandardItem("") <<
        new QStandardItem(GetAlarmState(alarmInfo.alarmBaseInfo.alarmAction)) <<
        new QStandardItem(QDateTime::fromSecsSinceEpoch(alarmInfo.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::AddRegionIntrusionAlarm(int alarmType, IRC_NET_REGION_INTRUSION_ALARM_INFO alarmInfo)
{
    QList<QStandardItem*> itemList;
    itemList << new QStandardItem(QString::number(alarmInfo.alarmBaseInfo.channel)) <<
        new QStandardItem(GetAlarmType(alarmType)) <<
        new QStandardItem("") <<
        new QStandardItem(QString::number(alarmInfo.id)) <<
        new QStandardItem(QString::fromLocal8Bit(alarmInfo.name)) <<
        new QStandardItem("") <<
        new QStandardItem(GetAlarmState(alarmInfo.alarmBaseInfo.alarmAction)) <<
        new QStandardItem(QDateTime::fromSecsSinceEpoch(alarmInfo.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::AddLineIntrusionAlarm(int alarmType, IRC_NET_LINE_INTRUSION_ALARM_INFO alarmInfo)
{
    QList<QStandardItem*> itemList;
    itemList << new QStandardItem(QString::number(alarmInfo.alarmBaseInfo.channel)) <<
        new QStandardItem(GetAlarmType(alarmType)) <<
        new QStandardItem("") <<
        new QStandardItem(QString::number(alarmInfo.id)) <<
        new QStandardItem(QString::fromLocal8Bit(alarmInfo.name)) <<
        new QStandardItem("") <<
        new QStandardItem(GetAlarmState(alarmInfo.alarmBaseInfo.alarmAction)) <<
        new QStandardItem(QDateTime::fromSecsSinceEpoch(alarmInfo.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::AddTempDiffAlarm(int alarmType, IRC_NET_TEMP_DIFF_ALARM_INFO alarmInfo)
{
    QList<QStandardItem*> itemList;
    itemList << new QStandardItem(QString::number(alarmInfo.alarmBaseInfo.channel)) <<
        new QStandardItem(GetAlarmType(alarmType)) <<
        new QStandardItem(GetThrestemp(alarmInfo.thresholdTemp)) <<
        new QStandardItem("") <<
        new QStandardItem(QString::fromLocal8Bit(alarmInfo.name)) <<
        new QStandardItem("") <<
        new QStandardItem(GetAlarmState(alarmInfo.alarmBaseInfo.alarmAction)) <<
        new QStandardItem(QDateTime::fromSecsSinceEpoch(alarmInfo.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::AddSmokeDetectAlarm(int alarmType, IRC_NET_SMOKE_DETECT_ALARM_INFO alarmInfo)
{
    QList<QStandardItem*> itemList;
    itemList << new QStandardItem(QString::number(alarmInfo.alarmBaseInfo.channel)) <<
        new QStandardItem(GetAlarmType(alarmType)) <<
        new QStandardItem("") <<
        new QStandardItem(QString::number(alarmInfo.id)) <<
        new QStandardItem(QString::fromLocal8Bit(alarmInfo.name)) <<
        new QStandardItem("") <<
        new QStandardItem(GetAlarmState(alarmInfo.alarmBaseInfo.alarmAction)) <<
        new QStandardItem(QDateTime::fromSecsSinceEpoch(alarmInfo.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::AddLocalAlarm(int alarmType, IRC_NET_LOCAL_ALARM_INFO alarmInfo)
{
    QList<QStandardItem*> itemList;
    itemList << new QStandardItem(QString::number(alarmInfo.alarmBaseInfo.channel)) <<
        new QStandardItem(GetAlarmType(alarmType)) <<
        new QStandardItem("") <<
        new QStandardItem(QString::number(alarmInfo.id)) <<
        new QStandardItem(QString::fromLocal8Bit("")) <<
        new QStandardItem("") <<
        new QStandardItem(GetAlarmState(alarmInfo.alarmBaseInfo.alarmAction)) <<
        new QStandardItem(QDateTime::fromSecsSinceEpoch(alarmInfo.alarmBaseInfo.timestamp, Qt::UTC).toString("yyyy-MM-dd hh:mm:ss"));
    m_model->appendRow(itemList);
}

void AlarmPage::ClearAlarm()
{
    m_model->removeRows(0, m_model->rowCount());
}

void AlarmPage::InitForm()
{
    m_model->setColumnCount(6);
    QStringList header;
    header << tr("Channel") << tr("Alarm Type") << tr("Alarm Threshold") << tr("Region ID") << tr("Region Name") << tr("Alarm Temp") << tr("Alram State") << tr("Datetime");
    m_model->setHorizontalHeaderLabels(header);
    ui.tableView->horizontalHeader()->setStretchLastSection(true);
}

QString AlarmPage::GetAlarmType(int type)
{
    QString ret;
    switch (type)
    {
    case IRC_NET_TEMP_ALARM_RULE_AVG_TEMP_GT:
        ret = tr("Avg Greater Than");
        break;
    case IRC_NET_TEMP_ALARM_RULE_AVG_TEMP_LT:
        ret = tr("Avg Less Than");
        break;
    case IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_GT:
        ret = tr("High Greater Than");
        break;
    case IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_LT:
        ret = tr("High Less Than");
        break;
    case IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_GT:
        ret = tr("Low Greater Than");
        break;
    case IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_LT:
        ret = tr("Low Less Than");
        break;
    case IRC_NET_ALARM_FIRE:
        ret = tr("Fire");
        break;
    case IRC_NET_ALARM_TEMP_RISE:
        ret = tr("Temp Rise");
        break;
    case IRC_NET_ALARM_REGION_INTRUSION:
        ret = tr("Region Intrusion");
        break;
    case IRC_NET_ALARM_LINE_INTRUSION:
        ret = tr("Line Intrusion");
        break;
    case IRC_NET_ALARM_TEMP_DIFF:
        ret = tr("Temp Diff");
        break;
    case IRC_NET_ALARM_SMOKE_DETECT:
        ret = tr("Smoke Detect");
        break;
    case IRC_NET_ALARM_LOCAL:
        ret = tr("Local Alarm");
        break;
    case IRC_NET_ALARM_FIRE_PULSE:
        ret = tr("Fire Pulse");
        break;
    default:
        break;
    }
    return ret;
}

QString AlarmPage::GetAlarmArea(int ruleType, int id)
{
    QString ret;
    switch (ruleType)
    {
    case IRC_NET_TEMP_RULE_POINT:
        ret = QString("P%1").arg(id);
        break;
    case IRC_NET_TEMP_RULE_LINE:
        ret = QString("L%1").arg(id);
        break;
    case IRC_NET_TEMP_RULE_RECT:
    case IRC_NET_TEMP_RULE_CIRCLE:
    case IRC_NET_TEMP_RULE_POLYGON:
        ret = QString("R%1").arg(id);
        break;
    default:
        break;
    }
    return ret;
}

QString AlarmPage::GetAlarmTemp(int type, const IRC_NET_TEMP_INFO& temp, int tempUnit)
{
    QString ret;
    switch (type)
    {
    case IRC_NET_TEMP_ALARM_RULE_AVG_TEMP_GT:
    case IRC_NET_TEMP_ALARM_RULE_AVG_TEMP_LT:
        ret = QString::number(temp.avgTemp, 'f', 2);
        break;
    case IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_GT:
    case IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_LT:
        ret = QString::number(temp.maxTemp, 'f', 2);
        break;
    case IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_GT:
    case IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_LT:
    case IRC_NET_ALARM_TEMP_RISE:
        ret = QString::number(temp.minTemp, 'f', 2);
        break;
    default:
        break;
    }
    switch (tempUnit)
    {
    case IRC_NET_TEMP_CENTIGRADE:
        ret += tr("C");
        break;
    case IRC_NET_TEMP_FAHRENHEIT:
        ret += tr("F");
        break;
    case IRC_NET_TEMP_KELVIN:
        ret += tr("K");
        break;
    default:
        break;
    }
    return ret;
}

QString AlarmPage::GetAlarmState(int state)
{
    QString ret;
    switch (state)
    {
    case IRC_NET_ALARM_ACTION_SINGLE:
        ret = tr("Single Alarm");
        break;
    case IRC_NET_ALARM_ACTION_START:
        ret = tr("Start Alarm");
        break;
    case IRC_NET_ALARM_ACTION_END:
        ret = tr("End Alarm");
        break;
    default:
        break;
    }
    return ret;
}

QString AlarmPage::GetThrestemp(float temp)
{
    return temp >= 0.1 ? QString::number(temp) : "";
}
