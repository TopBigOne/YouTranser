#include "EyerAVTranscode.hpp"

#include "EyerAVReader.hpp"
#include "EyerAVWriter.hpp"
#include "EyerAVDecoderBox.hpp"

namespace Eyer
{
    /**
     * @brief 构造函数 - 创建音视频转码器
     * @param _srcPath 源文件路径
     * @param _targetPath 目标文件路径
     * @param _params 转码参数（分辨率、帧率、音视频处理标志）
     *
     * 初始化转码器的所有组件：
     * - 视频和音频读取器
     * - 输出文件写入器
     * - 解码器容器（用于视频帧提取）
     */
    EyerAVTranscode::EyerAVTranscode(const EyerString & _srcPath, const EyerString & _targetPath, const EyerAVTranscodeParams & _params)
        : srcPath(_srcPath)
        , targetPath(_targetPath)
        , params(_params)
        , videoReader(_srcPath)
        , audioReader(_srcPath)
        , writer(_targetPath)
        , decoderBox(_srcPath)
    {

    }

    /**
     * @brief 析构函数 - 释放转码器资源
     *
     * 清理所有读取器、写入器和编解码器资源
     */
    EyerAVTranscode::~EyerAVTranscode()
    {

    }

    /**
     * @brief 执行音视频转码主流程
     * @return 0 表示成功
     *
     * 转码流程：
     * 1. 打开输出文件并初始化编码器
     * 2. 初始化解码器和重采样器
     * 3. 音视频交错转码（每 0.5 秒为一个批次）
     * 4. 清空编码器缓冲区（视频、音频）
     * 5. 写入文件尾并关闭文件
     */
    int EyerAVTranscode::Transcode()
    {
        // 打开输出文件
        writer.Open();

        // 初始化视频和音频编码器
        InitEncoder(writer);

        // 写入文件头（元数据）
        writer.WriteHand();

        // 初始化解码器
        InitDeocder();


        // 如果有音频流，初始化重采样器（转换为立体声 44100Hz FLTP 格式）
        EyerLog("decoderAudioStreamIndex: %d\n", decoderAudioStreamIndex);
        if(decoderAudioStreamIndex >= 0){
            resample.Init(
                    EyerAVChannelLayout::EYER_AV_CH_LAYOUT_STEREO,    // 目标：立体声
                    EyerAVSampleFormat::SAMPLE_FMT_FLTP,              // 目标：浮点平面格式
                    44100,                                             // 目标：44100Hz 采样率

                    audioDecoder.GetAVChannelLayout(),                // 源通道布局
                    audioDecoder.GetAVSampleFormat(),                 // 源采样格式
                    audioDecoder.GetSampleRate()                      // 源采样率
            );
        }

        // 如果有视频流，计算总帧数
        if(decoderVideoStreamIndex >= 0){
            frameCount = videoDuration * params.fps;  // 总帧数 = 时长 × 帧率
            if(frameCount <= 0){
                frameCount = 1;  // 至少 1 帧
            }
        }


        // 进行间隔编码，音视频交错处理（避免单一流过度缓冲）
        long frameOffset = 0;        // 当前处理到的视频帧索引
        double limitTime = 0.0;      // 当前批次的时间限制
        while(1){
            limitTime += 0.5;  // 每次处理 0.5 秒的音视频数据

            int retAudio = -1;
            int retVideo = -1;

            // 处理视频（如果需要）
            if(params.careVideo){
                retVideo = TranscodeVideo(limitTime, frameOffset);
            }

            // 短暂休眠，避免过度占用 CPU
            EyerTime::EyerSleepMilliseconds(5);

            // 处理音频（如果需要）
            if(params.careAudio){
                retAudio = TranscodeAudio(limitTime);
            }

            // 如果音视频都已处理完毕，退出循环
            if(retAudio != 0 && retVideo != 0){
                break;
            }
        }

        // 清空视频编码器缓冲区
        if(params.careVideo){
            if(encoderVideoStreamIndex >= 0){
                videoEncoder.SendFrameNull();  // 发送 NULL 帧，提示编码器结束
                while(1){
                    EyerAVPacket packet;
                    int ret = videoEncoder.RecvPacket(packet);
                    if(ret){
                        break;  // 编码器缓冲区已清空
                    }
                    // 调整时间戳并写入文件
                    packet.SetStreamIndex(encoderVideoStreamIndex);
                    packet.RescaleTs(videoEncoder.GetTimebase(), writer.GetTimebase(encoderVideoStreamIndex));
                    writer.WritePacket(packet);
                }
            }
        }


        // 清空重采样器和音频编码器缓冲区
        if(params.careAudio) {
            if(encoderAudioStreamIndex >= 0) {
                // 提示重采样器结束输入
                resample.PutAVFrameNULL();
                while (1) {
                    EyerAVFrame frame;
                    int ret = resample.GetFrame(frame, audioEncoder.GetFrameSize());
                    if (ret) {
                        break;  // 重采样器缓冲区已清空
                    }
                    // audioEncoder.SendFrame(frame);
                    EyerLog("....Clear Sample\n");
                }
                // 获取重采样器的最后一帧（可能不足帧大小）
                EyerAVFrame frame;
                int ret = resample.GetLastFrame(frame, audioEncoder.GetFrameSize());
                if (!ret) {
                    frame.SetPTS(audioOffset);
                    audioOffset += audioEncoder.GetFrameSize();
                    // audioEncoder.SendFrame(frame);
                    // EyerLog("....Clear Sample WWW\n");
                }

                // 清空音频编码器缓冲区
                audioEncoder.SendFrameNull();  // 发送 NULL 帧，提示编码器结束
                while (1) {
                    EyerAVPacket packet;
                    int ret = audioEncoder.RecvPacket(packet);
                    if (ret) {
                        break;  // 编码器缓冲区已清空
                    }
                    // 调整时间戳并写入文件
                    packet.SetStreamIndex(encoderAudioStreamIndex);
                    packet.RescaleTs(audioEncoder.GetTimebase(), writer.GetTimebase(encoderAudioStreamIndex));
                    writer.WritePacket(packet);
                }
            }
        }

        // 释放解码器资源
        UninitDeocder();

        // 写入文件尾并关闭文件
        writer.WriteTrailer();
        writer.Close();

        return 0;
    }

