/**
 * @file TranscodeTaskThread.cpp
 * @brief 视频转码任务线程类实现
 * @details 实现多线程视频转码功能,包括进度监听、错误处理、中断控制等核心逻辑
 */

#include "TranscodeTaskThread.hpp"

#include <QDebug>

#include <QFileInfo>

/**
 * @brief 构造函数实现
 * @param _input 输入视频文件路径
 * @details
 * - 保存输入文件路径
 * - 创建 EyerAVTranscoder 转码器实例
 * - QFileInfo 用于文件信息验证(当前未使用但可扩展)
 */
TranscodeTaskThread::TranscodeTaskThread(const QString & _input)
{
    input = _input;
    QFileInfo fInput(input);  // 可用于验证文件是否存在、获取文件名等
    transcoder = new Eyer::EyerAVTranscoder(input.toStdString());
}

/**
 * @brief 析构函数实现
 * @details 安全释放转码器资源,防止内存泄漏
 */
TranscodeTaskThread::~TranscodeTaskThread()
{
    if(transcoder != nullptr){
        delete transcoder;
        transcoder = nullptr;
    }
}

/**
 * @brief 停止转码任务实现
 * @return 0表示成功
 * @details
 * - 使用 RAII 锁(lock_guard)保护中断标志的线程安全访问
 * - 设置中断标志后,等待线程自然结束
 * - wait() 会阻塞直到 run() 方法执行完毕
 */
int TranscodeTaskThread::Stop()
{
    {
        std::lock_guard<std::mutex> lock(interruptFlagMut);
        interruptFlag = true;  // 请求中断转码
    }
    wait();  // 等待线程结束 (QThread::wait)
    return 0;
}

/**
 * @brief 设置转码参数实现
 * @param _params 转码参数对象(包含编码器、码率、分辨率等配置)
 * @return 0表示成功
 * @details 必须在调用 start() 启动线程之前设置
 */
int TranscodeTaskThread::SetParams(const Eyer::EyerAVTranscoderParams & _params)
{
    params = _params;
    return 0;
}

/**
 * @brief 设置输出文件路径实现
 * @param _output 输出视频文件路径
 * @return 0表示成功
 * @details 必须在调用 start() 启动线程之前设置
 */
int TranscodeTaskThread::SetOutput(const QString & _output)
{
    output = _output;
    return 0;
}

/**
 * @brief 线程执行函数实现 (QThread::run 重写)
 * @details
 * - 在独立线程中执行转码操作
 * - 配置转码器的输出路径、参数、监听器和中断接口
 * - transcoder->Transcode() 是阻塞调用,转码完成或失败后才返回
 * - EyerLog 用于调试日志输出
 */
void TranscodeTaskThread::run()
{
    EyerLog("TranscodeTaskThread Start\n");

    // 配置转码器
    transcoder->SetOutputPath(output.toStdString());
    transcoder->SetParams(params);
    transcoder->SetListener(this);  // 注册监听器接收进度/成功/失败回调

    // 开始转码 (阻塞调用,传入中断接口)
    transcoder->Transcode(this);

    EyerLog("TranscodeTaskThread End\n");
}

/**
 * @brief 中断检查函数实现 (EyerAVTranscoderInterrupt 接口)
 * @return true表示需要中断转码,false表示继续转码
 * @details
 * - 转码器会周期性调用此函数检查是否需要中断
 * - 使用互斥锁保护 interruptFlag 的线程安全读取
 * - 当用户调用 Stop() 时,此函数将返回 true 通知转码器停止
 */
bool TranscodeTaskThread::interrupt()
{
    std::lock_guard<std::mutex> lock(interruptFlagMut);
    return interruptFlag;
}

/**
 * @brief 转码进度回调实现 (EyerAVTranscoderListener 接口)
 * @param progress 转码进度值 (0.0-1.0,0表示开始,1表示完成)
 * @return 0表示继续转码
 * @details
 * - 转码器会周期性调用此函数报告进度
 * - 通过 Qt 信号 emit 将进度传递到 UI 线程
 * - UI 线程可以连接此信号更新进度条
 */
int TranscodeTaskThread::OnProgress(float progress)
{
    emit OnTaskProgress(progress);
    return 0;
}

/**
 * @brief 转码失败回调实现 (EyerAVTranscoderListener 接口)
 * @param code 错误对象,包含错误码和描述信息
 * @return 0表示成功
 * @details
 * - 转码失败时被调用 (如解码错误、编码错误、写入失败等)
 * - 记录错误日志到控制台
 * - 通过 Qt 信号将错误码传递到 UI 线程
 */
int TranscodeTaskThread::OnFail(Eyer::EyerAVTranscoderError & code)
{
    EyerLog("OnFail: %s\n", code.GetDesc().c_str());
    emit OnTaskFail(code.GetCode());  // 发送错误码到 UI 线程
    return 0;
}

/**
 * @brief 转码成功回调实现 (EyerAVTranscoderListener 接口)
 * @return 0表示成功
 * @details
 * - 转码成功完成时被调用
 * - 记录成功日志到控制台
 * - 通过 Qt 信号通知 UI 线程更新任务状态
 */
int TranscodeTaskThread::OnSuccess()
{
    EyerLog("OnSuccess\n");
    emit OnTaskSuccess();  // 通知 UI 线程转码成功
    return 0;
}

/**
 * @brief 获取转码状态实现
 * @return 转码状态枚举(准备中/进行中/成功/失败)
 * @details 委托给内部转码器获取当前状态
 */
Eyer::EyerAVTranscoderStatus TranscodeTaskThread::GetStatus(){
    return transcoder->GetStatus();
}

/**
 * @brief 设置转码状态实现
 * @param _status 转码状态枚举
 * @return 0表示成功
 * @details 委托给内部转码器设置状态
 */
int TranscodeTaskThread::SetStatus(const Eyer::EyerAVTranscoderStatus & _status)
{
    return transcoder->SetStatus(_status);
}

/**
 * @brief 设置错误描述信息实现
 * @param errorDesc 错误描述字符串
 * @return 0表示成功
 * @details
 * - 将 QString 转换为 std::string 再转为 C 字符串
 * - 委托给内部转码器保存错误描述
 */
int TranscodeTaskThread::SetErrorDesc(const QString & errorDesc)
{
    return transcoder->SetErrorDesc(errorDesc.toStdString().c_str());
}

/**
 * @brief 获取错误描述信息实现
 * @return 错误描述字符串
 * @details
 * - 从内部转码器获取 std::string 错误描述
 * - 转换为 QString 返回给调用者
 */
QString TranscodeTaskThread::GetErrorDesc()
{
    return transcoder->GetErrorDesc().c_str();
}

/**
 * @brief 获取输入文件路径实现
 * @return 输入视频文件路径
 * @details 返回构造时传入的输入文件路径
 */
QString TranscodeTaskThread::GetInputPath() const
{
    return input;
}
