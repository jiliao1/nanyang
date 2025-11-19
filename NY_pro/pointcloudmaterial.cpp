#include "pointcloudmaterial.h"
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QDepthTest>
#include <QDebug>

PointCloudMaterial::PointCloudMaterial(Qt3DCore::QNode *parent)
    : QMaterial(parent)
    , m_pointSize(3.0f)
{
    m_effect = new Qt3DRender::QEffect(this);

    // 创建着色器程序
    Qt3DRender::QShaderProgram *shaderProgram = new Qt3DRender::QShaderProgram(m_effect);
    
    // 顶点着色器 - 根据颜色自动调整点大小
    shaderProgram->setVertexShaderCode(R"(
        #version 150 core
        
        in vec3 vertexPosition;
        in vec3 vertexColor;
        
        out vec3 color;
        
        uniform mat4 modelViewProjection;
        uniform float pointSize;
        
        void main()
        {
            color = vertexColor;
            gl_Position = modelViewProjection * vec4(vertexPosition, 1.0);
            
            // 根据颜色判断：红色(1,0,0)使用pointSize，绿色(0,1,0)使用pointSize/2
            if (vertexColor.r > 0.5 && vertexColor.g < 0.5) {
                // 红色点
                gl_PointSize = pointSize;
            } else if (vertexColor.g > 0.5 && vertexColor.r < 0.5) {
                // 绿色点
                gl_PointSize = pointSize * 0.5;
            } else {
                // 其他颜色
                gl_PointSize = pointSize;
            }
        }
    )");
    
    // 片段着色器 - 使用顶点颜色
    shaderProgram->setFragmentShaderCode(R"(
        #version 150 core
        
        in vec3 color;
        out vec4 fragColor;
        
        void main()
        {
            fragColor = vec4(color, 1.0);
        }
    )");

    // 创建渲染通道
    Qt3DRender::QRenderPass *renderPass = new Qt3DRender::QRenderPass(m_effect);
    renderPass->setShaderProgram(shaderProgram);
    
    // 添加深度测试渲染状态
    Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest();
    depthTest->setDepthFunction(Qt3DRender::QDepthTest::Less);
    renderPass->addRenderState(depthTest);

    // 创建技术
    Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique(m_effect);
    technique->addRenderPass(renderPass);
    
    // 设置API过滤器(OpenGL 3.2+)
    Qt3DRender::QGraphicsApiFilter *apiFilter = technique->graphicsApiFilter();
    apiFilter->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
    apiFilter->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);
    apiFilter->setMajorVersion(3);
    apiFilter->setMinorVersion(2);
    
    // 添加过滤键
    Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey(technique);
    filterKey->setName(QStringLiteral("renderingStyle"));
    filterKey->setValue(QStringLiteral("forward"));
    technique->addFilterKey(filterKey);
    
    m_effect->addTechnique(technique);

    // 创建点大小参数 - 添加到效果以确保传递给着色器
    m_pointSizeParameter = new Qt3DRender::QParameter(QStringLiteral("pointSize"), m_pointSize);
    m_effect->addParameter(m_pointSizeParameter);
    
    setEffect(m_effect);
    
    qDebug() << "PointCloudMaterial created with initial point size:" << m_pointSize;
}

PointCloudMaterial::~PointCloudMaterial()
{
}

void PointCloudMaterial::setPointSize(float size)
{
    m_pointSize = size;
    m_pointSizeParameter->setValue(size);
    qDebug() << "PointCloudMaterial: Point size set to" << size;
}
