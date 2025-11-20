#include "robot.h"
#include "ui_robot.h"
#include "overlaywidget.h"

#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QDebug>
#include <QRandomGenerator> // 用于生成随机数据
#include <QStyleOptionSlider> // 用于滑块样式
#include <QVBoxLayout>       // 用于布局
#include "videowallwidget.h"
#include <QImage>
#include <QPixmap>
#include <QJsonArray>
#include "mapinteractiveview.h"
#include <QTextStream>        // 用于读 PCD 文本头
#include <QDataStream>        // 用于读 PCD 二进制数据
#include <QGraphicsEllipseItem> // 用于绘制点
#include <cmath>              // 用于 log2 和 pow 函数
#include <QGraphicsLineItem>    // 用于绘制箭头
#include <QtMath>               // 包含 qAtan2 和 qSin/qCos
#include <QNetworkProxy>        // 用于设置WebSocket代理
#include <QMessageBox>          // 用于消息对话框
#include <QPushButton>          // 用于按钮控件
#include <QHBoxLayout>          // 用于水平布局
#include <QDateTime>            // 用于时间戳
#include <QSqlDatabase>         // 用于数据库连接测试
#include <QSqlError>            // 用于数据库错误信息
#include <QCoreApplication>     // 用于获取应用程序路径
#include <QQuaternion>          // 用于四元数运算

// FFmpeg头文件
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
}

#pragma execution_character_set("utf-8")
bool m_isSDKInitialized=false;



robot::robot(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::robot)
    , m_mapIsLoaded(false) // 初始化地图加载状态
    , m_latestRobotPos_World(0.0, 0.0)
    , m_latestRobotYaw(0.0)
    , m_latestPtzPos_World(0.0, 0.0, 0.0)
    , m_latestPtzRotation()
    , m_currentPtzZoom(1)  // 初始化为滑块最小值1，而不是zoom像素值16
    , m_pointCounter(1) // 初始化点计数器
    , m_robotPositionMarker(nullptr) // 初始化机器人位置标记
    , m_fakeGreenDot(nullptr) // 初始化拓扑地图绿点标记
    , m_videoTcpSocket(nullptr)
    , m_infraredTcpSocket(nullptr)
    , m_videoWatchdog(nullptr)
    , m_infraredWatchdog(nullptr)
    , m_baseControlTimer(nullptr)
    , m_ptzControlTimer(nullptr)
    , m_pathPublishTimer(nullptr)
    , m_currentLinearX(0.0)
    , m_currentAngularZ(0.0)
    , m_currentPitchSpeed(0.0)
    , m_currentYawSpeed(0.0)
{
    ui->setupUi(this);
    
    // 设置两个图像label居中对齐
    ui->label_video->setAlignment(Qt::AlignCenter);
    ui->label_infrared->setAlignment(Qt::AlignCenter);
    
    // 设置label的缩放模式
    ui->label_video->setScaledContents(false);
    ui->label_infrared->setScaledContents(false);
    
    // 初始化TCP图像获取 - 替代HTTP方式
    m_videoHost = "192.168.16.146";
    m_videoPort = 9001;       // 视频TCP端口
    m_infraredPort = 9002;    // 红外TCP端口
    
    m_videoTcpSocket = new QTcpSocket(this);
    m_infraredTcpSocket = new QTcpSocket(this);
    
    // 禁用代理,避免proxy type错误
    m_videoTcpSocket->setProxy(QNetworkProxy::NoProxy);
    m_infraredTcpSocket->setProxy(QNetworkProxy::NoProxy);
    
    // 创建看门狗定时器,每5秒检查一次是否有新帧
    m_videoWatchdog = new QTimer(this);
    m_infraredWatchdog = new QTimer(this);
    connect(m_videoWatchdog, &QTimer::timeout, this, &robot::checkVideoTimeout);
    connect(m_infraredWatchdog, &QTimer::timeout, this, &robot::checkInfraredTimeout);
    
    // 创建控制指令定时器
    m_baseControlTimer = new QTimer(this);
    m_ptzControlTimer = new QTimer(this);
    connect(m_baseControlTimer, &QTimer::timeout, this, &robot::sendBaseControlPeriodically);
    connect(m_ptzControlTimer, &QTimer::timeout, this, &robot::sendPtzControlPeriodically);
    
    // 初始化时间戳,避免看门狗立即超时
    m_lastVideoFrame = QDateTime::currentDateTime();
    m_lastInfraredFrame = QDateTime::currentDateTime();
    
    // TCP视频流信号连接
    connect(m_videoTcpSocket, &QTcpSocket::connected, this, [this](){
        qDebug() << "========== 视频TCP连接成功 ==========";
        qDebug() << "主机:" << m_videoHost << "端口:" << m_videoPort;
        ui->connect_status->append("视频流已连接: " + m_videoHost + ":" + QString::number(m_videoPort));
        m_lastVideoFrame = QDateTime::currentDateTime();
        m_videoWatchdog->start(5000);
    });
    connect(m_videoTcpSocket, &QTcpSocket::disconnected, this, [this](){
        qDebug() << "========== 视频TCP断开连接 ==========";
        m_videoWatchdog->stop();
        QTimer::singleShot(3000, this, &robot::connectVideoStream); // 3秒后重连
    });
    connect(m_videoTcpSocket, &QTcpSocket::readyRead, this, &robot::processVideoData);
    connect(m_videoTcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            [this](QAbstractSocket::SocketError error){
                qDebug() << "========== 视频TCP错误 ==========";
                qDebug() << "错误代码:" << error;
                qDebug() << "错误信息:" << m_videoTcpSocket->errorString();
                ui->connect_status->append("视频TCP错误: " + m_videoTcpSocket->errorString());
            });
    
    // TCP红外流信号连接
    connect(m_infraredTcpSocket, &QTcpSocket::connected, this, [this](){
        qDebug() << "========== 红外TCP连接成功 ==========";
        qDebug() << "主机:" << m_videoHost << "端口:" << m_infraredPort;
        ui->connect_status->append("红外流已连接: " + m_videoHost + ":" + QString::number(m_infraredPort));
        m_lastInfraredFrame = QDateTime::currentDateTime();
        m_infraredWatchdog->start(5000);
    });
    connect(m_infraredTcpSocket, &QTcpSocket::disconnected, this, [this](){
        qDebug() << "========== 红外TCP断开连接 ==========";
        m_infraredWatchdog->stop();
        QTimer::singleShot(3000, this, &robot::connectInfraredStream); // 3秒后重连
    });
    connect(m_infraredTcpSocket, &QTcpSocket::readyRead, this, &robot::processInfraredData);
    connect(m_infraredTcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
            [this](QAbstractSocket::SocketError error){
                qDebug() << "========== 红外TCP错误 ==========";
                qDebug() << "错误代码:" << error;
                qDebug() << "错误信息:" << m_infraredTcpSocket->errorString();
                ui->connect_status->append("红外TCP错误: " + m_infraredTcpSocket->errorString());
            });
    
    // 初始化FFmpeg解码器
    qDebug() << "===== 初始化FFmpeg解码器 =====";
    if (!initFFmpegDecoder(&m_videoCodecCtx, &m_videoFrame, &m_videoSwFrame, 
                          &m_videoPacket, &m_videoHwAccelEnabled, "mjpeg")) {
        qDebug() << "视频解码器初始化失败,将无法显示视频";
    }
    if (!initFFmpegDecoder(&m_infraredCodecCtx, &m_infraredFrame, &m_infraredSwFrame, 
                          &m_infraredPacket, &m_infraredHwAccelEnabled, "mjpeg")) {
        qDebug() << "红外解码器初始化失败,将无法显示红外";
    }
    m_videoSwsCtx = nullptr;
    m_infraredSwsCtx = nullptr;
    
    dbase = new dataBase();
    
    // 初始化仪表读数相关变量
    m_isMeterReadingSubscribed = false;
    sp = new signup();
    QDate date = QDate::currentDate();
    QTime time_1 = QTime::currentTime();
    ui->dateEdit_7->setDate(date);
    //ui->tm_starttime_2->setTime(time_1);开始时间使用上面的time_1
    connect(this, &robot::newInfraredFrameReady, this, &robot::updateInfraredLabel);
    
    // 设置label背景自动填充,避免显示残留
    ui->label_video->setAutoFillBackground(true);
    ui->label_infrared->setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, Qt::black);
    ui->label_video->setPalette(palette);
    ui->label_infrared->setPalette(palette);

    m_webSocket = new QWebSocket;
    
    // 禁用代理 - WebSocket不支持某些代理类型
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::NoProxy);
    m_webSocket->setProxy(proxy);
    
    // 先连接信号,再打开连接 - 避免错过connected信号
    connect(m_webSocket, &QWebSocket::connected, this, &robot::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &robot::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &robot::onWebMessageReceived);
    
    // 添加错误处理
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            [this](QAbstractSocket::SocketError error){
                qDebug() << "========== WebSocket错误 ==========";
                qDebug() << "错误代码:" << error;
                qDebug() << "错误信息:" << m_webSocket->errorString();
                ui->connect_status->append("WebSocket错误: " + m_webSocket->errorString());
            });
    
    IpConnect dialog(this); // 创建对话框

    // 设置默认值，方便测试 - rosbridge端口为8079
    dialog.setIpAddress("192.168.16.146");
    dialog.setPort("8079"); // rosbridge websocket端口

    // 以模态方式显示对话框，程序会在此暂停
    if (dialog.exec() == QDialog::Accepted)
    {
        // 如果用户点击了 "OK"
        QString ip = dialog.getIpAddress();
        QString port = dialog.getPort();

        // 检查输入是否为空
        if (ip.isEmpty() || port.isEmpty() || ip == "...") {
            QMessageBox::critical(this, "未连接", "IP 地址和端口号均不能为空。网络功能将不可用。");
            // (您可以在此处禁用所有网络相关的按钮)
        } else {
            // 组合成 WebSocket URL 并尝试连接
            m_webSocketUrl = "ws://" + ip + ":" + port;
            qDebug() << "准备连接到:" << m_webSocketUrl;
            m_webSocket->open(QUrl(m_webSocketUrl));
            ui->connect_status->append("正在连接: " + m_webSocketUrl);
            qDebug() << "WebSocket状态:" << m_webSocket->state();
            m_robotHttpBaseUrl = "http://" + ip + ":8080";//http服务器的ip和端口！！！必须确认
        }
    }
    else
    {
        // 如果用户点击了 "Cancel"
        QMessageBox::warning(this, "未连接", "已取消连接，网络功能将不可用。");
        // (您可以在此处禁用所有网络相关的按钮)
    }

    /*数据库*/

    // ===== [重要] 程序启动时清除所有任务状态 =====
    // 停止可能存在的定时器
    if (m_pathPublishTimer) {
        m_pathPublishTimer->stop();
        qDebug() << "程序启动：已停止可能存在的路径发布定时器";
    }
    
    // 清空所有路径数据
    m_allSortedPoints = QJsonArray();
    m_navPathPoints = QJsonArray();
    m_patrolPathPoints = QJsonArray();
    m_pointList.clear();
    
    // 重置任务相关变量
    m_pointCounter = 1;
    m_currentPlanName.clear();
    m_currentPlanType.clear();
    m_currentTaskId.clear();
    m_currentTaskStartTime.clear();
    
    // 初始化TF时间戳为0
    m_lastRobotTfTimestamp = 0;
    m_lastPtzTfTimestamp = 0;
    
    qDebug() << "程序启动：已清除所有任务状态和路径数据";
    
    // 任务开始按钮默认启用,点击时会弹出文件选择对话框
    ui->Btn_missionStartORstop->setEnabled(true);
    ui->Btn_missionStartORstop->setText("任务开始"); // 确保文字正确
    
    // 选择巡检方案按钮默认启用
    ui->Btn_PlanSelect->setEnabled(true);

    // 方案名称框只读，防止用户手动输入
    ui->InspectionPlanName->setReadOnly(true);
    
    // 清空方案名称，确保每次启动都是干净状态
    ui->InspectionPlanName->clear();
    
    // 初始化时启用所有点操作按钮，不需要先加载地图
    ui->btn_AddNavPoint->setEnabled(true);
    ui->btn_AddInspectionPoint->setEnabled(true);
    ui->btn_UndoLastPoint->setEnabled(false);  // 没有点时仍禁用撤回
    ui->btn_ClearAllPoints->setEnabled(false);  // 没有点时仍禁用清除
    ui->btn_SavePlan->setEnabled(false);  // 没有点时仍禁用保存

    connect(ui->DetailBtn, SIGNAL(clicked()), this, SLOT(show_rpmore()));
    connect(ui->inquireBtn, SIGNAL(clicked()), this, SLOT(showReport()));
    connect(ui->printBtn, SIGNAL(clicked()), this, SLOT(show_rpprint()));

    /*地图部分*/
    // --- 初始化地图场景 (Scene) ---
    m_mapScene = new QGraphicsScene(this); // 创建场景
    // 将 UI 上的“相机”(View) 和“世界”(Scene) 关联起来
    ui->mapGraphicsView->setScene(m_mapScene);
    // [可选] 启用鼠标拖拽平移和滚轮缩放
    ui->mapGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->mapGraphicsView->setRenderHint(QPainter::Antialiasing);
    ui->mapGraphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->mapGraphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->mapGraphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    ui->mapGraphicsView->setInteractive(true); // 启用交互
    ui->mapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mapGraphicsView->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    ui->mapGraphicsView->setCacheMode(QGraphicsView::CacheBackground);

    // --- 初始化点云3D视图 ---
    m_pointCloudView = new PointCloudViewWidget(this);
    
    // 创建工具栏按钮
    QPushButton *btnResetView = new QPushButton("重置视角", this);
    QPushButton *btnSetPose = new QPushButton("2D Pose Estimate", this);
    
    btnResetView->setFixedSize(100, 30);
    btnSetPose->setFixedSize(120, 30);
    
    // 创建工具栏布局
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->addWidget(btnResetView);
    toolbarLayout->addWidget(btnSetPose);
    toolbarLayout->addStretch();
    
    // 将点云视图和工具栏添加到导航地图标签页
    QVBoxLayout *guideLayout = new QVBoxLayout(ui->tab_guide);
    guideLayout->setContentsMargins(5, 5, 5, 5);
    guideLayout->setSpacing(5);
    guideLayout->addLayout(toolbarLayout);
    guideLayout->addWidget(m_pointCloudView);
    ui->tab_guide->setLayout(guideLayout);
    
    // 连接按钮信号
    connect(btnResetView, &QPushButton::clicked, this, &robot::onResetViewClicked);
    connect(btnSetPose, &QPushButton::clicked, this, &robot::onSetPoseClicked);
    
    // 连接点云更新信号（已注释输出以减少日志）
    connect(m_pointCloudView, &PointCloudViewWidget::pointCloudUpdated, 
            [this](const QString &topic, int pointCount){
        // ui->connect_status->append(QString("点云 %1 已更新: %2 点").arg(topic).arg(pointCount));
        Q_UNUSED(topic);
        Q_UNUSED(pointCount);
    });
    
    // 连接位姿估计信号
    connect(m_pointCloudView, &PointCloudViewWidget::poseEstimated,
            this, &robot::onPoseEstimated);
    
    qDebug() << "点云视图已初始化并添加到导航地图标签页";

    // 为变倍滑块安装事件过滤器，禁用点击跳转
    ui->slider_ptzZoom->installEventFilter(this);

    // ===== 数据库连接测试 =====
    testDatabaseConnection();
    
    // ===== 自动加载拓扑地图 =====
    initTopologyMap();

}

robot::~robot()
{
    stopCameraStream();
    stopInfraredStream();
    
    // 清理FFmpeg解码器资源
    cleanupFFmpegDecoder(&m_videoCodecCtx, &m_videoFrame, &m_videoSwFrame, 
                        &m_videoSwsCtx, &m_videoPacket);
    cleanupFFmpegDecoder(&m_infraredCodecCtx, &m_infraredFrame, &m_infraredSwFrame, 
                        &m_infraredSwsCtx, &m_infraredPacket);
    
    // 清理TCP连接
    if (m_videoTcpSocket) {
        m_videoTcpSocket->disconnectFromHost();
        m_videoTcpSocket->deleteLater();
    }
    if (m_infraredTcpSocket) {
        m_infraredTcpSocket->disconnectFromHost();
        m_infraredTcpSocket->deleteLater();
    }
    
    delete ui;
}

void robot::on_btn_startVideo_clicked()
{
    // 启动TCP视频流连接
    ui->connect_status->append("开始连接视频TCP流: " + m_videoHost + ":" + QString::number(m_videoPort));
    connectVideoStream();
}

void robot::stopCameraStream()
{
    // 停止视频获取
    if (m_videoWatchdog && m_videoWatchdog->isActive()) {
        m_videoWatchdog->stop();
    }
    if (m_videoTcpSocket && m_videoTcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_videoTcpSocket->disconnectFromHost();
        ui->connect_status->append("视频流已停止");
        ui->label_video->clear();
        ui->label_video->setText("视频已停止");
    }
    m_videoBuffer.clear();
}

void robot::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

}

void robot::on_btn_quit_clicked()
{
    robot::close();
}
//数据库部分


void robot::on_connectButton_clicked()
{
    // 连接到NVIDIA机器人的rosbridge WebSocket服务器
    // IP: 192.168.16.146, Port: 8079
    m_webSocket->open(QUrl("ws://192.168.16.146:8079"));
}

// 事件过滤器 - 禁用滑块点击跳转
bool robot::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->slider_ptzZoom && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton)
        {
            // 获取滑块手柄的矩形区域
            QStyleOptionSlider opt;
            opt.initFrom(ui->slider_ptzZoom);
            opt.minimum = ui->slider_ptzZoom->minimum();
            opt.maximum = ui->slider_ptzZoom->maximum();
            opt.sliderPosition = ui->slider_ptzZoom->value();
            opt.orientation = ui->slider_ptzZoom->orientation();
            
            QRect handleRect = ui->slider_ptzZoom->style()->subControlRect(
                QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, ui->slider_ptzZoom);
            
            // 只有点击在手柄上时才允许事件继续
            if (!handleRect.contains(mouseEvent->pos()))
            {
                return true; // 拦截事件，阻止点击跳转
            }
        }
    }
    
    // 其他事件正常处理
    return QMainWindow::eventFilter(obj, event);
}



void robot::onFilterSeverityChanged(const QString &text)
{
    if (text == "All") {
        // 如果选择"All"，则清空筛选正则表达式，显示所有行
        m_proxyModel->setFilterRegularExpression("");
    } else {
        // 否则，使用固定字符串进行筛选
        m_proxyModel->setFilterFixedString(text);
    }
}

