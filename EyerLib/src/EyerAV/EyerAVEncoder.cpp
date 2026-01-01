#include "EyerAVEncoder.hpp"

#include "EyerAVEncoderPrivate.hpp"
#include "EyerAVPacketPrivate.hpp"
#include "EyerAVFramePrivate.hpp"

#include "EyerCore/EyerCore.hpp"

#include "EyerAVADTSUtil.hpp"

namespace Eyer {
    /**
     * @brief 构造函数 - 创建音视频编码器
     *
     * 初始化私有数据结构
     */
    EyerAVEncoder::EyerAVEncoder() {
        piml = new EyerAVEncoderPrivate();
    }

    /**
     * @brief 析构函数 - 释放编码器资源
     *
     * 释放流程：
     * 1. 如果编码器已打开，关闭编码器
     * 2. 释放编解码器上下文
     * 3. 释放私有数据结构
     */
    EyerAVEncoder::~EyerAVEncoder() {
        if (piml->codecContext != nullptr) {
            if (avcodec_is_open(piml->codecContext)) {
                avcodec_close(piml->codecContext);  // 关闭编码器
            }
            avcodec_free_context(&piml->codecContext);  // 释放上下文
            piml->codecContext = nullptr;
        }

        if (piml != nullptr) {
            delete piml;
            piml = nullptr;
        }
    }

    /**
     * @brief 初始化编码器
     * @param param 编码器参数（编解码器类型、分辨率、时间基准、采样率等）
     * @return 0 表示成功，非 0 表示失败
     *
     * 支持的编解码器：
     *
     * 视频编码器：
     * - H264 (libx264)：主流视频编码器，支持 CRF 质量控制
     * - H265 (libx265)：高效视频编码器，比 H264 压缩率更高
     * - VP8：Google 的开源视频编码器
     * - VP9：Google 的高效开源视频编码器
     * - MJPEG：Motion JPEG，每帧独立压缩
     * - PNG：无损图像编码器
     * - ProRes：Apple 的高质量视频编码器
     *
     * 音频编码器：
     * - AAC (libfdk_aac)：高质量音频编码器
     * - Opus (libopus)：现代低延迟音频编码器
     * - MP3：经典音频编码器
     * - FLAC：无损音频编码器
     * - PCM_S16LE：16 位小端 PCM（无压缩）
     * - PCM_S32LE：32 位小端 PCM（无压缩）
     *
     * 字幕编码器：
     * - SRT：SubRip 字幕格式
     *
     * 初始化流程：
     * 1. 根据编解码器类型查找对应的 FFmpeg 编码器
     * 2. 分配编解码器上下文
     * 3. 设置编码参数（分辨率、像素格式、时间基准等）
     * 4. 打开编码器
     */
    int EyerAVEncoder::Init(const EyerAVEncoderParam &param) {
        // 打印初始化日志，记录编解码器 ID、宽度和高度信息
        EyerLog("EyerAVEncoder::Init: %s, w: %d, h: %d\n", param.codecId.GetDescName().c_str(), param.width, param.height);

        // 创建一个空的 FFmpeg 字典，用于存储编码器的额外选项（如 CRF、预设等）
        AVDictionary *dict = NULL;

        // 声明编解码器指针，用于指向查找到的编码器
        const AVCodec *codec = nullptr;

        // ========== H.264 编码器配置 ==========
        // 检查用户是否请求 H.264 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_H264) {
            // 通过名称查找 libx264 软件编码器（第三方库）
            // 注释掉的代码是通过 ID 查找，但使用名称查找可以指定具体的编码器实现
            codec = avcodec_find_encoder_by_name("libx264");

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败（已存在的上下文）
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为视频（区别于音频、字幕等）
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置像素格式（如 YUV420P、RGB24 等），从参数对象获取 FFmpeg 对应的格式 ID
            piml->codecContext->pix_fmt = (AVPixelFormat)param.pixelFormat.GetFFmpegId();
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;
            // 设置编码线程数，用于多线程并行编码以提高性能
            piml->codecContext->thread_count = param.threadnum;

            // 设置时间基准的分母（时间基准表示每个时间戳单位对应的实际时间）
            // 例如：时间基准 1/25 表示每个时间戳单位是 1/25 秒（25 FPS）
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;

            // 向字典中设置 CRF（Constant Rate Factor）参数
            // CRF 控制视频质量（0-51）：0 为无损，23 为默认，51 为最差质量
            // 将整数 CRF 值转换为字符串后存入字典
            av_dict_set( &dict, "crf", EyerString::Number(param.crf).c_str(), 0);
        }


