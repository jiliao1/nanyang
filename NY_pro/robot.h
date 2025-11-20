#ifndef ROBOT_H
#define ROBOT_H

#include <QMainWindow>
#include <QCloseEvent>
#include "videomanager.h"
#include "overlaywidget.h"
#include "videowallwidget.h"
// IRC SDK 已禁用 - 使用其他方式获取图像
#include <QWebSocket>
#include "database.h"
#include "signup.h"
#include "xlsxdocument.h"    // QXlsx 核心类
#include "xlsxformat.h"      // 用于设置单元格格式（例如加粗、颜色）
#include <QFileDialog>       // 用于弹出“另存为”对话框
#include <QDesktopServices>  // 用于“是否打开文件”
#include <QUrl>
#include <QAbstractItemModel>
#include <QMessageBox>
#include <QFileInfo>         // 新增：用于获取文件名
#include <QJsonDocument>     // 新增：用于解析JSON
#include <QJsonObject>       // 新增：用于解析JSON
#include <QFile>             // 新增：用于读取文件
#include "ipconnect.h"
#include <QGraphicsScene>
#include <QList>
#include <QPointF>        // [新] 包含 QPointF
#include <QMouseEvent>    // [新] 包含 QMouseEvent (用于 Qt::MouseButton)
#include <QTcpSocket>     // TCP直连替代HTTP
#include <QTimer>         // 定时器
#include <QDateTime>      // 用于超时检测
#include "pointcloudviewwidget.h"  // 点云视图Widget


class QSortFilterProxyModel;
class VideoWallWidget;
class VideoManager;
class QGraphicsScene;     // [新] 前向声明
class QGraphicsView;

struct InspectionTarget {
    int id;
    double realX;
    double realY;
};

QT_BEGIN_NAMESPACE
namespace Ui { class robot; }
QT_END_NAMESPACE

class robot : public QMainWindow
{
    Q_OBJECT

public:
    robot(QWidget *parent = nullptr);
    ~robot();


signals:
    void newInfraredFrameReady(const QPixmap &pixmap);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_btn_startVideo_clicked();

    void on_btn_quit_clicked();

    void on_connectButton_clicked();



    //数据库


    void showReport();

    void show_rpmore();

    void show_rpprint();

    void PrintExcel(QTableView *tableView, QString &filename);
    // 筛选框变化时的槽函数
    void onFilterSeverityChanged(const QString &text);

    void on_simulateDataButton_clicked();

    void on_startInfrared_clicked();

    void on_Btn_BForward_pressed();

    void on_Btn_BForward_released();

    void on_Btn_BLeft_pressed();

    void on_Btn_BLeft_released();

    void on_Btn_BRight_pressed();

    void on_Btn_BRight_released();

    void on_Btn_BBack_pressed();

    void on_Btn_BBack_released();

    void on_Btn_ptzUp_pressed();

    void on_Btn_ptzUp_released();

    void on_Btn_ptzLeft_pressed();

    void on_Btn_ptzLeft_released();

    void on_Btn_ptzRight_pressed();

    void on_Btn_ptzRight_released();

    void on_Btn_ptzDown_pressed();

    void on_Btn_ptzDown_released();

    void on_slider_ptzZoom_valueChanged(int value);

    void on_Btn_missionStartORstop_clicked();

    void on_Btn_PlanSelect_clicked();

    void onWebSocketConnected();

    void onWebSocketDisconnected();

    void onWebMessageReceived(const QString &message);



    void on_loadMapButton_clicked();



    void on_btn_UndoLastPoint_clicked();

    void on_btn_ClearAllPoints_clicked();

    void updateInfraredLabel(const QPixmap &pixmap);

    // TCP视频流相关槽函数 - 旧的HTTP函数已移除
    void checkVideoTimeout();         // 检查视频流超时
    void checkInfraredTimeout();      // 检查红外流超时
    void checkNavigationTimeout();    // 检查导航视频流超时

    void on_btn_SavePlan_clicked();


    void on_btn_AddNavPoint_clicked();

    void on_btn_AddInspectionPoint_clicked();

    void on_DetailTableView_doubleClicked(const QModelIndex &index);

    // 点云相关槽函数
    void onPointCloudMessageReceived(const QString &topic, const QJsonObject &msg);
    
    // 点云视图控制槽函数
    void onResetViewClicked();    // 重置视角
    void onSetPoseClicked();      // 2D Pose Estimate
    void onPoseEstimated(QVector3D position, double yaw);  // 位姿估计完成

private:
    Ui::robot *ui;
    VideoWallWidget *m_videoWall;
    VideoManager *m_videoManager;

    QMediaPlayer* m_player;
    QVideoWidget* m_videoWidget;
    QFile* m_mediaFile;
    QWebSocket *m_webSocket; // 添加WebSocket客户端成员
    QString m_webSocketUrl;
    QString m_robotHttpBaseUrl;
    void sendBaseControlCommand(double linear_x, double angular_z);
    void sendPtzScanControl(double pitch_speed, double yaw_speed);
    void sendPtzZoomControl(int zoom_value);
    
