#include "EyerAVReader.hpp"

#include <string.h>

#include "EyerAVReaderPrivate.hpp"
#include "EyerAVPacketPrivate.hpp"
#include "EyerAVStreamPrivate.hpp"

#include "EyerAVPixelFormat.hpp"

namespace Eyer {
    /**
     * @brief 自定义 IO 读取回调函数
     * @param opaque 用户自定义数据指针（EyerAVReaderCustomIO 对象）
     * @param buf 读取数据的缓冲区
     * @param buf_size 缓冲区大小
     * @return 实际读取的字节数
     */
    static int EyerAVReader_Read_Packet(void *opaque, uint8_t *buf, int buf_size)
    {
        EyerAVReaderCustomIO * customIO = (EyerAVReaderCustomIO *)opaque;
        return customIO->Read(buf, buf_size);
    }

    /**
     * @brief 自定义 IO 定位回调函数
     * @param opaque 用户自定义数据指针（EyerAVReaderCustomIO 对象）
     * @param offset 偏移量
     * @param whence 定位方式（SEEK_SET, SEEK_CUR, SEEK_END）
     * @return 新的位置，失败返回负值
     */
    static int64_t EyerAVReader_Seek(void *opaque, int64_t offset, int whence)
    {
        EyerAVReaderCustomIO * customIO = (EyerAVReaderCustomIO *)opaque;
        return customIO->Seek(offset, whence);
    }

    /**
     * @brief 构造函数 - 创建音视频读取器
     * @param _path 音视频文件路径或 URL
     * @param _customIO 自定义 IO 接口（可选，用于自定义数据读取）
     *
     * 初始化 FFmpeg 上下文，注册所有编解码器和格式
     * 如果提供了自定义 IO，则创建自定义 IO 缓冲区
     */
    EyerAVReader::EyerAVReader(EyerString _path, EyerAVReaderCustomIO * _customIO) {
        piml = new EyerAVReaderPrivate();
        piml->path = _path;
        // av_log_set_level(AV_LOG_WARNING);
        av_register_all();        // 注册所有 FFmpeg 编解码器和格式
        avformat_network_init();  // 初始化网络组件

        piml->formatCtx = avformat_alloc_context();  // 分配格式上下文

        customIO = _customIO;
        if(customIO != nullptr) {
            // 创建自定义 IO 缓冲区（1MB）
            constexpr int32_t buffer_size = 1024 * 1024;
            unsigned char * buffer = new unsigned char[buffer_size];
            piml->formatCtx->pb = avio_alloc_context(buffer, buffer_size, 0, customIO, EyerAVReader_Read_Packet, NULL, EyerAVReader_Seek);
        }
    }

    /**
     * @brief 析构函数 - 释放资源
     *
     * 关闭文件，释放格式上下文和私有数据
     */
    EyerAVReader::~EyerAVReader() {
        if (piml->formatCtx != NULL) {
            Close();
            avformat_free_context(piml->formatCtx);
            piml->formatCtx = NULL;
        }
        if (piml != nullptr) {
            delete piml;
            piml = nullptr;
        }
    }

    /**
     * @brief 打开音视频文件
     * @return 0 表示成功，非 0 表示失败
     *
     * 打开输入文件，读取流信息并输出格式信息
     * 这是一站式函数，包含打开和查找流信息
     */
    int EyerAVReader::Open() {
//        AVDictionary *option = nullptr;
//        av_dict_set(&option, "decryption_key", "fa8afa76abca4a59a5a23df79677ca49", 0);

        int ret = avformat_open_input(&piml->formatCtx, piml->path.c_str(), NULL, NULL);
        if (ret) {
            // TODO Error Code
            piml->isOpen = false;
            return ret;
        }

        // piml->formatCtx->streams[0]->discard = AVDiscard::AVDISCARD_ALL;
        // piml->formatCtx->streams[1]->discard = AVDiscard::AVDISCARD_ALL;

        piml->isOpen = true;
        avformat_find_stream_info(piml->formatCtx, NULL);  // 查找流信息
        av_dump_format(piml->formatCtx, 0, piml->path.c_str(), 0);  // 输出格式信息
        // piml->formatCtx->streams[0]->codecpar->extradata

        for(int i=0;i<piml->formatCtx->nb_streams;i++){
            // EyerLog("stream[%d] start_time: %lld\n", i, piml->formatCtx->streams[i]->start_time);
        }
        return ret;
    }

    /**
     * @brief 仅打开输入文件（不读取流信息）
     * @return 0 表示成功，非 0 表示失败
     *
     * 仅执行打开操作，不查找流信息
     * 可以与 FindStreamInfo() 配合使用，实现分步骤打开
     */
    int EyerAVReader::OpenInput()
    {
        int ret = avformat_open_input(&piml->formatCtx, piml->path.c_str(), NULL, NULL);
        if (ret) {
            piml->isOpen = false;
            return ret;
        }
        piml->isOpen = true;
        return 0;
    }

