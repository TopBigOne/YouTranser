/**
 * @file YouTransConfig.cpp
 * @brief 转码参数配置窗口实现
 * @details 实现转码参数配置窗口的所有交互逻辑，包括下拉框联动、参数验证、数据同步等
 */

#include "YouTransConfig.hpp"
#include "ui_YouTransConfig.h"

#include <QDebug>

#include "YouTransAppConfig.hpp"

#include "EyerAVTranscoder/EyerAVTranscoderHeader.hpp"

/**
 * @brief 构造函数实现
 * @param inputParams 输入的转码参数 (用于显示当前配置)
 * @param parent 父窗口指针
 * @details
 * 初始化流程：
 * 1. 复制输入参数到内部 params 成员变量
 * 2. 初始化 UI 界面 (setupUi)
 * 3. 设置窗口标题和图标
 * 4. 设置所有 UI 标签的文本 (从 YouTransAppConfig 获取国际化文本)
 * 5. 设置线程数和并发任务数的范围和初始值
 * 6. 连接按钮点击信号槽 (确定/取消按钮)
 * 7. 清空容器格式下拉框
 * 8. 连接下拉框选择变化信号槽 (容器格式、视频编码器、音频编码器、声道布局)
 * 9. 查询系统支持的容器格式列表 (通过 EyerAVTranscoderSupport)
 * 10. 填充容器格式下拉框
 * 11. 设置 CRF 范围 (0-51) 和初始值
 * 12. 连接 CRF 数值变化信号槽
 * 13. 调用 SetCurrentData() 根据 inputParams 设置当前选中项
 */
