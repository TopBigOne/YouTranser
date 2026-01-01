/**
 * @file TaskItem.cpp
 * @brief 转码任务项组件类实现
 * @details 实现任务项的 UI 初始化、转码控制、状态显示等功能
 */

#include "TaskItem.hpp"
#include "ui_TaskItem.h"

#include <QFileInfo>

/**
 * @brief 构造函数实现
 * @param inputPath 输入视频文件路径
 * @param parent 父窗口指针
 * @details
 * - 初始化 UI 组件 (setupUi)
 * - 初始化进度条 (0-100)
 * - 设置输入文件路径显示
 * - 连接移除按钮和转码线程的信号槽
 * - 创建状态标签组件
 * - 刷新状态显示
 */
TaskItem::TaskItem(QString inputPath, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TaskItem)
    , taskThread(inputPath)  // 初始化转码线程,传入输入文件路径
{
    ui->setupUi(this);

    // 设置进度条范围 (0-100)
    ui->task_progress_bar->setMaximum(100);
    ui->task_progress_bar->setValue(0);

    // 显示输入文件路径
    ui->task_input_path->setText(inputPath);

    // 设置移除按钮文本并连接槽函数
    ui->task_btn_remove->setText("移除任务");
    connect(ui->task_btn_remove,       SIGNAL(clicked()),    this,   SLOT(OnBtnRemove()));

    // 平台差异处理:非 Windows 平台显示进度百分比,Windows 平台留空
#ifndef WIN32
    ui->task_progress_label->setText("0%");
#else
    ui->task_progress_label->setText("");
#endif

    // 初始化错误标签为隐藏状态
    ui->task_error_label->setText("");
    ui->task_error_label->setVisible(false);

    // 连接转码线程的信号到本地槽函数
    connect(&taskThread,                 SIGNAL(OnTaskProgress(float)),     this,   SLOT(OnTaskProgress(float)));
    connect(&taskThread,                 SIGNAL(OnTaskSuccess()),           this,   SLOT(OnTaskSuccess()));
    connect(&taskThread,                 SIGNAL(OnTaskFail(int)),           this,   SLOT(OnTaskFail(int)));

    // 创建并添加状态标签组件 (彩色状态指示器)
    statusLabel = new TaskItemStatusLabel(this);
    ui->status_label->addWidget(statusLabel);

    showStatus();  // 初始化状态显示
}

/**
 * @brief 析构函数实现
 * @details
 * - 停止转码任务
 * - 释放状态标签资源
 * - 释放 UI 资源
 */
TaskItem::~TaskItem()
{
    StopTask();  // 确保转码线程已停止
    if(statusLabel != nullptr){
        delete statusLabel;
        statusLabel = nullptr;
    }
    delete ui;
}


/**
 * @brief 设置转码参数实现
 * @param _params 转码参数对象
 * @return 0表示成功
 * @details 保存参数并传递给转码线程
 */
int TaskItem::SetParams(const YouTranscoderParams & _params)
{
    params = _params;
    return taskThread.SetParams(params);
}

/**
 * @brief 设置输出目录实现
 * @param _output 输出文件夹路径
 * @return 0表示成功
 */
int TaskItem::SetOutputDir(const QString & _output)
{
    outputDir = _output;
    return 0;
}

/**
 * @brief 设置输出文件名前缀实现
 * @param _filenamePrefix 文件名前缀字符串
 * @return 0表示成功
 */
int TaskItem::SetFilenamePrefix(const QString & _filenamePrefix)
{
    filenamePrefix = _filenamePrefix;
    return 0;
}

/**
 * @brief 启动转码任务实现
 * @return 0表示成功,-1表示失败
 * @details
 * - 解析文件名模板,支持变量替换:
 *   - ${origin_file_name}: 原始文件名 (不含扩展名)
 *   - ${output_video_codec}: 输出视频编码器名称
 *   - ${output_audio_codec}: 输出音频编码器名称
 *   - ${video_pixelfmt}: 视频像素格式
 * - 检测输出文件是否已存在,避免覆盖
 * - 启动转码线程
 */
