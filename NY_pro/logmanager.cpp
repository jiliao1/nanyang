#include "logmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPoint>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "xlsxdocument.h" // 包含 QXlsx 的头文件
#pragma execution_character_set("utf-8")

// 在构造函数中初始化
LogManager::LogManager(const QString &logFilePath, QObject *parent)
    : QObject(parent), m_logFilePath(logFilePath)
{
    // 将最小时间设置为很久以前，以确保第一条日志能被记录
    m_lastLeakTimestamp = QDateTime::fromSecsSinceEpoch(0);

    // 注册自定义类型
    qRegisterMetaType<LeakReport>("LeakReport");

    // 程序启动时，从Excel加载历史记录
    loadReportsFromExcel();
}

LogManager::~LogManager()
{
}

// 核心处理逻辑
void LogManager::processIncomingData(const QByteArray &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isObject()) {
        emit statusMessage("Error: Received invalid JSON data.");
        return;
    }

    QJsonObject root = doc.object();
    bool leakDetected = root["leak_detected"].toBool(false);

    // --- 在 connect_status 中显示实时识别情况 ---
    QString robotPos = root["location"].toString("Unknown");
    QString statusText;
    if(leakDetected) {
        QJsonObject info = root["leak_info"].toObject();
        QString coords = info["coordinates"].toString("N/A");
        QString severity = info["severity"].toString("N/A");
        statusText = QString("机器人位置: %1 | 发现泄漏! | 坐标: %2 | 严重等级: %3")
                         .arg(robotPos, coords, severity);

        // --- 检查是否需要记录日志 (延时) ---
        qint64 secondsSinceLastLeak = m_lastLeakTimestamp.secsTo(QDateTime::currentDateTime());
        if (secondsSinceLastLeak < 30) {
            emit statusMessage(statusText + " (日志记录冷却中...)");
            return; // 60秒内不再记录
        }

        // --- 准备写入日志 ---
        LeakReport newReport;
        newReport.timestamp = QDateTime::currentDateTime();
        newReport.robotPosition = robotPos;
        QStringList coordParts = coords.split(',');
        if (coordParts.size() == 2) {
            newReport.leakCoordinates = QPoint(coordParts[0].trimmed().toInt(), coordParts[1].trimmed().toInt());
        }
        newReport.severity = severity;

        if(writeReportToExcel(newReport)) {
             m_reports.prepend(newReport); // 添加到内存缓存的顶部
             m_lastLeakTimestamp = newReport.timestamp; // 更新最后记录时间
             emit reportAdded(newReport); // 通知UI
             emit statusMessage("新泄漏事件已记录到Excel。");
        } else {
            emit statusMessage("错误：无法写入Excel日志文件！");
        }

    } else {
        statusText = QString("机器人位置: %1 | 一切正常。").arg(robotPos);
        emit statusMessage(statusText);
    }
}

// 写入Excel
bool LogManager::writeReportToExcel(const LeakReport &report)
{
    QXlsx::Document xlsx(m_logFilePath);

    // 如果文件不存在，先创建表头
    if (!QFile::exists(m_logFilePath)) {
        xlsx.write(1, 1, "时间戳");
        xlsx.write(1, 2, "机器人位置");
        xlsx.write(1, 3, "泄漏情况");
        xlsx.write(1, 4, "严重等级");
    }

    // 找到最后一行并写入新数据
    int newRow = xlsx.dimension().lastRow() + 1;
    xlsx.write(newRow, 1, report.timestamp.toString("yyyy-MM-dd hh:mm:ss"));
    xlsx.write(newRow, 2, report.robotPosition);
    xlsx.write(newRow, 3, QString("%1, %2").arg(report.leakCoordinates.x()).arg(report.leakCoordinates.y()));
    xlsx.write(newRow, 4, report.severity);

    if (xlsx.save()) {
        return true;
    }
    return false;
}

// 加载Excel
void LogManager::loadReportsFromExcel()
{
    m_reports.clear();
    QXlsx::Document xlsx(m_logFilePath);
    if (!QFile::exists(m_logFilePath)) return;

    // 从第二行开始读取数据 (第一行是表头)
    for (int row = 2; row <= xlsx.dimension().lastRow(); ++row) {
        LeakReport report;
        report.timestamp = QDateTime::fromString(xlsx.read(row, 1).toString(), "yyyy-MM-dd hh:mm:ss");
        report.robotPosition = xlsx.read(row, 2).toString();
        QStringList coordParts = xlsx.read(row, 3).toString().split(',');
        if (coordParts.size() == 2) {
            report.leakCoordinates = QPoint(coordParts[0].trimmed().toInt(), coordParts[1].trimmed().toInt());
        }
        report.severity = xlsx.read(row, 4).toString();
        m_reports.append(report); // 添加到缓存
    }
    // 反转列表，让最新的在最前面
    std::reverse(m_reports.begin(), m_reports.end());
    emit statusMessage(QString("从Excel加载了 %1 条历史记录。").arg(m_reports.count()));
}

const QList<LeakReport>& LogManager::getAllReports() const
{
    return m_reports;
}


// 实现获取路径的函数
QString LogManager::getLogFilePath() const
{
    return m_logFilePath;
}

