#ifndef REPORTTABLEMODEL_H
#define REPORTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
//#include "logmanager.h" // 包含 LeakReport 结构体的定义

class ReportTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ReportTableModel(QObject *parent = nullptr);

    // --- 必须重写的虚函数 ---
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public slots:
    // 用于从外部设置模型的完整数据源
    void setReports(const QList<LeakReport> &reports);
    // 用于在收到新日志时，在模型顶部添加一行
    void prependReport(const LeakReport &report);

private:
    QList<LeakReport> m_reports; // 模型内部存储的数据
    QStringList m_headers;       // 表头
};

#endif // REPORTTABLEMODEL_H
