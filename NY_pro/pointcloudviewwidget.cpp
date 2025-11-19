#include "pointcloudviewwidget.h"
#include <QVBoxLayout>
#include <QDebug>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DCore/QTransform>
#include <QMouseEvent>
#include <QTimer>
#include <QtMath>
#include <QMap>

PointCloudViewWidget::PointCloudViewWidget(QWidget *parent)
    : QWidget(parent)
    , m_interactionMode(NavigationMode)
    , m_isPoseDragging(false)
    , m_poseArrowEntity(nullptr)
{
    // 创建Qt3DWindow
    m_view = new Qt3DExtras::Qt3DWindow();
    m_view->defaultFrameGraph()->setClearColor(QColor(30, 30, 30)); // 深灰色背景
    
    // 将3DWindow嵌入到Widget中
    m_container = QWidget::createWindowContainer(m_view, this);
    
    // 在容器和窗口上都安装事件过滤器以捕获鼠标事件
    m_container->installEventFilter(this);
    m_view->installEventFilter(this);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_container);
    setLayout(layout);
    
    // 创建根实体
    m_rootEntity = new Qt3DCore::QEntity();
    
    // 设置场景
    setupCamera();
    setupLighting();
    
    // 创建两个点云渲染器
    m_globalMapRenderer = new PointCloudRenderer(m_rootEntity);
    m_globalMapRenderer->setDefaultColor(QVector3D(0.0f, 1.0f, 0.0f)); // 全局地图 - 绿色
    
    m_alignedPointsRenderer = new PointCloudRenderer(m_rootEntity);
    m_alignedPointsRenderer->setDefaultColor(QVector3D(1.0f, 0.0f, 0.0f)); // 对齐点云 - 红色
    
    // 设置位姿箭头
    setupPoseArrow();
    
    // 设置中心十字标记
    setupCenterCross();
    
    // 设置根实体
    m_view->setRootEntity(m_rootEntity);
    
    qDebug() << "PointCloudViewWidget initialized";
}

PointCloudViewWidget::~PointCloudViewWidget()
{
}

void PointCloudViewWidget::setupCamera()
{
    m_camera = m_view->camera();
    m_camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    m_camera->setPosition(QVector3D(0, 0, 40.0f));
    m_camera->setViewCenter(QVector3D(0, 0, 0));
    m_camera->setUpVector(QVector3D(0, 1, 0));
    
    // 创建自定义相机控制器
    m_cameraController = new CustomCameraController(m_rootEntity);
    m_cameraController->setCamera(m_camera);
    m_cameraController->setPanSpeed(0.3f);      // 平移速度 - 降低灵敏度
    m_cameraController->setRotateSpeed(0.5f);   // 旋转速度
    m_cameraController->setZoomSpeed(1.0f);     // 缩放速度 - 提高灵敏度
    
    // 连接旋转状态信号,只在旋转时显示中心十字
    connect(m_cameraController, &CustomCameraController::rotatingChanged,
            this, &PointCloudViewWidget::setCenterCrossVisible);
    
    // 使用定时器定期更新十字标记位置，使其始终在视图中心
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &PointCloudViewWidget::updateCenterCrossPosition);
    updateTimer->start(50); // 每50ms更新一次
}

void PointCloudViewWidget::setupLighting()
{
    // 创建点光源
    m_lightEntity = new Qt3DCore::QEntity(m_rootEntity);
    m_light = new Qt3DRender::QPointLight(m_lightEntity);
    m_light->setColor("white");
    m_light->setIntensity(1.0f);
    
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(m_lightEntity);
    lightTransform->setTranslation(QVector3D(0, 50, 50));
    
    m_lightEntity->addComponent(m_light);
    m_lightEntity->addComponent(lightTransform);
    
    // 禁用光照以避免坐标系从不同方向看明暗不同
    m_lightEntity->setEnabled(false);
}

void PointCloudViewWidget::updateGlobalMap(const QVector<QVector3D> &points, 
                                          const QVector<QVector3D> &colors)
{
    m_globalMapRenderer->updatePointCloud(points, colors);
    emit pointCloudUpdated("/globalmap_downsample", points.size());
    qDebug() << "Global map updated:" << points.size() << "points";
}

