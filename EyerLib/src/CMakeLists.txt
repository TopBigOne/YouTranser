# 设置 CMake 最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# 定义 EyerLib 项目名称和版本号
PROJECT(EyerLib VERSION 1.0.0.0)

# 如果未指定构建类型，则默认设置为 Debug
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    MESSAGE(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Debug")
ENDIF()

# 输出构建配置信息
MESSAGE( STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}" )
MESSAGE( STATUS "CMAKE_PROJECT_NAME    = ${CMAKE_PROJECT_NAME}" )
MESSAGE( STATUS "PROJECT_VERSION_MAJOR = ${PROJECT_VERSION_MAJOR}" )
MESSAGE( STATUS "PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}" )
MESSAGE( STATUS "PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}" )
MESSAGE( STATUS "PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}" )
MESSAGE( STATUS "PROJECT_HOMEPAGE_URL = ${PROJECT_HOMEPAGE_URL}")
MESSAGE( STATUS "CMAKE_PROJECT_DESCRIPTION = ${CMAKE_PROJECT_DESCRIPTION}" )

# 设置第三方库目录名称
SET(THIRD_PART_DIST_DIR "Lib")

# 测试资源下载基础 URL
SET(DOWNLOAD_BASE_URL "https://eyer-test.oss-cn-beijing.aliyuncs.com/")

# 是否下载测试资源的标志
SET(IS_THIRD_PART_DIST_DIR "NO")
OPTION(DOWNLOAD_TEST_RES     "option for DOWNLOAD_TEST_RES"     OFF)
IF (DOWNLOAD_TEST_RES)
    SET(IS_THIRD_PART_DIST_DIR "YES")
ENDIF(DOWNLOAD_TEST_RES)

# 是否启用 EyerLib 测试
OPTION(ENABLE_EYER_TEST           "option for ENABLE_EYER_TEST"           ON)

# 根据不同平台进行配置
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    # Windows 平台特定配置
    ADD_DEFINITIONS(-D _CRT_SECURE_NO_WARNINGS)  # 禁用 CRT 安全警告
    ADD_DEFINITIONS(-D NOMINMAX)                 # 禁用 Windows.h 中的 min/max 宏
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Android")
    MESSAGE(STATUS "current platform: Android")
    # Android 平台使用特定的第三方库目录（根据架构）
    SET(THIRD_PART_DIST_DIR "Lib_android_${CMAKE_ANDROID_ARCH_ABI}")
    MESSAGE(STATUS "${THIRD_PART_DIST_DIR}")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    MESSAGE(STATUS "current platform: Darwin")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "iOS")
    MESSAGE(STATUS "current platform: iOS")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    MESSAGE(STATUS "current platform: Emscripten")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

# 设置 C++ 标准为 C++17
SET(CMAKE_CXX_STANDARD 17)

MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

# 添加本地依赖库的头文件目录
INCLUDE_DIRECTORIES (googletest/googletest/include)  # Google Test 测试框架
INCLUDE_DIRECTORIES (EyerGLAD/glad/include/)        # GLAD OpenGL 加载器
INCLUDE_DIRECTORIES (EyerLua/src/)                  # Lua 脚本引擎

# 添加 Qt 库的头文件目录
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/QtLib/QtGui/)
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/QtLib/)

# 添加第三方库的头文件目录
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/glfw_install/include)          # GLFW 窗口库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/curl_install/include)          # cURL 网络库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libxml2_install/include)       # libxml2 XML 解析库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libxml2_install/include/libxml2)
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libosip2_install/include)      # oSIP SIP 协议栈
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libexosip2_install/include)    # eXosip2 SIP 用户代理
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/pjproject_install/include)     # PJSIP 多媒体通信库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/JRTPLIB_install/include)       # JRTPLIB RTP 库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/opencv_install/include/opencv4)  # OpenCV 计算机视觉库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libyuv_install/include)        # libyuv 图像处理库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libpng_install/include)        # libpng 图像库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/openssl_install/include)       # OpenSSL 加密库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/glfw_install/include)          # GLFW（重复）
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/jsoncpp_install/include)       # JsonCpp JSON 库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/freetype_install/include)      # FreeType 字体渲染库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/fdk_aac_install/include/)      # FDK-AAC 音频编码器
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/freetype_install/include/freetype2)
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/skia_install/include/core)     # Skia 2D 图形库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/skia_install/)
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/bgfx_install/include/)         # bgfx 跨平台图形库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/ffmpeg_install/include)        # FFmpeg 音视频处理库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libraw_install/include)        # LibRaw RAW 图像解码库
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/rlottie_install/include)       # rlottie Lottie 动画库
# INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../Lib/ffmpeg-3.3.9_install/include)

