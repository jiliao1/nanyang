#ifndef POINTCLOUDVIEWWIDGET_H
#define POINTCLOUDVIEWWIDGET_H

#include <QWidget>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QTransform>
#include "pointcloudrenderer.h"
#include "customcameracontroller.h"

/**
 * @brief Qt3D点云视图Widget
 * 
 * 封装Qt3DWindow,提供可嵌入QWidget的3D点云视图
 * 支持多个点云层的同时显示
 * 支持2D Pose Estimate交互
 */
class PointCloudViewWidget : public QWidget
{
    Q_OBJECT

public:
    enum InteractionMode {
        NavigationMode,     // 导航模式(默认)
        SetPoseMode         // 设置位姿模式
    };

    explicit PointCloudViewWidget(QWidget *parent = nullptr);
    ~PointCloudViewWidget();

    /**
     * @brief 更新全局地图点云(/globalmap)
     * @param points 点坐标列表
     * @param colors 点颜色列表(可选)
     */
    void updateGlobalMap(const QVector<QVector3D> &points, 
                        const QVector<QVector3D> &colors = QVector<QVector3D>());

    /**
     * @brief 更新对齐点云(/aligned_points)
     * @param points 点坐标列表
     * @param colors 点颜色列表(可选)
     */
    void updateAlignedPoints(const QVector<QVector3D> &points, 
                            const QVector<QVector3D> &colors = QVector<QVector3D>());

    /**
     * @brief 更新TF坐标系变换
     * @param frame_id 坐标系名称(map, base_link, ptz_link)
     * @param translation 平移向量(x, y, z)
     * @param rotation 旋转四元数(x, y, z, w)
     */
    void updateTF(const QString &frame_id, const QVector3D &translation, const QQuaternion &rotation);

    /**
     * @brief 清空所有点云
     */
    void clearAll();

    /**
     * @brief 重置相机视角
     */
    void resetCamera();

    /**
     * @brief 设置背景颜色
     */
    void setBackgroundColor(const QColor &color);
    
    /**
     * @brief 设置交互模式
     */
    void setInteractionMode(InteractionMode mode);
    
    /**
     * @brief 获取当前交互模式
     */
    InteractionMode getInteractionMode() const { return m_interactionMode; }

signals:
    void pointCloudUpdated(const QString &topic, int pointCount);
    
    /**
     * @brief 用户设置了初始位姿
     * @param position 位置(x, y, z)
     * @param yaw 朝向角度(弧度)
     */
    void poseEstimated(QVector3D position, double yaw);

private:
    void setupScene();
    void setupCamera();
    void setupLighting();
    void setupPoseArrow();        // 设置位姿箭头
    void setupCenterCross();      // 设置中心十字标记
    void updatePoseArrow(const QVector3D &start, const QVector3D &end);
    void hidePoseArrow();
    
    bool eventFilter(QObject *obj, QEvent *event) override;  // 事件过滤器
    QVector3D screenToWorld(const QPoint &screenPos);        // 屏幕坐标转世界坐标

    Qt3DExtras::Qt3DWindow *m_view;
    QWidget *m_container;
    
    Qt3DCore::QEntity *m_rootEntity;
    Qt3DRender::QCamera *m_camera;
    CustomCameraController *m_cameraController;
    
    // 两个点云渲染器
    PointCloudRenderer *m_globalMapRenderer;
    PointCloudRenderer *m_alignedPointsRenderer;
    
    // 灯光
    Qt3DRender::QPointLight *m_light;
    Qt3DCore::QEntity *m_lightEntity;
    
    // 交互模式相关
    InteractionMode m_interactionMode;
    QVector3D m_poseStartPosition;    // 位姿起始位置
    bool m_isPoseDragging;            // 是否正在拖拽设置朝向
    
    // 位姿箭头实体
    Qt3DCore::QEntity *m_poseArrowEntity;
    Qt3DCore::QTransform *m_poseArrowTransform;
    
    // 中心十字标记
    Qt3DCore::QEntity *m_centerCrossEntity;
    Qt3DCore::QTransform *m_centerCrossTransform;
    
    // TF坐标系
    QMap<QString, Qt3DCore::QEntity*> m_frameEntities;      // 坐标系实体
    QMap<QString, Qt3DCore::QTransform*> m_frameTransforms; // 坐标系变换
    
    void updateCenterCrossPosition(); // 更新十字标记位置到视图中心
    void setCenterCrossVisible(bool visible); // 设置十字标记可见性
    void setupCoordinateFrame(const QString &frame_id); // 创建坐标系显示
};

#endif // POINTCLOUDVIEWWIDGET_H
