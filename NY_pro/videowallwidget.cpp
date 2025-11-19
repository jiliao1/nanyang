#include "videowallwidget.h"
#include <QMediaContent>
#include <QUrl>
#include <QDebug>

VideoWallWidget::VideoWallWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

VideoWallWidget::~VideoWallWidget()
{
    stopAllPlayers();
}

void VideoWallWidget::setupUi()
{
    m_gridLayout = new QGridLayout(this);
    m_gridLayout->setSpacing(2);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);

    for (int i = 0; i < 4; ++i) {
        ClickableVideoWidget *videoWidget = new ClickableVideoWidget(this);
        QMediaPlayer *player = new QMediaPlayer(this, QMediaPlayer::VideoSurface);

        // 将播放器和显示窗口关联
        player->setVideoOutput(videoWidget);
        videoWidget->setStyleSheet("background-color: black;");

        // 连接信号
        connect(videoWidget, &ClickableVideoWidget::doubleClicked, this, &VideoWallWidget::handleVideoDoubleClicked);
        // (新增) 连接错误信号，方便调试
        connect(player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoWallWidget::handlePlayerError);

        m_videoWidgets.append(videoWidget);
        m_players.append(player);
    }

    m_gridLayout->addWidget(m_videoWidgets[0], 0, 0);
    m_gridLayout->addWidget(m_videoWidgets[1], 0, 1);
    m_gridLayout->addWidget(m_videoWidgets[2], 1, 0);
    m_gridLayout->addWidget(m_videoWidgets[3], 1, 1);

    this->setLayout(m_gridLayout);
}

void VideoWallWidget::startPlayback()
{
    if (m_isPlaybackStarted) return;
    m_isPlaybackStarted = true;

    // 确保这里的路径与你的res.qrc文件完全一致
    const QStringList paths = {
        "qrc:/videos/video4.mp4",
        "qrc:/videos/video1.mp4",
        "qrc:/videos/video3.mp4",
        "qrc:/videos/video2.mp4"
    };

    for (int i = 0; i < m_players.count(); ++i) {
        qDebug() << "Attempting to play:" << paths[i];
        m_players[i]->setMedia(QUrl(paths[i]));
        m_players[i]->play();
    }
}

void VideoWallWidget::stopAllPlayers()
{
    for(QMediaPlayer* player : m_players) {
        player->stop();
    }
}

void VideoWallWidget::handlePlayerError()
{
    QMediaPlayer *player = qobject_cast<QMediaPlayer*>(sender());
    qWarning() << "QMediaPlayer Error:" << player->errorString();
}

void VideoWallWidget::handleVideoDoubleClicked()
{
    auto *senderWidget = qobject_cast<ClickableVideoWidget*>(sender());
    if (!senderWidget) return;

    if (!m_zoomedWidget) {
        m_zoomedWidget = senderWidget;
        for (auto *widget : m_videoWidgets) {
            if (widget != m_zoomedWidget) widget->setVisible(false);
        }
        m_gridLayout->addWidget(m_zoomedWidget, 0, 0, 2, 2);
    } else {
        m_gridLayout->removeWidget(m_zoomedWidget);
        int index = m_videoWidgets.indexOf(qobject_cast<ClickableVideoWidget*>(m_zoomedWidget));
        int row = index / 2;
        int col = index % 2;
        m_gridLayout->addWidget(m_zoomedWidget, row, col, 1, 1);

        for (auto *widget : m_videoWidgets) {
            widget->setVisible(true);
        }
        m_zoomedWidget = nullptr;
    }
}
