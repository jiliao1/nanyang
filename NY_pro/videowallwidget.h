#ifndef VIDEOWALLWIDGET_H
#define VIDEOWALLWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMouseEvent>
#include <QList>

// 我们需要一个自定义的VideoWidget来轻松捕获双击事件
class ClickableVideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    explicit ClickableVideoWidget(QWidget *parent = nullptr) : QVideoWidget(parent) {}

signals:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        emit doubleClicked();
        QVideoWidget::mouseDoubleClickEvent(event);
    }
};


class VideoWallWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWallWidget(QWidget *parent = nullptr);
    ~VideoWallWidget();

public slots:
    void startPlayback();
    void stopAllPlayers();

private slots:
    void handleVideoDoubleClicked();
    // (新增) 处理播放器错误
    void handlePlayerError();

private:
    void setupUi();

    QGridLayout *m_gridLayout;
    QList<QMediaPlayer*> m_players;
    QList<ClickableVideoWidget*> m_videoWidgets;
    QWidget *m_zoomedWidget = nullptr;
    bool m_isPlaybackStarted = false;
};

#endif // VIDEOWALLWIDGET_H
