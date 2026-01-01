/**
 * @file YouTransMainWindow.cpp
 * @brief YouTranser 主窗口类实现
 * @details 实现主界面的所有交互逻辑,包括任务管理、参数配置、文件选择等核心功能
 */

#include "YouTransMainWindow.hpp"
#include "ui_YouTransMainWindow.h"

#include "YouTransAppConfig.hpp"
#include "YouTransConfig.hpp"
#include "YouTransAboutWindow.hpp"

#include <QFileDialog>
#include <QDebug>
#include <QSettings>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QToolBar>
#include <QMenuBar>

/**
 * @brief 构造函数实现
 * @param parent 父窗口指针 (默认为 nullptr)
 * @details
 * - 初始化 UI 界面 (setupUi)
 * - 创建菜单栏和工具栏
 * - 设置窗口标题、图标、按钮文本
 * - 连接信号槽
 * - 从 QSettings 加载转码参数配置
 */
YouTransMainWindow::YouTransMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YouTransMainWindow)
{
    ui->setupUi(this);

    // 创建菜单栏
    QMenuBar * mainMenuBar = menuBar();

    // 添加"帮助"菜单
    QMenu * menuHelp = mainMenuBar->addMenu("帮助");

    // 添加"关于"菜单项并连接槽函数
    QAction * actionAbout = menuHelp->addAction("关于");
    connect(actionAbout,      SIGNAL(triggered()),    this,   SLOT(AboutActionClickListener()));

    // 加载应用配置
    Eyer::YouTransAppConfig config;
    setWindowTitle(config.GetAppName());  // 设置窗口标题
    setWindowIcon(QIcon(config.GetLogoPath()));  // 设置窗口图标
    ui->btn_set_input_path->setText(config.Get_TRANS_MAIN_BUTTON_SET_INPUT_PATH());
    ui->btn_set_output_path->setText(config.Get_TRANS_MAIN_BUTTON_SET_OUTPUT_PATH());
    ui->btn_start_stop_transcode->setText(config.Get_TRANS_MAIN_BUTTON_START_TRANS_CODE());
    ui->btn_transcode_config->setText(config.Get_TRANS_MAIN_BUTTON_START_SETTING_CONFIG());

    // 设置参数标签文本 - 视频参数
    ui->params_key_videocodec->setText("视频编码：");
    ui->params_key_pixelfmt->setText("图像格式：");
    ui->params_key_videocrf->setText("图像质量(CRF)：");

    // 设置参数标签文本 - 音频参数
    ui->params_key_audiocodec->setText("音频编码：");
    ui->params_key_audiochannellayout->setText("声道布局：");
    ui->params_key_audiosamplerate->setText("采样率：");

    // 设置参数标签文本 - 性能参数
    ui->params_key_decodethreadnum->setText("解码线程数：");
    ui->params_key_encodethreadnum->setText("编码线程数：");
    ui->params_key_transsametime->setText("同时进行任务数：");

    UpdateSystemLabel();  // 更新系统状态标签

    // 连接按钮点击信号到槽函数
    connect(ui->btn_set_input_path,       SIGNAL(clicked()),    this,   SLOT(SetInputPathClickListener()));
    connect(ui->btn_set_output_path,      SIGNAL(clicked()),    this,   SLOT(SetOutputPathClickListener()));
    connect(ui->btn_transcode_config,     SIGNAL(clicked()),    this,   SLOT(StartTranscodeConfigClickListener()));
    connect(ui->btn_start_stop_transcode, SIGNAL(clicked()),    this,   SLOT(StartTranscodeClickListener()));

    // 从 QSettings 加载转码参数配置 (持久化存储)
    QSettings setting(config.GetCompanyName(), config.GetAppId());
    QString transParamsJson = setting.value("TRANS_PARAMS_JSON", "").toString();
    EyerLog("\n%s\n", transParamsJson.toStdString().c_str());

    // 解析并验证 JSON 参数
    params.ParseJson(transParamsJson);

    // 保存标准化后的参数 (填充默认值后的完整配置)
    transParamsJson = params.ToJson();
    setting.setValue("TRANS_PARAMS_JSON", transParamsJson);

    ShowTranscoderParams();  // 显示当前转码参数
}

/**
 * @brief 析构函数实现
 * @details 释放配置窗口和 UI 资源
 */
