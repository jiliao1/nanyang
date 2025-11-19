#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>
#include <QWidget>
#include <QPixmap>

// 海康SDK功能已禁用,使用其他方式获取图像

class VideoManager : public QObject
{
    Q_OBJECT
public:
    explicit VideoManager(QObject *parent = nullptr);
    ~VideoManager();

signals:
    // 用于向UI界面发送状态信息或错误消息的信号
    void videoStatusChanged(const QString &message);
    // 当解码器成功解码出一帧画面时,发出此信号
    void frameDecoded(const QPixmap &pixmap);

public slots:
    // 开始播放
    void startRealPlay(WId winId); // WId 是窗口句柄的Qt类型,可以跨平台
    // 停止播放
    void stopRealPlay();

private:
    // 初始化(保留接口,实际可用其他实现)
    bool init();
    // 清理
    void cleanup();
};

#endif // VIDEOMANAGER_H