    // 控制指令发送定时器
    QTimer* m_baseControlTimer;    // 底盘控制定时器(10Hz)
    QTimer* m_ptzControlTimer;     // 云台控制定时器(5Hz)
    
    // 当前控制状态
    double m_currentLinearX;       // 当前线速度
    double m_currentAngularZ;      // 当前角速度
    double m_currentPitchSpeed;    // 当前pitch速度
    double m_currentYawSpeed;      // 当前yaw速度
    
    // 定时发送控制指令
    void sendBaseControlPeriodically();
    void sendPtzControlPeriodically();
    
    // --- 添加地图场景和路径点列表 ---
    QGraphicsScene *m_mapScene; // 用于承载 2D 地图的"世界"
    QList<QJsonObject> m_pointList; // 存储待发送的路径点
    QGraphicsEllipseItem *m_robotPositionMarker; //机器人位置
    
    // 点云视图
    PointCloudViewWidget *m_pointCloudView;  // 3D点云视图
    
    // TF变换数据存储
    struct TFTransform {
        QString parent_frame;
        QString child_frame;
        QVector3D translation;
        QQuaternion rotation;
        qint64 timestamp; // 毫秒级时间戳
    };
    QMap<QString, TFTransform> m_tfTransforms; // key: child_frame
    
    // 地图坐标转换参数
    float m_mapMinX;
    float m_mapMaxY;
    double m_mapPixelsPerMeter;
    bool m_mapIsLoaded; // 标记地图是否已加载

    // 路径点创建状态
    /**
    * @brief 存储从 /odom 收到的最新机器人位置 (世界坐标)
    */
    QPointF m_latestRobotPos_World;
    /**
    * @brief [新] 存储从 /odom 计算出的最新机器人朝向 (Yaw 弧度)
    */
    double m_latestRobotYaw;

    /**
    * @brief 存储云台在map坐标系中的位置（从TF变换获取）
    */
    QVector3D m_latestPtzPos_World;
    
    /**
    * @brief 存储云台的方向（四元数，从TF变换获取）
    */
    QQuaternion m_latestPtzRotation;
    
    /**
    * @brief 存储当前云台的缩放值
    */
    int m_currentPtzZoom;
    
    /**
    * @brief 记录最后一次收到机器人TF数据的时间戳
    */
    qint64 m_lastRobotTfTimestamp;
    
    /**
    * @brief 记录最后一次收到PTZ TF数据的时间戳
    */
    qint64 m_lastPtzTfTimestamp;
    
    int m_pointCounter;            // 路径点计数器


    void loadPcdMap(const QString &filePath);
    QPointF convertSceneToWorld(QPointF scenePos);
    QPointF convertWorldToScene(QPointF worldPos);
    /**
    * @brief [新] 辅助函数：将收到的四元数转为 Yaw 偏航角 (弧度)
    */
    double convertQuaternionToYaw(const QJsonObject& quat);

    /**
    * @brief [新] 辅助函数：在地图上绘制一个点 (替换旧 addPointToPath 的绘制部分)
    */
    void addPointToMap(const QPointF& scenePos, const QString& type, int id, double yaw);
    
    /**
     * @brief 发布初始位姿到ROS /initialpose话题
     */
    void publishInitialPose(const QVector3D &position, double yaw);
    
    /**
     * @brief 处理TF消息
     */
    void onTFMessageReceived(const QJsonObject &msg);
    
    /**
     * @brief 计算从 parent 到 child 的累积变换
     */
    bool computeTransform(const QString &parent, const QString &child, 
                         QVector3D &translation, QQuaternion &rotation);
    
    /**
     * @brief 查找从 parent 到 child 的变换链
     */
    bool findTransformChain(const QString &parent, const QString &child, QStringList &chain);

    // 数据报表页相关的模型


    QSortFilterProxyModel *m_proxyModel;
    signup* sp;
    dataBase* dbase;
    QString m_currentTaskId;         // 当前任务的ID
    QString m_currentTaskStartTime;  // 当前任务的开始时间
    QString m_currentPlanName;       // 当前加载的方案名称 (来自JSON)
    QString m_currentPlanType;       // 当前加载的方案类型 (来自JSON)

    // 路径发布相关
    QTimer* m_pathPublishTimer;      // 循环发布路径的定时器
    QJsonArray m_navPathPoints;      // 导航路径点（已排序）
    QJsonArray m_patrolPathPoints;   // 巡检路径点（已排序）
    QJsonArray m_allSortedPoints;    // 所有点合并排序后的数组
    void publishPathTopics();        // 发布路径话题的函数

    //摄像头 - 海康SDK已禁用
    void stopCameraStream();

