#ifndef FFMPEGPLAYER_H
#define FFMPEGPLAYER_H

#include <QObject>
#include <QImage>
#include <QThread>

// 因为FFmpeg的头文件是C语言风格的，需要用 extern "C" 来包裹
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class FFmpegPlayer : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegPlayer(QObject *parent = nullptr);
    ~FFmpegPlayer();

signals:
    // 当解码出一帧画面时，发出此信号
    void frameReady(const QImage &frame);
    // 当播放结束或发生错误时发出
    void finished();

public slots:
    // 开始解码指定路径的视频文件
    void start(const QString &filePath);
    // 停止解码并清理资源
    void stop();

private:
    void run(); // 解码循环

    // FFmpeg相关的上下文和数据结构
    AVFormatContext *m_formatCtx = nullptr;
    AVCodecContext *m_codecCtx = nullptr;
    AVFrame *m_frame = nullptr;
    AVFrame *m_frameRGB = nullptr;
    AVPacket *m_packet = nullptr;
    SwsContext *m_swsCtx = nullptr;
    int m_videoStreamIndex = -1;

    // 控制解码循环的标志
    volatile bool m_isStopped = true;
    QString m_filePath;
};

#endif // FFMPEGPLAYER_H