void robot::on_simulateDataButton_clicked()
{
    // 1. 检查任务是否已经开始（通过按钮文本判断）
//        if (ui->Btn_missionStartORstop->text() != "结束任务") {
//            QMessageBox::warning(this, "测试失败", "请先点击“任务开始”！");
//            return;
//        }

        // 2. 模拟一条巡检点数据
        // (在 "on_Btn_missionStartORstop_clicked" 中，m_currentTaskId 已被设置)

//        static int point_counter = 1; // 静态计数器，模拟巡检点编号

//        QString sim_point_id = QString::number(point_counter);
//        QString sim_equip_name = "模拟设备-" + sim_point_id;
//        QString sim_data = QString::number(QRandomGenerator::global()->bounded(50, 60) / 100.0) + "MPa";
//        QString sim_status = (point_counter % 5 == 0) ? "疑似故障" : "正常"; // 每5个点模拟一次故障
//        QString sim_img_path = (sim_status == "正常") ? "" : "C:/images/fault_" + sim_point_id + ".jpg";
//        QString sim_vid_path = (sim_status == "正常") ? "" : "C:/videos/fault_" + sim_point_id + ".mp4";

//        // 3. 调用我们新创建的数据库函数
//        bool success = dbase->add_report_detail(
//            m_currentTaskId,
//            sim_point_id,
//            sim_equip_name,
//            sim_data,
//            sim_status,
//            sim_img_path,
//            sim_vid_path
//        );

//        if (success) {
//            ui->connect_status->append(QString("成功插入模拟巡检点: %1").arg(sim_point_id));
//        } else {
//            ui->connect_status->append(QString("【失败】无法插入巡检点: %1").arg(sim_point_id));
//        }

//        point_counter++;


    if (m_currentPlanTargets.isEmpty()) {
            QMessageBox::warning(this, "提示", "请先点击【选择巡检方案】加载 trajectory_...json 文件！\n我们需要先读取里面的真实坐标才能进行比对。");
            return;
        }
    // 定义一组测试数据 (模拟 ROS 发来的坐标)
    // 这些坐标是根据你提供的 JSON 文件填写的，确保能命中
    struct TestData { double x; double y; QString desc; };
    QList<TestData> simulationSequence = {
        {3.574, 0.462, "到达点0 (Nav)"},     // 对应 JSON order 0
        {0.0,   0.0,   "移动中..."},         // 应该灭灯
        {0.733, 0.143, "到达点1 (Patrol)"},  // 对应 JSON order 1
        {0.0,   0.0,   "移动中..."},         // 应该灭灯
        {-1.461, 0.033, "到达点2 (Nav)"}     // 对应 JSON order 2
    };

    static int index = 0;
    TestData current = simulationSequence[index % simulationSequence.size()];

    ui->connect_status->append(QString(">> 模拟ROS坐标: (%1, %2) - %3")
                               .arg(current.x).arg(current.y).arg(current.desc));

    // 调用核心比对逻辑
    checkAndLightUp(current.x, current.y);

    index++;

}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////*/
void robot::showReport()
{
    QString type = ui->cbb_type_3->currentText();
    QString result = ui->cbb_result_2->currentText();
    QString start = ui->dateEdit_7->date().toString("yyyy-MM-dd");
    QString end = ui->dateEdit_8->date().toString("yyyy-MM-dd");
    ui->sw_rp->setCurrentIndex(0);
    ui->DetailBtn->setText("查看详情");

    dbase->showReport(ui->reportTableView, type, start, end, result);
}

void robot::show_rpmore()
{
    if (ui->DetailBtn->text() == "查看详情") {
            int curRow = -1;
            curRow = ui->reportTableView->currentIndex().row(); // 从主表获取选中的行

            if (curRow != -1) {
                // --- 获取选中的任务 ID (需要处理代理模型) ---
                QAbstractItemModel* mainModel = ui->reportTableView->model();
                QSortFilterProxyModel* proxyModel = qobject_cast<QSortFilterProxyModel*>(mainModel);
                QModelIndex sourceIndex;
                if (proxyModel) {
                     // 如果用了代理模型，需要映射回源模型的索引
                     sourceIndex = proxyModel->mapToSource(mainModel->index(curRow, 0)); // 第0列是ID
                } else {
                     sourceIndex = mainModel->index(curRow, 0); // 假设未使用代理
                }
                 QAbstractItemModel* sourceDataModel = proxyModel ? proxyModel->sourceModel() : mainModel;
                QString id = sourceDataModel->data(sourceIndex).toString();


                // --- 直接调用 dataBase 对象的公共槽函数 ---
                //     (dataBase::showRpmore 内部会处理数据库连接和错误)
                dbase->showRpmore(ui->DetailTableView, id); // 将 ID 传递给 dataBase

                // --- 更新 UI 状态 ---
                ui->sw_rp->setCurrentIndex(1); // 切换到详情视图
                ui->DetailBtn->setText("返回");
            }
            else {
                QMessageBox::warning(this, "提示", "请先在上方表格中选中要查看详情的任务行！");
                // return; // 这里不需要 return，因为后面没有其他逻辑
            }
        }
        else { // 当按钮是“返回”时
            ui->sw_rp->setCurrentIndex(0); // 切换回主列表视图
            // 清理详情表的模型（可选，但推荐）
            QAbstractItemModel* oldDetailModel = ui->DetailTableView->model();
             if (oldDetailModel) {
                oldDetailModel->deleteLater();
                ui->DetailTableView->setModel(nullptr);
             }
            ui->DetailBtn->setText("查看详情");
        }
}

void robot::show_rpprint()
{
    int curRow = -1;
    curRow = ui->reportTableView->currentIndex().row();
    if (curRow != -1) {
        QAbstractItemModel* model = ui->reportTableView->model();
        QModelIndex index = model->index(curRow, 3);
        QString id = model->data(index).toString();
        QString num = QString::number(rand() % 10);
        QString filename = "气体泄露机器人巡检结果" + QDateTime::currentDateTime().toString("yyyy-MM-dd") + "-" + num + ".xls";
        PrintExcel(ui->DetailTableView, filename);
    }
    else {
        QMessageBox::warning(this, ("错误操作"), ("请选中要打印的任务行"), ("确定"));
    }
}

void robot::PrintExcel(QTableView *tableView, QString &filename)
{
    // 1. 弹出“另存为”对话框
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存Excel文件",
                                                    filename,
                                                    "Excel 文件 (*.xlsx)");

    // 如果用户取消了保存，则直接返回
    if (fileName.isEmpty()) {
        return;
    }

    // 2. 创建一个新的Excel文档
    QXlsx::Document xlsx(fileName);

    // 3. 获取 QTableView 的数据模型
    QAbstractItemModel *model = tableView->model();
    if (!model) {
        QMessageBox::warning(this, "错误", "无法获取表格数据模型。");
        return;
    }

    const int colCount = model->columnCount();
    const int rowCount = model->rowCount();

    // --- 4. 定义单元格格式 ---

    // 标题格式：20号字体、加粗、水平垂直居中
    QXlsx::Format titleFormat;
    titleFormat.setFontSize(20);
    titleFormat.setFontBold(true);
    titleFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    titleFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);

    // 表头格式：加粗、灰色背景、居中、细边框
    QXlsx::Format headerFormat;
    headerFormat.setFontBold(true);
    headerFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    headerFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    headerFormat.setPatternBackgroundColor(QColor(191, 191, 191)); // 匹配旧代码的灰色
    headerFormat.setBorderStyle(QXlsx::Format::BorderThin);

    // 数据格式：居中、细边框
    QXlsx::Format dataFormat;
    dataFormat.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    dataFormat.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    dataFormat.setBorderStyle(QXlsx::Format::BorderThin);

    // --- 5. 写入标题 ---
    // (QXlsx的行和列索引从 1 开始)
    QString title = "铝锭转运机器人作业结果表"; // 您可以自定义标题
    xlsx.write(1, 1, title, titleFormat);
    // 合并单元格 (从第1行第1列 到 第1行第colCount列)
    xlsx.mergeCells(QXlsx::CellRange(1, 1, 1, colCount), titleFormat);
    // 设置标题行的高度
    xlsx.setRowHeight(1, 40);

    // --- 6. 写入表头 (第 2 行) ---
    for (int col = 0; col < colCount; ++col) {
        QString headerText = model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        // 写入单元格 (第2行, col+1列)
        xlsx.write(2, col + 1, headerText, headerFormat);
        // 设置列宽 (您可以根据需要调整，25 是一个比较通用的宽度)
        xlsx.setColumnWidth(col + 1, 25);
    }
    xlsx.setRowHeight(2, 25); // 设置表头行的高度

    // --- 7. 写入数据 (从 第 3 行 开始) ---
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            // 获取模型中的数据
            QModelIndex index = model->index(row, col);
            QString strdata = model->data(index, Qt::DisplayRole).toString();
            // 写入单元格 (第row+3行, col+1列)
            xlsx.write(row + 3, col + 1, strdata, dataFormat);
        }
    }

    // --- 8. 保存文件 ---
    if (xlsx.save()) {
        // 9. 询问是否打开文件
        if (QMessageBox::question(this, "完成", "文件已经导出，是否现在打开？",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }
    } else {
        QMessageBox::warning(this, "保存失败",
                             "无法保存Excel文件。\n请检查文件是否被其他程序占用或路径是否有效。");
    }
}






/*///////////////////////////////////////////////////////////////////////////////////////////////////////*/
//红外sdk - 已禁用,使用HTTP方式获取红外图像

void robot::on_startInfrared_clicked()
{
    // 启动TCP红外流连接
    ui->connect_status->append("开始连接红外TCP流: " + m_videoHost + ":" + QString::number(m_infraredPort));
    connectInfraredStream();
}

void robot::updateInfraredLabel(const QPixmap &pixmap)
{
    // 1. 获取 QLabel 当前的实际大小 (它已经被布局拉伸了)
    int w = ui->label_infrared->width();
    int h = ui->label_infrared->height();

    // 2. 调用 QPixmap::scaled() 来缩放图像
    //    - pixmap.scaled() 会生成一个新的 QPixmap
    //    - Qt::KeepAspectRatio 告诉它在缩放时保持原始比例
    //    - Qt::SmoothTransformation 使图像缩放时更平滑
    ui->label_infrared->setPixmap(pixmap.scaled(w, h,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}

void robot::stopInfraredStream()
{
    // 停止红外流获取
    if (m_infraredWatchdog && m_infraredWatchdog->isActive()) {
        m_infraredWatchdog->stop();
    }
    if (m_infraredTcpSocket && m_infraredTcpSocket->state() == QAbstractSocket::ConnectedState) {
        m_infraredTcpSocket->disconnectFromHost();
        ui->connect_status->append("红外视频流已停止");
        ui->label_infrared->clear();
        ui->label_infrared->setText("红外视频已停止");
    }
    m_infraredBuffer.clear();
}

//底盘控制和云台控制部分
void robot::sendBaseControlCommand(double linear_x, double angular_z)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "WebSocket未连接,无法发送底盘控制指令";
        return;
    }

    // 构建 Twist 消息
    QJsonObject linear_obj;
    linear_obj["x"] = linear_x;
    linear_obj["y"] = 0.0;
    linear_obj["z"] = 0.0;

    QJsonObject angular_obj;
    angular_obj["x"] = 0.0;
    angular_obj["y"] = 0.0;
    angular_obj["z"] = angular_z;

    QJsonObject msg_obj;
    msg_obj["linear"] = linear_obj;
    msg_obj["angular"] = angular_obj;

    // 构建 rosbridge publish 指令
    QJsonObject root_obj;
    root_obj["op"] = "publish";
    root_obj["topic"] = "/cmd_vel";
    root_obj["msg"] = msg_obj;

    // 发送JSON字符串
    QString jsonMessage = QJsonDocument(root_obj).toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(jsonMessage);
//    qDebug() << "发送底盘控制:" << jsonMessage;
}

// 用于发送云台控制指令
// 移除旧的sendPtzControlCommand函数,已由sendPtzScanControl替代

// 新增:云台扫描控制
void robot::sendPtzScanControl(double pitch_speed, double yaw_speed)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "WebSocket未连接,无法发送云台扫描控制指令";
        return;
    }

    // 构建 Vector3 消息
    QJsonObject msg_obj;
    msg_obj["x"] = 0.0;  // x未使用
    msg_obj["y"] = pitch_speed;  // y=pitch速度
    msg_obj["z"] = yaw_speed;    // z=yaw速度

    // 构建 rosbridge publish 指令
    QJsonObject root_obj;
    root_obj["op"] = "publish";
    root_obj["topic"] = "/ptz_scan_control";
    root_obj["msg"] = msg_obj;

    QString jsonMessage = QJsonDocument(root_obj).toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(jsonMessage);
    qDebug() << "发送云台扫描控制:" << jsonMessage;
}

// 新增:云台变倍控制
void robot::sendPtzZoomControl(int zoom_value)
{
    if (m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "WebSocket未连接,无法发送云台变倍控制指令";
        return;
    }

    // 构建 Vector3 消息
    QJsonObject msg_obj;
    msg_obj["x"] = static_cast<double>(zoom_value);  // x=变倍值(16~16384)
    msg_obj["y"] = 0.0;
    msg_obj["z"] = 0.0;

    // 构建 rosbridge publish 指令
    QJsonObject root_obj;
    root_obj["op"] = "publish";
    root_obj["topic"] = "/ptz_zoom_control";
    root_obj["msg"] = msg_obj;

    QString jsonMessage = QJsonDocument(root_obj).toJson(QJsonDocument::Compact);
    m_webSocket->sendTextMessage(jsonMessage);
    qDebug() << "发送云台变倍控制:" << jsonMessage;
}

// 定时发送底盘控制指令(10Hz)
void robot::sendBaseControlPeriodically()
{
    sendBaseControlCommand(m_currentLinearX, m_currentAngularZ);
}

// 定时发送云台控制指令(5Hz)
void robot::sendPtzControlPeriodically()
{
    sendPtzScanControl(m_currentPitchSpeed, m_currentYawSpeed);
}

// 底盘控制按钮 - 修改为使用定时器
void robot::on_Btn_BForward_pressed()
{
    qDebug() << "前进按钮按下";
    // 先停止之前的运动
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
    
    m_currentLinearX = 0.2;
    m_currentAngularZ = 0.0;
    m_baseControlTimer->start(100);  // 10Hz = 100ms
    qDebug() << "启动底盘控制定时器,10Hz";
}

void robot::on_Btn_BForward_released()
{
    qDebug() << "前进按钮释放";
    m_currentLinearX = 0.0;
    m_currentAngularZ = 0.0;
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);  // 立即发送停止指令
}

void robot::on_Btn_BLeft_pressed()
{
    qDebug() << "左转按钮按下";
    // 先停止之前的运动
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
    
    m_currentLinearX = 0.0;
    m_currentAngularZ = 0.2;
    m_baseControlTimer->start(100);
}

void robot::on_Btn_BLeft_released()
{
    qDebug() << "左转按钮释放";
    m_currentLinearX = 0.0;
    m_currentAngularZ = 0.0;
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
}

void robot::on_Btn_BRight_pressed()
{
    qDebug() << "右转按钮按下";
    // 先停止之前的运动
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
    
    m_currentLinearX = 0.0;
    m_currentAngularZ = -0.2;
    m_baseControlTimer->start(100);
}

void robot::on_Btn_BRight_released()
{
    qDebug() << "右转按钮释放";
    m_currentLinearX = 0.0;
    m_currentAngularZ = 0.0;
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
}

void robot::on_Btn_BBack_pressed()
{
    qDebug() << "后退按钮按下";
    // 先停止之前的运动
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
    
    m_currentLinearX = -0.2;
    m_currentAngularZ = 0.0;
    m_baseControlTimer->start(100);
}

void robot::on_Btn_BBack_released()
{
    qDebug() << "后退按钮释放";
    m_currentLinearX = 0.0;
    m_currentAngularZ = 0.0;
    m_baseControlTimer->stop();
    sendBaseControlCommand(0.0, 0.0);
}

// 云台控制按钮 - 修改为使用定时器
void robot::on_Btn_ptzUp_pressed()
{
    qDebug() << "云台上按钮按下";
    // 先停止之前的运动
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
    
    // 设置新的运动参数并启动
    m_currentPitchSpeed = -16.0;  // 向上pitch速度(交换后)
    m_currentYawSpeed = 0.0;
    m_ptzControlTimer->start(200);  // 5Hz = 200ms
}

void robot::on_Btn_ptzUp_released()
{
    qDebug() << "云台上按钮释放";
    m_currentPitchSpeed = 0.0;
    m_currentYawSpeed = 0.0;
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);  // 立即发送停止指令
}

void robot::on_Btn_ptzLeft_pressed()
{
    qDebug() << "云台左按钮按下";
    // 先停止之前的运动
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
    
    m_currentPitchSpeed = 0.0;
    m_currentYawSpeed = 16.0;  // 向左yaw速度
    m_ptzControlTimer->start(200);
}

void robot::on_Btn_ptzLeft_released()
{
    qDebug() << "云台左按钮释放";
    m_currentPitchSpeed = 0.0;
    m_currentYawSpeed = 0.0;
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
}

void robot::on_Btn_ptzRight_pressed()
{
    qDebug() << "云台右按钮按下";
    // 先停止之前的运动
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
    
    m_currentPitchSpeed = 0.0;
    m_currentYawSpeed = -16.0;  // 向右yaw速度
    m_ptzControlTimer->start(200);
}

void robot::on_Btn_ptzRight_released()
{
    qDebug() << "云台右按钮释放";
    m_currentPitchSpeed = 0.0;
    m_currentYawSpeed = 0.0;
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
}

void robot::on_Btn_ptzDown_pressed()
{
    qDebug() << "云台下按钮按下";
    // 先停止之前的运动
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
    
    m_currentPitchSpeed = 16.0;  // 向下pitch速度(交换后)
    m_currentYawSpeed = 0.0;
    m_ptzControlTimer->start(200);
}

void robot::on_Btn_ptzDown_released()
{
    qDebug() << "云台下按钮释放";
    m_currentPitchSpeed = 0.0;
    m_currentYawSpeed = 0.0;
    m_ptzControlTimer->stop();
    sendPtzScanControl(0.0, 0.0);
}

// 云台变焦滑动条处理函数
void robot::on_slider_ptzZoom_valueChanged(int value)
{
    // 映射滑动条值 1-23 到实际变焦值 16-16384
    // 使用线性映射
    // zoom_value = 16 + (16384 - 16) * (value - 1) / (23 - 1)
    int zoom_min = 16;
    int zoom_max = 16384;
    int slider_min = 1;
    int slider_max = 23;
    
    int zoom_value = zoom_min + (zoom_max - zoom_min) * (value - slider_min) / (slider_max - slider_min);
    
    // 更新缓存的缩放值
    m_currentPtzZoom = value;
    
    // 发送变焦控制命令
    sendPtzZoomControl(zoom_value);
    
    // 更新显示标签
    ui->label_zoomValue->setText(QString::number(value));
    
    qDebug() << "Zoom slider value:" << value << "-> actual zoom:" << zoom_value;
}

