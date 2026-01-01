/**
 * @file TaskItem.hpp
 * @brief 转码任务项组件类定义
 * @details 任务列表中的单个任务项 UI 组件,封装任务状态显示、进度条、控制按钮等功能
 */

#ifndef TASKITEM_HPP
#define TASKITEM_HPP

#include <QWidget>
#include "TranscodeTaskThread.hpp"
#include "TaskItemStatusLabel.hpp"
#include "YouTranscoderParams.hpp"

namespace Ui {
class TaskItem;
}

/**
 * @class TaskItem
 * @brief 转码任务项 UI 组件类
 * @details
 * - 继承自 QWidget,是任务列表中的可视化项
 * - 封装转码线程 TranscodeTaskThread 的控制逻辑
 * - 显示任务状态、进度、文件信息
 * - 提供开始、停止、删除任务的用户界面
 * - 通过信号槽与主窗口通信,上报任务状态变化
 */
class TaskItem : public QWidget
{
    Q_OBJECT  // Qt 元对象系统宏,支持信号槽机制

public:
    /**
     * @brief 构造函数
     * @param inputPath 输入视频文件路径
     * @param parent 父窗口指针 (默认为 nullptr)
     * @details 创建任务项并初始化 UI,关联转码线程
     */
    explicit TaskItem(QString inputPath, QWidget *parent = nullptr);

    /**
     * @brief 析构函数,释放 UI 资源和转码线程
     */
    ~TaskItem();

    /**
     * @brief 设置转码参数
     * @param _params 转码参数对象 (包含编码器、码率、分辨率等)
     * @return 0表示成功
     * @details 将参数传递给内部转码线程
     */
    int SetParams(const YouTranscoderParams & _params);

    /**
     * @brief 设置输出目录
     * @param _output 输出文件夹路径
     * @return 0表示成功
     */
    int SetOutputDir(const QString & _output);

    /**
     * @brief 设置输出文件名前缀
     * @param _filenamePrefix 文件名前缀字符串
     * @return 0表示成功
     */
    int SetFilenamePrefix(const QString & _filenamePrefix);

    /**
     * @brief 启动转码任务
     * @return 0表示成功
     * @details 启动内部转码线程,开始视频转换
     */
    int StartTask();

    /**
     * @brief 停止转码任务
     * @return 0表示成功
     * @details 停止转码线程并等待线程结束
     */
    int StopTask();

    /**
     * @brief 获取任务状态
     * @return 转码状态枚举 (准备中/进行中/成功/失败)
     */
    Eyer::EyerAVTranscoderStatus GetStatus();

    /**
     * @brief 设置任务状态
     * @param _status 转码状态枚举
     * @return 0表示成功
     */
    int SetStatus(const Eyer::EyerAVTranscoderStatus & _status);

    /**
     * @brief 刷新状态显示
     * @return 0表示成功
     * @details 更新 UI 上的状态标签和进度条
     */
    int showStatus();

public slots:
    /**
     * @brief 任务进度更新槽函数
     * @param progress 转码进度 (0.0-1.0)
     * @details 由转码线程发出进度信号时调用,更新进度条显示
     */
    void OnTaskProgress(float progress);

    /**
     * @brief 任务成功完成槽函数
     * @details 由转码线程发出成功信号时调用,更新 UI 状态并向上发出信号
     */
    void OnTaskSuccess();

    /**
     * @brief 任务失败槽函数
     * @param code 错误码
     * @details 由转码线程发出失败信号时调用,显示错误信息并向上发出信号
     */
    void OnTaskFail(int code);

    /**
     * @brief 移除按钮点击槽函数
     * @details 用户点击移除按钮时调用,向上发出移除信号
     */
    void OnBtnRemove();

signals:
    /**
     * @brief 任务成功完成信号
     * @details 发送到主窗口,通知任务成功完成
     */
    void TaskItem_OnTaskSuccess();

    /**
     * @brief 任务失败信号
     * @param code 错误码
     * @details 发送到主窗口,通知任务失败
     */
    void TaskItem_OnTaskFail(int code);

    /**
     * @brief 任务移除信号
     * @param taskitem 要移除的任务项指针 (this)
     * @details 发送到主窗口,请求从任务列表中移除此项
     */
    void TaskItem_OnRemove(TaskItem * taskitem);

private:
    Ui::TaskItem *ui;  ///< Qt Designer 生成的 UI 对象指针

    TranscodeTaskThread taskThread;  ///< 转码线程对象

    TaskItemStatusLabel * statusLabel = nullptr;  ///< 状态标签组件指针

    YouTranscoderParams params;  ///< 转码参数配置
    QString outputDir = "";  ///< 输出文件夹路径
    QString filenamePrefix = "";  ///< 输出文件名前缀
};

#endif // TASKITEM_HPP