YouTransMainWindow::~YouTransMainWindow()
{
    if(configWindow != nullptr){
        delete configWindow;
        configWindow = nullptr;
    }

    delete ui;
}

/**
 * @brief 窗口关闭事件处理实现
 * @param event 关闭事件对象
 * @details
 * - 显示确认对话框,警告正在进行的转码将被取消
 * - 如果用户选择"是",停止所有任务并关闭窗口
 * - 如果用户选择"否",忽略关闭事件,窗口保持打开
 */
void YouTransMainWindow::closeEvent(QCloseEvent * event)
{
    EyerLog("YouTransMainWindow Close\n");
    int choose = QMessageBox::question(this, tr("退出？"), tr("如果退出，正在转码的视频将会被取消。"), QMessageBox::Yes | QMessageBox::No);

    if(choose == QMessageBox::No) {
        event->ignore();  // 忽略关闭事件,窗口不关闭
    }
    else if(choose == QMessageBox::Yes){
        // 停止所有转码任务
        for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
            QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
            TaskItem *taskitem = (TaskItem *) child->widget();
            taskitem->StopTask();
        }
        event->accept();  // 接受关闭事件,窗口关闭
    }
}

/**
 * @brief 设置输入文件路径按钮点击槽函数实现
 * @details
 * - 打开文件选择对话框,支持多文件选择
 * - 为每个选择的文件创建 TaskItem 任务项
 * - 连接任务项的信号到主窗口的槽函数
 * - 将任务项添加到任务列表布局中
 */
void YouTransMainWindow::SetInputPathClickListener()
{
    QStringList filelist = QFileDialog::getOpenFileNames(this, tr("选择输出文件"));
    for(QString file : filelist){
        // TODO: 检查重复文件

        // 创建任务项
        TaskItem * taskitem = new TaskItem(file, this);
        // 连接任务项信号
        connect(taskitem,       SIGNAL(TaskItem_OnTaskSuccess()),           this,   SLOT(TaskItem_OnTaskSuccess()));
        connect(taskitem,       SIGNAL(TaskItem_OnTaskFail(int)),           this,   SLOT(TaskItem_OnTaskFail(int)));
        connect(taskitem,       SIGNAL(TaskItem_OnRemove(TaskItem *)),      this,   SLOT(TaskItem_OnRemove(TaskItem *)));
        // 添加到任务列表
        ui->task_list_content_layout->addWidget(taskitem);
    }
}

/**
 * @brief 设置输出文件路径按钮点击槽函数实现
 * @details
 * - 打开文件夹选择对话框
 * - 保存选择的输出文件夹到参数配置
 * - 使用 QSettings 持久化保存配置
 * - 刷新界面显示的参数信息
 */
void YouTransMainWindow::SetOutputPathClickListener()
{
    QString outDir = QFileDialog::getExistingDirectory(this, tr("选择输出文件夹"), params.GetOutputDir());
    if(!outDir.isEmpty()){
        params.SetOutputDir(outDir);  // 更新输出目录
        QString transParamsJson = params.ToJson();

        // 保存到 QSettings
        Eyer::YouTransAppConfig config;
        QSettings setting(config.GetCompanyName(), config.GetAppId());
        setting.setValue("TRANS_PARAMS_JSON", transParamsJson);
        ShowTranscoderParams();  // 更新界面显示
    }
}

/**
 * @brief 打开转码配置窗口按钮点击槽函数实现
 * @details
 * - 删除旧的配置窗口实例 (如果存在)
 * - 创建新的配置窗口 YouTransConfig
 * - 连接配置窗口关闭信号
 * - 设置为模态窗口并显示
 */
void YouTransMainWindow::StartTranscodeConfigClickListener()
{
    if(configWindow != nullptr){
        delete configWindow;
        configWindow = nullptr;
    }
    configWindow = new YouTransConfig(params, this);
    connect(configWindow,                 SIGNAL(OnConfigWindowsClose()),    this,   SLOT(OnConfigWindowsClose()));
    configWindow->setAttribute(Qt::WA_ShowModal, true);  // 设置为模态窗口
    configWindow->show();
}

/**
 * @brief 配置窗口关闭槽函数实现
 * @details
 * - 从配置窗口获取修改后的转码参数
 * - 使用 QSettings 持久化保存新参数
 * - 刷新界面显示的参数信息
 */