    /**
     * @brief 初始化解码器
     * @return 0 表示成功
     *
     * 初始化流程：
     * 1. 打开源文件的视频和音频流
     * 2. 查找最佳视频和音频流索引
     * 3. 根据流信息初始化对应的解码器
     * 4. 获取视频流时长（用于计算总帧数）
     */
    int EyerAVTranscode::InitDeocder()
    {
        {
            // 打开视频读取器
            videoReader.Open();
            // 查找最佳视频流
            decoderVideoStreamIndex = videoReader.GetVideoStreamIndex();
            if(decoderVideoStreamIndex >= 0){
                EyerAVStream stream = videoReader.GetStream(decoderVideoStreamIndex);
                // 获取视频时长（秒）
                videoDuration = stream.GetDuration();
                if(videoDuration <= 0){
                    videoDuration = 0;
                }
                // 初始化视频解码器
                videoDeocder.Init(stream);
                // TODO 判断创建失败
            }
        }
        {
            // 打开音频读取器
            audioReader.Open();
            // 查找最佳音频流
            decoderAudioStreamIndex = audioReader.GetAudioStreamIndex();
            if(decoderAudioStreamIndex >= 0){
                EyerAVStream stream = audioReader.GetStream(decoderAudioStreamIndex);
                // 初始化音频解码器
                audioDecoder.Init(stream);
            }
        }
        return 0;
    }

    /**
     * @brief 释放解码器资源
     * @return 0 表示成功
     *
     * 关闭视频和音频读取器，释放相关资源
     */
    int EyerAVTranscode::UninitDeocder()
    {
        videoReader.Close();
        audioReader.Close();
        return 0;
    }

