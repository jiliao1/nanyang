#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        // 设置黑色背景
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::black);
        setPalette(p);
    }

signals:
    void doubleClicked();

public slots:
    // 用于接收解码后的图像并重绘
    void onFrameReady(const QImage &frame)
    {
        m_frame = frame;
        update(); // 请求重绘
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        if (m_frame.isNull()) {
            return;
        }

        QPainter painter(this);
        // 将图像按比例缩放并绘制在控件中央
        painter.drawImage(this->rect(), m_frame, m_frame.rect());
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        emit doubleClicked();
        QWidget::mouseDoubleClickEvent(event);
    }

private:
    QImage m_frame;
};

#endif // VIDEOWIDGET_H