void YouTransMainWindow::OnConfigWindowsClose()
{
    params = configWindow->GetTranscodeParams();  // 获取修改后的参数
    QString transParamsJson = params.ToJson();

    // 保存到 QSettings
    Eyer::YouTransAppConfig config;
    QSettings setting(config.GetCompanyName(), config.GetAppId());
    setting.setValue("TRANS_PARAMS_JSON", transParamsJson);
    ShowTranscoderParams();  // 更新界面显示
}

/**
 * @brief 显示转码参数配置实现
 * @details
 * - 将内部参数对象的值显示到 UI 标签中
 * - 包括视频编码、像素格式、CRF、音频编码、声道布局、采样率等
 * - 处理特殊值显示 (如"和原视频保持一致")
 */
void YouTransMainWindow::ShowTranscoderParams()
{
    // 显示视频参数
    ui->params_val_videocodec->setText(params.GetVideoCodecId().GetDescName().c_str());
    ui->params_val_pixelfmt->setText(params.GetVideoPixelFormat().GetDescName().c_str());
    ui->params_val_videocrf->setText(Eyer::EyerString::Number(params.GetCRF()).c_str());

    // 显示音频编码
    ui->params_val_audiocodec->setText(params.GetAudioCodecId().GetDescName().c_str());

    // 显示声道布局 (特殊值 EYER_KEEP_SAME 显示为"和原视频保持一致")
    if(params.GetAudioChannelLayout() == Eyer::EyerAVChannelLayout::EYER_KEEP_SAME){
        ui->params_val_audiochannellayout->setText("和原视频保持一致");
    }
    else {
        ui->params_val_audiochannellayout->setText(QString(params.GetAudioChannelLayout().GetDescName().c_str()) + " (声道：" + QString::number(Eyer::EyerAVChannelLayout::GetChannelLayoutNBChannels(params.GetAudioChannelLayout())) +  ")");
    }

    // 显示采样率 (特殊值 -2 显示为"和原视频保持一致")
    if(params.GetSampleRate() == -2){
        ui->params_val_audiosamplerate->setText("和原视频保持一致");
    }
    else {
        ui->params_val_audiosamplerate->setText(Eyer::EyerString::Number(params.GetSampleRate()).c_str());
    }

    // 显示性能参数
    ui->params_val_decodethreadnum->setText(Eyer::EyerString::Number(params.GetDecodeThreadNum()).c_str());
    ui->params_val_encodethreadnum->setText(Eyer::EyerString::Number(params.GetEncodeThreadNum()).c_str());
    ui->params_val_transsametime->setText(Eyer::EyerString::Number(params.GetTransNumSametime()).c_str());

    // 显示输出路径和文件名前缀
    ui->params_val_output_path->setText(params.GetOutputDir() + "/" + params.GetFilenamePrefix());

    // 显示输出文件格式
    ui->params_val_output_filefmt_label->setText(QString("输出文件格式：") + params.GetOutputFileFmt().GetDesc().c_str());
}

/**
 * @brief 开始转码按钮点击槽函数实现
 * @details
 * - 将所有失败的任务重置为准备状态
 * - 调用内部实现函数启动转码
 */
void YouTransMainWindow::StartTranscodeClickListener()
{
    // 重置失败任务状态为准备状态
    for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
        QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
        TaskItem *taskitem = (TaskItem *) child->widget();
        if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::FAIL) {
            taskitem->SetStatus(Eyer::EyerAVTranscoderStatus::PREPARE);
            taskitem->showStatus();
        }
    }
    StartTranscodeClickListenerInternal();  // 启动转码
}

/**
 * @brief 开始转码内部实现函数Ï
 * @details
 * - 验证输出路径和任务列表
 * - 实现并发任务控制:根据配置的同时任务数启动转码
 * - 统计正在进行、准备、失败的任务数量
 * - 自动启动准备状态的任务,直到达到并发数上限
 */