    // TCP图像获取相关成员 - 替代HTTP方式
    QTcpSocket* m_videoTcpSocket;        // 视频TCP连接
    QTcpSocket* m_infraredTcpSocket;     // 红外TCP连接
    QTcpSocket* m_navigationTcpSocket;   // 导航视频TCP连接(第三路)
    QString m_videoHost;                 // Jetson IP地址
    quint16 m_videoPort;                 // 视频端口(9001)
    quint16 m_infraredPort;              // 红外端口(9002)
    quint16 m_navigationPort;            // 导航视频端口(8084)
    
    // JPEG流处理相关
    QByteArray m_videoBuffer;            // 视频数据缓冲
    QByteArray m_infraredBuffer;         // 红外数据缓冲
    QByteArray m_navigationBuffer;       // 导航视频数据缓冲
    QTimer* m_videoWatchdog;             // 视频流看门狗定时器
    QTimer* m_infraredWatchdog;          // 红外流看门狗定时器
    QTimer* m_navigationWatchdog;        // 导航视频流看门狗定时器
    QDateTime m_lastVideoFrame;          // 最后一帧视频时间
    QDateTime m_lastInfraredFrame;       // 最后一帧红外时间
    QDateTime m_lastNavigationFrame;     // 最后一帧导航视频时间
    
    // FFmpeg硬件解码相关成员
    struct AVCodecContext* m_videoCodecCtx;      // 视频解码器上下文
    struct AVCodecContext* m_infraredCodecCtx;   // 红外解码器上下文
    struct AVCodecContext* m_navigationCodecCtx; // 导航视频解码器上下文
    struct AVFrame* m_videoFrame;                // 视频解码帧
    struct AVFrame* m_infraredFrame;             // 红外解码帧
    struct AVFrame* m_navigationFrame;           // 导航视频解码帧
    struct AVFrame* m_videoSwFrame;              // 视频软件帧(硬件解码后传输到CPU)
    struct AVFrame* m_infraredSwFrame;           // 红外软件帧
    struct AVFrame* m_navigationSwFrame;         // 导航视频软件帧
    struct SwsContext* m_videoSwsCtx;            // 视频像素格式转换上下文
    struct SwsContext* m_infraredSwsCtx;         // 红外像素格式转换上下文
    struct SwsContext* m_navigationSwsCtx;       // 导航视频像素格式转换上下文
    struct AVPacket* m_videoPacket;              // 视频数据包
    struct AVPacket* m_infraredPacket;           // 红外数据包
    struct AVPacket* m_navigationPacket;         // 导航视频数据包
    bool m_videoHwAccelEnabled;                  // 视频硬件加速是否启用
    bool m_infraredHwAccelEnabled;               // 红外硬件加速是否启用
    bool m_navigationHwAccelEnabled;             // 导航视频硬件加速是否启用
    
    // FFmpeg初始化函数
    bool initFFmpegDecoder(AVCodecContext** codecCtx, AVFrame** frame, AVFrame** swFrame, 
                          AVPacket** packet, bool* hwAccelEnabled, const char* decoderName);
    void cleanupFFmpegDecoder(AVCodecContext** codecCtx, AVFrame** frame, AVFrame** swFrame, 
                             SwsContext** swsCtx, AVPacket** packet);
    QImage decodeJpegWithFFmpeg(const QByteArray& jpegData, AVCodecContext* codecCtx, 
                               AVFrame* frame, AVFrame* swFrame, SwsContext** swsCtx, 
                               bool hwAccelEnabled);
    
    // TCP流处理函数
    void processVideoData();             // 处理视频TCP数据
    void processInfraredData();          // 处理红外TCP数据
    void processNavigationData();        // 处理导航视频TCP数据
    void connectVideoStream();           // 连接视频TCP流
    void connectInfraredStream();        // 连接红外TCP流
    void connectNavigationStream();      // 连接导航视频TCP流
    
    // 点云数据处理
    void subscribeToPointClouds();       // 订阅点云话题
    void parsePointCloud2(const QJsonObject &msg, QVector<QVector3D> &points, QVector<QVector3D> &colors);

    // --- 红外SDK已禁用,使用其他方式 ---
    void stopInfraredStream();       // 停止红外流的函数
    
    // 表计读数相关
    void subscribeToMeterReading();           // 订阅表计读数话题
    void onMeterReadingReceived(float reading); // 处理收到的表计读数
    bool m_isMeterReadingSubscribed;          // 是否已订阅表计读数话题
    QList<float> m_meterReadings;             // 存储收到的表计读数
    
    // 数据库测试
    void testDatabaseConnection();            // 测试数据库连接和写入

    // --- 新增：老板特供版地图变量 ---

    // A. 存储方案中所有点的真实坐标 (从JSON加载进这里)
    QList<InspectionTarget> m_currentPlanTargets;
};
#endif // ROBOT_H