void PointCloudViewWidget::updateAlignedPoints(const QVector<QVector3D> &points, 
                                              const QVector<QVector3D> &colors)
{
    m_alignedPointsRenderer->updatePointCloud(points, colors);
    emit pointCloudUpdated("/aligned_points_downsample", points.size());
    qDebug() << "Aligned points updated:" << points.size() << "points";
}

void PointCloudViewWidget::clearAll()
{
    m_globalMapRenderer->clear();
    m_alignedPointsRenderer->clear();
    qDebug() << "All point clouds cleared";
}

void PointCloudViewWidget::resetCamera()
{
    m_camera->setPosition(QVector3D(0, 0, 40.0f));
    m_camera->setViewCenter(QVector3D(0, 0, 0));
    m_camera->setUpVector(QVector3D(0, 1, 0));
    
    // 同步重置控制器的内部参数
    if (m_cameraController) {
        m_cameraController->resetCameraParameters();
    }
}

void PointCloudViewWidget::setBackgroundColor(const QColor &color)
{
    m_view->defaultFrameGraph()->setClearColor(color);
}

void PointCloudViewWidget::setInteractionMode(InteractionMode mode)
{
    m_interactionMode = mode;
    m_isPoseDragging = false;
    
    if (mode == NavigationMode) {
        hidePoseArrow();
        m_container->setCursor(Qt::ArrowCursor);
        // 启用相机控制器
        if (m_cameraController) {
            m_cameraController->setControlEnabled(true);
        }
        qDebug() << "Switched to Navigation Mode";
    } else if (mode == SetPoseMode) {
        m_container->setCursor(Qt::CrossCursor);
        // 禁用相机控制器,让事件过滤器处理鼠标事件
        if (m_cameraController) {
            m_cameraController->setControlEnabled(false);
        }
        qDebug() << "Switched to SetPose Mode";
    }
}

void PointCloudViewWidget::setupPoseArrow()
{
    // 创建位姿箭头实体(圆柱体+圆锥体)
    m_poseArrowEntity = new Qt3DCore::QEntity(m_rootEntity);
    
    // 箭头杆(圆柱体) - 长度1.0，以中心为原点
    Qt3DCore::QEntity *shaftEntity = new Qt3DCore::QEntity(m_poseArrowEntity);
    Qt3DExtras::QCylinderMesh *shaftMesh = new Qt3DExtras::QCylinderMesh();
    shaftMesh->setRadius(0.05f);
    shaftMesh->setLength(0.7f); // 杆的长度
    
    Qt3DExtras::QPhongMaterial *shaftMaterial = new Qt3DExtras::QPhongMaterial();
    shaftMaterial->setDiffuse(QColor(255, 0, 255)); // 洋红色
    
    // 将圆柱体向上移动，使箭头底部在原点
    Qt3DCore::QTransform *shaftTransform = new Qt3DCore::QTransform();
    shaftTransform->setTranslation(QVector3D(0, 0.35f, 0)); // 向上偏移一半长度
    
    shaftEntity->addComponent(shaftMesh);
    shaftEntity->addComponent(shaftMaterial);
    shaftEntity->addComponent(shaftTransform);
    
    // 箭头头(圆锥体)
    Qt3DCore::QEntity *headEntity = new Qt3DCore::QEntity(m_poseArrowEntity);
    Qt3DExtras::QConeMesh *headMesh = new Qt3DExtras::QConeMesh();
    headMesh->setBottomRadius(0.15f);
    headMesh->setLength(0.3f);
    
    Qt3DExtras::QPhongMaterial *headMaterial = new Qt3DExtras::QPhongMaterial();
    headMaterial->setDiffuse(QColor(255, 0, 255)); // 洋红色
    
    // 将圆锥体放在圆柱体顶端
    Qt3DCore::QTransform *headTransform = new Qt3DCore::QTransform();
    headTransform->setTranslation(QVector3D(0, 0.85f, 0)); // 0.7 + 0.3/2 = 0.85
    
    headEntity->addComponent(headMesh);
    headEntity->addComponent(headMaterial);
    headEntity->addComponent(headTransform);
    
    // 主变换 - 控制整个箭头的位置和旋转
    m_poseArrowTransform = new Qt3DCore::QTransform();
    m_poseArrowEntity->addComponent(m_poseArrowTransform);
    
    // 默认隐藏
    m_poseArrowEntity->setEnabled(false);
}