YouTransConfig::YouTransConfig(const YouTranscoderParams & inputParams, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YouTransConfig)
{
    params = inputParams;  // 复制输入参数

    ui->setupUi(this);  // 初始化 UI

    // 设置窗口标题和图标
    Eyer::YouTransAppConfig appConfig;
    setWindowTitle(appConfig.Get_TRANS_CONFIG_WINDOWS_TITLE());
    setWindowIcon(QIcon(appConfig.GetLogoPath()));

    // 设置所有 UI 标签的文本 (从 YouTransAppConfig 获取国际化文本)
    ui->config_label_set_avfilefmt->setText(appConfig.Get_TRANS_CONFIG_AVFILEFMT());
    ui->config_label_set_video->setText(appConfig.Get_TRANS_CONFIG_SET_VIDEO());
    ui->config_label_set_audio->setText(appConfig.Get_TRANS_CONFIG_SET_AUDIO());
    ui->config_label_set_other->setText("其他：");

    ui->config_btn_okay->setText(appConfig.Get_TRANS_CONFIG_BUTTON_OK());
    ui->config_btn_cancel->setText(appConfig.Get_TRANS_CONFIG_BUTTON_CANCEL());

    ui->config_videopixfmt_combo_box_label->setText(appConfig.Get_TRANS_CONFIG_PIXELFMT_LABEL());
    ui->config_videocodec_coombo_box_label->setText(appConfig.Get_TRANS_CONFIG_VIDEOCODEC_LABEL());
    ui->config_vcrf_spin_box_label->setText(appConfig.Get_TRANS_CONFIG_CRF_LABEL());

    ui->config_audiocodec_combo_box_label->setText(appConfig.Get_TRANS_CONFIG_AUDIOCODEC_LABEL());
    ui->config_channellayout_combo_box_label->setText(appConfig.Get_TRANS_CONFIG_CHANNEL_LAYOUT_LABEL());
    ui->config_samplerate_combo_box_label->setText(appConfig.Get_TRANS_CONFIG_SAMPLE_RATE_LABEL());

    ui->config_label_set_filename->setText(appConfig.Get_TRANS_CONFIG_SET_FILENAME_LABEL());
    ui->config_filename_label->setText(appConfig.Get_TRAND_CONFIG_FILENAME_LABEL());

    // 设置解码线程数范围和初始值
    ui->config_decodethread_spin_box_label->setText("解码线程数：");
    ui->config_decodethread_spin_box->setValue(params.GetDecodeThreadNum());
    ui->config_decodethread_spin_box->setMaximum(10);
    ui->config_decodethread_spin_box->setMinimum(1);

    // 设置编码线程数范围和初始值
    ui->config_encodethread_spin_box_label->setText("编码线程数：");
    ui->config_encodethread_spin_box->setValue(params.GetEncodeThreadNum());
    ui->config_encodethread_spin_box->setMaximum(10);
    ui->config_encodethread_spin_box->setMinimum(1);

    // 设置并发任务数范围和初始值
    ui->config_transnum_sametime_spin_box_label->setText("同时进行的任务数：");
    ui->config_transnum_sametime_spin_box->setValue(params.GetTransNumSametime());
    ui->config_transnum_sametime_spin_box->setMaximum(10);
    ui->config_transnum_sametime_spin_box->setMinimum(1);

    // 连接按钮点击信号槽
    connect(ui->config_btn_okay,        SIGNAL(clicked()),    this,   SLOT(OkayClickListener()));
    connect(ui->config_btn_cancel,      SIGNAL(clicked()),    this,   SLOT(CancelClickListener()));

    ui->config_avfilefmt_combo_box->clear();  // 清空容器格式下拉框

    // 连接下拉框选择变化信号槽 (使用 DirectConnection 确保立即执行)
    connect(ui->config_avfilefmt_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(onAVFileFmtCurrentIndexChanged(int)), Qt::DirectConnection);
    connect(ui->config_videocodec_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(onVideoCodecCurrentIndexChanged(int)), Qt::DirectConnection);
    connect(ui->config_audiocodec_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(onAudioCodecCurrentIndexChanged(int)), Qt::DirectConnection);
    connect(ui->config_channellayout_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(onChannellayoutCurrentIndexChanged(int)), Qt::DirectConnection);

    // 查询系统支持的容器格式列表
    Eyer::EyerAVTranscoderSupport transcoderSupport;
    std::vector<Eyer::EyerAVFileFmt> avfileFmts = transcoderSupport.QuerySupportFmt();

    // 填充容器格式下拉框
    for(int i=0; i<avfileFmts.size(); i++){
        Eyer::EyerAVFileFmt fileFmt = avfileFmts[i];
        ui->config_avfilefmt_combo_box->addItem(QString(fileFmt.GetDesc().c_str()), fileFmt.GetId());
    }

    // Set Current Data
    ui->config_avfilefmt_combo_box->setCurrentIndex(params.GetOutputFileFmt().GetId());

    // 设置 CRF (Constant Rate Factor) 范围和初始值
    ui->config_crf_spin_box->setMaximum(51);  // CRF 最大值 51 (质量最差)
    ui->config_crf_spin_box->setMinimum(0);   // CRF 最小值 0 (视觉无损)
    ui->config_crf_spin_box->setValue(params.GetCRF());
    connect(ui->config_crf_spin_box, SIGNAL(valueChanged(int)), this, SLOT(onCRFValueChanged(int)));

    SetCurrentData();  // 根据 inputParams 设置当前选中项
}

/**
 * @brief 析构函数实现
 * @details 释放 UI 资源
 */
YouTransConfig::~YouTransConfig()
{
    delete ui;
}

/**
 * @brief 设置当前显示数据实现 (将 params 对象的值同步到 UI 控件)
 * @details
 * 根据 params 成员变量的值，更新各个下拉框和输入框的选中项：
 * 1. 根据容器格式 ID 查找并设置容器格式下拉框的选中项
 * 2. 根据视频编码器 ID 查找并设置视频编码器下拉框的选中项
 * 3. 根据像素格式 ID 查找并设置像素格式下拉框的选中项
 * 4. 设置 CRF 数值框的值
 * 5. 根据音频编码器 ID 查找并设置音频编码器下拉框的选中项
 * 6. 根据声道布局 ID 查找并设置声道布局下拉框的选中项
 * 7. 根据采样率查找并设置采样率下拉框的选中项
 * 8. 设置文件名前缀输入框的文本
 *
 * 使用 findData() 查找下拉框中对应的 item，如果找到 (index != -1) 则设置为选中状态
 */
