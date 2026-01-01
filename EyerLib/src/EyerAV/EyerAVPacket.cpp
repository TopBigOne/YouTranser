#include "EyerAVPacket.hpp"

#include "EyerAVReaderPrivate.hpp"
#include "EyerAVPacketPrivate.hpp"

namespace Eyer
{
    /**
     * @brief 构造函数 - 创建音视频数据包对象
     *
     * 初始化流程：
     * 1. 分配私有数据结构
     * 2. 分配 FFmpeg 的 AVPacket（用于存储编码后的音视频数据）
     *
     * AVPacket 用于存储压缩后的音视频数据（编码后的数据）
     * 与 AVFrame 不同，AVPacket 存储的是压缩数据，而非原始数据
     */
    EyerAVPacket::EyerAVPacket()
    {
        piml = new EyerAVPacketPrivate();
        piml->packet = av_packet_alloc();  // 分配 AVPacket 结构
    }

    /**
     * @brief 拷贝构造函数 - 从另一个数据包创建副本
     * @param packet 源数据包对象
     *
     * 实现流程：
     * 1. 先调用默认构造函数初始化
     * 2. 通过赋值操作符实现深拷贝
     * 3. 会复制 AVPacket 的所有数据和元信息
     */
    EyerAVPacket::EyerAVPacket(const EyerAVPacket & packet) : EyerAVPacket()
    {
        *this = packet;
    }

    /**
     * @brief 析构函数 - 释放数据包资源
     *
     * 释放流程：
     * 1. 如果 AVPacket 已分配，释放其占用的内存和数据
     * 2. 释放私有数据结构
     * 3. 将指针置为 nullptr，避免悬空指针
     *
     * 使用 av_packet_free 会自动释放 AVPacket 内部的数据缓冲区
     */
    EyerAVPacket::~EyerAVPacket()
    {
        if (piml->packet != nullptr) {
            av_packet_free(&piml->packet);  // 释放 AVPacket 及其内部数据
            piml->packet = nullptr;
        }

        if(piml != nullptr){
            delete piml;
            piml = nullptr;
        }
    }

    /**
     * @brief 赋值操作符重载 - 深拷贝数据包内容
     * @param packet 源数据包对象
     * @return 当前对象的引用
     *
     * 深拷贝流程：
     * 1. 先释放当前 AVPacket 的内存（避免内存泄漏）
     * 2. 使用 av_packet_clone 克隆源 AVPacket（完整拷贝数据和元信息）
     * 3. 复制附加字段（如秒级 PTS）
     *
     * av_packet_clone 会分配新内存并复制所有数据
     */
    EyerAVPacket & EyerAVPacket::operator = (const EyerAVPacket & packet)
    {
        if (piml->packet != nullptr) {
            av_packet_free(&piml->packet);  // 释放当前 AVPacket
            piml->packet = nullptr;
        }

        piml->packet = av_packet_clone(packet.piml->packet);  // 克隆源 AVPacket
        piml->secPTS = packet.piml->secPTS;  // 复制秒级时间戳

        return *this;
    }

    /**
     * @brief 设置显示时间戳（Presentation Timestamp）
     * @param pts 显示时间戳（时间基准单位）
     * @return 0 表示成功
     *
     * PTS 用于指定该数据包应何时被解码和显示
     * 实际时间（秒）= PTS × 时间基准.num / 时间基准.den
     * 例如：时间基准为 1/1000，PTS=2000 表示 2 秒
     */
    int EyerAVPacket::SetPTS(int64_t pts)
    {
        piml->packet->pts = pts;
        return 0;
    }

    /**
     * @brief 获取显示时间戳（Presentation Timestamp）
     * @return 显示时间戳（时间基准单位）
     *
     * PTS 用于指定数据包的显示时间
     * 对于音频，PTS 通常等于 DTS
     * 对于视频，PTS 可能与 DTS 不同（B 帧情况下，显示顺序与解码顺序不同）
     */
    int64_t EyerAVPacket::GetPTS()
    {
        return piml->packet->pts;
    }

    /**
     * @brief 获取解码时间戳（Decoding Timestamp）
     * @return 解码时间戳（时间基准单位）
     *
     * DTS 用于指定该数据包应何时被解码
     * DTS 总是单调递增的（解码顺序）
     * PTS 可能不是单调的（显示顺序，如 B 帧）
     *
     * 示例（带 B 帧的视频）：
     * - 解码顺序（DTS）：I P B B P（0, 1, 2, 3, 4）
     * - 显示顺序（PTS）：I B B P P（0, 2, 3, 1, 4）
     */
    int64_t EyerAVPacket::GetDTS()
    {
        return piml->packet->dts;
    }