void PointCloudViewWidget::setupCenterCross()
{
    // 创建中心十字标记实体
    m_centerCrossEntity = new Qt3DCore::QEntity(m_rootEntity);
    
    // 创建三条轴线（X, Y, Z）- 更长更粗以便更明显
    float lineLength = 2.0f;  // 增加长度
    float lineRadius = 0.05f; // 增加粗细
    
    // X轴 - 红色
    Qt3DCore::QEntity *xAxisEntity = new Qt3DCore::QEntity(m_centerCrossEntity);
    Qt3DExtras::QCylinderMesh *xAxisMesh = new Qt3DExtras::QCylinderMesh();
    xAxisMesh->setRadius(lineRadius);
    xAxisMesh->setLength(lineLength);
    
    Qt3DExtras::QPhongMaterial *xAxisMaterial = new Qt3DExtras::QPhongMaterial();
    xAxisMaterial->setDiffuse(QColor(255, 0, 0)); // 红色
    xAxisMaterial->setAmbient(QColor(200, 0, 0)); // 环境光
    
    Qt3DCore::QTransform *xAxisTransform = new Qt3DCore::QTransform();
    xAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f)); // 旋转到X轴方向
    
    xAxisEntity->addComponent(xAxisMesh);
    xAxisEntity->addComponent(xAxisMaterial);
    xAxisEntity->addComponent(xAxisTransform);
    
    // Y轴 - 绿色（默认方向）
    Qt3DCore::QEntity *yAxisEntity = new Qt3DCore::QEntity(m_centerCrossEntity);
    Qt3DExtras::QCylinderMesh *yAxisMesh = new Qt3DExtras::QCylinderMesh();
    yAxisMesh->setRadius(lineRadius);
    yAxisMesh->setLength(lineLength);
    
    Qt3DExtras::QPhongMaterial *yAxisMaterial = new Qt3DExtras::QPhongMaterial();
    yAxisMaterial->setDiffuse(QColor(0, 255, 0)); // 绿色
    yAxisMaterial->setAmbient(QColor(0, 200, 0)); // 环境光
    
    yAxisEntity->addComponent(yAxisMesh);
    yAxisEntity->addComponent(yAxisMaterial);
    
    // Z轴 - 蓝色
    Qt3DCore::QEntity *zAxisEntity = new Qt3DCore::QEntity(m_centerCrossEntity);
    Qt3DExtras::QCylinderMesh *zAxisMesh = new Qt3DExtras::QCylinderMesh();
    zAxisMesh->setRadius(lineRadius);
    zAxisMesh->setLength(lineLength);
    
    Qt3DExtras::QPhongMaterial *zAxisMaterial = new Qt3DExtras::QPhongMaterial();
    zAxisMaterial->setDiffuse(QColor(0, 0, 255)); // 蓝色
    zAxisMaterial->setAmbient(QColor(0, 0, 200)); // 环境光
    
    Qt3DCore::QTransform *zAxisTransform = new Qt3DCore::QTransform();
    zAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 90.0f)); // 旋转到Z轴方向
    
    zAxisEntity->addComponent(zAxisMesh);
    zAxisEntity->addComponent(zAxisMaterial);
    zAxisEntity->addComponent(zAxisTransform);
    
    // 十字标记变换 - 将跟随相机视图中心移动
    m_centerCrossTransform = new Qt3DCore::QTransform();
    m_centerCrossEntity->addComponent(m_centerCrossTransform);
    
    // 初始状态下隐藏中心十字，只在旋转时显示
    m_centerCrossEntity->setEnabled(false);
    
    // 初始化位置为视图中心
    updateCenterCrossPosition();
    
    qDebug() << "Center cross marker created (larger and always at view center)";
}

void PointCloudViewWidget::updateCenterCrossPosition()
{
    if (!m_camera || !m_centerCrossTransform) return;
    
    // 将十字标记放置在相机的视图中心点
    QVector3D viewCenter = m_camera->viewCenter();
    m_centerCrossTransform->setTranslation(viewCenter);
}

