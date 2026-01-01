/**
 * @file YouTransConfig.hpp
 * @brief 转码参数配置窗口头文件
 * @details 提供图形界面让用户配置转码参数，包括容器格式、视频编码器、音频编码器、
 *          像素格式、声道布局、采样率、CRF质量值、线程数、并发任务数等
 */

#ifndef YOUTRANSCONFIG_HPP
#define YOUTRANSCONFIG_HPP

#include <QMainWindow>

#include "YouTranscoderParams.hpp"

#include "EyerAVTranscoder/EyerAVTranscoderHeader.hpp"

namespace Ui {
class YouTransConfig;
}

/**
 * @class YouTransConfig
 * @brief 转码参数配置窗口类
 * @details
 * 这是一个模态对话框窗口，用于配置转码参数，主要功能包括：
 *
 * **容器格式选择** (AVFileFmt):
 * - 支持 MP4, MOV, MKV, AVI 等常见容器格式
 * - 选择容器格式后会自动更新对应的视频/音频编码器列表
 *
 * **视频参数配置**:
 * - 视频编码器选择 (H.264, H.265, ProRes, VP9, AV1 等)
 * - 像素格式选择 (yuv420p, yuv422p, yuv444p 等)
 * - CRF 质量值设置 (0-51，越小质量越高)
 *
 * **音频参数配置**:
 * - 音频编码器选择 (AAC, MP3, FLAC, Opus 等)
 * - 声道布局选择 (单声道, 立体声, 5.1 环绕声 等)
 * - 采样率选择 (44100 Hz, 48000 Hz 等)
 *
 * **性能参数配置**:
 * - 解码线程数 (1-10)
 * - 编码线程数 (1-10)
 * - 同时进行的任务数 (1-10)
 *
 * **文件命名配置**:
 * - 输出文件名前缀设置
 *
 * 配置完成后点击"确定"按钮，发射 OnConfigWindowsClose 信号通知主窗口
 */
