/**
 * @file TaskItemStatusLabel.cpp
 * @brief 转码任务状态图标组件实现
 * @details 实现自定义绘制的状态图标组件，支持四种状态的可视化显示和旋转动画
 */

#include "TaskItemStatusLabel.hpp"

#include <QPainter>
#include <QDebug>
#include "EyerCore/EyerCore.hpp"

/**
 * @brief 构造函数实现
 * @param parent 父窗口指针
 * @details
 * - 调用基类 QWidget 的构造函数
 * - 连接 QTimer 的 timeout 信号到 MyTimerEvent 槽函数
 * - 定时器用于驱动转码中状态的旋转动画
 */
TaskItemStatusLabel::TaskItemStatusLabel(QWidget * parent)
    : QWidget(parent)
{
    // 连接定时器信号，用于驱动旋转动画
    connect(&mTime, &QTimer::timeout, this, &TaskItemStatusLabel::MyTimerEvent);
}

/**
 * @brief 析构函数实现
 * @details 释放资源（当前无需手动释放）
 */
TaskItemStatusLabel::~TaskItemStatusLabel()
{

}

/**
 * @brief Qt 绘制事件回调函数实现 (重写 QWidget::paintEvent)
 * @param event 绘制事件对象
 * @details
 * Qt 框架自动调用此函数进行组件绘制，流程如下：
 * 1. 创建 QPainter 对象绑定到当前组件
 * 2. 获取组件的宽度和高度
 * 3. 清空绘制区域 (eraseRect)
 * 4. 根据当前状态调用对应的绘制函数：
 *    - ING (转码中) → drawIng
 *    - PREPARE (等待中) → drawWait
 *    - FAIL (失败) → drawAlert
 *    - SUCC (成功) → drawSucc
 */
void TaskItemStatusLabel::paintEvent(QPaintEvent *event)
{
    // 创建绘制器对象
    QPainter painter(this);
    QRect geo = this->geometry();

    int width = geo.width();
    int height = geo.height();

    // 清空绘制区域
    painter.eraseRect(0, 0, width, height);

    // 根据状态调用对应的绘制函数
    if(status == Eyer::EyerAVTranscoderStatus::ING){
        drawIng(painter, width, height);
    }
    else if(status == Eyer::EyerAVTranscoderStatus::PREPARE){
        drawWait(painter, width, height);
    }
    else if(status == Eyer::EyerAVTranscoderStatus::FAIL){
        drawAlert(painter, width, height);
    }
    else if(status == Eyer::EyerAVTranscoderStatus::SUCC){
        drawSucc(painter, width, height);
    }
}

/**
 * @brief 设置任务状态实现
 * @param _status 新的转码状态 (PREPARE/ING/SUCC/FAIL)
 * @return 0 表示成功
 * @details
 * 更新内部状态变量并触发重绘：
 * 1. 更新 status 成员变量
 * 2. 如果状态为 ING (转码中)，启动 QTimer 以驱动旋转动画
 * 3. 如果状态为其他值，停止 QTimer
 * 4. 调用 update() 触发 paintEvent 重绘
 */
int TaskItemStatusLabel::SetStatus(Eyer::EyerAVTranscoderStatus _status)
{
    status = _status;

    // 根据状态控制定时器
    if(status == Eyer::EyerAVTranscoderStatus::ING){
        mTime.start();  // 启动定时器以驱动旋转动画
    }
    else {
        mTime.stop();   // 停止定时器
    }
    update();  // 触发重绘
    return 0;
}

/**
 * @brief 定时器回调函数实现 (用于驱动旋转动画)
 * @details
 * 当任务状态为 ING (转码中) 时，QTimer 定时触发此函数
 * 调用 update() 触发 paintEvent 重绘，从而实现旋转动画的持续更新
 */
void TaskItemStatusLabel::MyTimerEvent ()
{
    update();  // 触发重绘，更新旋转动画
}

/**
 * @brief 绘制失败状态图标实现 (红色圆形 + 白色感叹号)
 * @param painter QPainter 绘制器对象
 * @param width 组件宽度
 * @param height 组件高度
 * @return 0 表示成功
 * @details
 * 绘制逻辑：
 * 1. 背景：绘制红色圆形 (RGB: 150, 0, 0)
 * 2. 前景：绘制白色感叹号图案
 *    - 三个小圆点 (分别位于上、中、下位置)
 *    - 一个多边形连接上下两个圆点，形成感叹号的竖线部分
 * 3. 启用抗锯齿渲染 (setRenderHint)
 */
