/**
 * @file TaskItemStatusLabel.hpp
 * @brief 转码任务状态图标组件头文件
 * @details 自定义 QWidget 组件，用于在任务列表中显示转码任务的实时状态图标
 *          支持四种状态：等待中、转码中、成功、失败，每种状态使用不同颜色和图案表示
 */

#ifndef TASKITEMSTATUSLABEL_HPP
#define TASKITEMSTATUSLABEL_HPP

#include <QWidget>
#include <QTimer>
#include "EyerAVTranscoder/EyerAVTranscoderStatus.hpp"

namespace Ui {
    class TaskItemStatusLabel;
}

/**
 * @class TaskItemStatusLabel
 * @brief 转码任务状态图标组件
 * @details
 * 这是一个自定义绘制的 QWidget，用于显示转码任务的四种状态：
 * - PREPARE (等待中): 青色圆形，内部三个白色小圆点 (drawWait)
 * - ING (转码中): 蓝色圆形，内部四个旋转的白色小圆点 (drawIng)，使用 QTimer 驱动动画
 * - SUCC (成功): 绿色圆形，内部白色对勾图案 (drawSucc)
 * - FAIL (失败): 红色圆形，内部白色感叹号图案 (drawAlert)
 *
 * 通过 QPainter 自定义绘制，无需使用图片资源文件
 */
class TaskItemStatusLabel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     * @details 初始化组件并连接 QTimer 的 timeout 信号到 MyTimerEvent 槽，用于驱动旋转动画
     */
    explicit TaskItemStatusLabel(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~TaskItemStatusLabel();

    /**
     * @brief Qt 绘制事件回调函数 (重写 QWidget::paintEvent)
     * @param event 绘制事件对象
     * @details
     * 根据当前 status 状态调用对应的绘制函数：
     * - PREPARE → drawWait (等待状态图标)
     * - ING → drawIng (转码中旋转动画图标)
     * - SUCC → drawSucc (成功对勾图标)
     * - FAIL → drawAlert (失败感叹号图标)
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 设置任务状态
     * @param _status 新的转码状态 (PREPARE/ING/SUCC/FAIL)
     * @return 0 表示成功
     * @details
     * 更新内部状态变量并触发重绘：
     * - 如果状态为 ING (转码中)，启动 QTimer 以驱动旋转动画
     * - 如果状态为其他值，停止 QTimer
     * - 调用 update() 触发重绘
     */
    int SetStatus(Eyer::EyerAVTranscoderStatus _status);

    /**
     * @brief 定时器回调函数 (用于驱动旋转动画)
     * @details
     * 当任务状态为 ING (转码中) 时，QTimer 定时触发此函数
     * 调用 update() 触发重绘，从而实现旋转动画效果
     */
    void MyTimerEvent ();

private:
    /**
     * @brief 绘制失败状态图标 (红色圆形 + 白色感叹号)
     * @param painter QPainter 绘制器对象
     * @param width 组件宽度
     * @param height 组件高度
     * @return 0 表示成功
     * @details
     * 绘制逻辑：
     * - 背景：红色圆形 (RGB: 150, 0, 0)
     * - 前景：白色感叹号由三个圆点 + 一个多边形构成
     */
    int drawAlert   (QPainter & painter, int width, int height);

    /**
     * @brief 绘制转码中状态图标 (蓝色圆形 + 旋转的四个白色小圆点)
     * @param painter QPainter 绘制器对象
     * @param width 组件宽度
     * @param height 组件高度
     * @return 0 表示成功
     * @details
     * 绘制逻辑：
     * - 背景：蓝色圆形 (RGB: 20, 0, 150)
     * - 前景：四个白色小圆点分布在上下左右四个方向
     * - 动画：使用 painter.rotate(time) 实现旋转效果，time 每次调用增加 0.01
     */
    int drawIng     (QPainter & painter, int width, int height);

    /**
     * @brief 绘制成功状态图标 (绿色圆形 + 白色对勾)
     * @param painter QPainter 绘制器对象
     * @param width 组件宽度
     * @param height 组件高度
     * @return 0 表示成功
     * @details
     * 绘制逻辑：
     * - 背景：绿色圆形 (RGB: 0, 150, 10)
     * - 前景：白色对勾由六个顶点的多边形构成
     */
    int drawSucc    (QPainter & painter, int width, int height);

    /**
     * @brief 绘制等待状态图标 (青色圆形 + 三个白色小圆点)
     * @param painter QPainter 绘制器对象
     * @param width 组件宽度
     * @param height 组件高度
     * @return 0 表示成功
     * @details
     * 绘制逻辑：
     * - 背景：青色圆形 (RGB: 0, 143, 150)
     * - 前景：三个白色小圆点水平排列 (表示等待中的省略号效果)
     */
    int drawWait    (QPainter & painter, int width, int height);

    /**
     * @brief 当前转码任务状态
     * @details 默认为 PREPARE (等待状态)
     */
    Eyer::EyerAVTranscoderStatus status = status.PREPARE;

    /**
     * @brief 旋转动画时间变量 (用于 drawIng 函数)
     * @details
     * - 范围：0.0 ~ 360.0 度
     * - 每次绘制时增加 0.01，达到 360.0 后重置为 0.0
     * - 用于控制转码中图标的旋转角度
     */
    float time = 0;

    /**
     * @brief Qt 定时器对象
     * @details
     * 当任务状态为 ING (转码中) 时启动，定时触发 MyTimerEvent()
     * 从而实现旋转动画的持续更新
     */
    QTimer mTime;
};

#endif // TASKITEMSTATUSLABEL_HPP