void YouTransMainWindow::StartTranscodeClickListenerInternal()
{
    // 验证输出路径
    if(params.GetOutputDir().isEmpty()){
        QMessageBox::critical(this, tr("危险弹窗"), tr("请先设置输出路径"));
        return;
    }
    // 验证任务列表
    if(ui->task_list_content_layout->count() <= 0){
        QMessageBox::critical(this, tr("危险弹窗"), tr("无任务"));
        return;
    }

    // 任务调度循环:根据并发数限制启动任务
    while(1) {
        // 统计正在进行的任务数
        int ingCount = 0;
        for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
            QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
            TaskItem *taskitem = (TaskItem *) child->widget();
            if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::ING) {
                ingCount++;
            }
        }

        // 统计准备中的任务数
        int prepareCount = 0;
        for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
            QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
            TaskItem *taskitem = (TaskItem *) child->widget();
            if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::PREPARE) {
                prepareCount++;
            }
        }

        // 统计失败的任务数
        int failCount = 0;
        for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
            QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
            TaskItem *taskitem = (TaskItem *) child->widget();
            if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::FAIL) {
                failCount++;
            }
        }

        EyerLog("ingCount: %d, prepareCount: %d, failCount: %d\n", ingCount, prepareCount, failCount);

        // 没有准备中的任务,退出循环
        if(prepareCount <= 0){
            break;
        }

        // 已达到并发数上限,退出循环
        if(ingCount >= params.GetTransNumSametime()){
            break;
        }

        // 启动一个准备中的任务
        for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
            QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
            TaskItem *taskitem = (TaskItem *) child->widget();
            if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::PREPARE) {
                taskitem->SetParams(params);
                taskitem->SetOutputDir(params.GetOutputDir());
                taskitem->SetFilenamePrefix(params.GetFilenamePrefix());
                taskitem->StartTask();  // 启动任务
                break;  // 只启动一个任务,等待下一次调用
            }
        }
    }

    UpdateSystemLabel();  // 更新系统状态标签
}

/**
 * @brief 任务成功完成槽函数实现
 * @details
 * - 任务成功后,尝试启动下一个等待的任务
 * - 实现任务队列的自动调度
 */
void YouTransMainWindow::TaskItem_OnTaskSuccess()
{
    StartTranscodeClickListenerInternal();  // 启动下一个任务
}

/**
 * @brief 任务失败槽函数实现
 * @param code 错误码 (当前未使用)
 * @details
 * - 任务失败后,尝试启动下一个等待的任务
 * - 实现任务队列的自动调度
 */
void YouTransMainWindow::TaskItem_OnTaskFail(int code)
{
    StartTranscodeClickListenerInternal();  // 启动下一个任务
}

/**
 * @brief 任务移除槽函数实现
 * @param taskitem 要移除的任务项指针
 * @details
 * - 停止任务的转码线程
 * - 从布局中移除 Widget
 * - 解除父子关系 (Qt 将自动释放内存)
 */
void YouTransMainWindow::TaskItem_OnRemove(TaskItem * taskitem)
{
    taskitem->StopTask();  // 停止转码线程
    ui->task_list_content_layout->removeWidget(taskitem);  // 从布局移除
    taskitem->setParent(nullptr);  // 解除父子关系,Qt 将释放内存
}

/**
 * @brief 更新系统状态标签实现
 * @return 0表示成功
 * @details
 * - 统计任务总数、成功数、失败数
 * - 更新状态栏显示的统计信息
 */
int YouTransMainWindow::UpdateSystemLabel()
{
    int taskOunt = ui->task_list_content_layout->count();  // 任务总数
    // 统计正在进行的任务数
    int ingCount = 0;
    for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
        QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
        TaskItem *taskitem = (TaskItem *) child->widget();
        if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::ING) {
            ingCount++;
        }
    }
    // 统计失败的任务数
    int failCount = 0;
    for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
        QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
        TaskItem *taskitem = (TaskItem *) child->widget();
        if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::FAIL) {
            failCount++;
        }
    }
    // 统计成功的任务数
    int succCount = 0;
    for (int i = 0; i < ui->task_list_content_layout->count(); i++) {
        QLayoutItem *child = ui->task_list_content_layout->itemAt(i);
        TaskItem *taskitem = (TaskItem *) child->widget();
        if (taskitem->GetStatus() == Eyer::EyerAVTranscoderStatus::SUCC) {
            succCount++;
        }
    }
    // 更新状态标签文本
    ui->system_status_label->setText("任务总数：" + QString::number(taskOunt) +
                                     " 成功：" + QString::number(succCount) +
                                     " 失败：" + QString::number(failCount));
    return 0;
}

/**
 * @brief 关于菜单项点击槽函数实现
 * @details
 * - 创建并显示关于对话框
 * - 显示应用版本、作者、许可证等信息
 */
void YouTransMainWindow::AboutActionClickListener()
{
    YouTransAboutWindow * about = new YouTransAboutWindow(this);
    about->show();
}