int TaskItemStatusLabel::drawAlert   (QPainter &painter, int width, int height)
{
    painter.setPen(QPen(Qt::blue, 1, Qt::NoPen));
    painter.setBrush(QColor(150, 0, 0));  // 红色背景

    painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿
    painter.drawEllipse(0, 0, width, height);  // 绘制红色圆形背景


    // 绘制白色感叹号图案
    painter.setBrush(QColor(235, 235, 235));  // 白色前景
    // painter.setBrush(QColor(0, 0, 0));
    float ellipseR = height * 0.035;  // 圆点半径

    // 绘制三个小圆点 (感叹号的底部点、中部点、顶部点)
    painter.drawEllipse(QPointF(width / 2, height * 0.75), ellipseR * 2, ellipseR * 2);  // 底部圆点
    painter.drawEllipse(QPointF(width / 2, height * 0.25), ellipseR * 2, ellipseR * 2);  // 顶部圆点
    painter.drawEllipse(QPointF(width / 2, height * 0.55), ellipseR * 2 * 0.8, ellipseR * 2 * 0.8);  // 中部圆点

    // 绘制连接上下圆点的多边形 (形成感叹号的竖线部分)
    painter.setBrush(QColor(235, 235, 235));
    static const QPointF points[4] = {
            QPointF(width / 2 - ellipseR * 2, height * 0.25),  // 左上
            QPointF(width / 2 + ellipseR * 2, height * 0.25),  // 右上

            QPointF(width / 2 + ellipseR * 2 * 0.8, height * 0.55),  // 右下
            QPointF(width / 2 - ellipseR * 2 * 0.8, height * 0.55),  // 左下
    };
    painter.drawPolygon(points, 4);

    return 0;
}

/**
 * @brief 绘制转码中状态图标实现 (蓝色圆形 + 旋转的四个白色小圆点)
 * @param painter QPainter 绘制器对象
 * @param width 组件宽度
 * @param height 组件高度
 * @return 0 表示成功
 * @details
 * 绘制逻辑：
 * 1. 背景：绘制蓝色圆形 (RGB: 20, 0, 150)
 * 2. 前景：绘制四个白色小圆点，分布在上下左右四个方向
 * 3. 动画：使用 painter.rotate(time) 实现旋转效果
 *    - time 变量每次调用增加 0.01 度
 *    - 达到 360.0 度后重置为 0.0，形成循环动画
 * 4. 使用坐标变换 (translate) 将旋转中心移到组件中心
 */
int TaskItemStatusLabel::drawIng     (QPainter & painter, int width, int height)
{
    painter.setPen(QPen(Qt::blue, 1, Qt::NoPen));
    painter.setBrush(QColor(20, 0, 150));  // 蓝色背景

    painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿
    painter.drawEllipse(0, 0, width, height);  // 绘制蓝色圆形背景

    painter.setBrush(QColor(235, 235, 235));  // 白色前景

    // 坐标变换：将旋转中心移到组件中心
    painter.translate(width / 2, height / 2);
    painter.rotate(time);  // 应用旋转动画
    float ellipseR = height * 0.05;  // 圆点半径

    // 绘制四个白色小圆点 (上下左右四个方向)
    painter.drawEllipse(QPointF(0.0, height / 4.0), ellipseR * 2, ellipseR * 2);   // 下方
    painter.drawEllipse(QPointF(0.0, -height / 4.0), ellipseR * 2, ellipseR * 2);  // 上方
    painter.drawEllipse(QPointF(width / 4.0, 0.0), ellipseR * 2, ellipseR * 2);    // 右方
    painter.drawEllipse(QPointF(- width / 4.0, 0.0), ellipseR * 2, ellipseR * 2);  // 左方

    // 更新旋转角度 (每次调用增加 0.01 度)
    // EyerLog("RRR: %f\n", time);
    time += 0.01;
    if(time >= 360.0){
        time = 0.0;  // 达到 360 度后重置
    }
    return 0;
}

