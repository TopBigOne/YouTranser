/**
 * @file YouTranscoderParams.hpp
 * @brief 转码参数类定义
 * @details 扩展 EyerAVTranscoderParams,添加 Qt 特有的功能如 JSON 序列化、文件名模板等
 */

#ifndef YOUTRANSCODERPARAMS_HPP
#define YOUTRANSCODERPARAMS_HPP

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

#include "EyerAVTranscoder/EyerAVTranscoderHeader.hpp"

/**
 * @class YouTranscoderParams
 * @brief Qt 版本的转码参数类
 * @details
 * - 继承自 Eyer::EyerAVTranscoderParams (核心转码参数)
 * - 添加 JSON 序列化/反序列化功能 (用于配置持久化)
 * - 添加输出文件名模板功能 (支持变量替换)
 * - 添加并发任务数控制
 * - 添加输出目录管理
 */
class YouTranscoderParams : public Eyer::EyerAVTranscoderParams
{
public:
    /**
     * @brief 默认构造函数
     * @details 初始化默认参数值
     */
    YouTranscoderParams();

    /**
     * @brief 析构函数
     */
    ~YouTranscoderParams();

    /**
     * @brief 拷贝构造函数
     * @param params 要拷贝的参数对象
     */
    YouTranscoderParams(const YouTranscoderParams & params);

    /**
     * @brief 赋值运算符重载
     * @param params 要赋值的参数对象
     * @return 引用自身
     */
    YouTranscoderParams & operator = (const YouTranscoderParams & params);

    /**
     * @brief 序列化为 JSON 字符串
     * @return JSON 格式的参数字符串
     * @details 将所有参数转换为 JSON 格式,用于持久化存储到 QSettings
     */
    QString ToJson();

    /**
     * @brief 从 JSON 字符串解析参数
     * @param jsonStr JSON 格式的参数字符串
     * @return 0表示成功
     * @details 从 JSON 字符串恢复参数,用于从 QSettings 加载配置
     */
    int ParseJson(const QString & jsonStr);


    /**
     * @brief 获取文件名前缀模板
     * @return 文件名前缀字符串 (包含变量占位符)
     * @details
     * 支持的变量:
     * - ${origin_file_name}: 原始文件名 (不含扩展名)
     * - ${output_video_codec}: 输出视频编码器名称 (如 H.264, H.265, ProRes)
     * - ${video_pixelfmt}: 视频像素格式 (如 yuv420p, yuv422p10le)
     * - ${output_audio_codec}: 输出音频编码器名称 (如 AAC, MP3)
     */
    QString GetFilenamePrefix();

    /**
     * @brief 设置文件名前缀模板
     * @param _filenamePrefix 文件名前缀字符串
     * @return 0表示成功
     */
    int SetFilenamePrefix(const QString & _filenamePrefix);

    /**
     * @brief 获取同时进行的任务数
     * @return 并发任务数 (默认为 2)
     * @details 控制同时运行的转码任务数量,避免系统资源耗尽
     */
    const int GetTransNumSametime() const;

    /**
     * @brief 设置同时进行的任务数
     * @param num 并发任务数
     * @return 0表示成功
     */
    int SetTransNumSametime(int num);

    /**
     * @brief 获取输出目录
     * @return 输出文件夹路径
     */
    QString GetOutputDir() const;

    /**
     * @brief 设置输出目录
     * @param dir 输出文件夹路径
     * @return 0表示成功
     */
    int SetOutputDir(const QString & dir);
private:
    /**
     * @brief 文件名前缀模板
     * @details 默认模板: 原始文件名_视频编码_像素格式_音频编码
     */
    QString filenamePrefix = "${origin_file_name}_${output_video_codec}_${video_pixelfmt}_${output_audio_codec}";

    QString outputDir = "";  ///< 输出文件夹路径
    int transNumSametime = 2;  ///< 同时进行的任务数 (默认2个)
};

#endif // YOUTRANSCODERPARAMS_HPP
