#include "overlaywidget.h"
#include <QDebug>

OverlayWidget::OverlayWidget(QWidget *parent) : QWidget(parent)
{
    // 设置窗口属性，使其可以悬浮并且是透明的
    setAttribute(Qt::WA_TranslucentBackground);
    // 允许鼠标事件穿透，这样可以操作下面的视频控件（如果需要）
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void OverlayWidget::setDetectionBoxes(const QList<QRect> &boxes)
{
    m_detectionBoxes = boxes;
    update(); // 请求重绘，会触发 paintEvent
}

void OverlayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_detectionBoxes.isEmpty()) {
        return; // 如果没有检测框，直接返回
    }

    // 遍历所有检测框并绘制
    for (const QRect &box : m_detectionBoxes)
    {
        // 可以根据气体泄漏严重程度设置不同颜色
        // 这里我们先用固定的红色
        painter.setPen(QPen(Qt::red, 3)); // 红色，3像素宽的画笔
        painter.drawRect(box);

        // 你还可以在框旁边绘制文字
        painter.setFont(QFont("Arial", 14));
        painter.drawText(box.topLeft() + QPoint(0, -5), "Gas Leak!");
    }
}
