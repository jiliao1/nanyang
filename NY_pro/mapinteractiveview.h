#ifndef MAPINTERACTIVEVIEW_H
#define MAPINTERACTIVEVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDebug>

/**
 * @brief 一个自定义的 QGraphicsView 子类，用于处理模式切换。
 * - PanMode: 允许抓手拖拽平移和滚轮缩放。
 * - AddPointMode: 禁用抓手，将左键点击转换为 sceneClicked 信号，并允许滚轮缩放。
 */
class MapInteractiveView : public QGraphicsView
{
    Q_OBJECT // 必须添加 Q_OBJECT 宏

public:
    // 定义视图的两种交互模式
    enum ViewMode {
        PanMode,
        AddPointMode
    };

    // 构造函数
    explicit MapInteractiveView(QWidget *parent = nullptr)
        : QGraphicsView(parent), m_mode(PanMode) // 默认启动时为平移模式
    {
        // 确保这些设置在构造时就被应用
        setDragMode(QGraphicsView::ScrollHandDrag); // 默认平移
        setInteractive(true);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        setResizeAnchor(QGraphicsView::AnchorUnderMouse);
        setRenderHint(QPainter::Antialiasing);

        // 禁用滚动条，这是为了帮助滚轮缩放
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

public slots:
    /**
     * @brief 公共槽函数，用于从外部 (如 robot.cpp) 切换视图模式。
     */
    void setViewMode(ViewMode mode)
    {
        if (m_mode == mode) return; // 模式未改变

        m_mode = mode;
        if (m_mode == PanMode) {
            // 切换到平移模式：启用抓手
            setDragMode(QGraphicsView::ScrollHandDrag);
        } else {
            // 切换到添加点模式：禁用抓手，以便 mousePressEvent 可以捕获点击
            setDragMode(QGraphicsView::NoDrag);
        }
    }

signals:
    /**
         * @brief [新信号] 当在 AddPointMode 下点击场景时发射。
         * @param scenePos 点击位置对应的场景坐标 (像素坐标)。
         * @param button 被按下的鼠标按钮 (Qt::LeftButton 或 Qt::RightButton)。
         */
    void sceneClickedWithButton(const QPointF &scenePos, Qt::MouseButton button);

protected:
    /**
     * @brief 重新实现滚轮事件，以强制缩放。
     */
    void wheelEvent(QWheelEvent *event) override
    {
        // 无论在哪种模式下，我们都希望滚轮=缩放

        // setTransformationAnchor(AnchorUnderMouse) 已经设置了
        // 我们只需要应用缩放
        double scaleFactor = 1.15; // 每次缩放 15%
        if (event->angleDelta().y() > 0) {
            // 向上滚动 = 放大
            scale(scaleFactor, scaleFactor);
        } else {
            // 向下滚动 = 缩小
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }

        // (注意：如果禁用了滚动条，就不需要 event->accept()，
        // 但如果滚动条还在，就需要 accept() 来防止事件传递)
    }

    /**
     * @brief 重新实现鼠标按下事件，以区分平移和点击。
     */
    void mousePressEvent(QMouseEvent *event) override
    {

        if (m_mode == AddPointMode)
                {
                    // 检查是左键还是右键
                    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
                    {
                        // 发射【带按钮信息】的新信号
                        emit sceneClickedWithButton(mapToScene(event->pos()), event->button());
                        // *不* 调用基类实现，防止触发拖拽
                        return;
                    }
                }

                // 如果是“平移模式”(PanMode)，或者点击了其他按钮（如中键）
                // 则调用基类的默认实现 (例如，启动抓手拖拽)
                QGraphicsView::mousePressEvent(event);
            }

private:
    ViewMode m_mode; // 存储当前模式
};

#endif // MAPINTERACTIVEVIEW_H
