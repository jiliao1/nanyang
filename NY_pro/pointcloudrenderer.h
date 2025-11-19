#ifndef POINTCLOUDRENDERER_H
#define POINTCLOUDRENDERER_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <QVector3D>
#include <QVector>

/**
 * @brief 点云渲染实体
 * 
 * 使用Qt3D渲染点云数据
 * 支持动态更新点云坐标和颜色
 */
class PointCloudRenderer : public Qt3DCore::QEntity
{
    Q_OBJECT

public:
    explicit PointCloudRenderer(Qt3DCore::QNode *parent = nullptr);
    ~PointCloudRenderer();

    /**
     * @brief 更新点云数据
     * @param points 点的坐标列表
     * @param colors 点的颜色列表(可选,如果为空则使用默认颜色)
     */
    void updatePointCloud(const QVector<QVector3D> &points, 
                         const QVector<QVector3D> &colors = QVector<QVector3D>());

    /**
     * @brief 设置点的大小
     * @param size 点大小(像素)
     */
    void setPointSize(float size);

    /**
     * @brief 设置默认颜色(当没有提供颜色数据时使用)
     * @param color 颜色(RGB 0-1范围)
     */
    void setDefaultColor(const QVector3D &color);

    /**
     * @brief 清空点云
     */
    void clear();

    /**
     * @brief 获取当前点数
     */
    int getPointCount() const { return m_pointCount; }

private:
    void setupGeometry();
    void updateBuffers(const QVector<QVector3D> &points, 
                      const QVector<QVector3D> &colors);

    Qt3DRender::QGeometry *m_geometry;
    Qt3DRender::QGeometryRenderer *m_geometryRenderer;
    Qt3DRender::QAttribute *m_positionAttribute;
    Qt3DRender::QAttribute *m_colorAttribute;
    Qt3DRender::QBuffer *m_vertexBuffer;
    Qt3DRender::QBuffer *m_colorBuffer;
    Qt3DExtras::QPerVertexColorMaterial *m_material;
    Qt3DCore::QTransform *m_transform;

    int m_pointCount;
    float m_pointSize;
    QVector3D m_defaultColor;
};

#endif // POINTCLOUDRENDERER_H