void YouTransConfig::SetCurrentData()
{
    // 设置容器格式下拉框的选中项
    {
        int index = ui->config_avfilefmt_combo_box->findData(params.GetOutputFileFmt().GetId());
        if ( index != -1 ) {
           ui->config_avfilefmt_combo_box->setCurrentIndex(index);
        }
    }
    // 设置视频编码器下拉框的选中项
    {
        int index = ui->config_videocodec_combo_box->findData(params.GetVideoCodecId().GetId());
        if ( index != -1 ) {
           ui->config_videocodec_combo_box->setCurrentIndex(index);
        }
    }
    // 设置像素格式下拉框的选中项
    {
        int index = ui->config_videopixfmt_combo_box->findData(params.GetVideoPixelFormat().GetId());
        if ( index != -1 ) {
           ui->config_videopixfmt_combo_box->setCurrentIndex(index);
        }
    }
    // 设置 CRF 数值框的值
    {
        ui->config_crf_spin_box->setValue(params.GetCRF());
    }
    // 设置音频编码器下拉框的选中项
    {
        int index = ui->config_audiocodec_combo_box->findData(params.GetAudioCodecId().GetId());
        if ( index != -1 ) {
           ui->config_audiocodec_combo_box->setCurrentIndex(index);
        }
    }
    // 设置声道布局下拉框的选中项
    {
        int index = ui->config_channellayout_combo_box->findData(params.GetAudioChannelLayout().GetId());
        if ( index != -1 ) {
           ui->config_channellayout_combo_box->setCurrentIndex(index);
        }
    }
    // 设置采样率下拉框的选中项
    {
        int index = ui->config_samplerate_combo_box->findData(params.GetSampleRate());
        if ( index != -1 ) {
           ui->config_samplerate_combo_box->setCurrentIndex(index);
        }
    }
    // 设置文件名前缀输入框的文本
    {
        ui->config_filename_edittext->setText(params.GetFilenamePrefix());
    }
}

/**
 * @brief 通用下拉框选择变化槽函数实现 (当前未使用)
 * @param index 选中项索引
 */
void YouTransConfig::onCurrentIndexChanged(int index)
{

}

/**
 * @brief 容器格式下拉框选择变化槽函数实现
 * @param index 选中项索引
 * @details
 * 当用户选择新的容器格式时触发，执行以下操作：
 * 1. 获取当前选中的容器格式 ID
 * 2. 根据 ID 获取对应的 Eyer::EyerAVFileFmt 对象
 * 3. 查询该容器格式支持的视频编码器列表 (通过 EyerAVTranscoderSupport)
 * 4. 清空并重新填充视频编码器下拉框
 * 5. 自动触发视频编码器的选择变化回调 (更新像素格式列表)
 * 6. 查询该容器格式支持的音频编码器列表
 * 7. 清空并重新填充音频编码器下拉框
 * 8. 自动触发音频编码器的选择变化回调 (更新声道布局/采样率列表)
 *
 * 这实现了容器格式与编码器之间的联动选择逻辑
 */
void YouTransConfig::onAVFileFmtCurrentIndexChanged(int index)
{
    // 获取当前选中的容器格式 ID
    int filefmtId = ui->config_avfilefmt_combo_box->currentData().toInt();
    Eyer::EyerAVFileFmt fmt = Eyer::EyerAVFileFmt::GetAVFileFmtById(filefmtId);

    // 查询该容器格式支持的视频编码器列表
    Eyer::EyerAVTranscoderSupport transcoderSupport;
    std::vector<Eyer::EyerAVCodecID> videoCodecs = transcoderSupport.QuerySupportVideoCodec(fmt);

    // 清空并重新填充视频编码器下拉框
    ui->config_videocodec_combo_box->clear();
    for(int i=0;i<videoCodecs.size();i++){
        Eyer::EyerAVCodecID codec = videoCodecs[i];
        ui->config_videocodec_combo_box->addItem(codec.GetDescName().c_str(), codec.GetId());
    }

    // 自动触发视频编码器的选择变化回调 (更新像素格式列表)
    {
        onVideoCodecCurrentIndexChanged(0);
    }

    // 查询该容器格式支持的音频编码器列表
    std::vector<Eyer::EyerAVCodecID> audioCodecs = transcoderSupport.QuerySupportAudioCodec(fmt);
    ui->config_audiocodec_combo_box->clear();
    for(int i=0;i<audioCodecs.size();i++){
        Eyer::EyerAVCodecID codec = audioCodecs[i];
        ui->config_audiocodec_combo_box->addItem(codec.GetDescName().c_str(), codec.GetId());
    }

    // 自动触发音频编码器的选择变化回调 (更新声道布局/采样率列表)
    {
        onAudioCodecCurrentIndexChanged(0);
    }
}