void robot::on_Btn_missionStartORstop_clicked()
{
    if (ui->Btn_missionStartORstop->text() == "任务开始") {//开始部分

        // 直接使用已加载的m_pointList
               if (m_pointList.isEmpty()) {
                   QMessageBox::warning(this, "操作无效", "请先点击“选择巡检方案”按钮加载一个方案。");
                   return;
               }
               // --- 1. 检查 WebSocket 连接 (新) ---
               if (m_webSocket->state() != QAbstractSocket::ConnectedState) { //
                   QMessageBox::warning(this, "未连接", "未连接到巡检车，无法开始任务。");
                   return;
               }

        // --- 3. 使用已加载的 m_pointList 构建路径数据（不再重新选择文件）---
        m_allSortedPoints = QJsonArray();
        QList<QJsonObject> allPoints;
        
        // 从 m_pointList 提取所有点并转换为发布格式
        for (const QJsonValue &val : m_pointList) {
            QJsonObject point = val.toObject();
            
            // 转换为发布格式（使用pos数组中的x,y）
            QJsonObject publishPoint;
            QJsonArray posArray = point["pos"].toArray();
            if (posArray.size() >= 2) {
                publishPoint["x"] = posArray[0].toDouble();
                publishPoint["y"] = posArray[1].toDouble();
                // 只有巡检点才有yaw字段，导航点不设置yaw
                if (point.contains("yaw")) {
                    publishPoint["yaw"] = point["yaw"].toDouble();
                }
                publishPoint["order"] = point["index"].toInt();
                publishPoint["ptzPose"] = point["ptzInfo"];
                publishPoint["zoomValue"] = point["zoomValue"];
                
                // 调试：打印zoomValue
                int zoomVal = point["zoomValue"].toInt(16);
                qDebug() << QString("构建点order=%1, zoomValue=%2")
                            .arg(publishPoint["order"].toInt())
                            .arg(zoomVal);
                
                allPoints.append(publishPoint);
            }
        }
        
        if (allPoints.isEmpty()) {
            QMessageBox::warning(this, "方案为空", "该巡检方案中没有有效的路径点，无法开始任务。");
            return;
        }
               
               // 按order字段排序
               std::sort(allPoints.begin(), allPoints.end(), [](const QJsonObject &a, const QJsonObject &b) {
                   return a["order"].toInt() < b["order"].toInt();
               });
               
               // 转换为QJsonArray并打印详细信息
               qDebug() << "========== 路径点排序结果 ==========";
               for (int i = 0; i < allPoints.size(); ++i) {
                   const QJsonObject &point = allPoints[i];
                   m_allSortedPoints.append(point);
                   
                   // 打印每个点的详细信息
                   QString pointType = point.contains("yaw") ? "巡检点" : "导航点";
                   int zoomValue = point["zoomValue"].toInt(16);
                   qDebug() << QString("第%1个点: order=%2, 类型=%3, 坐标=(%4, %5), zoomValue=%6")
                               .arg(i + 1)
                               .arg(point["order"].toInt())
                               .arg(pointType)
                               .arg(point["x"].toDouble(), 0, 'f', 2)
                               .arg(point["y"].toDouble(), 0, 'f', 2)
                               .arg(zoomValue);
                   if (point.contains("yaw")) {
                       qDebug() << QString("    yaw=%1 度")
                                   .arg(qRadiansToDegrees(point["yaw"].toDouble()), 0, 'f', 1);
                   }
               }
               qDebug() << "===================================";
               qDebug() << "已合并排序路径点:" << m_allSortedPoints.size() << "个点";
               
               // 4. 设置任务信息
               m_currentTaskStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
               
               // 5. 记录到数据库并获取自动生成的任务ID
               QString startDate = QDate::currentDate().toString("yyyy-MM-dd");
               bool dbRecorded = dbase->add_task("temp_id",  // 不再使用，会被忽略
                                                 m_currentPlanName,
                                                 m_currentPlanType,
                                                 startDate,
                                                 m_currentTaskStartTime,
                                                 "", // 结束时间留空
                                                 "正在进行"); // 状态
               if (dbRecorded) {
                   // 获取刚插入的任务ID (自增ID)
                   m_currentTaskId = dbase->getLastInsertId();
                   qDebug() << "✓ 任务ID: " << m_currentTaskId;
               } else {
                   qDebug() << "警告:数据库记录失败，但任务继续执行";
                   m_currentTaskId = QDateTime::currentDateTime().toString("yyyyMMddhhmmss"); // 使用时间戳作为备用
               }
               
               // 6. 订阅仪表读数话题
               subscribeToMeterReading();
               m_meterReadings.clear(); // 清空之前的读数
               
               // 7. 立即发布一次路径话题
               publishPathTopics();
               
               // 8. 启动定时器，每100ms循环发布（10Hz）
               if (!m_pathPublishTimer) {
                   m_pathPublishTimer = new QTimer(this);
                   connect(m_pathPublishTimer, &QTimer::timeout, this, &robot::publishPathTopics);
               }
               m_pathPublishTimer->start(100); // 每100毫秒发布一次（10Hz）

               // 9. 更新UI状态
               ui->Btn_missionStartORstop->setText("结束任务");
               ui->Btn_PlanSelect->setEnabled(false);
               ui->loadMapButton->setEnabled(false);
               ui->connect_status->append("任务已开始: " + m_currentPlanName);
               ui->connect_status->append(QString("正在循环发布路径话题 (%1个点)").arg(m_allSortedPoints.size()));
               
               // 初始化报告详情表格显示（用于实时显示仪表读数）
               dbase->showRpmore(ui->DetailTableView, m_currentTaskId);
               ui->sw_rp->setCurrentIndex(1); // 切换到详情视图，方便实时查看数据
               
               QMessageBox::information(this, "任务开始", 
                   QString("任务\"%1\"已开始。\n正在循环发布路径话题:\n/nav_path\n/ptz_path\n/zoom_path\n\n仪表读数将实时显示在【数据报表】标签页中。").arg(m_currentPlanName));
               
               ui->btn_UndoLastPoint->setEnabled(false);
               ui->btn_ClearAllPoints->setEnabled(false);
               ui->btn_SavePlan->setEnabled(false);
    }
    else {//停止部分
        // 停止路径发布定时器
        if (m_pathPublishTimer && m_pathPublishTimer->isActive()) {
            m_pathPublishTimer->stop();
            ui->connect_status->append("已停止路径话题发布");
        }
        
        // 清空路径数据
        m_allSortedPoints = QJsonArray();
        m_navPathPoints = QJsonArray();
        m_patrolPathPoints = QJsonArray();
        
        dbase->insertreport(m_currentPlanName,           // task_name
                            m_currentTaskStartTime,      // start_time
                            "已完成",                    // status
                            "admin",                     // operator_name
                            "巡检任务正常完成");           // notes
        ui->connect_status->append("任务完成: " + m_currentPlanName);
        QMessageBox::information(this, "任务完成",
                                 QString("任务“%1”已完成并存入报告。").arg(m_currentPlanName));

        // 2. 重置UI状态
        ui->Btn_missionStartORstop->setText("任务开始");
        ui->Btn_PlanSelect->setEnabled(true); // 允许选择新方案
        ui->InspectionPlanName->clear();      // 清空方案名
        ui->Btn_missionStartORstop->setEnabled(false); // 必须重新选择方案才能再次开始
        ui->loadMapButton->setEnabled(true); // 允许重载地图
        // [修改] 重新启用单选按钮

        ui->btn_UndoLastPoint->setEnabled(true);
        ui->btn_ClearAllPoints->setEnabled(true);
        ui->btn_SavePlan->setEnabled(true);

        // 3. 清理内部变量
        m_currentTaskId.clear();
        m_currentTaskStartTime.clear();
        m_currentPlanName.clear();
        m_currentPlanType.clear();

        // 4. 自动刷新报告列表
        showReport();
    }

}

void robot::on_Btn_PlanSelect_clicked()
{
    // ===== [新增] 停止之前可能正在运行的任务 =====
    if (m_pathPublishTimer && m_pathPublishTimer->isActive()) {
        m_pathPublishTimer->stop();
        ui->connect_status->append("检测到任务正在运行，已自动停止");
        qDebug() << "选择新方案时自动停止了旧任务的发布定时器";
    }
    
    // 清空之前任务的路径数据
    m_allSortedPoints = QJsonArray();
    m_navPathPoints = QJsonArray();
    m_patrolPathPoints = QJsonArray();
    
    // 1. 弹出文件对话框
        QString jsonFilePath = QFileDialog::getOpenFileName(this,
                                                        "选择巡检方案文件",
                                                        QDir::currentPath(),
                                                        "巡检方案 (*.json)");
        if (jsonFilePath.isEmpty()) {
            return;
        }

        // 2. 读取并解析 JSON 文件
        QFile file(jsonFilePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "读取失败", "无法打开所选的方案文件。\n" + file.errorString());
            return;
        }
        QByteArray jsonData = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            QMessageBox::critical(this, "文件无效", "所选文件不是一个有效的 JSON 对象格式。");
            return;
        }
        QJsonObject root = doc.object();

        // === 新增：解析数据到 m_currentPlanTargets 用于比对 ===
        m_currentPlanTargets.clear();

        // 1. 解析 navPoints (导航点)
        if (root.contains("navPoints")) {
            QJsonArray navArray = root["navPoints"].toArray();
            for (const QJsonValue &val : navArray) {
                QJsonObject obj = val.toObject();
                InspectionTarget target;
                target.id = obj["order"].toInt();
                target.realX = obj["x"].toDouble();
                target.realY = obj["y"].toDouble();
                m_currentPlanTargets.append(target);
            }
        }

        // 2. 解析 patrolPoints (巡检点)
        if (root.contains("patrolPoints")) {
            QJsonArray patrolArray = root["patrolPoints"].toArray();
            for (const QJsonValue &val : patrolArray) {
                QJsonObject obj = val.toObject();
                InspectionTarget target;
                target.id = obj["order"].toInt(); // 关键：这里也是用 order
                target.realX = obj["x"].toDouble();
                target.realY = obj["y"].toDouble();
                m_currentPlanTargets.append(target);
            }
        }

        qDebug() << "已加载比对目标点：" << m_currentPlanTargets.size() << "个";

        // 3. --- [新逻辑] 解析 JSON 并重新填充列表和地图 ---

        // 3A. 检查关键字段是否存在（兼容新旧两种格式）
        bool isNewFormat = root.contains("navPoints") || root.contains("patrolPoints");
        bool isOldFormat = root.contains("path_name") && root.contains("plan_type") && root.contains("path_points");
        
        if (!isNewFormat && !isOldFormat)
        {
            QMessageBox::warning(this, "文件无效", "方案JSON文件格式不正确。");
            return;
        }

        // 3B. 先清除当前所有点 (非常重要)
        on_btn_ClearAllPoints_clicked(); // (调用我们已有的清除函数)

        // 3C. 根据格式解析数据
        if (isNewFormat) {
            // 新格式：使用文件名作为方案名
            QFileInfo fileInfo(jsonFilePath);
            m_currentPlanName = fileInfo.baseName();
            m_currentPlanType = "巡检任务"; // 默认类型
            
            // 读取navPoints和patrolPoints
            QJsonArray navPoints = root.value("navPoints").toArray();
            QJsonArray patrolPoints = root.value("patrolPoints").toArray();
            
            // 处理导航点
            for (const QJsonValue &pointValue : navPoints) {
                QJsonObject point = pointValue.toObject();
                if (point.isEmpty()) continue;
                
                // 转换为内部格式
                QJsonObject internalPoint;
                internalPoint["type"] = "navigation";
                internalPoint["pos"] = QJsonArray{point["x"].toDouble(), point["y"].toDouble()};
                // 导航点不设置yaw字段，这样在发布时四元数会全为0
                internalPoint["index"] = point["order"].toInt() + 1; // order从0开始，index从1开始
                internalPoint["ptzInfo"] = point["ptzPose"];
                internalPoint["zoomValue"] = point["zoomValue"].toInt(16); // 默认zoom挡位16
                
                m_pointList.append(internalPoint);
                
                // 在地图上绘制
//                QPointF worldPos(point["x"].toDouble(), point["y"].toDouble());
//                QPointF scenePos = convertWorldToScene(worldPos);
//                addPointToMap(scenePos, "navigation", point["order"].toInt() + 1, 0.0);

                InspectionTarget target;
                target.id = point["order"].toInt();
                target.realX = point["x"].toDouble();
                target.realY = point["y"].toDouble();
                m_currentPlanTargets.append(target); // ✅ 新增：用于亮灯比对
                
                if (point["order"].toInt() + 1 >= m_pointCounter) {
                    m_pointCounter = point["order"].toInt() + 2;
                }
            }
            
            // 处理巡检点
            for (const QJsonValue &pointValue : patrolPoints) {
                QJsonObject point = pointValue.toObject();
                if (point.isEmpty()) continue;
                
                // 转换为内部格式
                QJsonObject internalPoint;
                internalPoint["type"] = "inspection";
                internalPoint["pos"] = QJsonArray{point["x"].toDouble(), point["y"].toDouble()};
                internalPoint["yaw"] = point["yaw"].toDouble();
                internalPoint["index"] = point["order"].toInt() + 1;
                internalPoint["ptzInfo"] = point["ptzPose"];
                internalPoint["zoomValue"] = point["zoomValue"].toInt(16); // 默认zoom挡位16
                
                m_pointList.append(internalPoint);
                
                // 在地图上绘制 - 拓扑地图模式下已禁用
                // QPointF worldPos(point["x"].toDouble(), point["y"].toDouble());
                // QPointF scenePos = convertWorldToScene(worldPos);
                // addPointToMap(scenePos, "inspection", point["order"].toInt() + 1, point["yaw"].toDouble());
                
                if (point["order"].toInt() + 1 >= m_pointCounter) {
                    m_pointCounter = point["order"].toInt() + 2;
                }
            }
        } else {
            // 旧格式：保持原有逻辑
            m_currentPlanName = root["path_name"].toString();
            m_currentPlanType = root["plan_type"].toString();

            // 3D. 解析 path_points 数组
            QJsonArray pathArray = root["path_points"].toArray();
            for (const QJsonValue &pointValue : pathArray)
            {
                QJsonObject pointObj = pointValue.toObject();
                if (pointObj.isEmpty() || !pointObj.contains("index")) {
                    continue; // 跳过无效的点
                }

                // 确保zoomValue字段有默认值
                if (!pointObj.contains("zoomValue") || pointObj["zoomValue"].isNull()) {
                    pointObj["zoomValue"] = 16; // 默认zoom挡位16
                }

                // I. 将点添加到数据列表
                m_pointList.append(pointObj);

                // II. 在地图上重新绘制这个点
                QString type = pointObj["type"].toString();
                int id = pointObj["index"].toInt();
                QJsonArray posArray = pointObj["pos"].toArray();
                if (posArray.count() < 2) continue;

                // 拓扑地图模式下已禁用地图标记绘制
                // QPointF worldPos(posArray[0].toDouble(), posArray[1].toDouble());
                // QPointF scenePos = convertWorldToScene(worldPos);
                // double yaw = pointObj["yaw"].toDouble();
                // addPointToMap(scenePos, type, id, yaw);

                if (id >= m_pointCounter) {
                    m_pointCounter = id + 1;
                }
            }
        }

        // 4. --- 更新 UI ---
        ui->InspectionPlanName->setText(m_currentPlanName);
        ui->Btn_missionStartORstop->setEnabled(true); // 使“任务开始”按钮可用
        ui->connect_status->append("巡检方案已加载: " + m_currentPlanName);
        QMessageBox::information(this, "加载成功",
                                 QString("方案“%1”已加载，包含 %2 个点。").arg(m_currentPlanName).arg(m_pointList.count()));
}

void robot::onWebSocketConnected()
{
    qDebug() << "========== WebSocket连接成功回调触发 ==========";
    qDebug() << "连接URL:" << m_webSocketUrl;
    ui->connect_status->append(QString("连接成功: %1").arg(m_webSocketUrl));
    QJsonObject sub_reached;
    sub_reached["op"] = "subscribe";
    sub_reached["topic"] = "/move_base_simple/goal";
    sub_reached["type"] = "geometry_msgs/PoseStamped";
    m_webSocket->sendTextMessage(QJsonDocument(sub_reached).toJson(QJsonDocument::Compact));
    
    ui->connect_status->append("已订阅抵达信号: /move_base_simple/goal");
    // 订阅点云话题
    subscribeToPointClouds();

}

void robot::onWebSocketDisconnected()
{
    qDebug() << "========== WebSocket断开连接 ==========";
    // 向状态栏打印断开连接的信息
    ui->connect_status->append(QString("已从 %1 断开连接。").arg(m_webSocketUrl));

    // 连接断开，移除机器人位置标记
        if (m_robotPositionMarker) {
            m_mapScene->removeItem(m_robotPositionMarker); //
            delete m_robotPositionMarker;
            m_robotPositionMarker = nullptr;
        }
    // 检查是否是在任务执行期间意外断开的
    if (ui->Btn_missionStartORstop->text() == "结束任务")
    {
        // 如果任务正在运行，说明是意外断开
        QMessageBox::warning(this, "连接中断", "与巡检车的连接意外丢失！任务已自动中止。");

        // 强制重置 UI 状态
        ui->Btn_missionStartORstop->setText("任务开始");
        ui->Btn_PlanSelect->setEnabled(true); // 允许重新选择方案
        ui->InspectionPlanName->clear();
        ui->Btn_missionStartORstop->setEnabled(false); // 必须重选方案

        // 清理内部变量（此时不调用 insertreport，因为任务未正常完成）
        m_currentTaskId.clear();
        m_currentTaskStartTime.clear();
        m_currentPlanName.clear();
        m_currentPlanType.clear();

        // [可选] 您可以在这里调用 dbase->updateTaskStatus(m_currentTaskId, "已中断");
        // (这需要在 database.h/cpp 中添加一个新函数)
    }
}

void robot::onWebMessageReceived(const QString &message)
{
    // 检测meter_reading消息（最高优先级调试）
    if (message.contains("meter_reading")) {

        qDebug() << "检测到meter_reading消息！完整内容:";
        qDebug() << message;

    }
    
    // 只对非点云消息打印详细信息（减少日志量）
//    if (!message.contains("PointCloud2")) {
//        qDebug() << "收到 WebSocket 消息: " << message.left(200); // 只打印前200字符
//    }

    // 1. 解析收到的 JSON 字符串
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "收到的消息不是一个有效的 JSON 对象。";
        return;
    }
    QJsonObject root = doc.object();

    // 2. 检查消息类型 (与同事约定好的 JSON 格式)
    if (root.contains("op") && root.value("op").toString() == "publish")
    {
        QString topic = root.value("topic").toString();
        
        // 调试：打印话题名称
        if (topic == "/meter_reading") {
            qDebug() << "[DEBUG] 检测到 /meter_reading 消息";
        }

        // --- [新逻辑块：处理 /odom 话题] ---
        if (topic == "/odom")
        {
            // 确保地图已加载，否则转换无意义
            if (!m_mapIsLoaded) return; //

            // 安全地解析 JSON 结构 (msg.pose.pose.position)
            QJsonObject msg = root.value("msg").toObject();
            QJsonObject pose_with_cov = msg.value("pose").toObject();
            QJsonObject pose = pose_with_cov.value("pose").toObject();
            QJsonObject position = pose.value("position").toObject();
            QJsonObject orientation = pose.value("orientation").toObject();

            if (position.isEmpty()) {
                qDebug() << "Odom: 无法解析 position 数据";
                return;
            }

            // 1. 存储机器人位置
            m_latestRobotPos_World.setX(position.value("x").toDouble());
            m_latestRobotPos_World.setY(position.value("y").toDouble());

            // 2. 存储机器人 Yaw
            m_latestRobotYaw = convertQuaternionToYaw(orientation);

//            // 3. 更新地图上的红点 (这部分不变)
//            QPointF scenePos = convertWorldToScene(m_latestRobotPos_World);

//            // 如果标记还不存在，就创建它
//            if (!m_robotPositionMarker) {
//                // 创建一个 10x10 的红色圆圈，锚点在中心 (-5, -5)
//                m_robotPositionMarker = m_mapScene->addEllipse(-5, -5, 10, 10,
//                                                               QPen(Qt::red), QBrush(Qt::red));
//                m_robotPositionMarker->setZValue(100); // 确保它在所有路径点之上
//            }

//            // 移动标记到新的位置
//            m_robotPositionMarker->setPos(scenePos);
            // 这样即使机器人没发 reached_waypoint 信号，只要路过，灯也会亮
            checkAndLightUp(m_latestRobotPos_World.x(), m_latestRobotPos_World.y());

            return; // 处理完毕
        }
        // --- [新增: 处理点云话题] ---
        else if (topic == "/globalmap_downsample" || topic == "/aligned_points_downsample")
        {
            QJsonObject msg = root.value("msg").toObject();
            onPointCloudMessageReceived(topic, msg);
            return;
        }
        // --- [新增: 处理TF话题] ---
        else if (topic == "/tf" || topic == "/tf_static")
        {
            QJsonObject msg = root.value("msg").toObject();
            onTFMessageReceived(msg);
            return;
        }
        else if (topic == "/move_base_simple/goal")
        {
            // 解析 geometry_msgs/PoseStamped
            QJsonObject msg = root.value("msg").toObject();
            QJsonObject pose = msg.value("pose").toObject();
            QJsonObject position = pose.value("position").toObject();

            double x = position.value("x").toDouble();
            double y = position.value("y").toDouble();

            qDebug() << "收到抵达坐标：" << x << y;

            // 调用比对逻辑
            checkAndLightUp(x, y);
        }
        // --- [新增: 处理仪表读数话题] ---
        else if (topic == "/meter_reading")
        {
            qDebug() << "[DEBUG] 进入 /meter_reading 处理分支";
            QJsonObject msg = root.value("msg").toObject();
            qDebug() << "[DEBUG] msg对象: " << msg;
            
            if (msg.contains("data")) {
                float reading = static_cast<float>(msg.value("data").toDouble());
                qDebug() << "[DEBUG] 解析出的读数值: " << reading;
                onMeterReadingReceived(reading);
            } else {
                qDebug() << "[DEBUG] ERROR: msg中没有data字段!";
            }
            return;
        }
    }
    else if (root.contains("type") && root["type"].toString() == "inspection_result")
    {
        // --- 这是一条巡检点数据 ---

        // 3. 提取数据 (键名必须与巡检车发送的一致！)
        QString point_id = root.value("point_id").toString("未知点");
        QString equip_name = root.value("equipment_name").toString("未知设备");
        QString data = root.value("data").toString("N/A");
        QString status = root.value("status").toString("未知");
        // A. 获取相对路径
        QString relative_img_path = root.value("image_url").toString(""); //
        QString relative_vid_path = root.value("video_url").toString(""); //

        // B. 转换为绝对 URL
        //    (仅当路径非空时才添加 http 前缀)
        QString abs_img_path = relative_img_path.isEmpty() ? "" : (m_robotHttpBaseUrl + relative_img_path);
        QString abs_vid_path = relative_vid_path.isEmpty() ? "" : (m_robotHttpBaseUrl + relative_vid_path);
        // (注意: point_time 可以由数据库自动生成，无需接收)

        // 4. 检查 m_currentTaskId 是否有效 (任务是否已开始)
        if (m_currentTaskId.isEmpty()) {
            qDebug() << "收到了巡检数据，但当前没有正在记录的任务。";
            return;
        }

        // 5. 调用数据库函数，将明细写入 report_details 表
        bool success = dbase->add_report_detail(
            m_currentTaskId,
            point_id,
            equip_name,
            data,
            status,
            abs_img_path, // <-- 传入转换后的 URL
            abs_vid_path  // <-- 传入转换后的 URL
        );

        // 6. 更新 UI 状态
        if (success) {
            ui->connect_status->append(QString("已存储巡检点 [%1]: %2").arg(point_id, status));
        } else {
            ui->connect_status->append(QString("【失败】存储巡检点 [%1] 到数据库失败。").arg(point_id));
        }
    }
    else if (root.contains("type") && root["type"].toString() == "task_completed")
    {
        // --- [推荐] 巡检车主动报告任务完成 ---
        ui->connect_status->append("巡检车报告：任务已完成。");

        // 检查 "结束任务" 按钮是否仍处于激活状态
        if (ui->Btn_missionStartORstop->text() == "结束任务") {
            // 自动触发“结束任务”按钮的点击逻辑
            on_Btn_missionStartORstop_clicked();
        }
    }
    else if (root.contains("type") && root["type"].toString() == "error")
    {
        // --- 巡检车报告错误 ---
        QString error_msg = root.value("message").toString("巡检车报告了一个未知错误。");
        QMessageBox::critical(this, "巡检车错误", error_msg);
        ui->connect_status->append("【错误】" + error_msg);
    }
    else
    {
        // 收到其他未知类型的消息
        qDebug() << "收到未知类型的 WebSocket 消息。";
    }
}