void PointCloudViewWidget::setCenterCrossVisible(bool visible)
{
    if (m_centerCrossEntity) {
        m_centerCrossEntity->setEnabled(visible);
    }
}

void PointCloudViewWidget::setupCoordinateFrame(const QString &frame_id)
{
    // 创建坐标系实体
    Qt3DCore::QEntity *frameEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DCore::QTransform *frameTransform = new Qt3DCore::QTransform();
    frameEntity->addComponent(frameTransform);
    
    // 存储实体和变换
    m_frameEntities[frame_id] = frameEntity;
    m_frameTransforms[frame_id] = frameTransform;
    
    // 创建三条轴线（X, Y, Z）
    // ptz_link使用一半大小
    float scale = (frame_id == "ptz_link") ? 0.5f : 1.0f;
    float axisLength = 1.0f * scale;  // 轴线长度
    float axisRadius = 0.05f * scale; // 轴线粗细
    
    // X轴 - 红色
    Qt3DCore::QEntity *xAxisEntity = new Qt3DCore::QEntity(frameEntity);
    Qt3DExtras::QCylinderMesh *xAxisMesh = new Qt3DExtras::QCylinderMesh();
    xAxisMesh->setRadius(axisRadius);
    xAxisMesh->setLength(axisLength);
    
    Qt3DExtras::QPhongMaterial *xAxisMaterial = new Qt3DExtras::QPhongMaterial();
    xAxisMaterial->setDiffuse(QColor(255, 0, 0)); // 红色
    xAxisMaterial->setAmbient(QColor(200, 0, 0)); // 环境光
    
    Qt3DCore::QTransform *xAxisTransform = new Qt3DCore::QTransform();
    xAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 0, 1), 90.0f));
    xAxisTransform->setTranslation(QVector3D(axisLength/2, 0, 0)); // 使轴从原点开始
    
    xAxisEntity->addComponent(xAxisMesh);
    xAxisEntity->addComponent(xAxisMaterial);
    xAxisEntity->addComponent(xAxisTransform);
    
    // Y轴 - 绿色
    Qt3DCore::QEntity *yAxisEntity = new Qt3DCore::QEntity(frameEntity);
    Qt3DExtras::QCylinderMesh *yAxisMesh = new Qt3DExtras::QCylinderMesh();
    yAxisMesh->setRadius(axisRadius);
    yAxisMesh->setLength(axisLength);
    
    Qt3DExtras::QPhongMaterial *yAxisMaterial = new Qt3DExtras::QPhongMaterial();
    yAxisMaterial->setDiffuse(QColor(0, 255, 0)); // 绿色
    yAxisMaterial->setAmbient(QColor(0, 200, 0)); // 环境光
    
    Qt3DCore::QTransform *yAxisTransform = new Qt3DCore::QTransform();
    yAxisTransform->setTranslation(QVector3D(0, axisLength/2, 0)); // 使轴从原点开始
    
    yAxisEntity->addComponent(yAxisMesh);
    yAxisEntity->addComponent(yAxisMaterial);
    yAxisEntity->addComponent(yAxisTransform);
    
    // Z轴 - 蓝色
    Qt3DCore::QEntity *zAxisEntity = new Qt3DCore::QEntity(frameEntity);
    Qt3DExtras::QCylinderMesh *zAxisMesh = new Qt3DExtras::QCylinderMesh();
    zAxisMesh->setRadius(axisRadius);
    zAxisMesh->setLength(axisLength);
    
    Qt3DExtras::QPhongMaterial *zAxisMaterial = new Qt3DExtras::QPhongMaterial();
    zAxisMaterial->setDiffuse(QColor(0, 0, 255)); // 蓝色
    zAxisMaterial->setAmbient(QColor(0, 0, 200)); // 环境光
    
    Qt3DCore::QTransform *zAxisTransform = new Qt3DCore::QTransform();
    zAxisTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 90.0f));
    zAxisTransform->setTranslation(QVector3D(0, 0, axisLength/2)); // 使轴从原点开始
    
    zAxisEntity->addComponent(zAxisMesh);
    zAxisEntity->addComponent(zAxisMaterial);
    zAxisEntity->addComponent(zAxisTransform);
    
    qDebug() << "Created coordinate frame:" << frame_id;
}

