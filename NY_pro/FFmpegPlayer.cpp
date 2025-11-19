#include "ffmpegplayer.h"
#include <QDebug>
#include <QFile>


static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    QFile *file = reinterpret_cast<QFile*>(opaque);
    if (!file || !file->isOpen()) {
        return AVERROR_EOF;
    }
    qint64 bytesRead = file->read(reinterpret_cast<char*>(buf), buf_size);
    if (bytesRead == 0 && file->atEnd()) {
        return AVERROR_EOF; // End of file
    }
    return static_cast<int>(bytesRead);
}

// FFmpeg需要的回调函数，用于在数据流中跳转
static int64_t seek_packet(void *opaque, int64_t offset, int whence)
{
    QFile *file = reinterpret_cast<QFile*>(opaque);
    if (!file || !file->isOpen()) {
        return -1;
    }

    if (whence == AVSEEK_SIZE) {
        return file->size();
    }

    if (file->seek(offset)) {
        return file->pos();
    }

    return -1;
}
FFmpegPlayer::FFmpegPlayer(QObject *parent) : QObject(parent)
{
    // 在Qt5.15及以后版本，av_register_all() 已被弃用并且是空操作
    // avformat_network_init(); // 如果需要播放网络流，可以调用这个
}

FFmpegPlayer::~FFmpegPlayer()
{
    stop();
}

void FFmpegPlayer::start(const QString &filePath)
{
    if (!m_isStopped) {
        return; // 已经在运行
    }
    m_filePath = filePath;
    m_isStopped = false;

    // QtConcurrent::run(this, &FFmpegPlayer::run);
    // 为了简化，我们先在当前线程运行，但推荐用QtConcurrent或QThread
    run();
}

void FFmpegPlayer::stop()
{
    m_isStopped = true;
}

void FFmpegPlayer::run()
{
    // --- 1. 初始化和打开文件 ---
//    QFile qrcFile(m_filePath);
//        if (!qrcFile.open(QIODevice::ReadOnly)) {
//            qWarning() << "Qt: Could not open resource file:" << m_filePath;
//            goto cleanup;
//        }

//        m_formatCtx = avformat_alloc_context();

//        // 分配自定义I/O的缓冲区
//        const int bufferSize = 4096;
//        unsigned char *avioBuffer = (unsigned char*)av_malloc(bufferSize);
//        AVIOContext *avioCtx = avio_alloc_context(avioBuffer, bufferSize, 0,
//                                                  &qrcFile, &read_packet, nullptr, &seek_packet);

//        m_formatCtx->pb = avioCtx;

//        if (avformat_open_input(&m_formatCtx, nullptr, nullptr, nullptr) != 0) {
//            qWarning() << "FFmpeg: Couldn't open input via custom I/O for" << m_filePath;
//            goto cleanup;
//        }

//        if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
//            qWarning() << "FFmpeg: Couldn't find stream information.";
//            goto cleanup;
//        }

//    // --- 2. 寻找视频流和解码器 ---
//    m_videoStreamIndex = av_find_best_stream(m_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
//    if (m_videoStreamIndex < 0) {
//        qWarning() << "FFmpeg: Couldn't find a video stream.";
//        goto cleanup;
//    }

//    AVCodecParameters *codecPar = m_formatCtx->streams[m_videoStreamIndex]->codecpar;
//     const AVCodec *codec = avcodec_find_decoder(codecPar->codec_id);
//    if (!codec) {
//        qWarning() << "FFmpeg: Codec not found.";
//        goto cleanup;
//    }

//    m_codecCtx = avcodec_alloc_context3(codec);
//    if (avcodec_parameters_to_context(m_codecCtx, codecPar) < 0) {
//        qWarning() << "FFmpeg: Couldn't copy codec context.";
//        goto cleanup;
//    }

//    if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
//        qWarning() << "FFmpeg: Couldn't open codec.";
//        goto cleanup;
//    }

//    // --- 3. 初始化帧和图像转换器 ---
//    m_frame = av_frame_alloc();
//    m_frameRGB = av_frame_alloc();

//    int width = m_codecCtx->width;
//    int height = m_codecCtx->height;

//    // 分配图像缓冲区
//    uint8_t *buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, width, height, 1));
//    av_image_fill_arrays(m_frameRGB->data, m_frameRGB->linesize, buffer, AV_PIX_FMT_RGB32, width, height, 1);

//    m_swsCtx = sws_getContext(width, height, m_codecCtx->pix_fmt,
//                              width, height, AV_PIX_FMT_RGB32,
//                              SWS_BILINEAR, nullptr, nullptr, nullptr);

//    m_packet = av_packet_alloc();

//    // --- 4. 解码循环 ---
//    while (!m_isStopped && av_read_frame(m_formatCtx, m_packet) >= 0) {
//        if (m_packet->stream_index == m_videoStreamIndex) {
//            if (avcodec_send_packet(m_codecCtx, m_packet) == 0) {
//                while (avcodec_receive_frame(m_codecCtx, m_frame) == 0) {
//                    // 转换图像格式
//                    sws_scale(m_swsCtx, (const uint8_t* const*)m_frame->data, m_frame->linesize, 0, height,
//                              m_frameRGB->data, m_frameRGB->linesize);

//                    // 创建QImage
//                    QImage img(m_frameRGB->data[0], width, height, QImage::Format_RGB32);
//                    emit frameReady(img.copy()); // 发送信号，用copy确保线程安全

//                    // 简单的延时以匹配帧率
//                    QThread::msleep(33);
//                }
//            }
//        }
//        av_packet_unref(m_packet);
//    }

//    // 如果是循环播放，可以seek到开头
//    // av_seek_frame(m_formatCtx, m_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
//    // goto start_decoding_loop;


//cleanup:
//    // --- 5. 清理资源 ---
//    m_isStopped = true;
//    if (m_codecCtx) avcodec_free_context(&m_codecCtx);
//    if (m_formatCtx) avformat_close_input(&m_formatCtx);
//    if (m_frame) av_frame_free(&m_frame);
//    if (m_frameRGB) av_frame_free(&m_frameRGB); // buffer也会被一并释放
//    if (m_packet) av_packet_free(&m_packet);
//    if (m_swsCtx) sws_freeContext(m_swsCtx);

//    emit finished();
}
