#ifndef DATABASE_H
#define DATABASE_H
#pragma once
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

 //#include <log4cplus/log4cplus.h>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTimer>
#include <QTime>
#include <QDebug>
#include <QTableView>
#include <QHeaderView>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSortFilterProxyModel>
#include <mutex>
#include <QSqlError>
#include <QCoreApplication>
//#include <log4cplus/log4cplus.h>
class dataBase : public QObject
{
    Q_OBJECT
public:
    dataBase();
    ~dataBase();
    QString m_connectionName;
private:
    QSqlDatabase db; // 数据库连接对象

    /**
         * @brief 内部函数：检查并确保数据库连接是打开的
         * @return true 连接成功或已连接; false 连接失败
         */
    bool openDatabase();

signals:


public slots:

    void ShowUserDB(QTableView* tv_user, QString name, QString purview, QString situation);
    bool exchangeuser( QString username, QString time, QString info);
    bool ExchangeUserPur(QString username, QString time, QString purview);
    bool exchangeUserpwd(QString username, QString time);
    bool deleteUser(QString username, QString time);

    void showReport(QTableView* tv_rp, QString type, QString start_time, QString end_time, QString result);
    void showRpmore(QTableView* tv_rps, QString id);


    void taskmore(QTableView* tv_coms, QString start_time, QString end_time);
    bool deletetask(QString id);

    bool add_task(QString id, QString name, QString type, QString startday, QString starttime, QString endtime, QString situation);
    QString getLastInsertId(); // 获取最后插入的自增ID

    void insertreport(QString task_name, QString start_time, QString status, QString operator_name, QString notes = "");
    QString GetTheSpendTime(QString, QString);
    /**
        * @brief [新函数] 向 report_details 表插入一条巡检点明细
        * @param task_id 关联的当前任务ID
        * @param point_id 巡检点编号 (例如 "1", "2", "开始")
        * @param equipment_name 设备名称
        * @param equipment_data 设备数据 (例如 "0.51MPa")
        * @param status 设备状态 (例如 "正常", "疑似故障")
        * @param image_path 识别图片的保存路径 (可为空)
        * @param video_path 异常视频的保存路径 (可为空)
        * @return true 插入成功, false 失败
        */
       bool add_report_detail(const QString &task_id, const QString &point_id,
                                const QString &equipment_name, const QString &equipment_data,
                                const QString &status, const QString &image_path, const QString &video_path);
};

#endif // DATABASE_H