void PointCloudViewWidget::updateTF(const QString &frame_id, const QVector3D &translation, const QQuaternion &rotation)
{
    // 如果坐标系不存在，先创建它
    if (!m_frameTransforms.contains(frame_id)) {
        setupCoordinateFrame(frame_id);
        qDebug() << "Created new TF frame:" << frame_id;
    }
    
    // 更新坐标系变换
    Qt3DCore::QTransform *transform = m_frameTransforms[frame_id];
    if (transform) {
        transform->setTranslation(translation);
        transform->setRotation(rotation);
        qDebug() << "Updated TF" << frame_id << "pos:" << translation << "rot:" << rotation;
    }
}

void PointCloudViewWidget::updatePoseArrow(const QVector3D &start, const QVector3D &end)
{
    QVector3D direction = end - start;
    float length = direction.length();
    
    if (length < 0.01f) {
        // 距离太短,不显示箭头
        hidePoseArrow();
        return;
    }
    
    QVector3D normalizedDir = direction.normalized();
    
    // 计算旋转角度
    // 箭头默认指向Y轴正方向(0,1,0),需要旋转到指向目标方向
    QVector3D defaultDir(0, 1, 0);
    QVector3D rotationAxis = QVector3D::crossProduct(defaultDir, normalizedDir);
    float dotProduct = QVector3D::dotProduct(defaultDir, normalizedDir);
    
    // 处理特殊情况：方向完全相反
    float rotationAngle = 0.0f;
    QQuaternion rotation;
    
    if (dotProduct < -0.9999f) {
        // 方向相反，旋转180度，使用X轴作为旋转轴
        rotation = QQuaternion::fromAxisAndAngle(QVector3D(1, 0, 0), 180.0f);
    } else if (dotProduct > 0.9999f) {
        // 方向相同，不需要旋转
        rotation = QQuaternion();
    } else {
        // 正常情况
        rotationAngle = qRadiansToDegrees(qAcos(dotProduct));
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, rotationAngle);
    }
    
    // 设置变换 - 固定缩放为1.0，箭头总长度约1.0单位
    m_poseArrowTransform->setScale(1.0f);
    m_poseArrowTransform->setTranslation(start); // 箭头底部在起点
    m_poseArrowTransform->setRotation(rotation);
    
    m_poseArrowEntity->setEnabled(true);
}

void PointCloudViewWidget::hidePoseArrow()
{
    if (m_poseArrowEntity) {
        m_poseArrowEntity->setEnabled(false);
    }
}

