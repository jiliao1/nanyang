#include "VideoLabel.h"
#include <QPainter>
#include <QDebug>
#include "IRCNetSDKDef.h"
#include <cmath>

VideoLabel::VideoLabel(QWidget* parent)
    : QWidget(parent)
{
}

VideoLabel::~VideoLabel()
{
}

void VideoLabel::SetDrawMode(int mode)
{
    m_drawPoints.clear();
    m_drawMode = mode;
}

void VideoLabel::SetImg(const QImage& img)
{
    if (false == m_imgMove && !img.isNull() && img.width() / img.height() > 5)
    {
        m_imgMove = true;
        m_stepLength = img.height() * 1.25;
    }
    m_img = img;
    update();
}

void VideoLabel::ResetParam()
{
    m_imgMove = false;
    m_imgStartPos = 0;
    m_stepLength = 0;
    m_img = QImage();
}

void VideoLabel::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    if (m_img.isNull())
    {
        return;
    }

    QPainter painter(this);
    if (!m_imgMove)
    {
        painter.drawPixmap(rect(), QPixmap::fromImage(m_img));
    }
    else
    {
        if (m_imgStartPos < 0)
        {
            m_imgStartPos = 0;
        }
        else if (m_imgStartPos > (m_img.width() - m_stepLength - 1))
        {
            m_imgStartPos = m_img.width() - m_stepLength - 1;
        }
        QImage img = m_img.copy(m_imgStartPos, 0, m_stepLength, m_img.height());
        painter.drawPixmap(rect(), QPixmap::fromImage(img));
    }
    painter.setPen(Qt::green);
    switch (m_drawMode)
    {
    case DRAW_LINE:
    {
        if (2 == m_drawPoints.size())
        {
            painter.drawLine(m_drawPoints.at(0), m_drawPoints.at(1));
        }
    }
    break;
    case DRAW_AREA:
        if (2 == m_drawPoints.size())
        {
            painter.drawRect(QRect(m_drawPoints.at(0), m_drawPoints.at(1)));
        }
        break;
    case DRAW_CIRCLE:
        if (2 == m_drawPoints.size())
        {
            int len = sqrt(pow(m_drawPoints.at(0).x() - m_drawPoints.at(1).x(), 2) + pow(m_drawPoints.at(0).y() - m_drawPoints.at(1).y(), 2));
            painter.drawEllipse(m_drawPoints.at(0), len, len);
        }
        break;
    case DRAW_POLYGON:
        if (m_drawPoints.size() > 1)
        {
            for (int i = 0; i < m_drawPoints.size() - 1; i++)
            {
                painter.drawLine(m_drawPoints.at(i), m_drawPoints.at(i + 1));
            }
        }
        break;
    case DRAW_POSITION:
    case DRAW_FOCUS:
    case DRAW_MANUAL:
    case DRAW_SHIELDED:
        if (2 == m_drawPoints.size())
        {
            painter.drawRect(QRect(m_drawPoints.at(0), m_drawPoints.at(1)));
        }
        break;
    }
}

void VideoLabel::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton)
    {
        switch (m_drawMode)
        {
        case DRAW_POINT:
        case DRAW_LINE:
        case DRAW_AREA:
        case DRAW_CIRCLE:
        case DRAW_POSITION:
        case DRAW_FOCUS:
        case DRAW_MANUAL:
        case DRAW_SHIELDED:
        case DRAW_ONCE:
        {
            m_drawPoints.clear();
            m_drawPoints.append(event->pos());
        }
        break;
        case DRAW_POLYGON:
        {
            if (m_drawPoints.size() < 8)
            {
                m_drawPoints.append(event->pos());
            }
        }
        break;
        case NO_DRAW:
        {
            if (m_imgMove)
            {
                m_lastPoint = event->pos();
            }
        }
        }
    }

}

void VideoLabel::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    switch (m_drawMode)
    {
    case DRAW_LINE:
    case DRAW_AREA:
    case DRAW_CIRCLE:
    case DRAW_POSITION:
    case DRAW_FOCUS:
    case DRAW_MANUAL:
    case DRAW_SHIELDED:
    {
        if (1 == m_drawPoints.size())
        {
            m_drawPoints.append(event->pos());
        }
        else if (2 == m_drawPoints.size())
        {
            m_drawPoints[1] = event->pos();
        }
    }
    break;
    case NO_DRAW:
    {
        if (m_imgMove)
        {
            m_imgStartPos += (event->pos().x() - m_lastPoint.x()) * -1;
            m_lastPoint = event->pos();
        }
    }
        
    }
}

void VideoLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton)
    {
        switch (m_drawMode)
        {
        case DRAW_POINT:
        {
            if (1 == m_drawPoints.size())
            {
                emit AddRule(m_drawMode, m_drawPoints);
                m_drawPoints.clear();
            }
        }
        break;
        case DRAW_ONCE:
        {
            if (1 == m_drawPoints.size())
            {
                emit GetOncePointTemp(m_drawPoints[0]);
                m_drawPoints.clear();
            }
        }
         break;
        case DRAW_LINE:
        case DRAW_AREA:
        case DRAW_CIRCLE:
        {
            if (2 == m_drawPoints.size())
            {
                emit AddRule(m_drawMode, m_drawPoints);
                m_drawPoints.clear();
            }
        }
        break;
        case DRAW_POSITION:
        {
            if (2 == m_drawPoints.size())
            {
                emit Postion(m_drawMode, m_drawPoints);
                m_drawPoints.clear();
            }
        }
        break;
        case DRAW_FOCUS:
        {
            if (2 == m_drawPoints.size())
            {
                emit RegionFocus(m_drawPoints);
                m_drawPoints.clear();
            }
        }
        break;
        case DRAW_MANUAL:
        {
            if (2 == m_drawPoints.size())
            {
                emit ManualTrack(m_drawPoints);
                m_drawPoints.clear();
            }
        }
        break;
        case DRAW_SHIELDED:
        {
            if (2 == m_drawPoints.size())
            {
                emit AddShielded(m_drawMode, m_drawPoints);
                m_drawPoints.clear();
            }
        }
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        if (DRAW_POLYGON == m_drawMode && m_drawPoints.size() > 1)
        {
            emit AddRule(IRC_NET_TEMP_RULE_POLYGON, m_drawPoints);
            m_drawPoints.clear();
        }
    }
}
