#include "reporttablemodel.h"
#include <QColor>
#pragma execution_character_set("utf-8")

ReportTableModel::ReportTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // 初始化表头
    m_headers << "时间戳" << "机器人位置" << "泄漏坐标" << "严重等级";
}

int ReportTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_reports.count();
}

int ReportTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_headers.count();
}

QVariant ReportTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_reports.count()) {
        return QVariant();
    }

    const LeakReport &report = m_reports.at(index.row());

    // --- 用于显示文本的逻辑 ---
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0: return report.timestamp.toString("yyyy-MM-dd hh:mm:ss");
            case 1: return report.robotPosition;
            case 2: return QString("(%1, %2)").arg(report.leakCoordinates.x()).arg(report.leakCoordinates.y());
            case 3: return report.severity;
            default: return QVariant();
        }
    }

    // --- 用于设置单元格背景色的逻辑 ---
    if (role == Qt::BackgroundColorRole && index.column() == 3) {
        if (report.severity == "高") {
            return QColor(255, 107, 107, 150); // 淡红色
        } else if (report.severity == "中") {
            return QColor(255, 229, 107, 150); // 淡黄色
        } else if (report.severity == "低") {
            return QColor(107, 255, 128, 150); // 淡绿色
        }
    }

    return QVariant();
}

QVariant ReportTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return m_headers.at(section);
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

void ReportTableModel::setReports(const QList<LeakReport> &reports)
{
    beginResetModel(); // 通知视图：模型即将被完全重置
    m_reports = reports;
    endResetModel();   // 通知视图：重置完成，请刷新
}

void ReportTableModel::prependReport(const LeakReport &report)
{
    beginInsertRows(QModelIndex(), 0, 0); // 通知视图：我们准备在顶部插入一行
    m_reports.prepend(report);
    endInsertRows(); // 通知视图：插入完成
}