# 添加第三方库的链接目录
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libpng_install/lib)        # libpng
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/glfw_install/lib)          # GLFW
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/jsoncpp_install/lib)       # JsonCpp
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/freetype_install/lib)      # FreeType
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/ffmpeg_install/lib)        # FFmpeg
# LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/ffmpeg-3.3.9_install/lib)
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/x264_install/lib)          # x264 编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/x265_install/lib)          # x265 编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/lame_install/lib)          # LAME MP3 编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libvpx_install/lib)        # libvpx VP8/VP9
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libopenh264_install/lib)   # OpenH264 编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/fdk_aac_install/lib)       # FDK-AAC 编码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/openssl_install/lib)       # OpenSSL
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/skia_install/lib)          # Skia
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/bgfx_install/lib)          # bgfx
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libopus_install/lib)       # Opus 音频编解码器
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/libraw_install/lib)        # LibRaw
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/opencv_install/lib)        # OpenCV
LINK_DIRECTORIES (${CMAKE_SOURCE_DIR}/../../${THIRD_PART_DIST_DIR}/rlottie_install/lib)       # rlottie

# 设置位置无关代码（PIC）标志，用于构建共享库
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# 代码覆盖率选项（仅用于 Debug 模式的 Linux 构建）
OPTION(ENABLE_GCOV "Enable gcov (debug, Linux builds only)" OFF)
IF (ENABLE_GCOV AND NOT WIN32 AND NOT APPLE)
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage -lgcov")
ENDIF()

# 根据平台设置特定的编译定义和链接标志
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    ADD_DEFINITIONS(-D EYER_PLATFORM_WINDOWS)  # 定义 Windows 平台宏
    IF(MSVC)
        # MSVC 编译器设置 UTF-8 编码
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    ENDIF (MSVC)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Android")
    MESSAGE(STATUS "current platform: Android")
    ADD_DEFINITIONS(-D EYER_PLATFORM_ANDROID)  # 定义 Android 平台宏
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    MESSAGE(STATUS "current platform: Darwin")
    ADD_DEFINITIONS(-D EYER_PLATFORM_DARWIN)   # 定义 macOS 平台宏
    # 设置 macOS 系统框架链接标志
    set(CMAKE_EXE_LINKER_FLAGS
        "-framework QuartzCore -framework Metal -framework IOKit -framework AudioToolbox -framework CoreGraphics -framework CoreMedia -framework Cocoa -framework VideoToolbox -framework AVFoundation -framework Security -framework CoreFoundation -framework CoreVideo -framework OpenGL -framework AppKit -framework CoreImage -framework VideoDecodeAcceleration"
    )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "iOS")
    MESSAGE(STATUS "current platform: iOS")
    ADD_DEFINITIONS(-D EYER_PLATFORM_IOS)      # 定义 iOS 平台宏
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    MESSAGE(STATUS "current platform: Emscripten")
    ADD_DEFINITIONS(-D EYER_PLATFORM_EMSCRIPTEN)  # 定义 Emscripten 平台宏
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

# 添加 MP4 语法支持定义
ADD_DEFINITIONS(-D EYER_MP4_SYNTAX)

# 定义模块选项函数：根据选项决定是否添加模块
FUNCTION(ModuleOption moduleName moduleOption)
    IF (${moduleOption})
        ADD_SUBDIRECTORY(${moduleName})
    ENDIF(${moduleOption})
ENDFUNCTION(ModuleOption)

# 定义模块测试选项函数：根据选项添加模块及其测试
FUNCTION(ModuleTestOption moduleName moduleTestName moduleOption)
    IF (${moduleOption})
        ADD_SUBDIRECTORY(${moduleName})           # 添加模块本身
        IF(ENABLE_EYER_TEST)
            ADD_SUBDIRECTORY(${moduleTestName})   # 如果启用测试，添加测试目录
        ENDIF(ENABLE_EYER_TEST)
    ENDIF(${moduleOption})
ENDFUNCTION(ModuleTestOption)

# MSVC 编译器运行时库配置
IF(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/MT")   # Release 模式使用静态链接的多线程运行时库
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")    # Debug 模式使用静态链接的多线程调试运行时库
ENDIF(MSVC)

# 添加 Google Test 测试框架
ADD_SUBDIRECTORY(googletest)

# ========== 基础核心模块 ==========

# EyerThread - 线程管理模块
OPTION(EyerThread                   "option for EyerThread"             ON)
ModuleTestOption("EyerThread"       "EyerThreadTest"           EyerThread)

# EyerCore - 核心基础库
OPTION(EyerCore                   "option for EyerCore"             ON)
ModuleTestOption("EyerCore"       "EyerCoreTest"           EyerCore)

# EyerAV - 音视频处理模块
OPTION(EyerAV                   "option for EyerAV"             ON)
ModuleTestOption("EyerAV"       "EyerAVTest"           EyerAV)

# EyerMath - 数学计算模块
OPTION(EyerMath                   "option for EyerMath"             ON)
ModuleTestOption("EyerMath"       "EyerMathTest"           EyerMath)

# EyerAVTranscoder - 音视频转码模块
OPTION(EyerAVTranscoder                   "option for EyerAVTranscoder"             ON)
ModuleTestOption("EyerAVTranscoder"       "EyerAVTranscoderTest"           EyerAVTranscoder)