#include "EyerTime.hpp"

#include <time.h>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>


namespace Eyer
{
    /**
     * @brief 获取当前时间戳（毫秒）
     * @return 自 1970-01-01 00:00:00 UTC 以来的毫秒数
     *
     * 使用 C++ chrono 库获取系统当前时间，精度为毫秒
     * 适用于时间测量、日志记录等场景
     */
    long long EyerTime::GetTime()
    {
        // 获取系统当前时间点，精度为毫秒
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        // 计算自 epoch 以来的毫秒数
	    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	    return (long long)tmp.count();
    }

    /**
     * @brief 线程休眠指定毫秒数
     * @param time 休眠时长（毫秒）
     * @return 0 表示成功
     *
     * 使当前线程暂停执行指定的毫秒数
     * 使用 C++ 标准库的 sleep_for 实现跨平台睡眠
     */
    int EyerTime::EyerSleepMilliseconds(int time)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        return 0;
    }

    /**
     * @brief 获取当前时间戳（纳秒）
     * @return 自 1970-01-01 00:00:00 UTC 以来的纳秒数
     *
     * 使用 C++ chrono 库获取系统当前时间，精度为纳秒
     * 适用于高精度时间测量场景
     */
    long long EyerTime::GetTimeNano()
    {
        // 获取系统当前时间点，精度为纳秒
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> tp = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
        // 计算自 epoch 以来的纳秒数
        auto tmp = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch());
        return (long long)tmp.count();
    }

    /**
     * @brief 将毫秒转换为时间字符串格式
     * @param milliseconds 毫秒数
     * @return 格式化的时间字符串 "HH:MM:SS,mmm"
     *
     * 将毫秒数转换为易读的时间格式，常用于字幕文件（如 SRT）
     * 示例：3661000 毫秒 -> "01:01:01,000"
     */
    EyerString EyerTime::Milliseconds_to_time(int milliseconds)
    {
        // 计算小时数
        int hours = milliseconds / (1000 * 60 * 60);
        // 计算分钟数（去掉小时部分）
        int minutes = (milliseconds % (1000 * 60 * 60)) / (1000 * 60);
        // 计算秒数（去掉分钟部分）
        int seconds = (milliseconds % (1000 * 60)) / 1000;
        // 计算剩余毫秒数
        int milliseconds_remainder = milliseconds % 1000;

        // 使用字符串流格式化输出
        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << hours << ":"      // 两位数小时，不足补0
           << std::setw(2) << std::setfill('0') << minutes << ":"    // 两位数分钟，不足补0
           << std::setw(2) << std::setfill('0') << seconds << ","    // 两位数秒，不足补0
           << std::setw(3) << std::setfill('0') << milliseconds_remainder;  // 三位数毫秒，不足补0

        return EyerString(ss.str());
    }

}