bool PointCloudViewWidget::eventFilter(QObject *obj, QEvent *event)
{
    // 只处理容器和视图窗口的事件
    if (obj != m_container && obj != m_view) {
        return QWidget::eventFilter(obj, event);
    }
    
    // 只在SetPoseMode下处理鼠标事件
    if (m_interactionMode != SetPoseMode) {
        return QWidget::eventFilter(obj, event);
    }
    
    // 获取鼠标位置 - 需要转换为容器坐标系
    QPoint mousePos;
    if (event->type() == QEvent::MouseButtonPress || 
        event->type() == QEvent::MouseMove || 
        event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mousePos = mouseEvent->pos();
        
        // 如果事件来自 m_view，需要映射到 m_container 坐标系
        if (obj == m_view) {
            mousePos = m_container->mapFromGlobal(m_view->mapToGlobal(mousePos));
        }
    }
    
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // 开始设置位姿 - 记录起点
            m_poseStartPosition = screenToWorld(mousePos);
            m_isPoseDragging = true;
            qDebug() << "Start setting pose at position:" << m_poseStartPosition;
            return true; // 拦截事件
        }
    }
    else if (event->type() == QEvent::MouseMove) {
        QVector3D currentPos = screenToWorld(mousePos);
        
        if (m_isPoseDragging) {
            // 拖拽中 - 显示从起点到当前位置的箭头
            updatePoseArrow(m_poseStartPosition, currentPos);
        } else {
            // 未拖拽 - 显示一个短箭头跟随鼠标（默认朝向为+X方向）
            QVector3D arrowEnd = currentPos + QVector3D(1.0f, 0.0f, 0.0f);
            updatePoseArrow(currentPos, arrowEnd);
        }
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton && m_isPoseDragging) {
            QVector3D endPos = screenToWorld(mousePos);
            
            // 只使用 XY 平面的方向计算 yaw（忽略 Z）
            // 在 XY 平面上，从起点指向终点的向量
            float dx = endPos.x() - m_poseStartPosition.x();
            float dy = endPos.y() - m_poseStartPosition.y();
            
            // 计算 yaw 角度 (ROS 标准: X轴向前, Y轴向左, 绕Z轴旋转)
            double yaw = qAtan2(dy, dx);
            
            // 位姿只使用 XY 坐标，Z 设为 0
            QVector3D pose2D(m_poseStartPosition.x(), m_poseStartPosition.y(), 0.0f);
            
            qDebug() << "Pose set - Position(XY):" << pose2D 
                     << "Delta(dx,dy):(" << dx << "," << dy << ")"
                     << "Yaw:" << qRadiansToDegrees(yaw) << "deg";
            
            // 发射信号 - 使用 2D 位置
            emit poseEstimated(pose2D, yaw);
            
            m_isPoseDragging = false;
            
            // 隐藏箭头并立即退出 SetPoseMode
            hidePoseArrow();
            setInteractionMode(NavigationMode);
            
            qDebug() << "Pose estimation completed, exited SetPoseMode";
            
            return true;
        }
        // 在 SetPoseMode 下拦截所有鼠标释放事件
        return true;
    }
    
    // 在 SetPoseMode 下拦截所有鼠标相关事件，防止传递给相机控制器
    if (event->type() == QEvent::MouseButtonPress || 
        event->type() == QEvent::MouseMove || 
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::Wheel) {
        return true; // 拦截所有鼠标事件
    }
    
    return QWidget::eventFilter(obj, event);
}

QVector3D PointCloudViewWidget::screenToWorld(const QPoint &screenPos)
{
    // 简化版: 将屏幕坐标投影到z=0平面
    // 更精确的实现需要射线与点云的交点计算
    
    QSize viewportSize = m_container->size();
    float ndcX = (2.0f * screenPos.x()) / viewportSize.width() - 1.0f;
    float ndcY = 1.0f - (2.0f * screenPos.y()) / viewportSize.height();
    
    // 获取相机参数
    QVector3D cameraPos = m_camera->position();
    QVector3D viewCenter = m_camera->viewCenter();
    QVector3D viewDir = (viewCenter - cameraPos).normalized();
    
    // 计算与z=0平面的交点(假设地面在z=0)
    // 射线参数方程: P = cameraPos + t * rayDir
    // 简化: 使用视图中心到相机的距离作为投影距离
    float distance = (viewCenter - cameraPos).length();
    
    // 获取相机的右向量和上向量
    QVector3D up = m_camera->upVector();
    QVector3D right = QVector3D::crossProduct(viewDir, up).normalized();
    up = QVector3D::crossProduct(right, viewDir).normalized();
    
    // 考虑视场角
    float fov = m_camera->fieldOfView();
    float aspect = m_camera->aspectRatio();
    float tanHalfFov = qTan(qDegreesToRadians(fov / 2.0f));
    
    // 计算射线方向
    QVector3D rayDir = viewDir 
                     + right * ndcX * tanHalfFov * aspect
                     + up * ndcY * tanHalfFov;
    rayDir.normalize();
    
    // 与z=0平面求交 (假设地面在z=0)
    float t;
    if (qAbs(rayDir.z()) > 0.0001f) {
        t = -cameraPos.z() / rayDir.z();
        if (t < 0) t = distance; // 如果射线向上,使用默认距离
    } else {
        t = distance; // 射线几乎平行于地面
    }
    
    QVector3D worldPos = cameraPos + rayDir * t;
    
    // qDebug() << "屏幕坐标" << screenPos << "-> 世界坐标" << worldPos; // 调试用，移动时会产生大量日志
    
    return worldPos;
}