/**
 * @brief 视频编码器下拉框选择变化槽函数实现
 * @param index 选中项索引
 * @details
 * 当用户选择新的视频编码器时触发，执行以下操作：
 * 1. 打印调试信息 (索引号)
 * 2. 获取当前选中的视频编码器 ID
 * 3. 根据 ID 获取对应的 Eyer::EyerAVCodecID 对象
 * 4. 查询该编码器支持的像素格式列表 (yuv420p, yuv422p, yuv444p 等)
 * 5. 清空并重新填充像素格式下拉框
 *
 * 这实现了视频编码器与像素格式之间的联动选择逻辑
 */
void YouTransConfig::onVideoCodecCurrentIndexChanged(int index)
{
    qDebug() << index << Qt::endl;  // 打印调试信息

    // 获取当前选中的视频编码器 ID
    int codecId = ui->config_videocodec_combo_box->currentData().toInt();
    Eyer::EyerAVCodecID videoCodec = Eyer::EyerAVCodecID::GetCodecIdById(codecId);

    // 查询该编码器支持的像素格式列表
    Eyer::EyerAVTranscoderSupport transcoderSupport;
    std::vector<Eyer::EyerAVPixelFormat> pixelFmts = transcoderSupport.QuerySupportPixelFormat(videoCodec);

    // 清空并重新填充像素格式下拉框
    ui->config_videopixfmt_combo_box->clear();
    for(int i=0;i<pixelFmts.size();i++){
        Eyer::EyerAVPixelFormat pixelFmt = pixelFmts[i];
        ui->config_videopixfmt_combo_box->addItem(pixelFmt.GetDescName().c_str(), pixelFmt.GetId());
    }
}

/**
 * @brief 音频编码器下拉框选择变化槽函数实现
 * @param index 选中项索引
 * @details
 * 当用户选择新的音频编码器时触发，执行以下操作：
 * 1. 获取当前选中的音频编码器 ID
 * 2. 根据 ID 获取对应的 Eyer::EyerAVCodecID 对象
 * 3. 查询该编码器支持的声道布局列表 (单声道, 立体声, 5.1 环绕声 等)
 * 4. 清空并重新填充声道布局下拉框
 *    - 如果声道布局为 EYER_KEEP_SAME，显示"和原视频保持一致"
 *    - 否则显示"(声道:N) 布局名称" (如 "(声道:2) 立体声")
 * 5. 查询该编码器支持的采样率列表 (44100 Hz, 48000 Hz 等)
 * 6. 清空并重新填充采样率下拉框
 *    - 如果采样率为 -2，显示"和原视频保持一致"
 *    - 否则显示"N Hz" (如 "48000 Hz")
 *
 * 这实现了音频编码器与声道布局/采样率之间的联动选择逻辑
 */
