#ifndef EYERLIB_EYERAVTRANSCODEPARAMS_HPP
#define EYERLIB_EYERAVTRANSCODEPARAMS_HPP

namespace Eyer
{
    /**
     * @brief 音视频转码参数类
     *
     * 用于配置音视频转码的各种参数，包括目标分辨率、帧率以及是否处理音视频流
     */
    class EyerAVTranscodeParams
    {
    public:
        /**
         * @brief 默认构造函数
         *
         * 使用默认参数初始化转码配置
         */
        EyerAVTranscodeParams();

        /**
         * @brief 拷贝构造函数
         * @param params 要拷贝的源参数对象
         *
         * 从另一个参数对象复制所有转码配置
         */
        EyerAVTranscodeParams(const EyerAVTranscodeParams & params);

        /**
         * @brief 赋值操作符重载
         * @param params 要赋值的源参数对象
         * @return 当前对象的引用
         *
         * 将另一个参数对象的所有配置复制到当前对象
         */
        EyerAVTranscodeParams & operator = (const EyerAVTranscodeParams & params);

        // ========== 视频参数 ==========

        int targetWidth = 0;    ///< 目标视频宽度（像素），0 表示保持原始宽度
        int targetHeight = 0;   ///< 目标视频高度（像素），0 表示保持原始高度
        float fps = 30.0;       ///< 目标帧率（帧/秒），默认 30fps

        // ========== 流处理标志 ==========

        bool careVideo = true;  ///< 是否处理视频流，true 表示转码视频，false 表示忽略视频
        bool careAudio = true;  ///< 是否处理音频流，true 表示转码音频，false 表示忽略音频
    };
}

#endif //EYERLIB_EYERAVTRANSCODEPARAMS_HPP
