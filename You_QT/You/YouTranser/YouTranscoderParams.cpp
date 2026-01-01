/**
 * @file YouTranscoderParams.cpp
 * @brief 转码参数类实现
 * @details 实现 JSON 序列化/反序列化、参数访问器等功能
 */

#include "YouTranscoderParams.hpp"

#include "EyerCore/EyerCore.hpp"

/**
 * @brief 默认构造函数实现
 * @details 使用父类默认构造函数初始化基础参数
 */
YouTranscoderParams::YouTranscoderParams()
{

}

/**
 * @brief 析构函数实现
 */
YouTranscoderParams::~YouTranscoderParams()
{

}

/**
 * @brief 拷贝构造函数实现
 * @param params 要拷贝的参数对象
 * @details 使用赋值运算符实现深拷贝
 */
YouTranscoderParams::YouTranscoderParams(const YouTranscoderParams & params)
{
    *this = params;
}

/**
 * @brief 赋值运算符重载实现
 * @param params 要赋值的参数对象
 * @return 引用自身
 * @details
 * - 调用父类赋值运算符拷贝基础参数
 * - 拷贝本类特有的成员变量
 */
YouTranscoderParams & YouTranscoderParams::operator = (const YouTranscoderParams & params)
{
    Eyer::EyerAVTranscoderParams::operator = (params);  // 调用父类赋值运算符
    filenamePrefix      = params.filenamePrefix;
    transNumSametime    = params.transNumSametime;
    outputDir           = params.outputDir;
    return *this;
}

/**
 * @brief 从 JSON 字符串解析参数实现
 * @param jsonStr JSON 格式的参数字符串
 * @return 0表示成功,-1表示失败
 * @details
 * - 解析 JSON 字符串并验证格式
 * - 检查 EyerLib 版本兼容性
 * - 依次解析所有参数字段并设置到对象中
 * - 如果版本不兼容,放弃加载配置并返回 -1
 */
int YouTranscoderParams::ParseJson(const QString & jsonStr)
{
    QJsonParseError jsonError;
    QJsonDocument doucment = QJsonDocument::fromJson(jsonStr.toUtf8(), &jsonError);
    if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (doucment.isObject())  {
            QJsonObject object = doucment.object();
            // 验证 EyerLib 版本兼容性
            if (object.contains("eyer_lib_version")) {
                QJsonValue value = object.value("eyer_lib_version");
                if(value.isString()){
                    QString eyer_lib_version_str = value.toString();
                    QString currentVersion(Eyer::EyerVersion::GetEyerLibVersion().c_str());
                    if(eyer_lib_version_str != currentVersion){
                        EyerLog("Give up!");  // 版本不兼容,放弃加载配置
                        return -1;
                    }
                }
                else {
                    return -1;
                }
            }
            else {
                return -1;
            }

            // 解析输出文件格式 (MP4/MOV/MKV等)
            if (object.contains("output_file_fmt")) {
                QJsonValue value = object.value("output_file_fmt");
                int avFileFmtId = value.toInt(Eyer::EyerAVFileFmt::MP4.GetId());
                Eyer::EyerAVFileFmt fmt = Eyer::EyerAVFileFmt::GetAVFileFmtById(avFileFmtId);
                EyerLog("AV File fmt: %s\n", fmt.GetDesc().c_str());
                SetOutputFileFmt(fmt);
            }

            // 解析视频编码器 (H.264/H.265/ProRes等)
            if (object.contains("video_codec")) {
                QJsonValue value = object.value("video_codec");
                int codecID = value.toInt(Eyer::EyerAVCodecID::CODEC_ID_H264.GetId());
                Eyer::EyerAVCodecID codec = Eyer::EyerAVCodecID::GetCodecIdById(codecID);
                EyerLog("Video Codec ID: %s\n", codec.GetDescName().c_str());
                SetVideoCodecId(codec);
            }
            // 解析像素格式 (yuv420p/yuv422p10le等)
            if (object.contains("pixel_format")) {
                QJsonValue value = object.value("pixel_format");
                int pixelfmtID = value.toInt(Eyer::EyerAVPixelFormat::EYER_YUV420P.GetId());
                Eyer::EyerAVPixelFormat pixelfmt = Eyer::EyerAVPixelFormat::GetById(pixelfmtID);
                EyerLog("Pixel Format: %s\n", pixelfmt.GetDescName().c_str());
                SetVideoPixelFormat(pixelfmt);
            }
            // 解析视频质量参数 CRF (Constant Rate Factor)
            if (object.contains("video_crf")) {
                QJsonValue value = object.value("video_crf");
                int crf = value.toInt(18);
                EyerLog("CRF: %d\n", crf);
                SetCRF(crf);
            }


            // 解析音频编码器 (AAC/MP3/FLAC等)
            if (object.contains("audio_codec")) {
                QJsonValue value = object.value("audio_codec");
                int codecID = value.toInt(Eyer::EyerAVCodecID::CODEC_ID_MP3.GetId());
                Eyer::EyerAVCodecID codec = Eyer::EyerAVCodecID::GetCodecIdById(codecID);
                EyerLog("Audio Codec ID: %s\n", codec.GetDescName().c_str());
                SetAudioCodecId(codec);
            }
            // 解析音频声道布局 (单声道/立体声/5.1等)
            if (object.contains("audio_channel_layout")) {
                QJsonValue value = object.value("audio_channel_layout");
                int channelLayoutID = value.toInt(Eyer::EyerAVChannelLayout::EYER_AV_CH_LAYOUT_STEREO.GetId());
                Eyer::EyerAVChannelLayout channelLayout = Eyer::EyerAVChannelLayout::GetById(channelLayoutID);
                EyerLog("Channel Layout: %s\n", channelLayout.GetDescName().c_str());
                SetChannelLayout(channelLayout);
            }
            // 解析音频采样率 (44100/48000等)
            if (object.contains("audio_sample_rate")) {
                QJsonValue value = object.value("audio_sample_rate");
                int sampleRate = value.toInt(44100);
                EyerLog("Sample Rate: %d\n", sampleRate);
                SetSampleRate(sampleRate);
            }


            // 解析文件名前缀模板
            if (object.contains("filename_prefix")) {
                QJsonValue value = object.value("filename_prefix");
                QString filenamePrefix = value.toString();
                EyerLog("Filename Prefix: %s\n", filenamePrefix.toStdString().c_str());
                SetFilenamePrefix(filenamePrefix);
            }



            // 解析解码线程数
            if (object.contains("decode_thread_num")) {
                QJsonValue value = object.value("decode_thread_num");
                EyerLog("decode_thread_num: %d\n", value.toInt(4));
                SetDecodeThreadNum(value.toInt(4));
            }
            // 解析编码线程数
            if (object.contains("encode_thread_num")) {
                QJsonValue value = object.value("encode_thread_num");
                EyerLog("encode_thread_num: %d\n", value.toInt(4));
                SetDecodeThreadNum(value.toInt(4));
            }

            // 解析同时进行的任务数
            if (object.contains("trans_num_sametime")) {
                QJsonValue value = object.value("trans_num_sametime");
                EyerLog("trans_num_sametime: %d\n", value.toInt(4));
                SetTransNumSametime(value.toInt(2));
            }


            // 解析输出目录
            if (object.contains("output_dir")) {
                QJsonValue value = object.value("output_dir");
                QString outputDir = value.toString();
                SetOutputDir(outputDir);
            }
        }
    }
    return 0;
}