void YouTransConfig::onAudioCodecCurrentIndexChanged(int index)
{
    // 获取当前选中的音频编码器 ID
    int codecId = ui->config_audiocodec_combo_box->currentData().toInt();
    Eyer::EyerAVCodecID audioCodec = Eyer::EyerAVCodecID::GetCodecIdById(codecId);

    // 查询该编码器支持的声道布局列表
    Eyer::EyerAVTranscoderSupport transcoderSupport;
    std::vector<Eyer::EyerAVChannelLayout> channelLayouts = transcoderSupport.QuerySupportChannelLayout(audioCodec);

    // 清空并重新填充声道布局下拉框
    ui->config_channellayout_combo_box->clear();
    for(int i=0;i<channelLayouts.size();i++){
        Eyer::EyerAVChannelLayout channelLayout = channelLayouts[i];
        int channels = Eyer::EyerAVChannelLayout::GetChannelLayoutNBChannels(channelLayout);
        if(channelLayout == Eyer::EyerAVChannelLayout::EYER_KEEP_SAME){
            ui->config_channellayout_combo_box->addItem("和原视频保持一致", channelLayout.GetId());
        }
        else{
            ui->config_channellayout_combo_box->addItem(QString("(声道:") + QString::number(channels) + "）  " + QString(channelLayout.GetDescName().c_str()), channelLayout.GetId());
        }
    }

    // 查询该编码器支持的采样率列表
    ui->config_samplerate_combo_box->clear();
    std::vector<int> sampleRates = transcoderSupport.QuerySupportSampleRate(audioCodec);
    for(int i=0;i<sampleRates.size();i++){
        int sampleRate = sampleRates[i];
        if(sampleRate == -2){  // -2 表示保持原视频的采样率
            ui->config_samplerate_combo_box->addItem("和原视频保持一致", sampleRate);
        }
        else {
            ui->config_samplerate_combo_box->addItem(QString("") + QString::number(sampleRate) + " Hz", sampleRate);
        }
    }
}

/**
 * @brief 声道布局下拉框选择变化槽函数实现 (当前未使用)
 * @param index 选中项索引
 */
void YouTransConfig::onChannellayoutCurrentIndexChanged(int index)
{

}

/**
 * @brief CRF 质量值变化槽函数实现
 * @param value 新的 CRF 值 (0-51)
 * @details
 * 当用户调整 CRF 值时触发，根据 CRF 值显示质量提示：
 * - 0-17: "very good" (视觉无损，文件体积大)
 * - 18-22: "good" (高质量，推荐值)
 * - 23-27: "so so" (中等质量)
 * - 28-51: "bad" (低质量，文件体积小)
 *
 * 更新 CRF 标签显示质量提示文本，帮助用户了解当前质量设置的效果
 */
void YouTransConfig::onCRFValueChanged(int value)
{
    QString alert = "";
    if(value >= 0 && value < 18){
        alert = "very good";  // 视觉无损
    }
    else if(value >= 18 && value < 23){
        alert = "good";       // 高质量
    }
    else if(value >= 23 && value < 28){
        alert = "so so";      // 中等质量
    }
    else{
        alert = "bad";        // 低质量
    }

    // 更新 CRF 标签显示质量提示
    Eyer::YouTransAppConfig appConfig;
    ui->config_vcrf_spin_box_label->setText(appConfig.Get_TRANS_CONFIG_CRF_LABEL() + alert);
}

/**
 * @brief 窗口关闭事件回调函数实现 (重写 QMainWindow::closeEvent)
 * @param event 关闭事件对象
 * @details 当前实现仅打印调试信息，未做特殊处理
 */
void YouTransConfig::closeEvent(QCloseEvent *event)
{
    qDebug() << "YouTransConfig closeEvent" << Qt::endl;
}

/**
 * @brief "确定"按钮点击槽函数实现
 * @details
 * 用户点击"确定"按钮后，执行以下操作：
 * 1. 从各个下拉框和输入框读取用户配置的参数
 * 2. 更新内部 params 对象的各个属性：
 *    a. 容器格式 (AVFileFmt) - 从容器格式下拉框读取
 *    b. 视频编码器 (VideoCodecId) - 从视频编码器下拉框读取
 *    c. 像素格式 (VideoPixelFormat) - 从像素格式下拉框读取
 *    d. CRF 质量值 - 从 CRF 数值框读取
 *    e. 音频编码器 (AudioCodecId) - 从音频编码器下拉框读取
 *    f. 声道布局 (ChannelLayout) - 从声道布局下拉框读取
 *    g. 采样率 (SampleRate) - 从采样率下拉框读取
 *    h. 文件名前缀 (FilenamePrefix) - 从文件名输入框读取并去除首尾空格
 *    i. 解码线程数 (DecodeThreadNum) - 从解码线程数数值框读取
 *    j. 编码线程数 (EncodeThreadNum) - 从编码线程数数值框读取
 *    k. 并发任务数 (TransNumSametime) - 从并发任务数数值框读取
 * 3. 打印调试信息 (输出用户选择的各项配置)
 * 4. 发射 OnConfigWindowsClose 信号通知主窗口参数已更新
 * 5. 关闭配置窗口
 */
