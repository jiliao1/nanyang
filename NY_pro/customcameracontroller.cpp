#include "customcameracontroller.h"
#include <QDebug>
#include <QtMath>

CustomCameraController::CustomCameraController(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
    , m_camera(nullptr)
    , m_leftButtonPressed(false)
    , m_rightButtonPressed(false)
    , m_controlEnabled(true)
    , m_panSpeed(0.01f)
    , m_rotateSpeed(0.5f)
    , m_zoomSpeed(0.1f)
    , m_viewCenter(0, 0, 0)
    , m_cameraDistance(40.0f)
    , m_azimuth(0.0f)
    , m_elevation(0.0f)
{
    // 创建鼠标设备
    m_mouseDevice = new Qt3DInput::QMouseDevice(this);
    
    // 创建鼠标处理器
    m_mouseHandler = new Qt3DInput::QMouseHandler(this);
    m_mouseHandler->setSourceDevice(m_mouseDevice);
    
    // 连接信号
    connect(m_mouseHandler, &Qt3DInput::QMouseHandler::pressed,
            this, &CustomCameraController::onPressed);
    connect(m_mouseHandler, &Qt3DInput::QMouseHandler::released,
            this, &CustomCameraController::onReleased);
    connect(m_mouseHandler, &Qt3DInput::QMouseHandler::positionChanged,
            this, &CustomCameraController::onPositionChanged);
    connect(m_mouseHandler, &Qt3DInput::QMouseHandler::wheel,
            this, &CustomCameraController::onWheel);
    
    addComponent(m_mouseHandler);
}

CustomCameraController::~CustomCameraController()
{
}

void CustomCameraController::setCamera(Qt3DRender::QCamera *camera)
{
    m_camera = camera;
    if (m_camera) {
        // 初始化相机参数
        m_viewCenter = m_camera->viewCenter();
        QVector3D cameraPos = m_camera->position();
        QVector3D delta = cameraPos - m_viewCenter;
        m_cameraDistance = delta.length();
        
        // 计算初始方位角和仰角
        m_azimuth = qRadiansToDegrees(qAtan2(delta.x(), delta.z()));
        m_elevation = qRadiansToDegrees(qAsin(delta.y() / m_cameraDistance));
    }
}

void CustomCameraController::onPressed(Qt3DInput::QMouseEvent *event)
{
    if (!m_controlEnabled) return; // 控制器被禁用时不处理
    
    if (event->button() == Qt3DInput::QMouseEvent::LeftButton) {
        m_leftButtonPressed = true;
        m_lastMousePosition = QPointF(event->x(), event->y());
    } else if (event->button() == Qt3DInput::QMouseEvent::RightButton) {
        m_rightButtonPressed = true;
        m_lastMousePosition = QPointF(event->x(), event->y());
        emit rotatingChanged(true); // 通知开始旋转
    }
}

void CustomCameraController::onReleased(Qt3DInput::QMouseEvent *event)
{
    if (!m_controlEnabled) return; // 控制器被禁用时不处理
    
    if (event->button() == Qt3DInput::QMouseEvent::LeftButton) {
        m_leftButtonPressed = false;
    } else if (event->button() == Qt3DInput::QMouseEvent::RightButton) {
        m_rightButtonPressed = false;
        emit rotatingChanged(false); // 通知旋转结束
    }
}

void CustomCameraController::onPositionChanged(Qt3DInput::QMouseEvent *event)
{
    if (!m_controlEnabled) return; // 控制器被禁用时不处理
    if (!m_camera) return;
    
    QPointF currentPos(event->x(), event->y());
    QPointF delta = currentPos - m_lastMousePosition;
    
    if (m_leftButtonPressed) {
        // 左键 - 平移
        // 获取相机的右向量和上向量
        QVector3D right = QVector3D::crossProduct(
            m_camera->viewCenter() - m_camera->position(),
            m_camera->upVector()
        ).normalized();
        QVector3D up = m_camera->upVector().normalized();
        
        // 根据鼠标移动平移视图中心
        float panX = -delta.x() * m_panSpeed * m_cameraDistance * 0.01f;
        float panY = delta.y() * m_panSpeed * m_cameraDistance * 0.01f;
        
        m_viewCenter += right * panX + up * panY;
        updateCameraPosition();
        
    } else if (m_rightButtonPressed) {
        // 右键 - 旋转
        m_azimuth += delta.x() * m_rotateSpeed;
        m_elevation += delta.y() * m_rotateSpeed;
        
        // 限制仰角范围
        m_elevation = qBound(-89.0f, m_elevation, 89.0f);
        
        updateCameraPosition();
    }
    
    m_lastMousePosition = currentPos;
}

void CustomCameraController::onWheel(Qt3DInput::QWheelEvent *event)
{
    if (!m_controlEnabled) return; // 控制器被禁用时不处理
    if (!m_camera) return;
    
    // 滚轮 - 缩放
    float delta = event->angleDelta().y();
    m_cameraDistance *= (1.0f - delta * m_zoomSpeed * 0.001f);
    
    // 限制距离范围
    m_cameraDistance = qBound(1.0f, m_cameraDistance, 1000.0f);
    
    updateCameraPosition();
}

void CustomCameraController::updateCameraPosition()
{
    if (!m_camera) return;
    
    // 根据方位角和仰角计算相机位置
    float azimuthRad = qDegreesToRadians(m_azimuth);
    float elevationRad = qDegreesToRadians(m_elevation);
    
    float x = m_cameraDistance * qCos(elevationRad) * qSin(azimuthRad);
    float y = m_cameraDistance * qSin(elevationRad);
    float z = m_cameraDistance * qCos(elevationRad) * qCos(azimuthRad);
    
    QVector3D newPosition = m_viewCenter + QVector3D(x, y, z);
    
    m_camera->setPosition(newPosition);
    m_camera->setViewCenter(m_viewCenter);
}

void CustomCameraController::resetCameraParameters()
{
    if (!m_camera) return;
    
    // 重置相机参数为初始值
    m_viewCenter = QVector3D(0, 0, 0);
    m_cameraDistance = 40.0f;
    m_azimuth = 0.0f;
    m_elevation = 0.0f;
    
    updateCameraPosition();
}

void CustomCameraController::setControlEnabled(bool enabled)
{
    m_controlEnabled = enabled;
    
    // 禁用时重置按钮状态，防止卡住
    if (!enabled) {
        m_leftButtonPressed = false;
        m_rightButtonPressed = false;
    }
    
    if (m_mouseHandler) {
        m_mouseHandler->setEnabled(enabled);
        qDebug() << "CustomCameraController" << (enabled ? "enabled" : "disabled");
    }
}