class YouTransConfig : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param inputParams 输入的转码参数 (用于显示当前配置)
     * @param parent 父窗口指针
     * @details
     * 初始化流程：
     * 1. 设置窗口标题和图标
     * 2. 初始化所有 UI 标签文本 (从 YouTransAppConfig 获取)
     * 3. 查询系统支持的容器格式列表 (通过 EyerAVTranscoderSupport)
     * 4. 填充容器格式下拉框
     * 5. 设置线程数和并发任务数的范围 (1-10)
     * 6. 设置 CRF 范围 (0-51)
     * 7. 连接所有信号槽 (下拉框选择变化、按钮点击等)
     * 8. 根据 inputParams 设置当前选中项 (SetCurrentData)
     */
    explicit YouTransConfig(const YouTranscoderParams & inputParams, QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     * @details 释放 UI 资源
     */
    ~YouTransConfig();

protected:
    /**
     * @brief 窗口关闭事件回调函数 (重写 QMainWindow::closeEvent)
     * @param event 关闭事件对象
     * @details 当前实现仅打印调试信息，未做特殊处理
     */
     void closeEvent(QCloseEvent *event);

public slots:
    /**
     * @brief 通用下拉框选择变化槽函数 (当前未使用)
     * @param index 选中项索引
     */
    void onCurrentIndexChanged(int index);

    /**
     * @brief 容器格式下拉框选择变化槽函数
     * @param index 选中项索引
     * @details
     * 当用户选择新的容器格式时触发，执行以下操作：
     * 1. 获取选中的容器格式 ID
     * 2. 查询该容器格式支持的视频编码器列表
     * 3. 更新视频编码器下拉框内容
     * 4. 查询该容器格式支持的音频编码器列表
     * 5. 更新音频编码器下拉框内容
     * 6. 自动触发视频/音频编码器的选择变化回调 (更新像素格式/声道布局)
     */
    void onAVFileFmtCurrentIndexChanged(int index);

    /**
     * @brief 视频编码器下拉框选择变化槽函数
     * @param index 选中项索引
     * @details
     * 当用户选择新的视频编码器时触发，执行以下操作：
     * 1. 获取选中的视频编码器 ID
     * 2. 查询该编码器支持的像素格式列表 (yuv420p, yuv422p 等)
     * 3. 更新像素格式下拉框内容
     */
    void onVideoCodecCurrentIndexChanged(int index);

    /**
     * @brief 音频编码器下拉框选择变化槽函数
     * @param index 选中项索引
     * @details
     * 当用户选择新的音频编码器时触发，执行以下操作：
     * 1. 获取选中的音频编码器 ID
     * 2. 查询该编码器支持的声道布局列表 (单声道, 立体声, 5.1 等)
     * 3. 更新声道布局下拉框内容
     * 4. 查询该编码器支持的采样率列表 (44100 Hz, 48000 Hz 等)
     * 5. 更新采样率下拉框内容
     */
    void onAudioCodecCurrentIndexChanged(int index);

    /**
     * @brief 声道布局下拉框选择变化槽函数 (当前未使用)
     * @param index 选中项索引
     */
    void onChannellayoutCurrentIndexChanged(int index);

    /**
     * @brief CRF 质量值变化槽函数
     * @param value 新的 CRF 值 (0-51)
     * @details
     * 当用户调整 CRF 值时触发，根据 CRF 值显示质量提示：
     * - 0-17: "very good" (视觉无损)
     * - 18-22: "good" (高质量)
     * - 23-27: "so so" (中等质量)
     * - 28-51: "bad" (低质量)
     * 更新 CRF 标签显示质量提示文本
     */
    void onCRFValueChanged(int value);

    /**
     * @brief "确定"按钮点击槽函数
     * @details
     * 用户点击"确定"按钮后，执行以下操作：
     * 1. 从各个下拉框和输入框读取用户配置的参数
     * 2. 更新内部 params 对象的各个属性：
     *    - 容器格式 (AVFileFmt)
     *    - 视频编码器 (VideoCodecId)
     *    - 像素格式 (VideoPixelFormat)
     *    - CRF 质量值
     *    - 音频编码器 (AudioCodecId)
     *    - 声道布局 (ChannelLayout)
     *    - 采样率 (SampleRate)
     *    - 文件名前缀 (FilenamePrefix)
     *    - 解码线程数 (DecodeThreadNum)
     *    - 编码线程数 (EncodeThreadNum)
     *    - 并发任务数 (TransNumSametime)
     * 3. 发射 OnConfigWindowsClose 信号通知主窗口参数已更新
     * 4. 关闭配置窗口
     */
    void OkayClickListener();

    /**
     * @brief "取消"按钮点击槽函数
     * @details 关闭配置窗口，不保存任何更改
     */
    void CancelClickListener();

    /**
     * @brief 设置当前显示数据 (将 params 对象的值同步到 UI 控件)
     * @details
     * 根据 params 成员变量的值，更新各个下拉框和输入框的选中项：
     * 1. 设置容器格式下拉框的选中项
     * 2. 设置视频编码器下拉框的选中项
     * 3. 设置像素格式下拉框的选中项
     * 4. 设置 CRF 数值框的值
     * 5. 设置音频编码器下拉框的选中项
     * 6. 设置声道布局下拉框的选中项
     * 7. 设置采样率下拉框的选中项
     * 8. 设置文件名前缀输入框的文本
     *
     * 此函数在构造函数中调用，用于初始化 UI 显示
     */
    void SetCurrentData();


    /**
     * @brief 获取转码参数对象
     * @return 当前配置的转码参数对象
     * @details 由主窗口调用，用于获取用户配置完成的转码参数
     */
    YouTranscoderParams GetTranscodeParams();

signals:
    /**
     * @brief 配置窗口关闭信号
     * @details
     * 当用户点击"确定"按钮时发射此信号，通知主窗口：
     * - 用户已完成参数配置
     * - 可以调用 GetTranscodeParams() 获取最新的配置参数
     */
    void OnConfigWindowsClose();

private:
    /**
     * @brief UI 界面对象指针 (由 Qt Designer 生成)
     */
    Ui::YouTransConfig *ui;

    /**
     * @brief 转码参数对象 (存储用户配置的所有转码参数)
     * @details
     * 包含以下参数：
     * - 容器格式 (MP4, MOV, MKV 等)
     * - 视频编码器 (H.264, H.265, ProRes 等)
     * - 像素格式 (yuv420p, yuv422p 等)
     * - CRF 质量值 (0-51)
     * - 音频编码器 (AAC, MP3, FLAC 等)
     * - 声道布局 (单声道, 立体声, 5.1 等)
     * - 采样率 (44100 Hz, 48000 Hz 等)
     * - 文件名前缀
     * - 解码线程数 (1-10)
     * - 编码线程数 (1-10)
     * - 并发任务数 (1-10)
     */
    YouTranscoderParams params;
};

#endif // YOUTRANSCONFIG_HPP