void robot::on_loadMapButton_clicked()
{
//    QString filePath = QFileDialog::getOpenFileName(this,
//                                                        "打开 PCD 点云地图",
//                                                        QDir::currentPath(),
//                                                        "PCD 文件 (*.pcd)");

//        if (filePath.isEmpty()) {
//            return; // 用户取消
//        }

//        loadPcdMap(filePath);
    initTopologyMap();

    ui->connect_status->append("已重置拓扑地图显示。");
}

void robot::loadPcdMap(const QString &filePath)
{
    QFile file(filePath);
       if (!file.open(QIODevice::ReadOnly)) {
           QMessageBox::critical(this, "文件错误", "无法打开 PCD 文件: " + file.errorString());
           return;
       }

       // --- 1. 解析头部 ---
       QTextStream headerStream(&file);
       QString line;
       int totalPoints = 0;
       int fieldsCount = 0;
       int x_offset = -1, y_offset = -1, z_offset = -1; // x, y, z 字段的字节偏移量
       int pointStep = 0; // 每个点占用的总字节数
       bool dataBinary = false;

       // [修复] 添加变量来存储数据起始位置
       qint64 dataStartPos = -1;
       // [修复] 添加数组来存储 SIZE 和 OFFSET
       QList<int> fieldSizes;
       QList<QString> fieldNames;


       while (!(line = headerStream.readLine()).isNull()) {
           if (line.startsWith("FIELDS")) {
               fieldNames = line.split(' '); // 包含 "FIELDS"
               fieldNames.removeFirst(); // 移除 "FIELDS"，只剩 x y z ...
               fieldsCount = fieldNames.count();
               x_offset = fieldNames.indexOf("x"); // 索引 0
               y_offset = fieldNames.indexOf("y"); // 索引 1
               z_offset = fieldNames.indexOf("z"); // 索引 2
           } else if (line.startsWith("SIZE")) {
               QStringList sizes = line.split(' ');
               sizes.removeFirst(); // 移除 "SIZE"
               for (const QString &s : sizes) {
                   fieldSizes.append(s.toInt());
               }
           } else if (line.startsWith("POINTS")) {
               totalPoints = line.split(' ').last().toInt();
           } else if (line.startsWith("DATA")) {
               dataBinary = line.split(' ').last() == "binary";
               dataStartPos = headerStream.pos(); // 二进制数据起始于此
               break; // 头部结束
           }
       }

       // --- [修复] 检查数据完整性 ---
       if (fieldNames.count() != fieldSizes.count() || fieldNames.count() == 0) {
            QMessageBox::critical(this, "文件格式错误", "PCD 文件的 FIELDS 和 SIZE 字段数量不匹配。");
            file.close();
            return;
       }

       // --- [修复] 动态计算偏移量和步长 ---
       int x_byte_offset = -1, y_byte_offset = -1, z_byte_offset = -1;
       pointStep = 0;
       for (int i = 0; i < fieldsCount; ++i) {
           if (i == x_offset) x_byte_offset = pointStep;
           if (i == y_offset) y_byte_offset = pointStep;
           if (i == z_offset) z_byte_offset = pointStep;
           pointStep += fieldSizes[i];
       }

       // 检查是否找到了所有需要的字段
       if (!dataBinary || totalPoints == 0 || x_byte_offset == -1 || y_byte_offset == -1 || z_byte_offset == -1 || pointStep == 0 || dataStartPos == -1) {
           QMessageBox::critical(this, "文件格式错误", QString("PCD 文件头部信息不完整、不是二进制格式、或缺少 x/y/z 字段。\n"
                                 "X-Offset: %1, Y-Offset: %2, Z-Offset: %3, Step: %4, DataPos: %5")
                                 .arg(x_byte_offset).arg(y_byte_offset).arg(z_byte_offset).arg(pointStep).arg(dataStartPos));
           file.close();
           return;
       }

       qDebug() << "PCD 加载中: 总点数" << totalPoints << "点步长" << pointStep << "X偏移" << x_byte_offset << "Y偏移" << y_byte_offset << "Z偏移" << z_byte_offset;
       qDebug() << "二进制数据起始于字节:" << dataStartPos;


       // --- 2. Pass 1: 查找地图边界 (min/max x, y, z) ---

       if (!file.seek(dataStartPos)) {
           QMessageBox::critical(this, "文件读取错误", "无法跳转到 PCD 数据起始位置 (Pass 1)。");
           file.close();
           return;
       }

       QDataStream dataStream(&file);
       dataStream.setByteOrder(QDataStream::LittleEndian); // PCD 通常是小端
       dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision); // 4 字节 float

       float min_x = std::numeric_limits<float>::max();
       float max_x = std::numeric_limits<float>::lowest();
       float min_y = std::numeric_limits<float>::max();
       float max_y = std::numeric_limits<float>::lowest();
       float min_z = std::numeric_limits<float>::max();
       float max_z = std::numeric_limits<float>::lowest();

       QByteArray pointBuffer(pointStep, 0);
       float x, y, z;

       qDebug() << "Pass 1: 正在查找地图边界...";
       for (int i = 0; i < totalPoints; ++i) {
           if (dataStream.readRawData(pointBuffer.data(), pointStep) != pointStep) {
                QMessageBox::critical(this, "文件读取错误", QString("PCD 文件数据不完整 (Pass 1)。在 %1 / %2 个点处停止。").arg(i).arg(totalPoints));
                file.close();
                return;
           }

           // 从缓冲区中提取 x, y, z
           memcpy(&x, pointBuffer.data() + x_byte_offset, sizeof(float));
           memcpy(&y, pointBuffer.data() + y_byte_offset, sizeof(float));
           memcpy(&z, pointBuffer.data() + z_byte_offset, sizeof(float));

           if (x < min_x) min_x = x;
           if (x > max_x) max_x = x;
           if (y < min_y) min_y = y;
           if (y > max_y) max_y = y;
           if (z < min_z) min_z = z;
           if (z > max_z) max_z = z;
       }
       qDebug() << "Pass 1: 完成。X 范围:" << min_x << "到" << max_x << "Y 范围:" << min_y << "到" << max_y << "Z 范围:" << min_z << "到" << max_z;


       // --- 3. Pass 2: 将点绘制到 QImage (带 Z 轴过滤) ---

       if (!file.seek(dataStartPos)) {
           QMessageBox::critical(this, "文件读取错误", "无法跳转到 PCD 数据起始位置 (Pass 2)。");
           file.close();
           return;
       }

       QDataStream dataStream2(&file); // 重新创建数据流
       dataStream2.setByteOrder(QDataStream::LittleEndian);
       dataStream2.setFloatingPointPrecision(QDataStream::SinglePrecision);

       // 定义图像大小
       const double pixelsPerMeter = 10.0; // 1米 = 10像素 (您可以调整这个缩放比例)
       double mapWidthMeters = max_x - min_x;
       double mapHeightMeters = max_y - min_y;
       int imgWidth = static_cast<int>(mapWidthMeters * pixelsPerMeter) + 1;
       int imgHeight = static_cast<int>(mapHeightMeters * pixelsPerMeter) + 1;

       if (imgWidth <= 0 || imgHeight <= 0 || imgWidth > 32767 || imgHeight > 32767) {
           QMessageBox::critical(this, "地图尺寸错误", "计算出的地图图像尺寸无效或过大。");
           file.close();
           return;
       }

       qDebug() << "Pass 2: 正在创建" << imgWidth << "x" << imgHeight << "的地图图像...";
       QImage mapImage(imgWidth, imgHeight, QImage::Format_Grayscale8); // 8位灰度图
       mapImage.fill(Qt::black); // 黑色背景

       // --- Z 轴切片定义 ---
       const float zSliceThreshold = 3; // 0.2米 = 20厘米 (只取地板上方 20 厘米)
       const float floorLevelMax = min_z + zSliceThreshold;
       qDebug() << "Z 轴过滤：只渲染 Z 值在" << min_z << "和" << floorLevelMax << "之间的点。";

       QPainter painter(&mapImage);
       painter.setPen(Qt::white); // 白色点

       for (int i = 0; i < totalPoints; ++i) {
           if (dataStream2.readRawData(pointBuffer.data(), pointStep) != pointStep) {
               QMessageBox::critical(this, "文件读取错误", QString("PCD 文件数据不完整 (Pass 2)。在 %1 / %2 个点处停止。").arg(i).arg(totalPoints));
               break;
           }

           // --- [修复] Z轴过滤 ---
           memcpy(&z, pointBuffer.data() + z_byte_offset, sizeof(float));
           // 只绘制地板附近的点
           if (z > floorLevelMax||z<0.3) {
               continue; // 跳过这个点 (例如天花板或高处障碍物)
           }

           memcpy(&x, pointBuffer.data() + x_byte_offset, sizeof(float));
           memcpy(&y, pointBuffer.data() + y_byte_offset, sizeof(float));


           // 转换到图像像素坐标 (Y 轴反转)
           int px = static_cast<int>((x - min_x) * pixelsPerMeter);
           int py = static_cast<int>((max_y - y) * pixelsPerMeter); // Y 轴反转

           if (px >= 0 && px < imgWidth && py >= 0 && py < imgHeight) {
               painter.drawPoint(px, py);
           }
       }
       painter.end(); // 结束绘制
       file.close();
       qDebug() << "Pass 2: 绘制完成。";


       // --- 4. 将 QImage 设置到场景中 ---
       m_mapScene->clear(); // 清除旧地图
       m_mapScene->setSceneRect(0, 0, imgWidth, imgHeight);
       m_mapScene->addPixmap(QPixmap::fromImage(mapImage));

       // --- [修改] 旋转逻辑 ---
       if (imgHeight > imgWidth) {
           qDebug() << "地图是竖屏 (h:" << imgHeight << ", w:" << imgWidth << ")，将视图旋转 90 度以适配横屏控件。";
           ui->mapGraphicsView->resetTransform(); // 重置可能存在的旧变换
           ui->mapGraphicsView->rotate(90);
       } else {
           qDebug() << "地图是横屏或方形，视图保持 0 度。";
           ui->mapGraphicsView->resetTransform(); // 重置可能存在的旧变换
       }

       // 缩放视图以适应地图
       ui->mapGraphicsView->fitInView(m_mapScene->sceneRect(), Qt::KeepAspectRatio);
       // --- 5. 存储转换参数为成员变量 ---
       m_mapMinX = min_x;
       m_mapMaxY = max_y; // 存储 Y 轴的最大值 (用于 Y 轴反转)
       m_mapPixelsPerMeter = pixelsPerMeter;
       m_mapIsLoaded = true;
       
       // 清空临时路径点列表，开始新的路径规划
       m_pointList.clear();
       m_pointCounter = 1; // 重置路径点计数器
       
       // 启用点操作按钮，可以开始添加路径点
       ui->btn_AddNavPoint->setEnabled(true);
       ui->btn_AddInspectionPoint->setEnabled(true);
       ui->btn_UndoLastPoint->setEnabled(false);  // 没有点时禁用撤回
       ui->btn_ClearAllPoints->setEnabled(false);  // 没有点时禁用清除
       ui->btn_SavePlan->setEnabled(false);  // 没有点时禁用保存
       
       ui->connect_status->append("PCD 地图加载并渲染完成。");
       ui->connect_status->append("已清空临时路径点，可以开始新的路径规划。");

}

QPointF robot::convertSceneToWorld(QPointF scenePos)
{
    if (!m_mapIsLoaded) return QPointF(); // 返回空 QPointF

    float worldX = (scenePos.x() / m_mapPixelsPerMeter) + m_mapMinX;
    float worldY = m_mapMaxY - (scenePos.y() / m_mapPixelsPerMeter); // Y 轴反转

    return QPointF(worldX, worldY);
}

QPointF robot::convertWorldToScene(QPointF worldPos)
{
    // 检查地图是否已加载
    if (!m_mapIsLoaded) return QPointF();

    // 这些是你在 loadPcdMap 中存储的转换参数
    double sceneX = (worldPos.x() - m_mapMinX) * m_mapPixelsPerMeter;
    double sceneY = (m_mapMaxY - worldPos.y()) * m_mapPixelsPerMeter; // Y 轴反转

    return QPointF(sceneX, sceneY);
}

/**
 * @brief [新] 辅助函数：将收到的四元数转为 Yaw 偏航角 (弧度)
 */
double robot::convertQuaternionToYaw(const QJsonObject& quat)
{
    double x = quat["x"].toDouble();
    double y = quat["y"].toDouble();
    double z = quat["z"].toDouble();
    double w = quat["w"].toDouble();

    // Yaw (z-axis rotation)
    double siny_cosp = 2.0 * (w * z + x * y);
    double cosy_cosp = 1.0 - 2.0 * (y * y + z * z);
    return qAtan2(siny_cosp, cosy_cosp);
}

void robot::addPointToMap(const QPointF& scenePos, const QString& type, int id, double yaw)
{
    QColor pointColor = (type == "inspection") ? Qt::blue : Qt::green;
        QString pointTag = "path_point_group_" + QString::number(id);
        QPen pointPen(pointColor);
        pointPen.setWidth(2);

        // 点
        QGraphicsEllipseItem* pointItem = m_mapScene->addEllipse( //
            scenePos.x() - 4, scenePos.y() - 4, 8, 8,
            pointPen, QBrush(pointColor));
        pointItem->setData(1, pointTag);

        // 箭头 (使用传入的 yaw，而不是 m_latestRobotYaw)
        QPointF arrowEnd = scenePos + QPointF(15 * qCos(yaw), -15 * qSin(yaw)); // <-- [修改]
        QGraphicsLineItem* lineItem = m_mapScene->addLine(QLineF(scenePos, arrowEnd), pointPen); //
        lineItem->setData(1, pointTag);

        // 编号
        QGraphicsSimpleTextItem* text = m_mapScene->addSimpleText(QString::number(id)); //
        text->setPos(scenePos + QPointF(5, 5));
        text->setBrush(pointColor);
        text->setData(1, pointTag);
}
/**
 * @brief [新] 槽函数：捕捉当前机器人位置，添加为路径点
 */
void robot::on_btn_AddNavPoint_clicked()
{
    // 检查是否已收到机器人位置数据
    if (m_latestRobotPos_World.isNull() || (m_latestRobotPos_World.x() == 0.0 && m_latestRobotPos_World.y() == 0.0)) {
        QMessageBox::warning(this, "数据缺失", 
            "尚未收到机器人位置数据，请检查：\n"
            "1. WebSocket是否已连接\n"
            "2. /tf 话题是否在发布\n"
            "3. map->base_link的TF变换是否存在");
        ui->connect_status->append("错误：无法获取机器人位置，当前位置为 (0, 0)");
        return;
    }
    
    // 检查TF数据新鲜度 (TF以1Hz发布，检查是否在1.5秒内收到过数据)
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 robotTfAge = currentTime - m_lastRobotTfTimestamp;
    qint64 ptzTfAge = currentTime - m_lastPtzTfTimestamp;
    
    if (robotTfAge > 1500 && m_lastRobotTfTimestamp > 0) {
        QMessageBox::warning(this, "数据过时", 
            QString("机器人TF数据已过时 (%1 秒前)，请稍等片刻后重试。\n\n"
                    "TF数据以1Hz频率发布，建议等待1-2秒后再添加点。")
            .arg(robotTfAge / 1000.0, 0, 'f', 1));
        ui->connect_status->append(QString("警告:机器人TF数据过时 (%1秒前)").arg(robotTfAge / 1000.0, 0, 'f', 1));
        return;
    }
    
    if (ptzTfAge > 1500 && m_lastPtzTfTimestamp > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "PTZ数据过时", 
            QString("PTZ TF数据已过时 (%1 秒前)。\n\n"
                    "TF数据以1Hz频率发布，建议等待1-2秒后再添加点。\n\n"
                    "是否仍要使用过时数据添加此导航点?")
            .arg(ptzTfAge / 1000.0, 0, 'f', 1),
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::No) {
            ui->connect_status->append("已取消添加导航点(PTZ数据过时)");
            return;
        }
        ui->connect_status->append(QString("警告:使用过时的PTZ数据 (%1秒前)").arg(ptzTfAge / 1000.0, 0, 'f', 1));
    }
    
    // 检查云台数据是否有效（在创建点之前检查）
    bool ptzDataValid = true;
    if (m_latestPtzPos_World.isNull() || 
        (m_latestPtzPos_World.x() == 0.0 && m_latestPtzPos_World.y() == 0.0 && m_latestPtzPos_World.z() == 0.0)) {
        ptzDataValid = false;
        qWarning() << "警告：云台位置数据无效 (0, 0, 0)";
        
        // 显示确认对话框
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "云台数据无效", 
            "云台位置数据无效 (0, 0, 0)，可能原因：\n"
            "1. TF变换 map->ptz_link 尚未接收\n"
            "2. TF变换链不完整\n\n"
            "是否仍要添加此路径点？\n"
            "（建议等待几秒后重试）",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::No) {
            ui->connect_status->append("已取消添加路径点");
            return;
        }
        ui->connect_status->append("警告：使用无效的云台数据添加路径点");
    }
    
    // 显示等待提示窗口
    QMessageBox *waitDialog = new QMessageBox(this);
    waitDialog->setWindowTitle("等待设备稳定");
    waitDialog->setText("正在等待设备稳定，请稍候...");
    waitDialog->setStandardButtons(QMessageBox::NoButton);
    waitDialog->setModal(true);
    waitDialog->show();
    
    // 处理事件以显示窗口
    QApplication::processEvents();
    
    // 等待2秒
    QTimer::singleShot(2000, this, [this, waitDialog, ptzDataValid]() {
        // 2秒后关闭等待窗口
        waitDialog->close();
        waitDialog->deleteLater();
        
        // 1. 创建 JSON 对象 (符合你的新格式)
        QJsonObject pointObj;
        pointObj["type"] = "navigation";
        // 格式: "pos": [x, y]
        pointObj["pos"] = QJsonArray{m_latestRobotPos_World.x(), m_latestRobotPos_World.y()};
        pointObj["yaw"] = m_latestRobotYaw;
        pointObj["index"] = m_pointCounter;
        
        // 添加云台位置和方向信息
        QJsonObject ptzInfo;
        ptzInfo["position"] = QJsonArray{
            m_latestPtzPos_World.x(),
            m_latestPtzPos_World.y(),
            m_latestPtzPos_World.z()
        };
        ptzInfo["orientation"] = QJsonArray{
            m_latestPtzRotation.x(),
            m_latestPtzRotation.y(),
            m_latestPtzRotation.z(),
            m_latestPtzRotation.scalar()
        };
        pointObj["ptzInfo"] = ptzInfo;
        pointObj["zoomValue"] = m_currentPtzZoom;

        // 输出调试信息
        qDebug() << "添加导航点 #" << m_pointCounter 
                 << " 位置:" << m_latestRobotPos_World 
                 << " yaw:" << m_latestRobotYaw << "(" << qRadiansToDegrees(m_latestRobotYaw) << "度)"
                 << " PTZ位置:" << m_latestPtzPos_World
                 << " PTZ方向:" << m_latestPtzRotation
                 << " 缩放:" << m_currentPtzZoom
                 << (ptzDataValid ? "[有效]" : "[无效-警告]");
        
        // 2. 添加到内存数据列表（仅临时存储，不保存到文件）
        m_pointList.append(pointObj); //

        // 3. 在地图上绘制 (使用当前红点位置) - 拓扑地图模式下已禁用
        // QPointF scenePos = convertWorldToScene(m_latestRobotPos_World);
        // addPointToMap(scenePos, "navigation", m_pointCounter, m_latestRobotYaw);

        m_pointCounter++; //
        
        // 有点后启用相关按钮
        ui->btn_UndoLastPoint->setEnabled(true);
        ui->btn_ClearAllPoints->setEnabled(true);
        ui->btn_SavePlan->setEnabled(true);
        
        ui->connect_status->append(QString("已添加路径点 #%1: 位置(%.2f, %.2f), 朝向%.1f°")
            .arg(m_pointCounter - 1)
            .arg(m_latestRobotPos_World.x())
            .arg(m_latestRobotPos_World.y())
            .arg(qRadiansToDegrees(m_latestRobotYaw)));
    });
}

