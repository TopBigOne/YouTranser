#include "EyerAVTranscodeParams.hpp"

namespace Eyer
{
    /**
     * @brief 默认构造函数
     *
     * 使用成员变量的默认初始化值创建转码参数对象：
     * - targetWidth = 0（保持原始宽度）
     * - targetHeight = 0（保持原始高度）
     * - fps = 30.0（默认帧率）
     * - careVideo = true（处理视频流）
     * - careAudio = true（处理音频流）
     */
    EyerAVTranscodeParams::EyerAVTranscodeParams()
    {

    }

    /**
     * @brief 拷贝构造函数
     * @param params 要拷贝的源参数对象
     *
     * 通过调用赋值操作符复制源对象的所有参数
     */
    EyerAVTranscodeParams::EyerAVTranscodeParams(const EyerAVTranscodeParams & params)
    {
        *this = params;
    }

    /**
     * @brief 赋值操作符重载
     * @param params 要赋值的源参数对象
     * @return 当前对象的引用
     *
     * 复制所有转码参数：
     * - 目标宽度和高度
     * - 目标帧率
     * - 音视频流处理标志
     */
    EyerAVTranscodeParams & EyerAVTranscodeParams::operator = (const EyerAVTranscodeParams & params)
    {
        targetWidth     = params.targetWidth;    // 复制目标宽度
        targetHeight    = params.targetHeight;   // 复制目标高度
        fps             = params.fps;            // 复制目标帧率
        careVideo       = params.careVideo;      // 复制视频处理标志
        careAudio       = params.careAudio;      // 复制音频处理标志
        return *this;
    }
}