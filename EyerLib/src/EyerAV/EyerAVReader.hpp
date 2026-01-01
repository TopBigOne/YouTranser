#ifndef EYERLIB_EYERAVREADER_HPP
#define EYERLIB_EYERAVREADER_HPP

#include "EyerCore/EyerCore.hpp"
#include "EyerAVPacket.hpp"
#include "EyerAVStream.hpp"
#include "EyerAVReaderCustomIO.hpp"

namespace Eyer
{
    class EyerAVReaderPrivate;

    /**
     * @brief 音视频文件读取器类
     *
     * 用于打开和读取各种音视频文件格式（MP4、AVI、MKV、FLV 等）
     * 支持本地文件、网络流和自定义 IO
     *
     * 主要功能：
     * - 打开音视频文件并解析流信息
     * - 读取编码后的音视频数据包（AVPacket）
     * - 跳转到指定时间点（Seek）
     * - 获取流信息（编解码器参数、时长、时间基准等）
     * - 支持自定义 IO 接口（用于读取加密文件、内存数据等）
     *
     * 使用示例：
     * @code
     * EyerAVReader reader("input.mp4");
     * reader.Open();
     * int videoIndex = reader.GetVideoStreamIndex();
     * EyerAVPacket packet;
     * while (reader.Read(packet) == 0) {
     *     if (packet.GetStreamIndex() == videoIndex) {
     *         // 处理视频数据包
     *     }
     * }
     * reader.Close();
     * @endcode
     */
    class EyerAVReader {
    public:
        /**
         * @brief 构造函数 - 创建音视频读取器
         * @param _path 音视频文件路径或 URL
         * @param _customIO 自定义 IO 接口（可选），用于自定义数据读取
         *
         * 支持多种输入源：
         * - 本地文件路径（如 "/path/to/video.mp4"）
         * - 网络 URL（如 "http://example.com/video.mp4"）
         * - 自定义 IO（通过 customIO 参数）
         */
        EyerAVReader(EyerString _path, EyerAVReaderCustomIO * _customIO = nullptr);

        /**
         * @brief 析构函数 - 释放资源
         *
         * 自动关闭文件并释放所有相关资源
         */
        ~EyerAVReader();

        /**
         * @brief 打开音视频文件（一站式函数）
         * @return 0 表示成功，非 0 表示失败
         *
         * 执行完整的打开流程：
         * 1. 打开输入文件
         * 2. 读取流信息
         * 3. 输出格式信息到控制台
         *
         * 这是最常用的打开方法
         */
        int Open();

        /**
         * @brief 仅打开输入文件（不读取流信息）
         * @return 0 表示成功，非 0 表示失败
         *
         * 与 FindStreamInfo() 配合使用，实现分步骤打开
         * 适用于需要自定义打开流程的场景
         */
        int OpenInput();

        /**
         * @brief 查找并读取流信息
         * @return 0 表示成功
         *
         * 读取媒体文件的流信息（编解码器参数等）
         * 并输出格式信息到控制台
         * 通常与 OpenInput() 配合使用
         */
        int FindStreamInfo();

        /**
         * @brief 检查文件是否已打开
         * @return true 表示已打开，false 表示未打开
         */
        bool IsOpen();

        /**
         * @brief 关闭音视频文件
         * @return 0 表示成功
         *
         * 关闭输入文件并释放相关资源
         * 析构函数会自动调用此方法
         */
        int Close();

        /**
         * @brief 获取文件总时长
         * @return 文件时长（秒）
         *
         * 返回整个媒体文件的播放时长
         */
        double GetDuration();

        /**
         * @brief 获取流数量
         * @return 文件中包含的流（音频、视频、字幕等）总数
         */
        int GetStreamCount();

        /**
         * @brief 获取指定索引的流信息（引用版本）
         * @param stream 输出参数，存储流信息
         * @param index 流索引（从 0 开始）
         * @return 0 表示成功，非 0 表示失败
         *
         * 复制流的编解码器参数、时间基准、时长等信息到 stream 对象
         */
        int GetStream(EyerAVStream & stream, int index);

        /**
         * @brief 获取指定索引的流信息（返回值版本）
         * @param index 流索引（从 0 开始）
         * @return EyerAVStream 对象（包含流信息）
         */
        EyerAVStream GetStream(int index);

        /**
         * @brief 获取指定流的时间基准
         * @param timebase 输出参数，存储时间基准
         * @param streamIndex 流索引
         * @return 0 表示成功
         *
         * 时间基准用于将时间戳转换为实际时间（秒）
         * 实际时间 = 时间戳 × timebase.num / timebase.den
         */
        int GetTimebase(EyerAVRational & timebase, int streamIndex);

        /**
         * @brief 跳转到指定时间点（全局跳转）
         * @param time 目标时间（秒）
         * @return >= 0 表示成功，负数表示失败
         *
         * 跳转到指定时间的关键帧（向后查找最近的关键帧）
         * 适用于所有流的统一跳转
         */
        int Seek(double time);

        /**
         * @brief 在指定流中跳转到指定时间戳
         * @param t 目标时间戳（流时间基准单位）
         * @param streamId 流索引
         * @return >= 0 表示成功，负数表示失败
         *
         * 在指定流中跳转，时间戳使用该流的时间基准
         */
        int SeekStream(int64_t t, int streamId);

        /**
         * @brief 在指定流中跳转到指定时间点
         * @param t 目标时间（秒）
         * @param streamId 流索引
         * @return >= 0 表示成功，负数表示失败
         *
         * 将秒转换为流时间戳后进行跳转
         */
        int SeekStream(double t, int streamId);

        /**
         * @brief 读取一个数据包（引用版本）
         * @param packet 输出参数，存储读取的数据包
         * @return 0 表示成功，负数表示失败或文件结束
         *
         * 读取下一帧数据包（编码后的数据）
         * 数据包包含音频、视频或字幕数据
         * 返回 AVERROR_EOF 表示文件结束
         */
        int Read(EyerAVPacket & packet);

        /**
         * @brief 读取一个数据包（指针版本）
         * @param packet 输出参数，存储读取的数据包
         * @return 0 表示成功，负数表示失败或文件结束
         *
         * 与引用版本功能相同，适用于需要指针的场景
         */
        int Read(EyerAVPacket * packet);

        /**
         * @brief 读取一个数据包（智能指针版本）
         * @param ret 输出参数，返回读取结果（0 成功，负数失败）
         * @return 智能指针，指向读取的数据包
         *
         * 使用智能指针自动管理内存
         * 适用于需要自动释放资源的场景
         */
        EyerSmartPtr<EyerAVPacket> Read(int & ret);

        /**
         * @brief 获取最佳音频流索引
         * @return 音频流索引，-1 表示未找到音频流
         *
         * 自动选择最佳的音频流（通常是主音频流）
         */
        int GetAudioStreamIndex() const;

        /**
         * @brief 获取最佳视频流索引
         * @return 视频流索引，-1 表示未找到视频流
         *
         * 自动选择最佳的视频流（通常是主视频流）
         */
        int GetVideoStreamIndex() const;

    private:
        // 禁用拷贝构造和赋值操作（避免资源管理问题）
        EyerAVReader(const EyerAVReader & reader) = delete;
        EyerAVReader & operator = (const EyerAVReader & reader) = delete;

    public:
        EyerAVReaderPrivate * piml = nullptr;  ///< 私有实现指针（PIMPL 模式）
        EyerAVReaderCustomIO * customIO = nullptr;  ///< 自定义 IO 接口指针
    };
}

#endif //EYERLIB_EYERAVREADER_HPP