    /**
     * @brief 转码音频流（直到指定时间点）
     * @param limitTime 时间限制（秒），处理到该时间点后返回
     * @return 0 表示还有数据，-1 表示音频流已处理完毕
     *
     * 处理流程：
     * 1. 读取音频数据包，跳过非音频流的包
     * 2. 解码音频数据包为原始音频帧
     * 3. 重采样为目标格式（44100Hz 立体声 FLTP）
     * 4. 编码并写入输出文件
     * 5. 当处理时间超过 limitTime 时返回（实现交错编码）
     * 6. 当文件读取完毕时，清空解码器缓冲区并返回 -1
     */
    int EyerAVTranscode::TranscodeAudio(double limitTime)
    {
        while(1){
            EyerAVPacket packet;
            int ret = audioReader.Read(packet);
            if(ret){
                // 文件读取完毕，清空音频解码器缓冲区
                audioDecoder.SendPacketNull();
                while(1){
                    EyerAVFrame frame;
                    ret = audioDecoder.RecvFrame(frame);
                    if(ret){
                        break;  // 解码器缓冲区已清空
                    }

                    EyerLog("Clear Audio Decoder\n");
                    // EncodeAudio(frame);
                    // 重采样解码后的帧
                    resample.PutAVFrame(frame);
                    while(1){
                        EyerAVFrame frameOut;
                        ret = resample.GetFrame(frameOut, audioEncoder.GetFrameSize());
                        if(ret){
                            break;  // 重采样器缓冲区不足一帧
                        }
                        // 设置 PTS 并编码
                        frameOut.SetPTS(audioOffset);
                        audioOffset += audioEncoder.GetFrameSize();
                        EncodeAudio(frameOut);
                    }
                }
                return -1;  // 音频流处理完毕
            }

            // 跳过非音频流的数据包
            if(packet.GetStreamIndex() != decoderAudioStreamIndex){
                continue;
            }

            // EyerLog("packet: %lld\n", packet.GetPTS());
            // EyerLog("packet sec: %f\n", packet.GetSecPTS());

            double lastFrameTime = 0.0;
            // 将数据包送入解码器
            audioDecoder.SendPacket(packet);
            while(1){
                EyerAVFrame frame;
                ret = audioDecoder.RecvFrame(frame);
                if(ret){
                    break;  // 解码器需要更多数据
                }

                // 重采样为目标格式
                resample.PutAVFrame(frame);
                while(1){
                    EyerAVFrame frameOut;
                    ret = resample.GetFrame(frameOut, audioEncoder.GetFrameSize());
                    if(ret){
                        break;  // 重采样器缓冲区不足一帧
                    }
                    // 设置 PTS 并编码
                    frameOut.SetPTS(audioOffset);
                    audioOffset += audioEncoder.GetFrameSize();
                    EncodeAudio(frameOut);
                }

                // 计算当前处理到的时间（秒）
                lastFrameTime = audioOffset * 1.0 / 44100;
            }
            // 如果处理时间超过限制，返回（实现音视频交错）
            if(lastFrameTime > limitTime){
                return 0;
            }
        }
        return 0;
    }

    /**
     * @brief 转码视频流（直到指定时间点）
     * @param limitTime 时间限制（秒），处理到该时间点后返回
     * @param frameOffset 当前帧索引（引用参数，会被修改）
     * @return 0 表示还有帧，-1 表示视频流已处理完毕
     *
     * 处理流程：
     * 1. 检查是否已处理完所有帧
     * 2. 根据帧索引计算时间戳（PTS）
     * 3. 从解码器容器获取指定时间点的帧
     * 4. 缩放帧到目标分辨率和像素格式（YUV420P）
     * 5. 编码并写入输出文件
     * 6. 当处理时间超过 limitTime 时返回（实现交错编码）
     */
    int EyerAVTranscode::TranscodeVideo(double limitTime, long & frameOffset)
    {
        // 检查是否已处理完所有帧
        if(frameOffset >= frameCount){
            return -1;  // 视频流处理完毕
        }

        while(1){
            // 根据帧索引计算时间戳（秒）
            double pts = frameOffset * 1.0 / params.fps;

            // 从解码器容器获取指定时间点的视频帧
            EyerAVFrame frame;
            int ret = decoderBox.GetFrame(frame, pts);
            if(ret){
                EyerLog("ret: %d\n", ret);
                return -1;  // 获取帧失败，可能已到文件末尾
            }
            /*
            // 可选：转换为 RGBA 并镜像
            EyerAVFrame frameRGBA;
            frame.Scale(frameRGBA, EyerAVPixelFormat::RGBA);

            EyerAVFrame frameRGBAMirror;
            frameRGBA.Mirror(frameRGBAMirror, 2);
            */

            // 缩放帧到目标分辨率和像素格式
            EyerAVFrame distFrame;
            // frame.Scale(distFrame, EyerAVPixelFormat::YUV420P, params.targetWidth, params.targetHeight);
            frame.Scale(distFrame, EyerAVPixelFormat::EYER_YUV420P, params.targetWidth, params.targetHeight);

            // 设置时间戳（毫秒）
            distFrame.SetPTS(pts * 1000);

            // 将帧送入编码器
            videoEncoder.SendFrame(distFrame);
            while (1){
                EyerAVPacket outPacket;
                int ret = videoEncoder.RecvPacket(outPacket);
                if(ret){
                    break;  // 编码器需要更多帧
                }

                // 设置流索引并调整时间戳
                outPacket.SetStreamIndex(encoderVideoStreamIndex);
                outPacket.RescaleTs(videoEncoder.GetTimebase(), writer.GetTimebase(encoderVideoStreamIndex));
                writer.WritePacket(outPacket);
            }

            // 如果处理时间超过限制，返回（实现音视频交错）
            if(pts > limitTime){
                frameOffset++;
                return 0;
            }

            frameOffset++;
        }

        return 0;
    }