/**
 * @brief [新] 槽函数：捕捉当前机器人和云台位置，添加为巡检点
 */
void robot::on_btn_AddInspectionPoint_clicked()
{
    // 检查是否已收到机器人位置数据
    if (m_latestRobotPos_World.isNull() || (m_latestRobotPos_World.x() == 0.0 && m_latestRobotPos_World.y() == 0.0)) {
        QMessageBox::warning(this, "数据缺失", 
            "尚未收到机器人位置数据，请检查：\n"
            "1. WebSocket是否已连接\n"
            "2. /tf 话题是否在发布\n"
            "3. map->base_link的TF变换是否存在");
        ui->connect_status->append("错误：无法获取机器人位置，当前位置为 (0, 0)");
        return;
    }
    
    // 检查TF数据新鲜度 (TF以1Hz发布，检查是否在1.5秒内收到过数据)
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 robotTfAge = currentTime - m_lastRobotTfTimestamp;
    qint64 ptzTfAge = currentTime - m_lastPtzTfTimestamp;
    
    if (robotTfAge > 1500 && m_lastRobotTfTimestamp > 0) {
        QMessageBox::warning(this, "数据过时", 
            QString("机器人TF数据已过时 (%1 秒前)，请稍等片刻后重试。\n\n"
                    "TF数据以1Hz频率发布，建议等待1-2秒后再添加点。")
            .arg(robotTfAge / 1000.0, 0, 'f', 1));
        ui->connect_status->append(QString("警告:机器人TF数据过时 (%1秒前)").arg(robotTfAge / 1000.0, 0, 'f', 1));
        return;
    }
    
    if (ptzTfAge > 1500 && m_lastPtzTfTimestamp > 0) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "PTZ数据过时", 
            QString("PTZ TF数据已过时 (%1 秒前)。\n\n"
                    "TF数据以1Hz频率发布，建议等待1-2秒后再添加点。\n\n"
                    "是否仍要使用过时数据添加此巡检点?")
            .arg(ptzTfAge / 1000.0, 0, 'f', 1),
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::No) {
            ui->connect_status->append("已取消添加巡检点(PTZ数据过时)");
            return;
        }
        ui->connect_status->append(QString("警告:使用过时的PTZ数据 (%1秒前)").arg(ptzTfAge / 1000.0, 0, 'f', 1));
    }
    
    // 检查云台数据是否有效（在创建点之前检查）
    bool ptzDataValid = true;
    if (m_latestPtzPos_World.isNull() || 
        (m_latestPtzPos_World.x() == 0.0 && m_latestPtzPos_World.y() == 0.0 && m_latestPtzPos_World.z() == 0.0)) {
        ptzDataValid = false;
        qWarning() << "警告：云台位置数据无效 (0, 0, 0)";
        
        // 显示确认对话框
        QMessageBox::StandardButton reply = QMessageBox::question(this, 
            "云台数据无效", 
            "云台位置数据无效 (0, 0, 0)，可能原因：\n"
            "1. TF变换 map->ptz_link 尚未接收\n"
            "2. TF变换链不完整\n\n"
            "是否仍要添加此巡检点？\n"
            "（建议等待几秒后重试）",
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::No) {
            ui->connect_status->append("已取消添加巡检点");
            return;
        }
        ui->connect_status->append("警告：使用无效的云台数据添加巡检点");
    }

    // 显示等待提示窗口
    QMessageBox *waitDialog = new QMessageBox(this);
    waitDialog->setWindowTitle("等待设备稳定");
    waitDialog->setText("正在等待设备稳定，请稍候...");
    waitDialog->setStandardButtons(QMessageBox::NoButton);
    waitDialog->setModal(true);
    waitDialog->show();
    
    // 处理事件以显示窗口
    QApplication::processEvents();
    
    // 等待2秒
    QTimer::singleShot(2000, this, [this, waitDialog, ptzDataValid]() {
        // 2秒后关闭等待窗口
        waitDialog->close();
        waitDialog->deleteLater();
        
        // 1. 创建 JSON 对象
        QJsonObject pointObj;
        pointObj["type"] = "inspection";
        pointObj["pos"] = QJsonArray{m_latestRobotPos_World.x(), m_latestRobotPos_World.y()};
        pointObj["yaw"] = m_latestRobotYaw;
        pointObj["index"] = m_pointCounter;
        
        // 添加云台位置和方向信息（与导航点相同的格式）
        QJsonObject ptzInfo;
        ptzInfo["position"] = QJsonArray{
            m_latestPtzPos_World.x(),
            m_latestPtzPos_World.y(),
            m_latestPtzPos_World.z()
        };
        ptzInfo["orientation"] = QJsonArray{
            m_latestPtzRotation.x(),
            m_latestPtzRotation.y(),
            m_latestPtzRotation.z(),
            m_latestPtzRotation.scalar()
        };
        pointObj["ptzInfo"] = ptzInfo;
        pointObj["zoomValue"] = m_currentPtzZoom;

        // 输出详细调试信息
        qDebug() << "========== 添加巡检点 #" << m_pointCounter << " ==========";
        qDebug() << "机器人位置:" << m_latestRobotPos_World;
        qDebug() << "机器人朝向:" << m_latestRobotYaw << "弧度 (" << qRadiansToDegrees(m_latestRobotYaw) << "度)";
        qDebug() << "PTZ位置:" << m_latestPtzPos_World << (ptzDataValid ? "[✓有效]" : "[✗无效-警告]");
        qDebug() << "PTZ方向(四元数):" << m_latestPtzRotation << (ptzDataValid ? "[✓有效]" : "[✗无效-警告]");
        qDebug() << "Zoom滑块值:" << m_currentPtzZoom << "(1-23范围)";
        qDebug() << "============================================";

        // 2. 添加到内存数据列表（仅临时存储，不保存到文件）
        m_pointList.append(pointObj);

        // 3. 在地图上绘制 - 拓扑地图模式下已禁用
        // QPointF scenePos = convertWorldToScene(m_latestRobotPos_World);
        // addPointToMap(scenePos, "inspection", m_pointCounter, m_latestRobotYaw);

        m_pointCounter++;
        
        // 有点后启用相关按钮
        ui->btn_UndoLastPoint->setEnabled(true);
        ui->btn_ClearAllPoints->setEnabled(true);
        ui->btn_SavePlan->setEnabled(true);
        
        ui->connect_status->append(QString("已添加巡检点 #%1: 位置(%.2f, %.2f), 朝向%.1f°")
            .arg(m_pointCounter - 1)
            .arg(m_latestRobotPos_World.x())
            .arg(m_latestRobotPos_World.y())
            .arg(qRadiansToDegrees(m_latestRobotYaw)));
    });
}

void robot::on_btn_UndoLastPoint_clicked()
{


    // 1. 检查列表是否为空
    if (m_pointList.isEmpty()) { //
        ui->connect_status->append("没有可撤回的点。");
        return;
    }

    // 2. 从数据列表中移除最后一个点
    m_pointList.pop_back();

    // 3. m_pointCounter 是“下一个”点的编号，所以 (m_pointCounter - 1) 是刚刚那个点
    m_pointCounter--; //
    QString tagToRemove = "path_point_group_" + QString::number(m_pointCounter);

    // 4. 遍历场景，移除所有带这个标签的图形项 (点、线、文字)
    QList<QGraphicsItem*> itemsToRemove;
    for (QGraphicsItem* item : m_mapScene->items()) { //
        // 检查 key 1 (我们用于分组的key)
        if (item->data(1) == tagToRemove) {
            itemsToRemove.append(item);
        }
    }

    // 5. 安全地删除它们
    for (QGraphicsItem* item : itemsToRemove) {
        m_mapScene->removeItem(item);
        delete item;
    }

    ui->connect_status->append(QString("已撤回点 %1。").arg(m_pointCounter));
    
    // 如果没有点了，禁用相关按钮
    if (m_pointList.isEmpty()) {
        ui->btn_UndoLastPoint->setEnabled(false);
        ui->btn_ClearAllPoints->setEnabled(false);
        ui->btn_SavePlan->setEnabled(false);
    }
}

void robot::on_btn_ClearAllPoints_clicked()
{


        // 1. 清空数据列表
        m_pointList.clear(); //
        m_pointCounter = 1; //

        // 2. 遍历场景，移除所有属于路径点的图形项
        QList<QGraphicsItem*> itemsToRemove;
        for (QGraphicsItem* item : m_mapScene->items()) { //
            // 检查 key 1 (我们用于分组的key) 是否有值
            // (isNull() 检查它是否被设置过)
            // (我们不能移除 key=0 的 temp_marker，也不能移除没有 key 的地图)
            if (!item->data(1).isNull()) {
                itemsToRemove.append(item);
            }
        }

        for (QGraphicsItem* item : itemsToRemove) {
            m_mapScene->removeItem(item);
            delete item;
        }

        ui->connect_status->append("已清除所有路径点。");
        
        // 清除后禁用相关按钮
        ui->btn_UndoLastPoint->setEnabled(false);
        ui->btn_ClearAllPoints->setEnabled(false);
        ui->btn_SavePlan->setEnabled(false);
}