    /**
     * @brief 获取数据包所属的流索引
     * @return 流索引（从 0 开始）
     *
     * 流索引用于标识数据包属于哪个流（音频、视频或字幕）
     * 例如：0 可能是视频流，1 可能是音频流
     */
    int EyerAVPacket::GetStreamIndex()
    {
        return piml->packet->stream_index;
    }

    /**
     * @brief 设置数据包所属的流索引
     * @param streamIndex 流索引（从 0 开始）
     * @return 0 表示成功
     *
     * 在写入输出文件前，需要设置正确的流索引
     * 流索引由输出文件的流顺序决定
     */
    int EyerAVPacket::SetStreamIndex(int streamIndex)
    {
        piml->packet->stream_index = streamIndex;
        return 0;
    }

    /**
     * @brief 时间戳重缩放（转换时间基准）
     * @param _codecTimebase 编解码器时间基准（源时间基准）
     * @param _streamTimebase 流时间基准（目标时间基准）
     * @return 0 表示成功
     *
     * 时间戳重缩放流程：
     * 1. 当不同流的时间基准不同时，需要重缩放时间戳
     * 2. 转换公式：new_pts = old_pts × (src.num/src.den) / (dst.num/dst.den)
     * 3. 同时转换 PTS、DTS 和 duration
     *
     * 使用场景：
     * - 将编码器时间戳转换为输出文件流时间戳
     * - 从一个容器格式转换到另一个容器格式
     *
     * 示例：
     * - 编码器时间基准：1/1000（毫秒）
     * - 输出流时间基准：1/30000（MPEG-TS 常用）
     * - PTS=1000 转换后为 30000
     */
    int EyerAVPacket::RescaleTs(const EyerAVRational & _codecTimebase, const EyerAVRational & _streamTimebase)
    {
        // 将 EyerAVRational 转换为 FFmpeg 的 AVRational
        AVRational codecTimebase;
        codecTimebase.num = _codecTimebase.num;
        codecTimebase.den = _codecTimebase.den;

        AVRational streamTimebase;
        streamTimebase.num = _streamTimebase.num;
        streamTimebase.den = _streamTimebase.den;

        // 调用 FFmpeg 的时间戳重缩放函数
        av_packet_rescale_ts(piml->packet, codecTimebase, streamTimebase);

        return 0;
    }

    /**
     * @brief 获取数据包的数据大小
     * @return 数据大小（字节）
     *
     * 返回编码后的音视频压缩数据的字节数
     * 例如：H.264 编码后的一帧数据可能是几千到几万字节
     */
    int EyerAVPacket::GetSize()
    {
        return piml->packet->size;
    }

    /**
     * @brief 获取数据包的数据指针
     * @return 数据指针（指向压缩数据）
     *
     * 返回指向编码后音视频数据的指针
     * 警告：不要手动释放该指针，由 AVPacket 管理生命周期
     */
    uint8_t * EyerAVPacket::GetDatePtr()
    {
        return piml->packet->data;
    }

    /**
     * @brief 获取附加数据（side data）的大小
     * @return 附加数据大小（字节）
     *
     * 附加数据用于存储额外的元信息，大小因类型而异
     */
    int EyerAVPacket::GetSideSize()
    {
        return piml->packet->side_data->size;
    }

    /**
     * @brief 获取附加数据（side data）指针
     * @return 附加数据指针
     *
     * 附加数据用于存储额外的元信息，例如：
     * - 字幕数据
     * - 音频通道映射
     * - 加密信息
     * - 色彩空间信息
     * - Matroska 块附加数据（AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL）
     */
    uint8_t * EyerAVPacket::GetSideDatePtr()
    {
        if(AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL == piml->packet->side_data->type)
            EyerLog("piml->packet->side_data->type: %d\n", piml->packet->side_data->type);  // 调试日志：输出附加数据类型
        return piml->packet->side_data->data;
    }

    /**
     * @brief 获取以秒为单位的显示时间戳
     * @return 显示时间（秒）
     *
     * 返回预先计算并缓存的秒级时间戳
     * 该值由读取器或其他组件设置，避免重复计算
     */
    double EyerAVPacket::GetSecPTS()
    {
        return piml->secPTS;
    }

    /**
     * @brief 设置空数据包标志
     * @return 0 表示成功
     *
     * 空数据包用于标记特殊状态，例如：
     * - 流结束标志
     * - 编码器刷新缓冲区
     * - 流切换标记
     */
    int EyerAVPacket::SetPKGNULLFlag()
    {
        piml->nullFlag = true;
        return 0;
    }

    /**
     * @brief 检查是否为空数据包
     * @return true 表示空数据包，false 表示正常数据包
     *
     * 用于判断数据包是否为特殊的空包标记
     */
    bool EyerAVPacket::IsNullPKG()
    {
        return piml->nullFlag;
    }
}