int TaskItem::StartTask()
{
    QFileInfo fInput(taskThread.GetInputPath());

    // 获取文件名前缀模板
    QString filename = params.GetFilenamePrefix();

    // 移除多余空白字符
    filename = filename.simplified();

    // 替换模板变量:原始文件名
    filename = filename.replace("${origin_file_name}", fInput.baseName());
    // 替换模板变量:输出视频编码器
    filename = filename.replace("${output_video_codec}", params.GetVideoCodecId().GetDescName().c_str());
    // 替换模板变量:输出音频编码器
    filename = filename.replace("${output_audio_codec}", params.GetAudioCodecId().GetDescName().c_str());

    // 处理像素格式变量:如果是 KEEP_SAME,需要读取原视频信息
    Eyer::EyerAVPixelFormat pixfmt = params.GetVideoPixelFormat();
    if(pixfmt == Eyer::EyerAVPixelFormat::EYER_KEEP_SAME){
        // 打开原视频文件获取像素格式
        Eyer::EyerAVReader mediaInfo(taskThread.GetInputPath().toStdString());
        int ret = mediaInfo.Open();
        if(ret){
            // 打开失败,设置错误状态
            taskThread.SetStatus(Eyer::EyerAVTranscoderStatus::FAIL);
            taskThread.SetErrorDesc("打开文件失败");
            showStatus();
            return -1;
        }
        // 获取视频流的像素格式
        int videoIndex = mediaInfo.GetVideoStreamIndex();
        if(videoIndex >= 0){
            Eyer::EyerAVStream videoStream = mediaInfo.GetStream(videoIndex);
            pixfmt = videoStream.GetPixelFormat();
        }
        mediaInfo.Close();
    }
    // 替换模板变量:像素格式
    filename = filename.replace("${video_pixelfmt}", pixfmt.GetDescName().c_str());

    // 文件名清理:移除不合法字符
    filename = filename.replace("/", "_");  // 替换路径分隔符
    filename = filename.replace("H.265", "265");  // 简化编码器名称
    filename = filename.replace("H.264", "264");

    // 组合完整输出路径:目录 + 文件名 + 扩展名
    QString output = outputDir + "/" + filename + "." + params.GetOutputFileFmt().GetSuffix().c_str();

    // 检查输出文件是否已存在
    QFileInfo fOutput(output);
    if(fOutput.exists()){
        // 文件已存在,设置错误状态
        taskThread.SetStatus(Eyer::EyerAVTranscoderStatus::FAIL);
        taskThread.SetErrorDesc("输出路径有重复文件");
        showStatus();
        return -1;
    }

    // 设置输出路径并启动转码
    taskThread.SetOutput(output);
    taskThread.SetStatus(Eyer::EyerAVTranscoderStatus::ING);
    taskThread.start();  // 启动转码线程
    return 0;
}

/**
 * @brief 停止转码任务实现
 * @return 0表示成功
 * @details 调用转码线程的停止方法
 */
int TaskItem::StopTask()
{
    taskThread.Stop();
    return 0;
}

/**
 * @brief 任务进度更新槽函数实现
 * @param progress 转码进度 (0.0-1.0)
 * @details
 * - 更新进度条显示 (转换为 0-100)
 * - 更新进度百分比标签 (非 Windows 平台)
 * - 刷新状态显示
 */
void TaskItem::OnTaskProgress(float progress)
{
    ui->task_progress_bar->setValue(progress * 100);
#ifndef WIN32
    // 非 Windows 平台显示进度百分比
    ui->task_progress_label->setText( QString::number((int)(progress * 100)) + "%");
#else
    ui->task_progress_label->setText("");
#endif
    showStatus();
}

/**
 * @brief 任务成功完成槽函数实现
 * @details
 * - 设置进度条为 100%
 * - 刷新状态显示
 * - 向上发出成功信号到主窗口
 */
void TaskItem::OnTaskSuccess()
{
    ui->task_progress_bar->setValue(100);
#ifndef WIN32
    ui->task_progress_label->setText("100%");
#else
    ui->task_progress_label->setText("");
#endif
    showStatus();
    emit TaskItem_OnTaskSuccess();  // 通知主窗口
}

/**
 * @brief 任务失败槽函数实现
 * @param code 错误码 (传递给主窗口)
 * @details
 * - 刷新状态显示 (会显示错误信息)
 * - 向上发出失败信号到主窗口
 */
void TaskItem::OnTaskFail(int code)
{
    showStatus();
    emit TaskItem_OnTaskFail(code);  // 通知主窗口
}

/**
 * @brief 刷新状态显示实现
 * @return 0表示成功
 * @details
 * - 更新状态指示器颜色 (准备/进行中/成功/失败)
 * - 如果任务失败,显示红色错误信息
 * - 如果任务正常,隐藏错误信息
 */
int TaskItem::showStatus()
{
    Eyer::EyerAVTranscoderStatus status = taskThread.GetStatus();
    // 根据字体大小动态调整状态标签尺寸
    int width = ui->task_input_path->fontInfo().pixelSize() * 2.0;
    statusLabel->setMinimumSize(width, width);
    statusLabel->SetStatus(status);  // 设置状态颜色

    if(status == Eyer::EyerAVTranscoderStatus::FAIL){
        // 失败状态:显示红色错误信息
        ui->task_error_label->setVisible(true);
        ui->task_error_label->setText(taskThread.GetErrorDesc());
        QPalette pe;
        pe.setColor(QPalette::WindowText, Qt::darkRed);
        ui->task_error_label->setPalette(pe);
    }
    else {
        // 正常状态:隐藏错误信息
        ui->task_error_label->setVisible(false);
    }
    return 0;
}

/**
 * @brief 获取任务状态实现
 * @return 转码状态枚举
 * @details 委托给转码线程获取状态
 */
Eyer::EyerAVTranscoderStatus TaskItem::GetStatus()
{
    return taskThread.GetStatus();
}

/**
 * @brief 设置任务状态实现
 * @param _status 转码状态枚举
 * @return 0表示成功
 * @details 委托给转码线程设置状态
 */
int TaskItem::SetStatus(const Eyer::EyerAVTranscoderStatus & _status)
{
    return taskThread.SetStatus(_status);
}

/**
 * @brief 移除按钮点击槽函数实现
 * @details 向上发出移除信号,请求主窗口移除此任务项
 */
void TaskItem::OnBtnRemove()
{
    emit TaskItem_OnRemove(this);  // 传递 this 指针给主窗口
}