    /**
     * @brief 查找流信息
     * @return 0 表示成功
     *
     * 读取媒体文件的流信息（编解码器参数等）
     * 并输出格式信息到控制台
     */
    int EyerAVReader::FindStreamInfo()
    {
        avformat_find_stream_info(piml->formatCtx, NULL);
        av_dump_format(piml->formatCtx, 0, piml->path.c_str(), 0);
        return 0;
    }

    /**
     * @brief 检查文件是否已打开
     * @return true 表示已打开，false 表示未打开
     */
    bool EyerAVReader::IsOpen()
    {
        return piml->isOpen;
    }

    /**
     * @brief 关闭音视频文件
     * @return 0 表示成功
     *
     * 关闭输入文件并释放相关资源
     */
    int EyerAVReader::Close() {
        avformat_close_input(&piml->formatCtx);
        piml->isOpen = false;
        return 0;
    }

    /**
     * @brief 获取流数量
     * @return 文件中包含的流（音频、视频、字幕等）总数
     */
    int EyerAVReader::GetStreamCount()
    {
        return piml->formatCtx->nb_streams;
    }

    /**
     * @brief 获取指定索引的流信息
     * @param stream 输出参数，存储流信息
     * @param index 流索引
     * @return 0 表示成功，非 0 表示失败
     *
     * 复制流的编解码器参数、时间基准、时长等信息到 stream 对象
     */
    int EyerAVReader::GetStream(EyerAVStream & stream, int index)
    {
        stream.piml->stream_id  = piml->formatCtx->streams[index]->index;
        stream.piml->timebase   = piml->formatCtx->streams[index]->time_base;

        /*
        AVDictionaryEntry * tag = nullptr;
        tag = av_dict_get(piml->formatCtx->streams[index]->metadata, "rotate", tag, 0);
        if (tag == nullptr) {
            stream.piml->angle = 0;
        } else {
            int angle = atoi(tag->value);
            stream.piml->angle = angle;
        }
         */

        // 计算流时长（秒）
        stream.piml->duration = piml->formatCtx->streams[index]->duration * 1.0 * stream.piml->timebase.num / stream.piml->timebase.den;

        // EyerString name = EyerAVPixelFormat::GetByFFmpegId(piml->formatCtx->streams[index]->codecpar->format).GetName();

        // 复制编解码器参数
        return avcodec_parameters_copy(stream.piml->codecpar, piml->formatCtx->streams[index]->codecpar);
    }

    /**
     * @brief 获取指定索引的流信息（返回值版本）
     * @param index 流索引
     * @return EyerAVStream 对象（包含流信息）
     */
    EyerAVStream EyerAVReader::GetStream(int index)
    {
        EyerAVStream stream;
        GetStream(stream, index);
        return stream;
    }

    /**
     * @brief 获取指定流的时间基准
     * @param timebase 输出参数，存储时间基准
     * @param streamIndex 流索引
     * @return 0 表示成功
     *
     * 时间基准用于将时间戳转换为实际时间（秒）
     * 实际时间 = 时间戳 * timebase.num / timebase.den
     */
    int EyerAVReader::GetTimebase(EyerAVRational & timebase, int streamIndex)
    {
        timebase.num = piml->formatCtx->streams[streamIndex]->time_base.num;
        timebase.den = piml->formatCtx->streams[streamIndex]->time_base.den;
        return 0;
    }

    /**
     * @brief 获取文件总时长
     * @return 文件时长（秒）
     */
    double EyerAVReader::GetDuration()
    {
        return piml->formatCtx->duration / AV_TIME_BASE;
    }

    /**
     * @brief 跳转到指定时间点
     * @param time 目标时间（秒）
     * @return >= 0 表示成功，负数表示失败
     *
     * 跳转到指定时间的关键帧（向后查找最近的关键帧）
     * 使用 AV_TIME_BASE 单位进行跳转
     */
    int EyerAVReader::Seek(double time)
    {
        /**
        * Seek to the keyframe at timestamp.
        * 'timestamp' in 'stream_index'.
        *
        * @param s media file handle
        * @param stream_index If stream_index is (-1), a default
        * stream is selected, and timestamp is automatically converted
        * from AV_TIME_BASE units to the stream specific time_base.
        * @param timestamp Timestamp in AVStream.time_base units
        *        or, if no stream is specified, in AV_TIME_BASE units.
        * @param flags flags which select direction and seeking mode
        * @return >= 0 on success
        */
        int64_t t = time * AV_TIME_BASE;
        int ret = av_seek_frame(piml->formatCtx, -1, t, AVSEEK_FLAG_BACKWARD);
        return ret;
    }

    /**
     * @brief 在指定流中跳转到指定时间戳
     * @param t 目标时间戳（流时间基准单位）
     * @param streamId 流索引
     * @return >= 0 表示成功，负数表示失败
     *
     * 在指定流中跳转，时间戳使用该流的时间基准
     */
    int EyerAVReader::SeekStream(int64_t t, int streamId)
    {
        int ret = av_seek_frame(piml->formatCtx, streamId, t, AVSEEK_FLAG_BACKWARD);
        return ret;
    }

