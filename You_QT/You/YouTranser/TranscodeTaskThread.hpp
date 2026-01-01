/**
 * @file TranscodeTaskThread.hpp
 * @brief 视频转码任务线程类定义
 * @details 封装视频转码功能的 Qt 线程,支持异步转码、进度通知、中断控制等
 */

#ifndef TRANSCODETASKTHREAD_HPP
#define TRANSCODETASKTHREAD_HPP

#include <mutex>

#include <QThread>

#include "EyerAVTranscoder/EyerAVTranscoderHeader.hpp"

/**
 * @class TranscodeTaskThread
 * @brief 视频转码任务线程
 * @details 继承自 QThread 实现多线程转码,同时实现转码监听器和中断接口
 *          支持 H.264/H.265/ProRes 等多种编码格式的转换
 */
class TranscodeTaskThread : public QThread,
                           public Eyer::EyerAVTranscoderListener,    // 转码事件监听器
                           public Eyer::EyerAVTranscoderInterrupt    // 转码中断控制器
{
    Q_OBJECT  // Qt 元对象系统宏,支持信号槽机制
public:
    /**
     * @brief 构造函数
     * @param _input 输入视频文件路径
     */
    TranscodeTaskThread(const QString & _input);

    /**
     * @brief 析构函数,释放转码器资源
     */
    ~TranscodeTaskThread();

    /**
     * @brief 停止转码任务
     * @return 0表示成功
     * @details 设置中断标志并等待线程结束
     */
    int Stop();

    /**
     * @brief 获取输入文件路径
     * @return 输入视频文件路径
     */
    QString GetInputPath() const;

    /**
     * @brief 设置转码参数
     * @param _params 转码参数对象(包含编码器、码率、分辨率等配置)
     * @return 0表示成功
     */
    int SetParams(const Eyer::EyerAVTranscoderParams & _params);

    /**
     * @brief 设置输出文件路径
     * @param _output 输出视频文件路径
     * @return 0表示成功
     */
    int SetOutput(const QString & _output);

    /**
     * @brief 获取转码状态
     * @return 转码状态枚举(准备中/进行中/成功/失败)
     */
    Eyer::EyerAVTranscoderStatus GetStatus();

    /**
     * @brief 设置转码状态
     * @param _status 转码状态枚举
     * @return 0表示成功
     */
    int SetStatus(const Eyer::EyerAVTranscoderStatus & _status);

    /**
     * @brief 获取错误描述信息
     * @return 错误描述字符串
     */
    QString GetErrorDesc();

    /**
     * @brief 设置错误描述信息
     * @param errorDesc 错误描述字符串
     * @return 0表示成功
     */
    int SetErrorDesc(const QString & errorDesc);

    /**
     * @brief 线程执行函数(重写 QThread::run)
     * @details 在独立线程中执行视频转码操作
     */
    void run() override;

    /**
     * @brief 转码进度回调函数
     * @param progress 转码进度(0.0-1.0)
     * @return 0表示继续,非0表示中断
     * @details 实现 EyerAVTranscoderListener 接口,转码过程中被周期性调用
     */
    virtual int OnProgress(float progress) override;

    /**
     * @brief 转码失败回调函数
     * @param code 错误码对象
     * @return 0表示成功
     * @details 转码失败时被调用,包含详细的错误信息
     */
    virtual int OnFail(Eyer::EyerAVTranscoderError & code) override;

    /**
     * @brief 转码成功回调函数
     * @return 0表示成功
     * @details 转码完成时被调用
     */
    virtual int OnSuccess() override;

    /**
     * @brief 中断检查函数
     * @return true表示需要中断,false表示继续
     * @details 实现 EyerAVTranscoderInterrupt 接口,转码过程中被周期性调用
     */
    virtual bool interrupt() override;

signals:
    /**
     * @brief 转码进度信号
     * @param progress 转码进度(0.0-1.0)
     * @details 发送到 UI 线程,用于更新进度条
     */
    void OnTaskProgress(float progress);

    /**
     * @brief 转码失败信号
     * @param code 错误码
     * @details 发送到 UI 线程,用于显示错误信息
     */
    void OnTaskFail(int code);

    /**
     * @brief 转码成功信号
     * @details 发送到 UI 线程,用于更新任务状态
     */
    void OnTaskSuccess();

private:
    QString input;   ///< 输入视频文件路径
    QString output;  ///< 输出视频文件路径

    Eyer::EyerAVTranscoder * transcoder = nullptr;  ///< 转码器实例指针

    Eyer::EyerAVTranscoderParams params;  ///< 转码参数配置

    std::mutex interruptFlagMut;  ///< 中断标志互斥锁(保护多线程访问)
    bool interruptFlag = false;   ///< 中断标志(true表示请求停止转码)
};

#endif // TRANSCODETASKTHREAD_HPP
