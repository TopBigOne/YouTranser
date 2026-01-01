/**
 * @file YouTransMainWindow.hpp
 * @brief YouTranser 主窗口类定义
 * @details 应用程序主界面,提供文件选择、转码配置、任务管理等核心功能
 */

#ifndef YOUTRANSMAINWINDOW_HPP
#define YOUTRANSMAINWINDOW_HPP

#include <QMainWindow>
#include <QCloseEvent>

#include "TaskItem.hpp"

#include "YouTransConfig.hpp"

namespace Ui {
class YouTransMainWindow;
}

/**
 * @enum MainStatus
 * @brief 主窗口状态枚举
 */
enum MainStatus
{
    ING,    ///< 转码进行中状态 (Transcoding In Progress)
    OTHER   ///< 其他状态 (空闲/等待/完成等)
};

/**
 * @class YouTransMainWindow
 * @brief YouTranser 主窗口类
 * @details
 * - 继承自 QMainWindow,提供菜单栏、工具栏等标准窗口功能
 * - 管理转码任务列表,支持批量任务处理
 * - 提供转码参数配置界面
 * - 处理文件拖放、任务进度更新等用户交互
 */
class YouTransMainWindow : public QMainWindow
{
    Q_OBJECT  // Qt 元对象系统宏,支持信号槽机制

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针 (默认为 nullptr)
     */
    explicit YouTransMainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数,释放 UI 资源
     */
    ~YouTransMainWindow();

    /**
     * @brief 显示转码参数配置对话框
     * @details 打开参数配置窗口,让用户设置编码器、码率、分辨率等
     */
    void ShowTranscoderParams();

    /**
     * @brief 更新系统状态标签
     * @return 0表示成功
     * @details 更新状态栏或标题栏显示的系统信息
     */
    int UpdateSystemLabel();


protected:
    /**
     * @brief 窗口关闭事件处理函数
     * @param event 关闭事件对象
     * @details
     * - 重写 QMainWindow::closeEvent
     * - 处理正在进行的转码任务,提示用户确认关闭
     * - 保存窗口状态和配置
     */
    void closeEvent(QCloseEvent * event);

public slots:
    /**
     * @brief 设置输入文件路径按钮点击槽函数
     * @details 打开文件选择对话框,选择输入视频文件
     */
    void SetInputPathClickListener();

    /**
     * @brief 设置输出文件路径按钮点击槽函数
     * @details 打开文件保存对话框,选择输出视频保存位置
     */
    void SetOutputPathClickListener();

    /**
     * @brief 打开转码配置窗口按钮点击槽函数
     * @details 显示 YouTransConfig 配置对话框
     */
    void StartTranscodeConfigClickListener();

    /**
     * @brief 开始转码按钮点击槽函数
     * @details
     * - 验证输入/输出路径
     * - 创建转码任务
     * - 启动转码线程
     */
    void StartTranscodeClickListener();

    /**
     * @brief 配置窗口关闭槽函数
     * @details 配置窗口关闭时被调用,可以保存配置参数
     */
    void OnConfigWindowsClose();

    /**
     * @brief 任务成功完成槽函数
     * @details
     * - 由 TaskItem 发出的成功信号触发
     * - 更新任务列表状态
     * - 显示成功通知
     */
    void TaskItem_OnTaskSuccess();

    /**
     * @brief 任务失败槽函数
     * @param code 错误码
     * @details
     * - 由 TaskItem 发出的失败信号触发
     * - 显示错误信息对话框
     */
    void TaskItem_OnTaskFail(int code);

    /**
     * @brief 任务移除槽函数
     * @param taskItem 要移除的任务项指针
     * @details
     * - 从任务列表中移除指定任务
     * - 释放任务资源
     */
    void TaskItem_OnRemove(TaskItem * taskItem);

    /**
     * @brief 关于菜单项点击槽函数
     * @details 显示关于对话框,包含应用版本、作者等信息
     */
    void AboutActionClickListener();

private:

    /**
     * @brief 开始转码内部实现函数
     * @details 实际执行转码启动逻辑,由公共槽函数调用
     */
    void StartTranscodeClickListenerInternal();

    Ui::YouTransMainWindow *ui;  ///< Qt Designer 生成的 UI 对象指针

    YouTransConfig * configWindow = nullptr;  ///< 转码配置窗口指针

    YouTranscoderParams params;  ///< 当前转码参数配置
};

#endif // YOUTRANSMAINWINDOW_HPP