void YouTransConfig::OkayClickListener()
{
    // 读取容器格式
    // AVFile fmt
    int fmtId = ui->config_avfilefmt_combo_box->currentData().toInt();
    Eyer::EyerAVFileFmt avfileFmt = Eyer::EyerAVFileFmt::GetAVFileFmtById(fmtId);
    params.SetOutputFileFmt(avfileFmt);
    qDebug() << avfileFmt.GetDesc().c_str() << Qt::endl;

    // 读取视频编码器
    // Video Codec
    int videoCodecId = ui->config_videocodec_combo_box->currentData().toInt();
    Eyer::EyerAVCodecID videoCodec = Eyer::EyerAVCodecID::GetCodecIdById(videoCodecId);
    params.SetVideoCodecId(videoCodec);
    qDebug() << videoCodec.GetDescName().c_str() << Qt::endl;

    // 读取像素格式
    // Video Pixel Fmt
    int pixelFmtId = ui->config_videopixfmt_combo_box->currentData().toInt();
    Eyer::EyerAVPixelFormat pixelFmt = Eyer::EyerAVPixelFormat::GetById(pixelFmtId);
    params.SetVideoPixelFormat(pixelFmt);
    qDebug() << pixelFmt.GetDescName().c_str() << Qt::endl;

    // 读取 CRF 质量值
    // CRF
    int crf = ui->config_crf_spin_box->value();
    params.SetCRF(crf);

    // 读取音频编码器
    // Audio Codec
    int audioCodecId = ui->config_audiocodec_combo_box->currentData().toInt();
    Eyer::EyerAVCodecID audioCodec = Eyer::EyerAVCodecID::GetCodecIdById(audioCodecId);
    params.SetAudioCodecId(audioCodec);
    qDebug() << audioCodec.GetDescName().c_str() << Qt::endl;

    // 读取声道布局
    // Channel Layout
    int channellayoutId = ui->config_channellayout_combo_box->currentData().toInt();
    Eyer::EyerAVChannelLayout audioChannelLayout = Eyer::EyerAVChannelLayout::GetById(channellayoutId);
    params.SetChannelLayout(audioChannelLayout);
    qDebug() << audioChannelLayout.GetDescName().c_str() << Qt::endl;

    // 读取采样率
    // Sample Rate
    int sampleRate = ui->config_samplerate_combo_box->currentData().toInt();
    params.SetSampleRate(sampleRate);

    // 读取文件名前缀 (去除首尾空格)
    // Filename Prefix
    QString filenamePrefix = ui->config_filename_edittext->toPlainText();
    filenamePrefix = filenamePrefix.simplified();  // 去除首尾空格和多余空格
    params.SetFilenamePrefix(filenamePrefix);

    // 读取解码线程数
    int decodethreadNum = ui->config_decodethread_spin_box->value();
    params.SetDecodeThreadNum(decodethreadNum);

    // 读取编码线程数
    int encodethreadNum = ui->config_encodethread_spin_box->value();
    params.SetEncodeThreadNum(encodethreadNum);

    // 读取并发任务数
    int transnumSametime = ui->config_transnum_sametime_spin_box->value();
    params.SetTransNumSametime(transnumSametime);

    // 发射信号通知主窗口参数已更新
    emit OnConfigWindowsClose();
    // 关闭配置窗口
    close();
}

/**
 * @brief "取消"按钮点击槽函数实现
 * @details 关闭配置窗口，不保存任何更改 (params 对象保持原值)
 */
void YouTransConfig::CancelClickListener()
{
    close();  // 直接关闭窗口
}

/**
 * @brief 获取转码参数对象实现
 * @return 当前配置的转码参数对象
 * @details 由主窗口调用，用于获取用户配置完成的转码参数
 */
YouTranscoderParams YouTransConfig::GetTranscodeParams()
{
    return params;
}