/**
 * @brief 绘制成功状态图标实现 (绿色圆形 + 白色对勾)
 * @param painter QPainter 绘制器对象
 * @param width 组件宽度
 * @param height 组件高度
 * @return 0 表示成功
 * @details
 * 绘制逻辑：
 * 1. 背景：绘制绿色圆形 (RGB: 0, 150, 10)
 * 2. 前景：绘制白色对勾图案
 *    - 对勾由六个顶点的多边形构成
 *    - 使用缩放系数 k = 0.6 调整对勾大小
 * 3. 启用抗锯齿渲染
 */
int TaskItemStatusLabel::drawSucc    (QPainter & painter, int width, int height)
{
    painter.setPen(QPen(Qt::blue, 1, Qt::NoPen));
    painter.setBrush(QColor(0, 150, 10));  // 绿色背景

    painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿
    painter.drawEllipse(0, 0, width, height);  // 绘制绿色圆形背景

    // 绘制白色对勾图案
    painter.setBrush(QColor(235, 235, 235));  // 白色前景
    float k = 0.6;  // 对勾缩放系数
    static const QPointF points[6] = {
            QPointF(0               * width * k + width * (1.0 - k) * 0.5,   50.45 / 100.0 * height * k + height * (1.0 - k) * 0.5),
            QPointF(10.7    / 100.0 * width * k + width * (1.0 - k) * 0.5,   41.8  / 100.0 * height * k + height * (1.0 - k) * 0.5),
            QPointF(35.05   / 100.0 * width * k + width * (1.0 - k) * 0.5,   60.85 / 100.0 * height * k + height * (1.0 - k) * 0.5),
            QPointF(97.45   / 100.0 * width * k + width * (1.0 - k) * 0.5,   6.85  / 100.0 * height * k + height * (1.0 - k) * 0.5),
            QPointF(100.0   / 100.0 * width * k + width * (1.0 - k) * 0.5,   12.8  / 100.0 * height * k + height * (1.0 - k) * 0.5),
            QPointF(41.95   / 100.0 * width * k + width * (1.0 - k) * 0.5,   93.15 / 100.0 * height * k + height * (1.0 - k) * 0.5),
    };
    painter.drawPolygon(points, 6);

    return 0;
}

/**
 * @brief 绘制等待状态图标实现 (青色圆形 + 三个白色小圆点)
 * @param painter QPainter 绘制器对象
 * @param width 组件宽度
 * @param height 组件高度
 * @return 0 表示成功
 * @details
 * 绘制逻辑：
 * 1. 背景：绘制青色圆形 (RGB: 0, 143, 150)
 * 2. 前景：绘制三个白色小圆点水平排列 (表示等待中的省略号效果)
 *    - 圆点直径为组件高度的 1/5
 *    - 圆点间距为圆点直径的 1.5 倍
 * 3. 启用抗锯齿渲染
 */
int TaskItemStatusLabel::drawWait    (QPainter & painter, int width, int height)
{
    painter.setPen(QPen(Qt::blue, 1, Qt::NoPen));
    painter.setBrush(QColor(0, 143, 150));  // 青色背景

    painter.setRenderHint(QPainter::Antialiasing);  // 启用抗锯齿
    painter.drawEllipse(0, 0, width, height);  // 绘制青色圆形背景

    // 绘制三个白色小圆点 (水平排列，表示等待中的省略号效果)
    painter.setBrush(QColor(235, 235, 235));  // 白色前景
    int smallEllipseC = height / 5;  // 小圆点直径
    painter.setRenderHint(QPainter::Antialiasing);

    // 中间圆点
    painter.drawEllipse(width / 2 - smallEllipseC / 2, height / 2 - smallEllipseC / 2, smallEllipseC, smallEllipseC);
    // 左侧圆点
    painter.drawEllipse(width / 2 - smallEllipseC / 2 - smallEllipseC * 1.5, height / 2 - smallEllipseC / 2, smallEllipseC, smallEllipseC);
    // 右侧圆点
    painter.drawEllipse(width / 2 - smallEllipseC / 2 + smallEllipseC * 1.5, height / 2 - smallEllipseC / 2, smallEllipseC, smallEllipseC);

    return 0;
}
