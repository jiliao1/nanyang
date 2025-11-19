#ifndef CUSTOMCAMERACONTROLLER_H
#define CUSTOMCAMERACONTROLLER_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DInput/QMouseDevice>
#include <Qt3DInput/QMouseHandler>
#include <Qt3DInput/QWheelEvent>
#include <Qt3DInput/QMouseEvent>
#include <QVector3D>

/**
 * @brief 自定义相机控制器
 * 
 * 鼠标操作:
 * - 左键拖动: 平移
 * - 右键拖动: 旋转
 * - 滚轮: 缩放
 * 
 * 操作中心: 窗口中心点
 */
class CustomCameraController : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    explicit CustomCameraController(Qt3DCore::QNode *parent = nullptr);
    ~CustomCameraController();
    
    // 判断是否正在旋转
    bool isRotating() const { return m_rightButtonPressed; }
    
    void setCamera(Qt3DRender::QCamera *camera);
    Qt3DRender::QCamera* camera() const { return m_camera; }
    
    // 设置移动和旋转速度
    void setPanSpeed(float speed) { m_panSpeed = speed; }
    void setRotateSpeed(float speed) { m_rotateSpeed = speed; }
    void setZoomSpeed(float speed) { m_zoomSpeed = speed; }
    
    // 重置相机参数
    void resetCameraParameters();
    
    // 启用/禁用控制器
    void setControlEnabled(bool enabled);
    
signals:
    void rotatingChanged(bool rotating);
    
private slots:
    void onPressed(Qt3DInput::QMouseEvent *event);
    void onReleased(Qt3DInput::QMouseEvent *event);
    void onPositionChanged(Qt3DInput::QMouseEvent *event);
    void onWheel(Qt3DInput::QWheelEvent *event);
    
private:
    Qt3DRender::QCamera *m_camera;
    Qt3DInput::QMouseDevice *m_mouseDevice;
    Qt3DInput::QMouseHandler *m_mouseHandler;
    
    // 鼠标状态
    bool m_leftButtonPressed;
    bool m_rightButtonPressed;
    QPointF m_lastMousePosition;
    
    // 控制器启用状态
    bool m_controlEnabled;
    
    // 速度参数
    float m_panSpeed;      // 平移速度
    float m_rotateSpeed;   // 旋转速度
    float m_zoomSpeed;     // 缩放速度
    
    // 相机参数
    QVector3D m_viewCenter; // 观察中心点
    float m_cameraDistance; // 相机到中心点的距离
    float m_azimuth;        // 方位角(绕Y轴旋转)
    float m_elevation;      // 仰角(上下旋转)
    
    void updateCameraPosition();
};

#endif // CUSTOMCAMERACONTROLLER_H
