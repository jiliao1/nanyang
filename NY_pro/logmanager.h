#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QPoint>
#include <QList>
#include <QVariant>

// 定义一个结构体来清晰地表示一条泄漏报告
struct LeakReport
{
    QDateTime timestamp;
    QString robotPosition;
    QPoint leakCoordinates;
    QString severity; // "High", "Medium", "Low"
};
// 为了能在信号槽中传递，需要注册
Q_DECLARE_METATYPE(LeakReport)

class LogManager : public QObject
{
    Q_OBJECT
public:
    explicit LogManager(const QString& logFilePath, QObject *parent = nullptr);
    ~LogManager();

    // 获取所有已加载的报告
    const QList<LeakReport>& getAllReports() const;
    //获取日志文件的完整路径
    QString getLogFilePath() const;

signals:
    // 当有新的报告被成功添加后，发出此信号通知UI更新
    void reportAdded(const LeakReport &report);
    // 用于向UI发送状态消息，比如"成功写入一条新日志"
    void statusMessage(const QString &message);

public slots:
    // 核心槽函数，用于处理从网络或其他地方传入的原始数据
    void processIncomingData(const QByteArray &jsonData);

private:
    // 将一条报告写入到Excel文件中
    bool writeReportToExcel(const LeakReport &report);
    // 从Excel文件中加载所有历史报告
    void loadReportsFromExcel();

    QString m_logFilePath;          // Excel日志文件的路径
    QDateTime m_lastLeakTimestamp;  // 用于记录上次泄漏的时间，以实现延时
    QList<LeakReport> m_reports;    // 内存中的报告缓存，用于UI快速显示
};

#endif // LOGMANAGER_H
