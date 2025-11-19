#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QList>
#include <QRect>
#include <QPainter>

class OverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = nullptr);

public slots:
    // 一个公共槽，用于从外部更新需要绘制的方框列表
    void setDetectionBoxes(const QList<QRect> &boxes);

protected:
    // 重写 paintEvent 函数，所有绘制都在这里进行
    void paintEvent(QPaintEvent *event) override;

private:
    QList<QRect> m_detectionBoxes; // 存储要绘制的方框
};

#endif // OVERLAYWIDGET_H
