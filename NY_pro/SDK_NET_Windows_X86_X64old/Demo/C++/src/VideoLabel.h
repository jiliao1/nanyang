#pragma once
#include <QWidget>
#include <QMouseEvent>
#include <QPaintEvent>

class VideoLabel : public QWidget
{
    Q_OBJECT
public:
    enum DrawMode
    {
        NO_DRAW = -1,
        DRAW_POINT,
        DRAW_LINE,
        DRAW_AREA,
        DRAW_CIRCLE,
        DRAW_POLYGON,
        DRAW_POSITION,
        DRAW_SHIELDED,
        DRAW_ONCE,
        DRAW_FOCUS,
        DRAW_MANUAL
    };

    VideoLabel(QWidget* parent = nullptr);
    ~VideoLabel();
    void SetDrawMode(int mode);
    void SetImg(const QImage& img);
    void ResetParam();

signals:
    void AddRule(int ruleType, QList<QPoint> points);
    void GetOncePointTemp(QPoint point);
    void Postion(int ruleType, QList<QPoint> points);
    void RegionFocus(QList<QPoint> points);
    void ManualTrack(QList<QPoint> points);
    void AddShielded(int ruleType, QList<QPoint> points);
protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;

private:
    int m_drawMode = -1;
    QImage m_img;
    bool m_imgMove = false;
    int m_imgStartPos = 0;
    int m_stepLength = 0;
    QPoint m_lastPoint;
    
    QList<QPoint> m_drawPoints;
};