    /**
     * @brief 在指定流中跳转到指定时间点
     * @param time 目标时间（秒）
     * @param streamId 流索引
     * @return >= 0 表示成功，负数表示失败
     *
     * 将秒转换为流时间戳后进行跳转
     */
    int EyerAVReader::SeekStream(double time, int streamId)
    {
        EyerAVRational timebase;
        GetTimebase(timebase, streamId);
        int64_t tt = time * timebase.den / timebase.num;  // 秒转时间戳
        int ret = av_seek_frame(piml->formatCtx, streamId, tt, AVSEEK_FLAG_BACKWARD);

        // int64_t t = time * AV_TIME_BASE;
        // int ret = av_seek_frame(piml->formatCtx, -1, t, AVSEEK_FLAG_BACKWARD);
        return ret;
    }

    /**
     * @brief 读取一个数据包（指针版本）
     * @param packet 输出参数，存储读取的数据包
     * @return 0 表示成功，负数表示失败或文件结束
     *
     * 读取下一帧数据包，并调整 PTS（减去起始时间）
     * 同时计算秒级时间戳（secPTS）
     */
    int EyerAVReader::Read(EyerAVPacket * packet)
    {
        int ret = av_read_frame(piml->formatCtx, packet->piml->packet);
        if(!ret){
            int streamIndex = packet->GetStreamIndex();

            // 减去流的起始时间，使 PTS 从 0 开始
            int64_t start_time = piml->formatCtx->streams[streamIndex]->start_time;
            packet->piml->packet->pts -= start_time;

            // 计算秒级时间戳
            int64_t pts = packet->GetPTS();
            packet->piml->secPTS = pts * av_q2d(piml->formatCtx->streams[streamIndex]->time_base);
        }
        return ret;
    }

    /**
     * @brief 读取一个数据包（引用版本）
     * @param packet 输出参数，存储读取的数据包
     * @return 0 表示成功，负数表示失败或文件结束
     *
     * 读取下一帧数据包，处理 PTS 和 secPTS
     * 对 AV_NOPTS_VALUE 进行检查，避免无效值计算
     */
    int EyerAVReader::Read(EyerAVPacket & packet)
    {
        int ret = av_read_frame(piml->formatCtx, packet.piml->packet);
        if(!ret){
            int streamIndex = packet.GetStreamIndex();
            int64_t start_time = piml->formatCtx->streams[streamIndex]->start_time;
            // 检查起始时间是否有效
            if(start_time != AV_NOPTS_VALUE){
                packet.piml->packet->pts -= start_time;
            }
            int64_t pts = packet.piml->packet->pts;
            // 检查 PTS 是否有效
            if(pts != AV_NOPTS_VALUE){
                packet.piml->secPTS = pts * av_q2d(piml->formatCtx->streams[streamIndex]->time_base);
            }

            /*
            uint8_t * buf       = (uint8_t *)packet.piml->packet->data;
            uint8_t * sidebuf   = (uint8_t *)packet.piml->packet->side_data->data + 8;

            int keyframe  = !(buf[0] & 1);
            int profile   =  (buf[0]>>1) & 7;
            int invisible = !(buf[0] & 0x10);

            int sidekeyframe  = !(sidebuf[0] & 1);
            int sideprofile   =  (sidebuf[0]>>1) & 7;
            int sideinvisible = !(sidebuf[0] & 0x10);
            */
            // EyerLog("key: %2d, %5d   xxxxxxxxxxxxx  key: %2d, %5d\n", keyframe, packet.piml->packet->size, sidekeyframe, packet.piml->packet->side_data->size);
        }
        return ret;
    }

    /**
     * @brief 读取一个数据包（智能指针版本）
     * @param ret 输出参数，返回读取结果（0 成功，负数失败）
     * @return 智能指针，指向读取的数据包
     *
     * 创建新的数据包对象并读取数据
     * 使用智能指针自动管理内存
     */
    EyerSmartPtr<EyerAVPacket> EyerAVReader::Read(int & ret)
    {
        EyerAVPacket * packetPtr = new EyerAVPacket();
        EyerSmartPtr<EyerAVPacket> packet(packetPtr);
        ret = Read(packetPtr);
        return packet;
    }

    /**
     * @brief 获取最佳视频流索引
     * @return 视频流索引，-1 表示未找到视频流
     *
     * 自动选择最佳的视频流（通常是主视频流）
     */
    int EyerAVReader::GetVideoStreamIndex() const
    {
        if(piml->formatCtx == nullptr){
            return -1;
        }
        int videoStream = av_find_best_stream(piml->formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        return videoStream;
    }

    /**
     * @brief 获取最佳音频流索引
     * @return 音频流索引，-1 表示未找到音频流
     *
     * 自动选择最佳的音频流（通常是主音频流）
     */
    int EyerAVReader::GetAudioStreamIndex() const
    {
        if (piml->formatCtx == nullptr) {
            return -1;
        }
        int audioStream = av_find_best_stream(piml->formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        return audioStream;
    }
}