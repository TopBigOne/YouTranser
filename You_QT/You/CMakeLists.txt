# 设置 CMake 最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# 定义项目名称和版本号
PROJECT(YouPicker VERSION 1.0.0.0)

# 输出项目基本信息
MESSAGE( STATUS "CMAKE_PROJECT_NAME    = ${CMAKE_PROJECT_NAME}" )
MESSAGE( STATUS "PROJECT_VERSION_MAJOR = ${PROJECT_VERSION_MAJOR}" )
MESSAGE( STATUS "PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}" )
MESSAGE( STATUS "PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}" )
MESSAGE( STATUS "PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}" )
MESSAGE( STATUS "PROJECT_HOMEPAGE_URL  = ${PROJECT_HOMEPAGE_URL}")
MESSAGE( STATUS "CMAKE_PROJECT_DESCRIPTION = ${CMAKE_PROJECT_DESCRIPTION}" )

# 为 macOS 设置 Qt 路径
IF(NOT DEFINED CMAKE_PREFIX_PATH)
    IF(APPLE)
        SET(CMAKE_PREFIX_PATH "/Users/dev/Qt/6.9.1/macos")
    ENDIF()
ENDIF()

# 设置第三方库目录名称
SET(THIRD_PART_DIST_DIR "Lib")

# 设置 EyerLib 源码目录路径，用于代码导航和 IDE 支持
SET(EYERLIB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../../EyerLib/src")

# 手动收集 EyerLib 源文件，便于 IDE 导航

# 收集 EyerAV 模块的所有源文件（音视频处理模块）
FILE(GLOB EYERLIB_AV_SOURCES
    "${EYERLIB_SOURCE_DIR}/EyerAV/*.cpp"
    "${EYERLIB_SOURCE_DIR}/EyerAV/*.hpp"
    "${EYERLIB_SOURCE_DIR}/EyerAV/*.h"
    "${EYERLIB_SOURCE_DIR}/EyerAV/*.mm"
)

# 收集 EyerCore 模块的所有源文件（核心基础库）
FILE(GLOB EYERLIB_CORE_SOURCES
    "${EYERLIB_SOURCE_DIR}/EyerCore/*.cpp"
    "${EYERLIB_SOURCE_DIR}/EyerCore/*.hpp"
)

# 收集 EyerThread 模块的所有源文件（线程管理模块）
FILE(GLOB EYERLIB_THREAD_SOURCES
    "${EYERLIB_SOURCE_DIR}/EyerThread/*.cpp"
    "${EYERLIB_SOURCE_DIR}/EyerThread/*.hpp"
)

# 收集 EyerMath 模块的所有源文件（数学计算模块）
FILE(GLOB EYERLIB_MATH_SOURCES
    "${EYERLIB_SOURCE_DIR}/EyerMath/*.cpp"
    "${EYERLIB_SOURCE_DIR}/EyerMath/*.hpp"
)

# 收集 EyerAVTranscoder 模块的所有源文件（音视频转码模块）
FILE(GLOB EYERLIB_TRANSCODER_SOURCES
    "${EYERLIB_SOURCE_DIR}/EyerAVTranscoder/*.cpp"
    "${EYERLIB_SOURCE_DIR}/EyerAVTranscoder/*.hpp"
)

# 合并所有收集到的源文件
SET(EYERLIB_ALL_SOURCES
    ${EYERLIB_AV_SOURCES}
    ${EYERLIB_CORE_SOURCES}
    ${EYERLIB_THREAD_SOURCES}
    ${EYERLIB_MATH_SOURCES}
    ${EYERLIB_TRANSCODER_SOURCES}
)

# 创建用于 IDE 导航的静态库（不参与实际构建）
# 使用 STATIC 库而不是 INTERFACE 库，帮助 IDE 构建符号索引
ADD_LIBRARY(EyerLib_IDE_Navigation STATIC EXCLUDE_FROM_ALL ${EYERLIB_ALL_SOURCES})
TARGET_INCLUDE_DIRECTORIES(EyerLib_IDE_Navigation PRIVATE
    ${EYERLIB_SOURCE_DIR}/EyerAV
    ${EYERLIB_SOURCE_DIR}/EyerCore
    ${EYERLIB_SOURCE_DIR}/EyerThread
    ${EYERLIB_SOURCE_DIR}/EyerMath
    ${EYERLIB_SOURCE_DIR}/EyerAVTranscoder
    ${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/ffmpeg_install/include
)

# 添加 EyerLib 各模块的源码目录到包含路径，用于代码导航
INCLUDE_DIRECTORIES(${EYERLIB_SOURCE_DIR}/EyerAV)
INCLUDE_DIRECTORIES(${EYERLIB_SOURCE_DIR}/EyerCore)
INCLUDE_DIRECTORIES(${EYERLIB_SOURCE_DIR}/EyerThread)
INCLUDE_DIRECTORIES(${EYERLIB_SOURCE_DIR}/EyerMath)
INCLUDE_DIRECTORIES(${EYERLIB_SOURCE_DIR}/EyerAVTranscoder)

# 添加预编译的 EyerLib 和 you_picker 库的头文件目录
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/EyerLib_install/include/)
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/you_picker_install/include/)

# 添加预编译的 EyerLib 和 you_picker 库的链接目录
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/EyerLib_install/lib/)
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/you_picker_install/lib/)

# 添加第三方库的链接目录
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/ffmpeg_install/lib/)     # FFmpeg 音视频处理库
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/openssl_install/lib/)    # OpenSSL 加密库
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/x264_install/lib/)       # x264 视频编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/x265_install/lib/)       # x265/HEVC 视频编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/fdk_aac_install/lib/)    # FDK-AAC 音频编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/lame_install/lib/)       # LAME MP3 编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libvpx_install/lib/)     # VP8/VP9 视频编码器

# 根据不同的操作系统平台进行配置
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Android")
    MESSAGE(STATUS "current platform: Android")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    MESSAGE(STATUS "current platform: Darwin")
    # 为 macOS 设置需要链接的系统框架
    SET(CMAKE_EXE_LINKER_FLAGS
        "-framework QuartzCore -framework Metal -framework IOKit -framework AudioToolbox -framework CoreGraphics -framework CoreMedia -framework Cocoa -framework VideoToolbox -framework AVFoundation -framework Security -framework CoreFoundation -framework CoreVideo -framework OpenGL -framework AppKit -framework CoreImage -framework VideoDecodeAcceleration"
    )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "iOS")
    MESSAGE(STATUS "current platform: iOS")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    MESSAGE(STATUS "current platform: Emscripten")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

# 添加子目录（子项目）
# ADD_SUBDIRECTORY(YouPicker)  # YouPicker 项目已注释
ADD_SUBDIRECTORY(YouCommon)   # 公共组件模块
ADD_SUBDIRECTORY(YouTranser)  # 主转码模块
