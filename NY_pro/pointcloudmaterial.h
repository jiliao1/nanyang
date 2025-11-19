#ifndef POINTCLOUDMATERIAL_H
#define POINTCLOUDMATERIAL_H

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QGraphicsApiFilter>

/**
 * @brief 支持可变点大小的点云材质
 */
class PointCloudMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT

public:
    explicit PointCloudMaterial(Qt3DCore::QNode *parent = nullptr);
    ~PointCloudMaterial();

    void setPointSize(float size);
    float pointSize() const { return m_pointSize; }

private:
    Qt3DRender::QEffect *m_effect;
    Qt3DRender::QParameter *m_pointSizeParameter;
    float m_pointSize;
};

#endif // POINTCLOUDMATERIAL_H