        // ========== H.265/HEVC 编码器配置 ==========
        // 检查用户是否请求 H.265/HEVC 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_H265) {
            // 通过名称查找 libx265 软件编码器（x265 是开源的 H.265 编码器）
            codec = avcodec_find_encoder_by_name("libx265");
            // 注释掉的代码：可选使用 macOS 的 VideoToolbox 硬件加速编码器
            // codec = avcodec_find_encoder_by_name("hevc_videotoolbox");

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置全局质量参数（使用量化参数 QP）
            // FF_QP2LAMBDA 是将 QP 值转换为 lambda 值的宏
            // 75 表示 QP 值，数值越大压缩率越高但质量越低
            piml->codecContext->global_quality = FF_QP2LAMBDA * 75;
            // 启用质量缩放标志，告诉编码器使用基于质量的编码模式
            piml->codecContext->flags |= AV_CODEC_FLAG_QSCALE;

            // 设置编解码器类型为视频
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置编解码器标签为 'hvc1'（H.265 的 FourCC 代码）
            // MKTAG 宏将 4 个字符转换为 32 位整数标签
            piml->codecContext->codec_tag = MKTAG('h', 'v', 'c', '1');
            // 设置像素格式（从参数对象获取）
            piml->codecContext->pix_fmt = (AVPixelFormat)param.pixelFormat.GetFFmpegId();
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;
            // 设置编码线程数，用于多线程并行编码
            piml->codecContext->thread_count = param.threadnum;

            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;