    /**
     * @brief 编码并写入音频帧
     * @param frame 要编码的音频帧
     * @return 0 表示成功
     *
     * 处理流程：
     * 1. 将帧送入音频编码器
     * 2. 从编码器接收编码后的数据包
     * 3. 调整时间戳并写入输出文件
     */
    int EyerAVTranscode::EncodeAudio(EyerAVFrame & frame)
    {
        // 将帧送入编码器
        audioEncoder.SendFrame(frame);
        while (1){
            EyerAVPacket packet;
            int ret = audioEncoder.RecvPacket(packet);
            if(ret){
                break;  // 编码器需要更多帧
            }

            // 设置流索引并调整时间戳
            packet.SetStreamIndex(encoderAudioStreamIndex);
            packet.RescaleTs(audioEncoder.GetTimebase(), writer.GetTimebase(encoderAudioStreamIndex));
            writer.WritePacket(packet);
        }
        return 0;
    }

    /**
     * @brief 初始化编码器
     * @param writer 输出文件写入器
     * @return 0 表示成功
     *
     * 初始化流程：
     * 1. 打开源文件读取流信息
     * 2. 如果有视频流且需要处理视频：
     *    - 根据源视频流创建编码器参数
     *    - 设置目标分辨率和时间基准
     *    - 初始化视频编码器并添加到输出文件
     * 3. 如果有音频流且需要处理音频：
     *    - 创建 MP3 音频编码器参数（44100Hz 立体声 FLTP）
     *    - 初始化音频编码器并添加到输出文件
     * 4. 关闭临时读取器
     */
    int EyerAVTranscode::InitEncoder(EyerAVWriter & writer)
    {
        int ret = 0;

        // 临时打开源文件以获取流信息
        EyerAVReader reader(srcPath);
        reader.Open();

        // 查找视频流
        int videoIndex = reader.GetVideoStreamIndex();
        EyerLog("Video Index: %d\n", videoIndex);
        if(videoIndex >= 0){
            // 如果需要处理视频，初始化视频编码器
            if(params.careVideo){
                EyerAVStream stream = reader.GetStream(videoIndex);
                EyerAVEncoderParam videoEncoderParams;
                videoEncoderParams.InitFromStream(stream);  // 从源流继承参数

                // 设置编码时间基准（1/1000 秒，即毫秒）
                EyerAVRational encodeTimebase;
                encodeTimebase.num = 1;
                encodeTimebase.den = 1000;

                videoEncoderParams.SetTimebase(encodeTimebase);
                // 设置目标分辨率
                videoEncoderParams.SetWH(params.targetWidth, params.targetHeight);

                // 初始化视频编码器
                ret = videoEncoder.Init(videoEncoderParams);
                if(ret){
                    EyerLog("Video Encoder Init Fail\n");
                }
                // 将视频流添加到输出文件
                encoderVideoStreamIndex = writer.AddStream(videoEncoder);
            }
        }

        // 查找音频流
        int audioIndex = reader.GetAudioStreamIndex();
        EyerLog("Audio Index: %d\n", audioIndex);
        if(audioIndex >= 0){
            // 如果需要处理音频，初始化音频编码器
            if(params.careAudio) {
                EyerAVEncoderParam audioEncoderParams;
                // 初始化为 MP3 编码器（立体声，44100Hz，浮点平面格式）
                audioEncoderParams.InitMP3(EyerAVChannelLayout::EYER_AV_CH_LAYOUT_STEREO, EyerAVSampleFormat::SAMPLE_FMT_FLTP, 44100);
                ret = audioEncoder.Init(audioEncoderParams);
                if (ret) {
                    EyerLog("Audio Encoder Init Fail\n");
                }
                // 将音频流添加到输出文件
                encoderAudioStreamIndex = writer.AddStream(audioEncoder);
            }
        }
        reader.Close();
        return 0;
    }
}