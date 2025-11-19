#include "pointcloudrenderer.h"
#include <QDebug>

PointCloudRenderer::PointCloudRenderer(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
    , m_pointCount(0)
    , m_pointSize(2.0f)
    , m_defaultColor(1.0f, 1.0f, 1.0f) // 默认白色
{
    // 创建几何体
    m_geometry = new Qt3DRender::QGeometry(this);
    
    // 创建顶点缓冲区
    m_vertexBuffer = new Qt3DRender::QBuffer(m_geometry);
    m_colorBuffer = new Qt3DRender::QBuffer(m_geometry);
    
    // 创建顶点属性(位置)
    m_positionAttribute = new Qt3DRender::QAttribute(m_geometry);
    m_positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());
    m_positionAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    m_positionAttribute->setVertexSize(3); // x, y, z
    m_positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    m_positionAttribute->setBuffer(m_vertexBuffer);
    m_positionAttribute->setByteStride(3 * sizeof(float));
    m_positionAttribute->setCount(0);
    
    // 创建颜色属性
    m_colorAttribute = new Qt3DRender::QAttribute(m_geometry);
    m_colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());
    m_colorAttribute->setVertexBaseType(Qt3DRender::QAttribute::Float);
    m_colorAttribute->setVertexSize(3); // r, g, b
    m_colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    m_colorAttribute->setBuffer(m_colorBuffer);
    m_colorAttribute->setByteStride(3 * sizeof(float));
    m_colorAttribute->setCount(0);
    
    // 添加属性到几何体
    m_geometry->addAttribute(m_positionAttribute);
    m_geometry->addAttribute(m_colorAttribute);
    
    // 创建几何渲染器
    m_geometryRenderer = new Qt3DRender::QGeometryRenderer(this);
    m_geometryRenderer->setGeometry(m_geometry);
    m_geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Points);
    
    // 创建支持顶点颜色的材质
    m_material = new Qt3DExtras::QPerVertexColorMaterial(this);
    
    // 创建变换
    m_transform = new Qt3DCore::QTransform(this);
    
    // 添加组件到实体
    addComponent(m_geometryRenderer);
    addComponent(m_material);
    addComponent(m_transform);
    
    qDebug() << "PointCloudRenderer initialized";
}

PointCloudRenderer::~PointCloudRenderer()
{
}

void PointCloudRenderer::updatePointCloud(const QVector<QVector3D> &points, 
                                         const QVector<QVector3D> &colors)
{
    if (points.isEmpty()) {
        qDebug() << "Warning: Attempting to update with empty point cloud";
        clear();
        return;
    }
    
    m_pointCount = points.size();
    updateBuffers(points, colors);
    
    // qDebug() << "Point cloud updated:" << m_pointCount << "points"; // 已屏蔽以减少日志
}

void PointCloudRenderer::updateBuffers(const QVector<QVector3D> &points, 
                                      const QVector<QVector3D> &colors)
{
    // 准备顶点数据
    QByteArray vertexData;
    vertexData.resize(points.size() * 3 * sizeof(float));
    float *vertexPtr = reinterpret_cast<float*>(vertexData.data());
    
    for (int i = 0; i < points.size(); ++i) {
        *vertexPtr++ = points[i].x();
        *vertexPtr++ = points[i].y();
        *vertexPtr++ = points[i].z();
    }
    
    // 更新顶点缓冲区
    m_vertexBuffer->setData(vertexData);
    m_positionAttribute->setCount(points.size());
    
    // 准备颜色数据
    QByteArray colorData;
    colorData.resize(points.size() * 3 * sizeof(float));
    float *colorPtr = reinterpret_cast<float*>(colorData.data());
    
    if (!colors.isEmpty() && colors.size() == points.size()) {
        // 使用提供的颜色
        for (int i = 0; i < colors.size(); ++i) {
            *colorPtr++ = colors[i].x(); // R
            *colorPtr++ = colors[i].y(); // G
            *colorPtr++ = colors[i].z(); // B
        }
    } else {
        // 使用默认颜色
        for (int i = 0; i < points.size(); ++i) {
            *colorPtr++ = m_defaultColor.x();
            *colorPtr++ = m_defaultColor.y();
            *colorPtr++ = m_defaultColor.z();
        }
    }
    
    // 更新颜色缓冲区
    m_colorBuffer->setData(colorData);
    m_colorAttribute->setCount(points.size());
    
    // 更新渲染器的顶点数量
    m_geometryRenderer->setVertexCount(points.size());
}

void PointCloudRenderer::setPointSize(float size)
{
    m_pointSize = size;
    // 注意: Qt3D 的原生点渲染点大小是固定的
    // 要实现不同大小需要使用自定义着色器或实例化网格
}

void PointCloudRenderer::setDefaultColor(const QVector3D &color)
{
    m_defaultColor = color;
}

void PointCloudRenderer::clear()
{
    m_pointCount = 0;
    m_vertexBuffer->setData(QByteArray());
    m_colorBuffer->setData(QByteArray());
    m_positionAttribute->setCount(0);
    m_colorAttribute->setCount(0);
    m_geometryRenderer->setVertexCount(0);
    
    qDebug() << "Point cloud cleared";
}