void robot::on_btn_SavePlan_clicked()
{
    // 1. 检查是否有数据
    if (m_pointList.isEmpty()) {
        QMessageBox::warning(this, "列表为空", "没有可保存的路径点。");
        return;
    }

    // 2. 弹出文件保存对话框
    QString filePath = QFileDialog::getSaveFileName(this,
                                                "保存巡检方案",
                                                QDir::currentPath(),
                                                "巡检方案 (*.json)");
    if (filePath.isEmpty()) {
        return; // 用户取消
    }

    // 3. 构建新的 JSON 结构，按照指定顺序添加字段
    QJsonObject rootObj;
    
    // 分别存储导航点和巡检点
    QJsonArray navPoints;
    QJsonArray patrolPoints;
    int globalOrder = 0;
    
    // 4. 遍历所有点，按类型分类
    for (const QJsonObject &point : m_pointList) {
        QString type = point.value("type").toString();
        QJsonArray posArray = point.value("pos").toArray();
        double x = posArray[0].toDouble();
        double y = posArray[1].toDouble();
        double yaw = point.value("yaw").toDouble();
        
        QJsonObject newPoint;
        newPoint["x"] = x;
        newPoint["y"] = y;
        newPoint["order"] = globalOrder++;
        
        // 添加云台位置信息（所有点都使用ptzInfo格式）
        if (point.contains("ptzInfo")) {
            QJsonObject ptzInfo = point.value("ptzInfo").toObject();
            QJsonArray ptzPosArray = ptzInfo.value("position").toArray();
            QJsonArray ptzOrientArray = ptzInfo.value("orientation").toArray();
            
            QJsonObject ptzPose;
            QJsonObject position;
            position["x"] = ptzPosArray[0].toDouble();
            position["y"] = ptzPosArray[1].toDouble();
            position["z"] = ptzPosArray[2].toDouble();
            
            QJsonObject orientation;
            orientation["x"] = ptzOrientArray[0].toDouble();
            orientation["y"] = ptzOrientArray[1].toDouble();
            orientation["z"] = ptzOrientArray[2].toDouble();
            orientation["w"] = ptzOrientArray[3].toDouble();
            
            ptzPose["position"] = position;
            ptzPose["orientation"] = orientation;
            newPoint["ptzPose"] = ptzPose;
            newPoint["zoomValue"] = point.value("zoomValue").toInt(16);
        }
        
        if (type == "navigation") {
            navPoints.append(newPoint);
        } else if (type == "inspection") {
            newPoint["yaw"] = yaw; // 巡检点需要yaw
            patrolPoints.append(newPoint);
        }
    }
    
    // 按照指定顺序添加字段到JSON对象
    rootObj["version"] = "1.0";
    rootObj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    rootObj["pointOrder"] = globalOrder;
    rootObj["navPoints"] = navPoints;
    rootObj["patrolPoints"] = patrolPoints;
    
    // 添加统计信息
    QJsonObject summary;
    summary["navPointsCount"] = navPoints.size();
    summary["patrolPointsCount"] = patrolPoints.size();
    summary["totalPoints"] = globalOrder;

    // 5. 手动构建JSON字符串以保持字段顺序
    QString jsonString = "{\n";
    jsonString += "  \"version\": \"1.0\",\n";
    jsonString += "  \"timestamp\": \"" + QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + "\",\n";
    jsonString += "  \"pointOrder\": " + QString::number(globalOrder) + ",\n";
    
    // 手动构建导航点数组，保持字段顺序: x, y, order, ptzPose, zoomValue
    jsonString += "  \"navPoints\": [\n";
    for (int i = 0; i < navPoints.size(); ++i) {
        QJsonObject pt = navPoints[i].toObject();
        jsonString += "    {\n";
        jsonString += "      \"x\": " + QString::number(pt["x"].toDouble(), 'g', 17) + ",\n";
        jsonString += "      \"y\": " + QString::number(pt["y"].toDouble(), 'g', 17) + ",\n";
        jsonString += "      \"order\": " + QString::number(pt["order"].toInt()) + ",\n";
        
        // ptzPose
        if (pt.contains("ptzPose")) {
            QJsonObject ptzPose = pt["ptzPose"].toObject();
            QJsonObject position = ptzPose["position"].toObject();
            QJsonObject orientation = ptzPose["orientation"].toObject();
            
            jsonString += "      \"ptzPose\": {\n";
            jsonString += "        \"position\": {\n";
            jsonString += "          \"x\": " + QString::number(position["x"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"y\": " + QString::number(position["y"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"z\": " + QString::number(position["z"].toDouble(), 'g', 17) + "\n";
            jsonString += "        },\n";
            jsonString += "        \"orientation\": {\n";
            jsonString += "          \"x\": " + QString::number(orientation["x"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"y\": " + QString::number(orientation["y"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"z\": " + QString::number(orientation["z"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"w\": " + QString::number(orientation["w"].toDouble(), 'g', 17) + "\n";
            jsonString += "        }\n";
            jsonString += "      },\n";
        }
        
        jsonString += "      \"zoomValue\": " + QString::number(pt["zoomValue"].toInt()) + "\n";
        jsonString += "    }";
        if (i < navPoints.size() - 1) jsonString += ",";
        jsonString += "\n";
    }
    jsonString += "  ],\n";
    
    // 手动构建巡检点数组，保持字段顺序: x, y, yaw, order, ptzPose, zoomValue
    jsonString += "  \"patrolPoints\": [\n";
    for (int i = 0; i < patrolPoints.size(); ++i) {
        QJsonObject pt = patrolPoints[i].toObject();
        jsonString += "    {\n";
        jsonString += "      \"x\": " + QString::number(pt["x"].toDouble(), 'g', 17) + ",\n";
        jsonString += "      \"y\": " + QString::number(pt["y"].toDouble(), 'g', 17) + ",\n";
        jsonString += "      \"yaw\": " + QString::number(pt["yaw"].toDouble(), 'g', 17) + ",\n";
        jsonString += "      \"order\": " + QString::number(pt["order"].toInt()) + ",\n";
        
        // ptzPose
        if (pt.contains("ptzPose")) {
            QJsonObject ptzPose = pt["ptzPose"].toObject();
            QJsonObject position = ptzPose["position"].toObject();
            QJsonObject orientation = ptzPose["orientation"].toObject();
            
            jsonString += "      \"ptzPose\": {\n";
            jsonString += "        \"position\": {\n";
            jsonString += "          \"x\": " + QString::number(position["x"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"y\": " + QString::number(position["y"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"z\": " + QString::number(position["z"].toDouble(), 'g', 17) + "\n";
            jsonString += "        },\n";
            jsonString += "        \"orientation\": {\n";
            jsonString += "          \"x\": " + QString::number(orientation["x"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"y\": " + QString::number(orientation["y"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"z\": " + QString::number(orientation["z"].toDouble(), 'g', 17) + ",\n";
            jsonString += "          \"w\": " + QString::number(orientation["w"].toDouble(), 'g', 17) + "\n";
            jsonString += "        }\n";
            jsonString += "      },\n";
        }
        
        jsonString += "      \"zoomValue\": " + QString::number(pt["zoomValue"].toInt()) + "\n";
        jsonString += "    }";
        if (i < patrolPoints.size() - 1) jsonString += ",";
        jsonString += "\n";
    }
    jsonString += "  ],\n";
    
    // 添加统计信息
    jsonString += "  \"summary\": {\n";
    jsonString += "    \"navPointsCount\": " + QString::number(navPoints.size()) + ",\n";
    jsonString += "    \"patrolPointsCount\": " + QString::number(patrolPoints.size()) + ",\n";
    jsonString += "    \"totalPoints\": " + QString::number(globalOrder) + "\n";
    jsonString += "  }\n";
    jsonString += "}";

    // 6. 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "保存失败", "无法写入文件: " + file.errorString());
        return;
    }

    file.write(jsonString.toUtf8());
    file.close();

    ui->connect_status->append("巡检方案已保存: " + filePath);
    ui->connect_status->append(QString("包含 %1 个导航点, %2 个巡检点")
        .arg(navPoints.size()).arg(patrolPoints.size()));
    QMessageBox::information(this, "保存成功", 
        QString("巡检方案已成功保存。\n导航点: %1\n巡检点: %2")
        .arg(navPoints.size()).arg(patrolPoints.size()));
}

void robot::on_DetailTableView_doubleClicked(const QModelIndex &index)
{
    // 1. 检查点击是否有效
    if (!index.isValid()) {
        return;
    }

    // 2. 获取被点击单元格中的数据 (URL 字符串)
    //    (注意：QTableView 可能有代理模型 QSortFilterProxyModel)
    QSortFilterProxyModel* proxyModel = qobject_cast<QSortFilterProxyModel*>(ui->DetailTableView->model());
    QModelIndex sourceIndex;
    if (proxyModel) {
        sourceIndex = proxyModel->mapToSource(index);
    } else {
        sourceIndex = index;
    }

    QString urlString = sourceIndex.model()->data(sourceIndex, Qt::DisplayRole).toString();

    // 3. 检查这是否是一个我们关心的列 (图片或视频)
    //    根据 dataBase::showRpmore，第 5 列是图片，第 6 列是视频
    int column = sourceIndex.column();
    if (column != 5 && column != 6) {
        qDebug() << "Double-clicked on a non-URL column, ignoring.";
        return;
    }

    // 4. 检查 URL 是否为空
    if (urlString.isEmpty()) {
        qDebug() << "URL is empty, nothing to open.";
        return;
    }

    // 5. 尝试在系统默认浏览器中打开这个 URL
    if (!QDesktopServices::openUrl(QUrl(urlString))) {
        QMessageBox::warning(this, "打开失败", "无法打开链接:\n" + urlString);
    }
}

// 旧的MJPEG HTTP函数已删除,改用TCP直连
// 看门狗超时检查函数
void robot::checkVideoTimeout()
{
    // 检查距离最后一帧是否超过10秒
    qint64 secondsSinceLastFrame = m_lastVideoFrame.secsTo(QDateTime::currentDateTime());
    
    if (secondsSinceLastFrame > 10) {
        qDebug() << "视频流超时!正在重连...";
        ui->connect_status->append(QString("视频流超时(%1秒无新帧),正在重连...").arg(secondsSinceLastFrame));
        
        // 断开并重连TCP
        if (m_videoTcpSocket->state() == QAbstractSocket::ConnectedState) {
            m_videoTcpSocket->disconnectFromHost();
        }
        m_videoBuffer.clear();
        
        // 重新连接
        m_lastVideoFrame = QDateTime::currentDateTime();
        QTimer::singleShot(1000, this, &robot::connectVideoStream);
    }
}

void robot::checkInfraredTimeout()
{
    // 检查距离最后一帧是否超过10秒
    qint64 secondsSinceLastFrame = m_lastInfraredFrame.secsTo(QDateTime::currentDateTime());
    
    if (secondsSinceLastFrame > 10) {
        qDebug() << "红外流超时!正在重连...";
        ui->connect_status->append(QString("红外流超时(%1秒无新帧),正在重连...").arg(secondsSinceLastFrame));
        
        // 断开并重连TCP
        if (m_infraredTcpSocket->state() == QAbstractSocket::ConnectedState) {
            m_infraredTcpSocket->disconnectFromHost();
        }
        m_infraredBuffer.clear();
        
        // 重新连接
        m_lastInfraredFrame = QDateTime::currentDateTime();
        QTimer::singleShot(1000, this, &robot::connectInfraredStream);
    }
}

// ==================== TCP视频流处理函数 ====================

void robot::connectVideoStream()
{
    if (m_videoTcpSocket->state() == QAbstractSocket::ConnectedState ||
        m_videoTcpSocket->state() == QAbstractSocket::ConnectingState) {
        qDebug() << "视频流已经在连接中,状态:" << m_videoTcpSocket->state();
        return;  // 已经连接或正在连接
    }
    
    qDebug() << "===== 连接视频TCP流 =====";
    qDebug() << "主机:" << m_videoHost;
    qDebug() << "端口:" << m_videoPort;
    m_videoBuffer.clear();
    m_videoTcpSocket->connectToHost(m_videoHost, m_videoPort);
}

void robot::processVideoData()
{
    // 读取所有可用数据
    QByteArray newData = m_videoTcpSocket->readAll();
    m_videoBuffer.append(newData);
    
    // 解析JPEG帧
    while (true) {
        // 查找JPEG起始标记 0xFF 0xD8
        int startPos = m_videoBuffer.indexOf("\xFF\xD8");
        if (startPos == -1) {
            // 没有找到起始标记,保留所有数据等待更多
            break;
        }
        
        // 查找JPEG结束标记 0xFF 0xD9
        int endPos = m_videoBuffer.indexOf("\xFF\xD9", startPos + 2);
        if (endPos == -1) {
            // 没有找到结束标记,说明帧不完整
            // 如果缓冲区太大,丢弃起始标记之前的数据
            if (startPos > 0) {
                m_videoBuffer.remove(0, startPos);
            }
            break;
        }
        
        // 提取完整的JPEG数据(包括结束标记)
        QByteArray jpegData = m_videoBuffer.mid(startPos, endPos - startPos + 2);
        
        // 从缓冲区删除已处理的数据
        m_videoBuffer.remove(0, endPos + 2);
        
        // 使用FFmpeg解码图像
        QImage image = decodeJpegWithFFmpeg(jpegData, m_videoCodecCtx, m_videoFrame, 
                                           m_videoSwFrame, &m_videoSwsCtx, m_videoHwAccelEnabled);
        if (!image.isNull()) {
            // 更新最后帧时间
            m_lastVideoFrame = QDateTime::currentDateTime();
            
            // 转换为QPixmap并缩放显示
            QPixmap pixmap = QPixmap::fromImage(image);
            int w = ui->label_video->width();
            int h = ui->label_video->height();
            ui->label_video->setPixmap(pixmap.scaled(w, h, 
                                                     Qt::KeepAspectRatio, 
                                                     Qt::SmoothTransformation));
            ui->label_video->update();
            
            qDebug() << "视频帧:" << image.size() << "字节:" << jpegData.size();
        } else {
            qDebug() << "视频JPEG解码失败,大小:" << jpegData.size();
        }
    }
    
    // 防止缓冲区积压 - 保持低延迟
    if (m_videoBuffer.size() > 200 * 1024) {  // 超过200KB
        qDebug() << "视频缓冲区积压:" << m_videoBuffer.size() << "bytes,清理中...";
        // 查找最后一个起始标记
        int lastStart = m_videoBuffer.lastIndexOf("\xFF\xD8");
        if (lastStart > 0) {
            // 只保留最后一个未完成的帧
            m_videoBuffer = m_videoBuffer.mid(lastStart);
        } else {
            // 没有找到起始标记,清空
            m_videoBuffer.clear();
        }
    }
}

void robot::connectInfraredStream()
{
    if (m_infraredTcpSocket->state() == QAbstractSocket::ConnectedState ||
        m_infraredTcpSocket->state() == QAbstractSocket::ConnectingState) {
        qDebug() << "红外流已经在连接中,状态:" << m_infraredTcpSocket->state();
        return;  // 已经连接或正在连接
    }
    
    qDebug() << "===== 连接红外TCP流 =====";
    qDebug() << "主机:" << m_videoHost;
    qDebug() << "端口:" << m_infraredPort;
    m_infraredBuffer.clear();
    m_infraredTcpSocket->connectToHost(m_videoHost, m_infraredPort);
}

void robot::processInfraredData()
{
    // 读取所有可用数据
    QByteArray newData = m_infraredTcpSocket->readAll();
    m_infraredBuffer.append(newData);
    
    // 解析JPEG帧
    while (true) {
        // 查找JPEG起始标记 0xFF 0xD8
        int startPos = m_infraredBuffer.indexOf("\xFF\xD8");
        if (startPos == -1) {
            // 没有找到起始标记,保留所有数据等待更多
            break;
        }
        
        // 查找JPEG结束标记 0xFF 0xD9
        int endPos = m_infraredBuffer.indexOf("\xFF\xD9", startPos + 2);
        if (endPos == -1) {
            // 没有找到结束标记,说明帧不完整
            // 如果缓冲区太大,丢弃起始标记之前的数据
            if (startPos > 0) {
                m_infraredBuffer.remove(0, startPos);
            }
            break;
        }
        
        // 提取完整的JPEG数据(包括结束标记)
        QByteArray jpegData = m_infraredBuffer.mid(startPos, endPos - startPos + 2);
        
        // 从缓冲区删除已处理的数据
        m_infraredBuffer.remove(0, endPos + 2);
        
        // 使用FFmpeg解码图像
        QImage image = decodeJpegWithFFmpeg(jpegData, m_infraredCodecCtx, m_infraredFrame, 
                                           m_infraredSwFrame, &m_infraredSwsCtx, m_infraredHwAccelEnabled);
        if (!image.isNull()) {
            // 更新最后帧时间
            m_lastInfraredFrame = QDateTime::currentDateTime();
            
            // 转换为QPixmap并缩放显示
            QPixmap pixmap = QPixmap::fromImage(image);
            int w = ui->label_infrared->width();
            int h = ui->label_infrared->height();
            ui->label_infrared->setPixmap(pixmap.scaled(w, h, 
                                                        Qt::KeepAspectRatio, 
                                                        Qt::SmoothTransformation));
            ui->label_infrared->update();
            
            qDebug() << "红外帧:" << image.size() << "字节:" << jpegData.size();
        } else {
            qDebug() << "红外JPEG解码失败,大小:" << jpegData.size();
        }
    }
    
    // 防止缓冲区积压 - 保持低延迟
    if (m_infraredBuffer.size() > 200 * 1024) {  // 超过200KB
        qDebug() << "红外缓冲区积压:" << m_infraredBuffer.size() << "bytes,清理中...";
        // 查找最后一个起始标记
        int lastStart = m_infraredBuffer.lastIndexOf("\xFF\xD8");
        if (lastStart > 0) {
            // 只保留最后一个未完成的帧
            m_infraredBuffer = m_infraredBuffer.mid(lastStart);
        } else {
            // 没有找到起始标记,清空
            m_infraredBuffer.clear();
        }
    }
}

// ========== FFmpeg硬件解码实现 ==========

/**
 * @brief 初始化FFmpeg MJPEG解码器,自动尝试硬件加速
 * @param codecCtx 解码器上下文指针的指针
 * @param frame 解码帧指针的指针
 * @param swFrame 软件帧指针的指针(用于硬件解码后传输到CPU)
 * @param packet 数据包指针的指针
 * @param hwAccelEnabled 硬件加速是否启用的标志
 * @param decoderName 解码器名称(如"mjpeg")
 * @return 初始化成功返回true,失败返回false
 */
bool robot::initFFmpegDecoder(AVCodecContext** codecCtx, AVFrame** frame, AVFrame** swFrame, 
                             AVPacket** packet, bool* hwAccelEnabled, const char* decoderName)
{
    *hwAccelEnabled = false;
    
    // 1. 查找MJPEG解码器
    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_MJPEG);
    if (!codec) {
        qDebug() << "未找到MJPEG解码器";
        return false;
    }
    
    // 2. 分配解码器上下文
    *codecCtx = avcodec_alloc_context3(codec);
    if (!*codecCtx) {
        qDebug() << "无法分配解码器上下文";
        return false;
    }
    
    // 3. 尝试启用硬件加速 - 优先级: DXVA2 > D3D11VA > 软件解码
    AVBufferRef* hw_device_ctx = nullptr;
    
    // 尝试DXVA2 (适用于大多数显卡)
    if (av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_DXVA2, nullptr, nullptr, 0) == 0) {
        (*codecCtx)->hw_device_ctx = av_buffer_ref(hw_device_ctx);
        *hwAccelEnabled = true;
        qDebug() << decoderName << "硬件解码: DXVA2已启用";
        av_buffer_unref(&hw_device_ctx);
    }
    // 尝试D3D11VA (Windows 8+)
    else if (av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_D3D11VA, nullptr, nullptr, 0) == 0) {
        (*codecCtx)->hw_device_ctx = av_buffer_ref(hw_device_ctx);
        *hwAccelEnabled = true;
        qDebug() << decoderName << "硬件解码: D3D11VA已启用";
        av_buffer_unref(&hw_device_ctx);
    }
    else {
        qDebug() << decoderName << "硬件加速不可用,使用软件解码";
    }
    
    // 4. 打开解码器
    if (avcodec_open2(*codecCtx, codec, nullptr) < 0) {
        qDebug() << decoderName << "无法打开解码器";
        avcodec_free_context(codecCtx);
        return false;
    }
    
    // 5. 分配帧结构
    *frame = av_frame_alloc();
    *swFrame = av_frame_alloc();
    *packet = av_packet_alloc();
    
    if (!*frame || !*swFrame || !*packet) {
        qDebug() << decoderName << "无法分配帧或数据包";
        if (*frame) av_frame_free(frame);
        if (*swFrame) av_frame_free(swFrame);
        if (*packet) av_packet_free(packet);
        avcodec_free_context(codecCtx);
        return false;
    }
    
    qDebug() << decoderName << "解码器初始化成功,硬件加速:" << (*hwAccelEnabled ? "是" : "否");
    return true;
}

/**
 * @brief 清理FFmpeg解码器资源
 */
void robot::cleanupFFmpegDecoder(AVCodecContext** codecCtx, AVFrame** frame, AVFrame** swFrame, 
                                SwsContext** swsCtx, AVPacket** packet)
{
    if (*swsCtx) {
        sws_freeContext(*swsCtx);
        *swsCtx = nullptr;
    }
    if (*frame) {
        av_frame_free(frame);
    }
    if (*swFrame) {
        av_frame_free(swFrame);
    }
    if (*packet) {
        av_packet_free(packet);
    }
    if (*codecCtx) {
        avcodec_free_context(codecCtx);
    }
}

/**
 * @brief 使用FFmpeg解码JPEG数据
 * @param jpegData JPEG压缩数据
 * @param codecCtx 解码器上下文
 * @param frame 解码帧
 * @param swFrame 软件帧(硬件解码时使用)
 * @param swsCtx 像素格式转换上下文的指针
 * @param hwAccelEnabled 是否启用硬件加速
 * @return 解码后的QImage,失败返回null image
 */
QImage robot::decodeJpegWithFFmpeg(const QByteArray& jpegData, AVCodecContext* codecCtx, 
                                  AVFrame* frame, AVFrame* swFrame, SwsContext** swsCtx, 
                                  bool hwAccelEnabled)
{
    if (!codecCtx || jpegData.isEmpty()) {
        return QImage();
    }
    
    // 1. 准备数据包
    AVPacket* packet = av_packet_alloc();
    packet->data = (uint8_t*)jpegData.data();
    packet->size = jpegData.size();
    
    // 2. 发送数据包到解码器
    int ret = avcodec_send_packet(codecCtx, packet);
    av_packet_free(&packet);
    
    if (ret < 0) {
        qDebug() << "发送数据包到解码器失败:" << ret;
        return QImage();
    }
    
    // 3. 接收解码后的帧
    ret = avcodec_receive_frame(codecCtx, frame);
    if (ret < 0) {
        return QImage();
    }
    
    // 4. 如果是硬件解码,需要将数据从GPU传输到CPU
    AVFrame* cpuFrame = frame;
    if (hwAccelEnabled && frame->format != AV_PIX_FMT_YUVJ420P && frame->format != AV_PIX_FMT_YUVJ422P) {
        ret = av_hwframe_transfer_data(swFrame, frame, 0);
        if (ret < 0) {
            qDebug() << "硬件帧传输失败,回退到软件解码";
            return QImage();
        }
        cpuFrame = swFrame;
    }
    
    // 5. 转换像素格式 YUV -> RGB888
    int width = cpuFrame->width;
    int height = cpuFrame->height;
    
    // 创建或重用SwsContext
    if (!*swsCtx) {
        *swsCtx = sws_getContext(
            width, height, (AVPixelFormat)cpuFrame->format,
            width, height, AV_PIX_FMT_RGB24,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }
    
    if (!*swsCtx) {
        qDebug() << "无法创建像素格式转换上下文";
        return QImage();
    }
    
    // 6. 分配RGB缓冲区
    QImage image(width, height, QImage::Format_RGB888);
    uint8_t* dest[1] = { image.bits() };
    int destLinesize[1] = { (int)image.bytesPerLine() };
    
    // 7. 执行格式转换
    sws_scale(*swsCtx, cpuFrame->data, cpuFrame->linesize, 0, height, dest, destLinesize);
    
    return image;
}

// ========== 点云相关实现 ==========

/**
 * @brief 订阅点云话题
 */
void robot::subscribeToPointClouds()
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "WebSocket未连接,无法订阅点云";
        return;
    }
    
    // 订阅 /globalmap_downsample 点云
    QJsonObject subscribeGlobalMap;
    subscribeGlobalMap["op"] = "subscribe";
    subscribeGlobalMap["topic"] = "/globalmap_downsample";
    subscribeGlobalMap["type"] = "sensor_msgs/PointCloud2";
    subscribeGlobalMap["throttle_rate"] = 1000; // 1Hz,避免数据量过大
    
    QJsonDocument docGlobal(subscribeGlobalMap);
    m_webSocket->sendTextMessage(docGlobal.toJson(QJsonDocument::Compact));
    
    // 订阅 /aligned_points_downsample 点云
    QJsonObject subscribeAligned;
    subscribeAligned["op"] = "subscribe";
    subscribeAligned["topic"] = "/aligned_points_downsample";
    subscribeAligned["type"] = "sensor_msgs/PointCloud2";
    subscribeAligned["throttle_rate"] = 500; // 2Hz
    
    QJsonDocument docAligned(subscribeAligned);
    m_webSocket->sendTextMessage(docAligned.toJson(QJsonDocument::Compact));
    
    // 订阅 /tf 话题
    QJsonObject subscribeTF;
    subscribeTF["op"] = "subscribe";
    subscribeTF["topic"] = "/tf";
    subscribeTF["type"] = "tf2_msgs/TFMessage";
    subscribeTF["throttle_rate"] = 100; // 10Hz
    
    QJsonDocument docTF(subscribeTF);
    m_webSocket->sendTextMessage(docTF.toJson(QJsonDocument::Compact));
    
    // 订阅 /tf_static 话题（静态变换）
    QJsonObject subscribeTFStatic;
    subscribeTFStatic["op"] = "subscribe";
    subscribeTFStatic["topic"] = "/tf_static";
    subscribeTFStatic["type"] = "tf2_msgs/TFMessage";
    
    QJsonDocument docTFStatic(subscribeTFStatic);
    m_webSocket->sendTextMessage(docTFStatic.toJson(QJsonDocument::Compact));
    
    qDebug() << "已订阅点云话题: /globalmap_downsample 和 /aligned_points_downsample";
    qDebug() << "已订阅TF话题: /tf 和 /tf_static (10Hz)";
    ui->connect_status->append("已订阅点云话题: /globalmap_downsample 和 /aligned_points_downsample");
    ui->connect_status->append("已订阅TF话题: /tf 和 /tf_static");
}

/**
 * @brief 订阅仪表读数话题
 */
void robot::subscribeToMeterReading()
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "WebSocket未连接,无法订阅仪表读数";
        return;
    }
    
    // 避免重复订阅
    if (m_isMeterReadingSubscribed) {
        qDebug() << "仪表读数话题已订阅";
        return;
    }
    
    // 订阅 /meter_reading 话题
    QJsonObject subscribeMeter;
    subscribeMeter["op"] = "subscribe";
    subscribeMeter["topic"] = "/meter_reading";
    subscribeMeter["type"] = "std_msgs/Float32";
    
    QJsonDocument docMeter(subscribeMeter);
    m_webSocket->sendTextMessage(docMeter.toJson(QJsonDocument::Compact));
    
    m_isMeterReadingSubscribed = true;
    
    qDebug() << "已订阅仪表读数话题: /meter_reading";
    ui->connect_status->append("已订阅仪表读数话题: /meter_reading");
}

/**
 * @brief 处理仪表读数消息并存入数据库
 */
void robot::onMeterReadingReceived(float reading)
{
    qDebug() << "========================================";
    qDebug() << "收到仪表读数:" << reading;
    qDebug() << "========================================";
    
    // 存储读数到列表
    m_meterReadings.append(reading);
    
    // 如果当前有正在进行的任务，记录到数据库
    if (!m_currentTaskId.isEmpty()) {
        // 生成巡检点ID（使用读数序号）
        QString pointId = QString("meter_%1").arg(m_meterReadings.size());
        
        // 格式化读数为字符串
        QString readingStr = QString::number(reading, 'f', 3); // 保留3位小数
        
        // 确定状态（可根据实际需求设置阈值判断）
        QString status = "正常";
        // 示例：如果读数超出某个范围，标记为异常
        // if (reading < 0.3 || reading > 0.7) {
        //     status = "疑似故障";
        // }
        
        // 记录到数据库
        bool success = dbase->add_report_detail(
            m_currentTaskId,           // 任务ID
            pointId,                   // 巡检点编号
            "仪表读数传感器",          // 设备名称
            readingStr + " MPa",      // 设备数据
            status,                    // 状态
            "",                        // 图片路径（仪表读数无图片）
            ""                         // 视频路径
        );
        
        if (success) {
            qDebug() << "✓ 仪表读数已存入数据库: " << readingStr << " MPa";
            ui->connect_status->append(QString("仪表读数 #%1: %2 MPa [%3]")
                .arg(m_meterReadings.size())
                .arg(readingStr)
                .arg(status));
            
            // 自动刷新报告详情表格显示最新数据
            dbase->showRpmore(ui->DetailTableView, m_currentTaskId);
        } else {
            qDebug() << "✗ 仪表读数存入数据库失败";
        }
    } else {
        qDebug() << "⚠ 当前无任务，仪表读数未存入数据库";
    }
}

/**
 * @brief 处理点云消息
 */
void robot::onPointCloudMessageReceived(const QString &topic, const QJsonObject &msg)
{
    qDebug() << "收到点云消息:" << topic;
    
    QVector<QVector3D> points;
    QVector<QVector3D> colors;
    
    // 解析PointCloud2消息
    parsePointCloud2(msg, points, colors);
    
    if (points.isEmpty()) {
        qDebug() << "点云解析失败或为空";
        return;
    }
    
    // 更新对应的点云渲染器
    if (topic == "/globalmap_downsample") {
        m_pointCloudView->updateGlobalMap(points, colors);
    } else if (topic == "/aligned_points_downsample") {
        m_pointCloudView->updateAlignedPoints(points, colors);
    }
}

/**
 * @brief 解析sensor_msgs/PointCloud2消息
 * 
 * PointCloud2格式说明:
 * - header: 包含时间戳和坐标系
 * - height/width: 点云维度(无序点云height=1)
 * - fields: 字段定义(x,y,z,rgb等)
 * - data: base64编码的二进制数据
 */
void robot::parsePointCloud2(const QJsonObject &msg, QVector<QVector3D> &points, QVector<QVector3D> &colors)
{
    // 获取点云基本信息
    int height = msg.value("height").toInt(1);
    int width = msg.value("width").toInt(0);
    int point_step = msg.value("point_step").toInt(16); // 每个点的字节数
    bool is_bigendian = msg.value("is_bigendian").toBool(false);
    
    if (width == 0) {
        qDebug() << "点云宽度为0";
        return;
    }
    
    int total_points = height * width;
    qDebug() << "点云信息: width=" << width << "height=" << height << "total=" << total_points;
    
    // 获取字段信息 - 只解析xyz
    QJsonArray fields = msg.value("fields").toArray();
    int x_offset = -1, y_offset = -1, z_offset = -1;
    
    for (const QJsonValue &field : fields) {
        QJsonObject fieldObj = field.toObject();
        QString name = fieldObj.value("name").toString();
        int offset = fieldObj.value("offset").toInt();
        
        if (name == "x") x_offset = offset;
        else if (name == "y") y_offset = offset;
        else if (name == "z") z_offset = offset;
    }
    
    if (x_offset < 0 || y_offset < 0 || z_offset < 0) {
        qDebug() << "缺少x/y/z字段";
        return;
    }
    
    // 获取并解码数据
    QString dataBase64 = msg.value("data").toString();
    if (dataBase64.isEmpty()) {
        qDebug() << "点云数据为空";
        return;
    }
    
    QByteArray binaryData = QByteArray::fromBase64(dataBase64.toUtf8());
    
    // 解析点云数据 - 只使用xyz,不使用颜色通道
    points.reserve(total_points);
    // colors留空,让渲染器使用默认颜色
    
    for (int i = 0; i < total_points; ++i) {
        int index = i * point_step;
        
        if (index + point_step > binaryData.size()) {
            qDebug() << "数据索引超出范围:" << index;
            break;
        }
        
        // 读取xyz坐标(假设为float32)
        float x, y, z;
        memcpy(&x, binaryData.constData() + index + x_offset, sizeof(float));
        memcpy(&y, binaryData.constData() + index + y_offset, sizeof(float));
        memcpy(&z, binaryData.constData() + index + z_offset, sizeof(float));
        
        // 过滤无效点(NaN或Inf)
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
            continue;
        }
        
        points.append(QVector3D(x, y, z));
    }
    
    qDebug() << "成功解析" << points.size() << "个点";
}

/**
 * @brief 重置3D视图相机视角
 */
void robot::onResetViewClicked()
{
    if (m_pointCloudView) {
        m_pointCloudView->resetCamera();
        ui->connect_status->append("相机视角已重置");
        qDebug() << "相机视角已重置";
    }
}

/**
 * @brief 2D Pose Estimate - 设置机器人初始位姿
 * 
 * 模拟RViz的2D Pose Estimate功能:
 * 1. 用户在3D视图中点击设置位置
 * 2. 拖拽设置朝向
 * 3. 发布到 /initialpose 话题
 */
void robot::onSetPoseClicked()
{
    if (m_pointCloudView) {
        // 切换到设置位姿模式
        m_pointCloudView->setInteractionMode(PointCloudViewWidget::SetPoseMode);
        ui->connect_status->append("====================================");
        ui->connect_status->append("进入2D Pose Estimate模式");
        ui->connect_status->append("操作步骤:");
        ui->connect_status->append("1. 移动鼠标到目标位置");
        ui->connect_status->append("2. 按住左键不松开");
        ui->connect_status->append("3. 拖拽鼠标调整方向");
        ui->connect_status->append("4. 松开左键确认发送");
        ui->connect_status->append("====================================");
    }
}

/**
 * @brief 用户完成位姿估计后的回调
 * @param position 位置(x, y, z)
 * @param yaw 朝向角度(弧度)
 */
void robot::onPoseEstimated(QVector3D position, double yaw)
{
    ui->connect_status->append("------------------------------------");
    ui->connect_status->append(QString("✓ 位姿已设置: 位置(%.2f, %.2f, %.2f), 朝向%.2f°")
        .arg(position.x()).arg(position.y()).arg(position.z())
        .arg(qRadiansToDegrees(yaw)));
    
    // 发布到 /initialpose 话题
    publishInitialPose(position, yaw);
    
    ui->connect_status->append("已退出2D Pose Estimate模式");
    ui->connect_status->append("------------------------------------");
}

/**
 * @brief 发布初始位姿到ROS
 * @param position 位置
 * @param yaw 朝向角度(弧度)
 */
void robot::publishInitialPose(const QVector3D &position, double yaw)
{
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        ui->connect_status->append("WebSocket未连接,无法发布位姿");
        return;
    }
    
    qDebug() << "准备发布位姿 - 位置:" << position << "Yaw(弧度):" << yaw << "Yaw(度):" << qRadiansToDegrees(yaw);
    
    // 从yaw角度计算四元数
    // 绕Z轴旋转的四元数: q = [cos(yaw/2), 0, 0, sin(yaw/2)]
    double halfYaw = yaw / 2.0;
    double qw = qCos(halfYaw);
    double qx = 0.0;
    double qy = 0.0;
    double qz = qSin(halfYaw);
    
    qDebug() << "四元数 - qx:" << qx << "qy:" << qy << "qz:" << qz << "qw:" << qw;
    
    // 构造 geometry_msgs/PoseWithCovarianceStamped 消息
    QJsonObject poseMsg;
    
    // Header
    QJsonObject header;
    header["frame_id"] = "map";
    header["stamp"] = QJsonObject(); // rosbridge会自动填充时间戳
    poseMsg["header"] = header;
    
    // Pose
    QJsonObject pose;
    QJsonObject poseWithCovariance;
    
    // Position
    QJsonObject positionObj;
    positionObj["x"] = position.x();
    positionObj["y"] = position.y();
    positionObj["z"] = position.z(); // 使用实际的z坐标
    
    // Orientation
    QJsonObject orientation;
    orientation["x"] = qx;
    orientation["y"] = qy;
    orientation["z"] = qz;
    orientation["w"] = qw;
    
    pose["position"] = positionObj;
    pose["orientation"] = orientation;
    poseWithCovariance["pose"] = pose;
    
    // Covariance (6x6矩阵,36个元素)
    QJsonArray covariance;
    for (int i = 0; i < 36; ++i) {
        if (i == 0 || i == 7 || i == 35) {
            covariance.append(0.25); // x, y, yaw的方差
        } else {
            covariance.append(0.0);
        }
    }
    poseWithCovariance["covariance"] = covariance;
    
    poseMsg["pose"] = poseWithCovariance;
    
    // 构造发布消息
    QJsonObject publishMsg;
    publishMsg["op"] = "publish";
    publishMsg["topic"] = "/initialpose";
    publishMsg["type"] = "geometry_msgs/PoseWithCovarianceStamped";
    publishMsg["msg"] = poseMsg;
    
    // 发送消息
    QJsonDocument doc(publishMsg);
    m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    
    ui->connect_status->append(QString("已发布初始位姿到/initialpose"));
    qDebug() << "Published initial pose:" << position << "yaw:" << yaw;
}

/**
 * @brief 处理TF消息
 */
void robot::onTFMessageReceived(const QJsonObject &msg)
{
    // TF消息是一个包含多个TransformStamped的数组
    QJsonArray transforms = msg.value("transforms").toArray();
    
    qDebug() << "Received TF message with" << transforms.size() << "transforms";
    
    for (const QJsonValue &tfValue : transforms) {
        QJsonObject tf = tfValue.toObject();
        
        // 解析frame_id
        QString child_frame = tf.value("child_frame_id").toString();
        QString parent_frame = tf.value("header").toObject().value("frame_id").toString();
        
        // 移除可能的前导斜杠
        if (child_frame.startsWith("/")) child_frame = child_frame.mid(1);
        if (parent_frame.startsWith("/")) parent_frame = parent_frame.mid(1);
        
        qDebug() << "TF:" << parent_frame << "->" << child_frame;
        
        // 解析transform
        QJsonObject transform = tf.value("transform").toObject();
        QJsonObject translation = transform.value("translation").toObject();
        QJsonObject rotation = transform.value("rotation").toObject();
        
        // 提取平移
        QVector3D trans(
            translation.value("x").toDouble(),
            translation.value("y").toDouble(),
            translation.value("z").toDouble()
        );
        
        // 提取旋转四元数
        QQuaternion rot(
            rotation.value("w").toDouble(),
            rotation.value("x").toDouble(),
            rotation.value("y").toDouble(),
            rotation.value("z").toDouble()
        );
        
        // 存储所有TF变换（不再过滤）
        TFTransform tfTransform;
        tfTransform.parent_frame = parent_frame;
        tfTransform.child_frame = child_frame;
        tfTransform.translation = trans;
        tfTransform.rotation = rot;
        tfTransform.timestamp = QDateTime::currentMSecsSinceEpoch();
        
        m_tfTransforms[child_frame] = tfTransform;
        
        qDebug() << "Stored TF" << child_frame << "parent:" << parent_frame 
                 << "trans:" << trans << "rot:" << rot;
    }
    
    // 计算并更新三个坐标系的位置
    if (m_pointCloudView) {
        qDebug() << "Updating coordinate frames visualization...";
        qDebug() << "Current TF transforms count:" << m_tfTransforms.size();
        
        // map坐标系固定在原点
        m_pointCloudView->updateTF("map", QVector3D(0, 0, 0), QQuaternion());
        
        // base_link相对于map的变换
        QVector3D baseLinkTrans;
        QQuaternion baseLinkRot;
        if (computeTransform("map", "base_link", baseLinkTrans, baseLinkRot)) {
            qDebug() << "Computed base_link transform:" << baseLinkTrans << baseLinkRot;
            m_pointCloudView->updateTF("base_link", baseLinkTrans, baseLinkRot);
            
            // 【新增】更新机器人在map坐标系中的位置和朝向，用于路径点记录
            m_latestRobotPos_World.setX(baseLinkTrans.x());
            m_latestRobotPos_World.setY(baseLinkTrans.y());
            
            // 从四元数计算yaw角度
            // yaw = atan2(2*(qw*qz + qx*qy), 1 - 2*(qy^2 + qz^2))
            double qw = baseLinkRot.scalar();
            double qx = baseLinkRot.x();
            double qy = baseLinkRot.y();
            double qz = baseLinkRot.z();
            m_latestRobotYaw = qAtan2(2.0 * (qw * qz + qx * qy), 1.0 - 2.0 * (qy * qy + qz * qz));
            
            // 更新机器人TF数据接收时间戳
            m_lastRobotTfTimestamp = QDateTime::currentMSecsSinceEpoch();
            
            qDebug() << "Updated robot position in map:" << m_latestRobotPos_World << "yaw:" << m_latestRobotYaw;
            
            // 更新2D地图上的红点位置
            if (m_mapIsLoaded && m_robotPositionMarker) {
                QPointF scenePos = convertWorldToScene(m_latestRobotPos_World);
                m_robotPositionMarker->setPos(scenePos);
            }
        } else {
            qDebug() << "Failed to compute base_link transform";
        }
        
        // ptz_link相对于map的变换
        QVector3D ptzLinkTrans;
        QQuaternion ptzLinkRot;
        if (computeTransform("map", "ptz_link", ptzLinkTrans, ptzLinkRot)) {
            qDebug() << "✓ Computed ptz_link transform:" << ptzLinkTrans << ptzLinkRot;
            m_pointCloudView->updateTF("ptz_link", ptzLinkTrans, ptzLinkRot);
            
            // 【新增】更新云台在map坐标系中的位置和方向
            m_latestPtzPos_World = ptzLinkTrans;
            m_latestPtzRotation = ptzLinkRot;
            
            // 更新PTZ TF数据接收时间戳
            m_lastPtzTfTimestamp = QDateTime::currentMSecsSinceEpoch();
            
            qDebug() << "✓ [PTZ数据更新] 位置:" << m_latestPtzPos_World 
                     << "旋转:" << m_latestPtzRotation
                     << "| 当前zoom滑块值:" << m_currentPtzZoom;
        } else {
            qWarning() << "✗ [PTZ错误] 无法计算ptz_link变换 - TF链不完整或延迟";
            qWarning() << "✗ 当前PTZ数据保持为:" << m_latestPtzPos_World << m_latestPtzRotation;
            qDebug() << "当前可用的TF变换链:";
            for (auto it = m_tfTransforms.begin(); it != m_tfTransforms.end(); ++it) {
                qDebug() << "  " << it.value().parent_frame << "->" << it.key();
            }
        }
    }
}

/**
 * @brief 计算从parent到child的累积变换
 */
bool robot::computeTransform(const QString &parent, const QString &child, 
                            QVector3D &translation, QQuaternion &rotation)
{
    qDebug() << "Computing transform from" << parent << "to" << child;
    
    if (parent == child) {
        translation = QVector3D(0, 0, 0);
        rotation = QQuaternion();
        return true;
    }
    
    // 如果child是map，返回单位变换
    if (child == "map") {
        translation = QVector3D(0, 0, 0);
        rotation = QQuaternion();
        return true;
    }
    
    // 尝试查找从parent到child的变换链
    QStringList chain;
    if (!findTransformChain(parent, child, chain)) {
        qDebug() << "No transform chain found from" << parent << "to" << child;
        // 如果找不到链，默认返回原点
        translation = QVector3D(0, 0, 0);
        rotation = QQuaternion();
        return false;
    }
    
    qDebug() << "Transform chain:" << chain;
    
    // 沿着链累积变换
    // 初始化为单位变换
    translation = QVector3D(0, 0, 0);
    rotation = QQuaternion(1, 0, 0, 0); // 单位四元数 (w, x, y, z)
    
    for (int i = 0; i < chain.size() - 1; i++) {
        QString from = chain[i];
        QString to = chain[i + 1];
        
        if (!m_tfTransforms.contains(to)) {
            qDebug() << "Missing TF for" << to;
            return false;
        }
        
        TFTransform &tf = m_tfTransforms[to];
        
        qDebug() << "  Step" << i << ":" << from << "->" << to 
                 << "trans:" << tf.translation << "rot:" << tf.rotation;
        
        // 正确的变换组合公式:
        // T_result = T_current * T_new
        // translation_result = translation_current + rotation_current.rotatedVector(translation_new)
        // rotation_result = rotation_current * rotation_new
        
        // 先旋转新的平移向量，然后加到当前平移上
        translation = translation + rotation.rotatedVector(tf.translation);
        // 组合旋转
        rotation = rotation * tf.rotation;
        
        qDebug() << "  Accumulated trans:" << translation << "rot:" << rotation;
    }
    
    qDebug() << "Final transform:" << translation << rotation;
    return true;
}

/**
 * @brief 查找从parent到child的变换链
 */
bool robot::findTransformChain(const QString &parent, const QString &child, QStringList &chain)
{
    chain.clear();
    
    // 从child开始向上查找到parent
    QString current = child;
    chain.prepend(current);
    
    int maxDepth = 20; // 防止无限循环
    int depth = 0;
    
    while (current != parent && depth < maxDepth) {
        if (!m_tfTransforms.contains(current)) {
            return false;
        }
        
        current = m_tfTransforms[current].parent_frame;
        chain.prepend(current);
        depth++;
    }
    
    return (current == parent);
}

/**
 * @brief 发布路径话题（nav_path、ptz_path、zoom_path）
 * 该函数从m_allSortedPoints读取合并并排序后的点，构建并发布三个话题
 */
void robot::publishPathTopics()
{
    if (m_allSortedPoints.isEmpty()) {
        qWarning() << "没有路径点数据，无法发布话题";
        return;
    }
    
    // 步骤二：构建 nav_path 消息 (nav_msgs/Path)
    QJsonObject navPathMsg;
    QJsonObject header;
    header["seq"] = 0;
    header["stamp"] = QJsonObject{
        {"secs", QDateTime::currentSecsSinceEpoch()},
        {"nsecs", (QDateTime::currentMSecsSinceEpoch() % 1000) * 1000000}
    };
    header["frame_id"] = "map";
    navPathMsg["header"] = header;
    
    QJsonArray poses;
    for (const QJsonValue &pointVal : m_allSortedPoints) {
        QJsonObject point = pointVal.toObject();
        
        QJsonObject poseStamped;
        poseStamped["header"] = header;
        
        QJsonObject pose;
        QJsonObject position;
        position["x"] = point["x"].toDouble();
        position["y"] = point["y"].toDouble();
        position["z"] = 0.0;
        
        QJsonObject orientation;
        // 导航点四元数全部为0，巡检点使用yaw转四元数
        if (point.contains("yaw")) {
            // 巡检点：使用yaw转四元数
            double yaw = point["yaw"].toDouble();
            // yaw转四元数: qz = sin(yaw/2), qw = cos(yaw/2)
            orientation["x"] = 0.0;
            orientation["y"] = 0.0;
            orientation["z"] = qSin(yaw / 2.0);
            orientation["w"] = qCos(yaw / 2.0);
        } else {
            // 导航点：四元数全部为0
            orientation["x"] = 0.0;
            orientation["y"] = 0.0;
            orientation["z"] = 0.0;
            orientation["w"] = 0.0;
        }
        
        pose["position"] = position;
        pose["orientation"] = orientation;
        poseStamped["pose"] = pose;
        
        poses.append(poseStamped);
    }
    navPathMsg["poses"] = poses;
    
    // 步骤三：构建 ptz_path 消息 (nav_msgs/Path)
    QJsonObject ptzPathMsg;
    ptzPathMsg["header"] = header;
    
    QJsonArray ptzPoses;
    for (const QJsonValue &pointVal : m_allSortedPoints) {
        QJsonObject point = pointVal.toObject();
        
        QJsonObject poseStamped;
        poseStamped["header"] = header;
        
        QJsonObject pose;
        // 使用点中的ptzPose信息
        if (point.contains("ptzPose")) {
            QJsonObject ptzPose = point["ptzPose"].toObject();
            pose["position"] = ptzPose["position"];
            pose["orientation"] = ptzPose["orientation"];
        } else {
            // 如果没有PTZ信息，使用默认值
            pose["position"] = QJsonObject{{"x", 0.0}, {"y", 0.0}, {"z", 0.0}};
            pose["orientation"] = QJsonObject{{"x", 0.0}, {"y", 0.0}, {"z", 0.0}, {"w", 1.0}};
        }
        
        poseStamped["pose"] = pose;
        ptzPoses.append(poseStamped);
    }
    ptzPathMsg["poses"] = ptzPoses;
    
    // 步骤四：构建 zoom_path 消息 (std_msgs/Float32MultiArray)
    // 简化格式：只包含data数组，不包含layout
    QJsonObject zoomPathMsg;
    
    // 调试：检查m_allSortedPoints
    qDebug() << "====== Zoom Path构建开始 ======";
    qDebug() << "m_allSortedPoints总数:" << m_allSortedPoints.size();
    
    // 构建 data 数组，按照order顺序
    // 将slider值(1-23)转换为实际zoom像素值(16-16384)
    QJsonArray zoomData;
    const int zoom_min = 16;
    const int zoom_max = 16384;
    const int slider_min = 1;
    const int slider_max = 23;
    
    for (int i = 0; i < m_allSortedPoints.size(); ++i) {
        QJsonValue pointVal = m_allSortedPoints[i];
        QJsonObject point = pointVal.toObject();
        
        // 详细调试信息
        qDebug() << QString("点%1 - 检查zoomValue字段...").arg(i);
        qDebug() << "  包含的字段:" << point.keys();
        
        // 获取zoomValue并转换
        QJsonValue zoomVal = point["zoomValue"];
        qDebug() << "  zoomValue类型:" << (zoomVal.isDouble() ? "double" : zoomVal.isNull() ? "null" : "other");
        qDebug() << "  zoomValue原始值:" << zoomVal.toVariant().toString();
        
        int sliderValue = static_cast<int>(zoomVal.toDouble(1.0)); // 获取slider值，默认1
        qDebug() << "  转换后sliderValue:" << sliderValue;
        
        // 线性映射：slider值(1-23) -> zoom像素值(16-16384)
        int zoomPixelValue = zoom_min + (zoom_max - zoom_min) * (sliderValue - slider_min) / (slider_max - slider_min);
        zoomData.append(static_cast<double>(zoomPixelValue));
        qDebug() << QString("  Zoom转换: slider=%1 -> 像素值=%2").arg(sliderValue).arg(zoomPixelValue);
    }
    
    // 只设置data字段
    zoomPathMsg["data"] = zoomData;
    
    // 调试：打印zoom_path消息
    qDebug() << "====== Zoom Data ======";
    qDebug() << "Data array size:" << zoomData.size();
    for (int i = 0; i < zoomData.size(); ++i) {
        qDebug() << QString("  data[%1] = %2").arg(i).arg(zoomData[i].toDouble());
    }
    qDebug() << "Complete zoom_path message:" << QJsonDocument(zoomPathMsg).toJson(QJsonDocument::Compact);
    qDebug() << "====== Zoom Path构建结束 ======";
    
    // 步骤五：循环发布三个话题
    // 发布 /nav_path
    QJsonObject navRoot;
    navRoot["op"] = "publish";
    navRoot["topic"] = "/nav_path";
    navRoot["msg"] = navPathMsg;
    m_webSocket->sendTextMessage(QJsonDocument(navRoot).toJson(QJsonDocument::Compact));
    
    // 发布 /ptz_path
    QJsonObject ptzRoot;
    ptzRoot["op"] = "publish";
    ptzRoot["topic"] = "/ptz_path";
    ptzRoot["msg"] = ptzPathMsg;
    m_webSocket->sendTextMessage(QJsonDocument(ptzRoot).toJson(QJsonDocument::Compact));
    
    // 发布 /zoom_path
    QJsonObject zoomRoot;
    zoomRoot["op"] = "publish";
    zoomRoot["topic"] = "/zoom_path";
    zoomRoot["msg"] = zoomPathMsg;
    m_webSocket->sendTextMessage(QJsonDocument(zoomRoot).toJson(QJsonDocument::Compact));
    
    // 打印发布的路径信息
    qDebug() << "========== 发布路径话题 ==========";
    qDebug() << "总点数:" << poses.size();
    if (!m_allSortedPoints.isEmpty()) {
        QJsonObject firstPoint = m_allSortedPoints[0].toObject();
        QString firstType = firstPoint.contains("yaw") ? "巡检点" : "导航点";
        qDebug() << QString("第1个点(目标点): order=%1, 类型=%2, 坐标=(%3, %4)")
                    .arg(firstPoint["order"].toInt())
                    .arg(firstType)
                    .arg(firstPoint["x"].toDouble(), 0, 'f', 2)
                    .arg(firstPoint["y"].toDouble(), 0, 'f', 2);
        
        if (m_allSortedPoints.size() > 1) {
            QJsonObject secondPoint = m_allSortedPoints[1].toObject();
            QString secondType = secondPoint.contains("yaw") ? "巡检点" : "导航点";
            qDebug() << QString("第2个点: order=%1, 类型=%2, 坐标=(%3, %4)")
                        .arg(secondPoint["order"].toInt())
                        .arg(secondType)
                        .arg(secondPoint["x"].toDouble(), 0, 'f', 2)
                        .arg(secondPoint["y"].toDouble(), 0, 'f', 2);
        }
    }
    qDebug() << "话题: /nav_path, /ptz_path, /zoom_path";
    qDebug() << "===================================";
}

/**
 * @brief 测试数据库连接和写入功能
 */
void robot::testDatabaseConnection()
{
    // 输出到界面和控制台
    ui->connect_status->append("========================================");
    ui->connect_status->append("【数据库测试】开始");
    qDebug() << "========================================";
    qDebug() << "开始数据库连接测试";
    
    // 检查 dbase 指针
    if (!dbase) {
        QString msg = "❌ 致命错误: dbase 指针为空！";
        ui->connect_status->append(msg);
        qDebug() << msg;
        return;
    }
    ui->connect_status->append("✓ dbase 对象已初始化");
    qDebug() << "✓ dbase 对象已初始化";
    
    // 强制重新创建连接进行测试
    QString testConnName = "qt_sql_default_connection_robot";
    
    ui->connect_status->append("步骤1: 检查旧连接...");
    if (QSqlDatabase::contains(testConnName)) {
        ui->connect_status->append("  发现旧连接,准备移除");
        QSqlDatabase oldDb = QSqlDatabase::database(testConnName);
        ui->connect_status->append(QString("  旧连接状态: isValid=%1, isOpen=%2")
            .arg(oldDb.isValid() ? "true" : "false")
            .arg(oldDb.isOpen() ? "true" : "false"));
        
        if (oldDb.isOpen()) {
            oldDb.close();
            ui->connect_status->append("  已关闭旧连接");
        }
        
        // 必须先获取连接名称的副本,因为remove后就无法访问了
        QSqlDatabase::removeDatabase(testConnName);
        ui->connect_status->append("  ✓ 已移除旧连接");
    } else {
        ui->connect_status->append("  未发现旧连接");
    }
    
    ui->connect_status->append("");
    ui->connect_status->append("步骤2: 诊断插件路径...");
    QStringList pluginPaths = QCoreApplication::libraryPaths();
    ui->connect_status->append("  Qt插件搜索路径:");
    for (const QString &path : pluginPaths) {
        ui->connect_status->append("    - " + path);
    }
    
    ui->connect_status->append("");
    ui->connect_status->append("步骤3: 尝试不同的驱动名称...");
    
    // 尝试 QMYSQL3
    ui->connect_status->append("  尝试: QMYSQL3");
    QSqlDatabase testDb3 = QSqlDatabase::addDatabase("QMYSQL3", testConnName);
    bool valid3 = testDb3.isValid();
    ui->connect_status->append(QString("    isValid: %1").arg(valid3 ? "true" : "false"));
    
    if (valid3) {
        ui->connect_status->append("  ✓ QMYSQL3 驱动有效!");
        // 使用这个连接继续
    } else {
        // 移除失败的连接,尝试 QMYSQL
        QSqlDatabase::removeDatabase(testConnName);
        ui->connect_status->append("  尝试: QMYSQL");
        QSqlDatabase testDb = QSqlDatabase::addDatabase("QMYSQL", testConnName);
        bool valid = testDb.isValid();
        ui->connect_status->append(QString("    isValid: %1").arg(valid ? "true" : "false"));
        
        if (!valid) {
            ui->connect_status->append("❌ 两个驱动都无效!");
            ui->connect_status->append("可用驱动: " + QSqlDatabase::drivers().join(", "));
            ui->connect_status->append("");
            ui->connect_status->append("💡 可能原因:");
            ui->connect_status->append("1. qsqlmysql.dll 版本与 Qt 版本不匹配");
            ui->connect_status->append("2. libmysql.dll 版本不兼容");
            ui->connect_status->append("3. 缺少 MSVC 运行库");
            return;
        }
    }
    
    // 获取有效的连接
    QSqlDatabase newDb = QSqlDatabase::database(testConnName);
    ui->connect_status->append("  ✓ 已获取有效连接");
    
    ui->connect_status->append("");
    ui->connect_status->append("步骤4: 设置连接参数...");
    newDb.setHostName("127.0.0.1");
    newDb.setPort(3306);
    newDb.setDatabaseName("robot");
    newDb.setUserName("root");
    newDb.setPassword("");
    ui->connect_status->append("  Host: 127.0.0.1:3306");
    ui->connect_status->append("  Database: robot");
    ui->connect_status->append("  User: root");
    
    ui->connect_status->append("");
    ui->connect_status->append("步骤5: 尝试打开连接...");
    bool opened = newDb.open();
    ui->connect_status->append(QString("  open()返回: %1").arg(opened ? "true" : "false"));
    ui->connect_status->append(QString("  isOpen()状态: %1").arg(newDb.isOpen() ? "true" : "false"));
    
    if (!opened || !newDb.isOpen()) {
        ui->connect_status->append("❌ 数据库连接失败!");
        ui->connect_status->append("错误信息: " + newDb.lastError().text());
        ui->connect_status->append("错误类型: " + QString::number(newDb.lastError().type()));
        return;
    }
    
    ui->connect_status->append("✅ 数据库连接成功!");
    
    // 2. 测试写入任务数据
    QString testTaskName = "【测试任务】数据库连接验证";
    QString testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    ui->connect_status->append("准备调用 add_task...");
    ui->connect_status->append("  任务名称: " + testTaskName);
    ui->connect_status->append("  开始时间: " + testStartTime);
    qDebug() << "准备调用 add_task...";
    qDebug() << "  任务名称:" << testTaskName;
    qDebug() << "  开始时间:" << testStartTime;
    
    bool taskAdded = dbase->add_task(
        "test_id",                      // 临时ID（会被忽略）
        testTaskName,                   // 任务名称
        "系统测试",                     // 任务类型
        QDate::currentDate().toString("yyyy-MM-dd"),  // 日期
        testStartTime,                  // 开始时间
        "",                             // 结束时间（空）
        "测试中"                        // 状态
    );
    
    QString resultMsg = QString("add_task 返回值: %1").arg(taskAdded ? "true" : "false");
    ui->connect_status->append(resultMsg);
    qDebug() << "add_task 返回值:" << taskAdded;
    
    if (!taskAdded) {
        ui->connect_status->append("❌ 写入任务失败！");
        qDebug() << "❌ 写入任务失败！";
        return;
    }
    
    // 3. 获取刚插入的任务ID
    QString testTaskId = dbase->getLastInsertId();
    if (testTaskId.isEmpty()) {
        qDebug() << "❌ 获取任务ID失败！";
        ui->connect_status->append("❌ 获取任务ID失败");
        return;
    }
    
    qDebug() << "✅ 任务已写入数据库，ID:" << testTaskId;
    ui->connect_status->append("✅ 测试任务已写入，ID: " + testTaskId);
    
    // 4. 测试写入巡检点明细
    qDebug() << "准备调用 add_report_detail...";
    qDebug() << "  task_id:" << testTaskId;
    qDebug() << "  point_id: test_point_001";
    
    bool detailAdded = dbase->add_report_detail(
        testTaskId,                     // 任务ID (QString会自动转int)
        "test_point_001",               // 巡检点ID
        "测试设备",                     // 设备名称
        "0.75 MPa",                     // 设备数据 (现在支持带单位)
        "正常",                         // 状态
        "",                             // 图片路径
        ""                              // 视频路径
    );
    
    if (!detailAdded) {
        qDebug() << "❌ 写入巡检点明细失败！请查看上方的SQL错误信息";
        ui->connect_status->append("❌ 写入巡检点明细失败");
        return;
    }
    
    qDebug() << "✅ 巡检点明细已写入数据库";
    ui->connect_status->append("✅ 测试数据已写入report_details表");
    
    qDebug() << "";
    qDebug() << "========================================";
    qDebug() << "数据库测试完成！请在MySQL中验证：";
    qDebug() << "1. 打开phpMyAdmin (http://localhost/phpmyadmin)";
    qDebug() << "2. 选择数据库 'robot'";
    qDebug() << "3. 查看 'task' 表，应该能看到测试任务";
    qDebug() << "4. 查看 'report_details' 表，应该能看到测试数据";
    qDebug() << "========================================";
    qDebug() << "";
    
    ui->connect_status->append("");
    ui->connect_status->append("========================================");
    ui->connect_status->append("💡 验证步骤：");
    ui->connect_status->append("1. 打开浏览器访问 http://localhost/phpmyadmin");
    ui->connect_status->append("2. 选择数据库 'robot'");
    ui->connect_status->append("3. 查看 'task' 表（任务ID: " + testTaskId + "）");
    ui->connect_status->append("4. 查看 'report_details' 表");
    ui->connect_status->append("========================================");
}

void robot::initTopologyMap()
{
    // 1. 清理场景，防止重影
        m_mapScene->clear();

        // 2. 加载拓扑图背景 - 使用绝对路径
        QString imagePath = "C:/Users/shunshun/Desktop/image/502.png";
        qDebug() << "尝试加载拓扑图:" << imagePath;
        
        // 检查文件是否存在
        QFileInfo fileInfo(imagePath);
        if (!fileInfo.exists()) {
            qDebug() << "❌ 文件不存在:" << imagePath;
            ui->connect_status->append("❌ 文件不存在: " + imagePath);
            m_mapScene->addRect(0, 0, 1038, 466, QPen(Qt::red), QBrush(Qt::white));
            m_mapScene->addText("文件不存在，请检查路径")->setDefaultTextColor(Qt::red);
            ui->mapGraphicsView->fitInView(m_mapScene->sceneRect(), Qt::KeepAspectRatio);
            return;
        }
        
        qDebug() << "文件存在，大小:" << fileInfo.size() << "字节";
        
        QPixmap bgImage(imagePath);

        if (bgImage.isNull()) {
            qDebug() << "❌ 错误：无法加载图片（可能格式不支持或文件损坏）";
            ui->connect_status->append("❌ 错误：无法加载图片，请检查文件格式");
            // 如果图片加载失败，绘制一个红色的矩形框作为占位符，避免程序崩溃
            m_mapScene->addRect(0, 0, 1038, 466, QPen(Qt::red), QBrush(Qt::white));
            m_mapScene->addText("图片加载失败，请检查文件格式")->setDefaultTextColor(Qt::red);
        } else {
            qDebug() << "✅ 图片加载成功，尺寸:" << bgImage.width() << "x" << bgImage.height();
            m_mapScene->addPixmap(bgImage);
            
            // 设置场景范围比图片大很多，提供更大的拖动空间
            QRectF imageRect = bgImage.rect();
            qreal margin = 300; // 300像素边距，提供更大的拖动范围
            m_mapScene->setSceneRect(imageRect.adjusted(-margin, -margin, margin, margin));
            
            qDebug() << "场景范围已设置:" << m_mapScene->sceneRect();
            ui->connect_status->append(QString("✅ 拓扑图加载成功 (%1x%2)").arg(bgImage.width()).arg(bgImage.height()));
            
            // 启用滚动条
            ui->mapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            ui->mapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            
            // 不自动缩放以适应视图，保持图片原始大小
            ui->mapGraphicsView->resetTransform();
            
            // 居中显示图片
            ui->mapGraphicsView->centerOn(imageRect.center());
        }
        
        ui->mapGraphicsView->show();

        // 3. 【关键】填入你刚刚量好的精准坐标 (ID -> 像素X, 像素Y)
        m_topoPixelMap.clear();

        // 根据你提供的测量数据：
        m_topoPixelMap.insert(0, QPointF(525, 88));  // 巡检点1 (中上) - 对应 JSON order 0
        m_topoPixelMap.insert(1, QPointF(150, 88));  // 巡检点2 (左上) - 对应 JSON order 1
        m_topoPixelMap.insert(2, QPointF(150, 378)); // 巡检点3 (左下) - 对应 JSON order 2
        m_topoPixelMap.insert(3, QPointF(888, 378)); // 巡检点4 (右下) - 对应 JSON order 3
        m_topoPixelMap.insert(4, QPointF(888, 88));  // 巡检点5 (右上) - 对应 JSON order 4

        // 4. 创建显示的绿点
        // 直径40像素，醒目一点
        m_fakeGreenDot = m_mapScene->addEllipse(0, 0, 40, 40,
                                                QPen(Qt::black, 2),
                                                QBrush(Qt::green));

        // 设置变换原点为圆心 (半径20)，这样设置坐标时圆心会准确对齐你的测量点
        m_fakeGreenDot->setTransformOriginPoint(20, 20);

        // 确保绿点在图的最上层
        m_fakeGreenDot->setZValue(100);

        // 默认隐藏，只有匹配到坐标时才显示
        m_fakeGreenDot->setVisible(false);
}

void robot::checkAndLightUp(double receivedX, double receivedY)
{
    // 设定一个容差范围 (比如 0.3 米)
    // 机器人实际到达的位置和 JSON 里的计划位置可能会有一点误差
    double tolerance = 0.3;
    int matchedId = -1;

    // 1. 遍历我们刚才加载的所有点，找找最近的是哪个
    for (const InspectionTarget &target : m_currentPlanTargets) {
        double dx = receivedX - target.realX;
        double dy = receivedY - target.realY;
        double distance = std::sqrt(dx*dx + dy*dy);

        if (distance < tolerance) {
            matchedId = target.id; // 找到了对应的 order ID
            break;
        }
    }

    // 2. 根据结果控制绿点
    if (matchedId != -1 && m_topoPixelMap.contains(matchedId)) {
        // === 匹配成功！===

        // 查表找到图片上的像素位置
        QPointF pixelPos = m_topoPixelMap[matchedId];

        // 瞬移过去 (减去半径的一半 10，让圆心对齐)
        m_fakeGreenDot->setPos(pixelPos.x() - 10, pixelPos.y() - 10);

        // 亮灯
        m_fakeGreenDot->setVisible(true);

        ui->connect_status->append(QString("抵达站点 (Order %1)，显示更新。").arg(matchedId));

    } else {
        // === 没匹配到 (或者在两个点中间) ===
        // 按照你的要求：如果不亮最好根本就不显示
        m_fakeGreenDot->setVisible(false);
        qDebug() << "当前坐标未匹配到已知站点，绿点隐藏。";
    }

}