/**
 * @brief 序列化为 JSON 字符串实现
 * @return JSON 格式的参数字符串
 * @details
 * - 将所有参数转换为 JSON 对象
 * - 包含参数的 ID 和描述信息 (便于调试和查看)
 * - 包含 EyerLib 版本信息 (用于兼容性检查)
 */
QString YouTranscoderParams::ToJson()
{
    QJsonObject jsonObj;
    // 添加 EyerLib 版本信息
    jsonObj.insert("eyer_lib_version", Eyer::EyerVersion::GetEyerLibVersion().c_str());

    // 添加输出文件格式
    jsonObj.insert("output_file_fmt", GetOutputFileFmt().GetId());
    jsonObj.insert("output_file_fmt_desc", GetOutputFileFmt().GetDesc().c_str());

    // 添加视频编码器
    jsonObj.insert("video_codec", GetVideoCodecId().GetId());
    jsonObj.insert("video_codec_desc", GetVideoCodecId().GetDescName().c_str());

    // 添加像素格式
    jsonObj.insert("pixel_format", GetVideoPixelFormat().GetId());
    jsonObj.insert("pixel_format_desc", GetVideoPixelFormat().GetDescName().c_str());

    // 添加视频质量 CRF
    jsonObj.insert("video_crf", GetCRF());

    // 添加音频编码器
    jsonObj.insert("audio_codec", GetAudioCodecId().GetId());
    jsonObj.insert("audio_codec_desc", GetAudioCodecId().GetDescName().c_str());

    // 添加音频声道布局
    jsonObj.insert("audio_channel_layout", GetAudioChannelLayout().GetId());
    jsonObj.insert("audio_channel_layout_desc", GetAudioChannelLayout().GetDescName().c_str());

    // 添加音频采样率
    jsonObj.insert("audio_sample_rate", GetSampleRate());

    // 添加文件名前缀模板
    jsonObj.insert("filename_prefix", GetFilenamePrefix());

    // 添加解码/编码线程数
    jsonObj.insert("decode_thread_num", GetDecodeThreadNum());
    jsonObj.insert("encode_thread_num", GetEncodeThreadNum());

    // 添加同时进行的任务数
    jsonObj.insert("trans_num_sametime", GetTransNumSametime());

    jsonObj.insert("trans_num_sametime", GetTransNumSametime());

    // 添加输出目录
    jsonObj.insert("output_dir", GetOutputDir());

    // 转换为 JSON 字符串
    QJsonDocument doc(jsonObj);
    return doc.toJson();
}

/**
 * @brief 获取文件名前缀模板实现
 * @return 文件名前缀字符串
 */
QString YouTranscoderParams::GetFilenamePrefix()
{
    return filenamePrefix;
}

/**
 * @brief 设置文件名前缀模板实现
 * @param _filenamePrefix 文件名前缀字符串
 * @return 0表示成功
 */
int YouTranscoderParams::SetFilenamePrefix(const QString & _filenamePrefix)
{
    filenamePrefix = _filenamePrefix;
    return 0;
}

/**
 * @brief 获取同时进行的任务数实现
 * @return 并发任务数
 */
const int YouTranscoderParams::GetTransNumSametime() const
{
    return transNumSametime;
}

/**
 * @brief 设置同时进行的任务数实现
 * @param num 并发任务数
 * @return 0表示成功
 */
int YouTranscoderParams::SetTransNumSametime(int num)
{
    transNumSametime = num;
    return 0;
}

/**
 * @brief 获取输出目录实现
 * @return 输出文件夹路径
 */
QString YouTranscoderParams::GetOutputDir() const
{
    return outputDir;
}

/**
 * @brief 设置输出目录实现
 * @param dir 输出文件夹路径
 * @return 0表示成功
 */
int YouTranscoderParams::SetOutputDir(const QString & dir)
{
    outputDir = dir;
    return 0;
}