            // 向字典中设置 CRF（Constant Rate Factor）参数
            // 这会覆盖上面的 global_quality 设置，使用 CRF 模式进行质量控制
            av_dict_set( &dict, "crf", EyerString::Number(param.crf).c_str(), 0);
        }


        // ========== VP8 编码器配置（Google WebM 视频编码器）==========
        // 检查用户是否请求 VP8 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_VP8) {
            // 通过编解码器 ID 查找 VP8 编码器
            codec = avcodec_find_encoder(AV_CODEC_ID_VP8);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为视频
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置像素格式（从参数对象获取）
            piml->codecContext->pix_fmt = (AVPixelFormat)param.pixelFormat.GetFFmpegId();
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;

            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;
        }


        // ========== VP9 编码器配置（Google 高效视频编码器）==========
        // 检查用户是否请求 VP9 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_VP9) {
            // 通过编解码器 ID 查找 VP9 编码器
            codec = avcodec_find_encoder(AV_CODEC_ID_VP9);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为视频
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置像素格式（从参数对象获取）
            piml->codecContext->pix_fmt = (AVPixelFormat)param.pixelFormat.GetFFmpegId();
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;

            // 设置编码线程数为 32，VP9 编码器可以利用多线程显著提高编码速度
            piml->codecContext->thread_count = 32;
            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;
        }

        // ========== MJPEG 编码器配置（Motion JPEG，每帧独立 JPEG 压缩）==========
        // 检查用户是否请求 MJPEG 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_MJPEG) {
            // 通过编解码器 ID 查找 MJPEG 编码器
            codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为视频
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置像素格式为 YUVJ420P（JPEG 使用的全范围 YUV 420 色彩空间）
            // 'J' 表示 JPEG 风格的全范围（0-255），而非标准视频的有限范围（16-235）
            piml->codecContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;

            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;
        }

        // ========== PNG 编码器配置（无损图像编码器）==========
        // 检查用户是否请求 PNG 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_PNG) {
            // 通过编解码器 ID 查找 PNG 编码器
            codec = avcodec_find_encoder(AV_CODEC_ID_PNG);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为视频
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置像素格式（从参数对象获取）
            piml->codecContext->pix_fmt = (AVPixelFormat)param.pixelFormat.GetFFmpegId();
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;

            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;
        }

        // ========== AAC 音频编码器配置（高质量音频编码器）==========
        // 检查用户是否请求 AAC 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_AAC) {
            // 注释掉的代码：通过 ID 查找可能找到内置的 AAC 编码器
            // codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            // 通过名称查找 libfdk_aac 编码器（FDK-AAC 是 Fraunhofer 的高质量 AAC 编码器）
            codec = avcodec_find_encoder_by_name("libfdk_aac");

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为音频
            piml->codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
            // 设置采样格式（如 S16、FLTP 等），从参数对象获取 FFmpeg 格式 ID
            // 采样格式决定了每个音频采样点的数据类型和排列方式
            piml->codecContext->sample_fmt = (AVSampleFormat)param.sampleFormat.ffmpegId;

            // 设置采样率（单位：Hz），常见值有 44100、48000 等
            // 采样率决定了每秒钟采样的次数，影响音频的频率范围
            piml->codecContext->sample_rate = param.sample_rate;

            // 设置声道布局（如立体声、5.1 环绕声等），从参数对象获取 FFmpeg 布局 ID
            piml->codecContext->channel_layout = param.channelLayout.GetFFmpegId();
            // 根据声道布局计算声道数量（如立体声为 2，5.1 环绕声为 6）
            piml->codecContext->channels = av_get_channel_layout_nb_channels(piml->codecContext->channel_layout);

            // 设置音频时间基准的分母为采样率
            // 对于音频，通常使用采样率作为时间基准，这样每个采样点对应一个时间戳单位
            piml->codecContext->time_base.den = param.sample_rate;
            // 设置音频时间基准的分子为 1
            piml->codecContext->time_base.num = 1;
        }

        // ========== Opus 音频编码器配置（现代低延迟音频编码器）==========
        // 检查用户是否请求 Opus 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_LIB_OPUS) {
            // 注释掉的代码：这里原本可能是复制粘贴的注释
            // codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            // 通过名称查找 libopus 编码器（Opus 是现代的开源低延迟音频编码器）
            codec = avcodec_find_encoder_by_name("libopus");

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为音频
            piml->codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
            // 设置采样格式（从参数对象获取）
            piml->codecContext->sample_fmt = (AVSampleFormat)param.sampleFormat.ffmpegId;

            // 设置采样率（单位：Hz）
            piml->codecContext->sample_rate = param.sample_rate;

            // 设置声道布局（从参数对象获取）
            piml->codecContext->channel_layout = param.channelLayout.GetFFmpegId();
            // 根据声道布局计算声道数量
            piml->codecContext->channels = av_get_channel_layout_nb_channels(piml->codecContext->channel_layout);

            // 设置音频时间基准的分母为采样率
            piml->codecContext->time_base.den = param.sample_rate;
            // 设置音频时间基准的分子为 1
            piml->codecContext->time_base.num = 1;
        }

        // ========== MP3 音频编码器配置（经典音频编码器）==========
        // 检查用户是否请求 MP3 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_MP3) {
            // 通过编解码器 ID 查找 MP3 编码器
            codec = avcodec_find_encoder(AV_CODEC_ID_MP3);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为音频
            piml->codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
            // 设置采样格式（从参数对象获取）
            piml->codecContext->sample_fmt = (AVSampleFormat)param.sampleFormat.ffmpegId;

            // 设置采样率（单位：Hz）
            piml->codecContext->sample_rate = param.sample_rate;

            // 设置声道布局（从参数对象获取）
            piml->codecContext->channel_layout = param.channelLayout.GetFFmpegId();
            // 根据声道布局计算声道数量
            piml->codecContext->channels = av_get_channel_layout_nb_channels(piml->codecContext->channel_layout);
        }

        // ========== FLAC 音频编码器配置（无损音频编码器）==========
        // 检查用户是否请求 FLAC 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_FLAC) {
            // 通过编解码器 ID 查找 FLAC 编码器
            codec = avcodec_find_encoder(AV_CODEC_ID_FLAC);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为音频
            piml->codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
            // 设置采样格式（从参数对象获取）
            piml->codecContext->sample_fmt = (AVSampleFormat)param.sampleFormat.ffmpegId;

            // 设置采样率（单位：Hz）
            piml->codecContext->sample_rate = param.sample_rate;

            // 设置声道布局（从参数对象获取）
            piml->codecContext->channel_layout = param.channelLayout.GetFFmpegId();
            // 根据声道布局计算声道数量
            piml->codecContext->channels = av_get_channel_layout_nb_channels(piml->codecContext->channel_layout);
        }


        // ========== PCM_S16LE 音频编码器配置（16 位小端 PCM，无压缩）==========
        // 检查用户是否请求 PCM_S16LE 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_PCM_S16LE) {
            // 通过编解码器 ID 查找 PCM_S16LE 编码器
            // PCM（Pulse Code Modulation）是未压缩的原始音频数据
            // S16LE 表示：Signed 16-bit Little-Endian（有符号 16 位小端序）
            codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为音频
            piml->codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
            // 设置采样格式（从参数对象获取）
            piml->codecContext->sample_fmt = (AVSampleFormat)param.sampleFormat.ffmpegId;

            // 设置采样率（单位：Hz）
            piml->codecContext->sample_rate = param.sample_rate;

            // 设置声道布局（从参数对象获取）
            piml->codecContext->channel_layout = param.channelLayout.GetFFmpegId();
            // 根据声道布局计算声道数量
            piml->codecContext->channels = av_get_channel_layout_nb_channels(piml->codecContext->channel_layout);
        }

        // ========== PCM_S32LE 音频编码器配置（32 位小端 PCM，无压缩）==========
        // 检查用户是否请求 PCM_S32LE 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_PCM_S32LE) {
            // 通过编解码器 ID 查找 PCM_S32LE 编码器
            // S32LE 表示：Signed 32-bit Little-Endian（有符号 32 位小端序）
            // 相比 S16LE 有更高的音频精度和动态范围
            codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S32LE);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);

            // 设置编解码器类型为音频
            piml->codecContext->codec_type = AVMEDIA_TYPE_AUDIO;
            // 设置采样格式（从参数对象获取）
            piml->codecContext->sample_fmt = (AVSampleFormat)param.sampleFormat.ffmpegId;

            // 设置采样率（单位：Hz）
            piml->codecContext->sample_rate = param.sample_rate;

            // 设置声道布局（从参数对象获取）
            piml->codecContext->channel_layout = param.channelLayout.GetFFmpegId();
            // 根据声道布局计算声道数量
            piml->codecContext->channels = av_get_channel_layout_nb_channels(piml->codecContext->channel_layout);
        }



        // ========== ProRes 编码器配置（Apple 高质量视频编码器）==========
        // 检查用户是否请求 ProRes 编码器
        if (param.codecId == EyerAVCodecID::CODEC_ID_PRORES) {
            // 通过编解码器 ID 查找 ProRes 编码器
            // ProRes 是 Apple 的专业视频编码格式，广泛用于视频后期制作
            codec = avcodec_find_encoder(AV_CODEC_ID_PRORES);

            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;

                // 返回错误码 -1，表示初始化失败
                return -1;
            }

            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);
            // 设置编解码器类型为视频
            piml->codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
            // 设置像素格式（从参数对象获取）
            piml->codecContext->pix_fmt = (AVPixelFormat)param.pixelFormat.GetFFmpegId();
            // 设置视频宽度（单位：像素）
            piml->codecContext->width = param.width;
            // 设置视频高度（单位：像素）
            piml->codecContext->height = param.height;
            // 设置编码线程数，用于多线程并行编码
            piml->codecContext->thread_count = param.threadnum;

            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;
        }

        // ========== SRT 字幕编码器配置（SubRip 字幕格式）==========
        // 检查用户是否请求 SRT 字幕编码器
        if(param.codecId == EyerAVCodecID::CODEC_ID_SRT){
            // 通过编解码器 ID 查找 SRT 编码器
            // SRT（SubRip）是常见的文本字幕格式
            codec = avcodec_find_encoder(AV_CODEC_ID_SRT);
            // 检查编码器上下文是否已存在（防止重复初始化）
            if (piml->codecContext != nullptr) {
                // 如果编码器已打开，先关闭它
                if (avcodec_is_open(piml->codecContext)) {
                    avcodec_close(piml->codecContext);
                }
                // 释放编码器上下文内存
                avcodec_free_context(&piml->codecContext);
                // 将指针置空
                piml->codecContext = nullptr;
                // 返回错误码 -1，表示初始化失败
                return -1;
            }
            // 为找到的编解码器分配一个新的编码器上下文结构体
            piml->codecContext = avcodec_alloc_context3(codec);
            // 设置编解码器类型为字幕
            piml->codecContext->codec_type = AVMEDIA_TYPE_SUBTITLE;

            // 设置时间基准的分母
            piml->codecContext->time_base.den = param.timebase.den;
            // 设置时间基准的分子
            piml->codecContext->time_base.num = param.timebase.num;
        }

        // 使用找到的编解码器和可选参数字典打开编码器
        // dict 包含之前设置的额外选项（如 CRF 参数）
        // 打开后编码器就可以接受帧并输出编码后的数据包
        int ret = avcodec_open2(piml->codecContext, codec, &dict);
        // 打印编码器打开结果日志，记录编解码器 ID、分辨率和返回值
        // 返回值 0 表示成功，负数表示失败
        EyerLog("avcodec_open2 param.codecId: %s, w: %d, h: %d, %d\n", param.codecId.GetDescName().c_str(), param.width, param.height, ret);

        // 返回 avcodec_open2 的结果（0 表示成功，负数表示失败）
        return ret;
    }

    /**
     * @brief 获取编码器的帧大小
     * @return 每帧的采样数（音频编码器）
     *
     * 对于音频编码器，返回编码器要求的每帧采样数
     * 对于视频编码器，此值通常为 0
     */
    int EyerAVEncoder::GetFrameSize()
    {
        return piml->codecContext->frame_size;
    }

    /**
     * @brief 发送帧到编码器
     * @param frame 要编码的音视频帧
     * @return 0 表示成功，负数表示失败
     *
     * 将帧送入编码器进行编码
     * 编码器可能缓冲帧，需要调用 RecvPacket() 获取编码后的数据包
     */
    int EyerAVEncoder::SendFrame(EyerAVFrame &frame)
    {
        // frame.piml->frame->pict_type = AV_PICTURE_TYPE_I;
        return avcodec_send_frame(piml->codecContext, frame.piml->frame);
    }

    /**
     * @brief 发送 NULL 帧到编码器（表示编码结束）
     * @return 0 表示成功，负数表示失败
     *
     * 发送 NULL 帧通知编码器不再有新帧输入
     * 之后应调用 RecvPacket() 清空编码器缓冲区
     */
    int EyerAVEncoder::SendFrameNull()
    {
        return avcodec_send_frame(piml->codecContext, nullptr);
    }

    /**
     * @brief 从编码器接收编码后的数据包
     * @param packet 输出参数，存储编码后的数据包
     * @return 0 表示成功，负数表示需要更多帧或错误
     *
     * 从编码器获取编码后的数据包
     * 如果返回 AVERROR(EAGAIN)，表示需要发送更多帧
     */
    int EyerAVEncoder::RecvPacket(EyerAVPacket &packet)
    {
        return avcodec_receive_packet(piml->codecContext, packet.piml->packet);
    }

    /**
     * @brief 获取编码器的时间基准（引用版本）
     * @param timebase 输出参数，存储时间基准
     * @return 0 表示成功
     *
     * 时间基准用于时间戳和实际时间的转换
     * 实际时间（秒）= 时间戳 * timebase.num / timebase.den
     */
    int EyerAVEncoder::GetTimebase(EyerAVRational &timebase)
    {
        timebase.num = piml->codecContext->time_base.num;
        timebase.den = piml->codecContext->time_base.den;
        return 0;
    }

    /**
     * @brief 获取编码器的时间基准（返回值版本）
     * @return EyerAVRational 时间基准对象
     */
    EyerAVRational EyerAVEncoder::GetTimebase()
    {
        EyerAVRational timebase;
        GetTimebase(timebase);
        return timebase;
    }

    /**
     * @brief 获取编码器的媒体类型
     * @return 媒体类型（音频、视频、字幕等）
     */
    EyerAVMediaType EyerAVEncoder::GetMediaType()
    {
        return EyerAVMediaType::GetMediaTypeByFFmpegId(piml->codecContext->codec_type);
    }

    /**
     * @brief 获取编码器的声道布局
     * @return 声道布局（立体声、5.1 声道等）
     *
     * 仅对音频编码器有效
     */
    EyerAVChannelLayout EyerAVEncoder::GetChannelLayout()
    {
        return EyerAVChannelLayout::GetByFFmpegId(piml->codecContext->channel_layout);
    }

    /**
     * @brief 获取编码器的采样格式
     * @return 采样格式（S16、FLTP 等）
     *
     * 仅对音频编码器有效
     */
    EyerAVSampleFormat EyerAVEncoder::GetSampleFormat()
    {
        return EyerAVSampleFormat::GetByFFmpegId(piml->codecContext->sample_fmt);
    }

    /**
     * @brief 获取编码器的采样率
     * @return 采样率（Hz）
     *
     * 仅对音频编码器有效，如 44100、48000 等
     */
    int EyerAVEncoder::GetSampleRate()
    {
        return piml->codecContext->sample_rate;
    }

    /**
     * @brief 获取 ADTS 头信息（用于 AAC 流式传输）
     * @param packetSize 数据包大小
     * @return ADTS 头对象
     *
     * 生成 AAC ADTS 头，用于 AAC 数据的流式传输
     * ADTS（Audio Data Transport Stream）是 AAC 的一种封装格式
     */
    EyerAVADTS EyerAVEncoder::GetADTS(int packetSize)
    {
        EyerAVADTSUtil adtsUtil;
        EyerAVADTS avadts = adtsUtil.GetADTS(piml->codecContext->extradata, piml->codecContext->extradata_size, packetSize);

        return avadts;
    }

    /**
     * @brief 获取编码器的编解码器类型
     * @return 媒体类型（音频、视频、字幕等）
     *
     * 与 GetMediaType() 功能相同
     */
    EyerAVMediaType EyerAVEncoder::GetCodecType()
    {
        return EyerAVMediaType::GetMediaTypeByFFmpegId(piml->codecContext->codec_type);
    }
}
