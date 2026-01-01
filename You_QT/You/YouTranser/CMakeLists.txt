# 设置 CMake 最低版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# 定义 YouTranser 项目名称和版本号
PROJECT(YouTranser VERSION 0.1 LANGUAGES CXX)

# 设置当前目录为包含目录
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# 启用 Qt 自动化工具
SET(CMAKE_AUTOUIC ON)  # 自动处理 .ui 文件
SET(CMAKE_AUTOMOC ON)  # 自动处理 Qt 的 moc（元对象编译器）
SET(CMAKE_AUTORCC ON)  # 自动处理 Qt 资源文件

# 设置 C++ 标准为 C++17
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 库（优先查找 Qt6，若找不到则使用 Qt5）
FIND_PACKAGE(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network core)
FIND_PACKAGE(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network core)

# 添加包含目录
INCLUDE_DIRECTORIES (../)  # 父目录
INCLUDE_DIRECTORIES (./)   # 当前目录

# 定义项目源文件列表
SET(PROJECT_SOURCES
        main.cpp                          # 程序入口文件

        # 主窗口相关文件
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui

        # YouTranser 主窗口相关文件
        YouTransMainWindow.hpp
        YouTransMainWindow.cpp
        YouTransMainWindow.ui

        # 配置窗口相关文件
        YouTransConfig.hpp
        YouTransConfig.cpp
        YouTransConfig.ui

        # 关于窗口相关文件
        YouTransAboutWindow.hpp
        YouTransAboutWindow.cpp
        YouTransAboutWindow.ui

        # 任务项组件相关文件
        TaskItem.hpp
        TaskItem.cpp
        TaskItem.ui

        # 任务状态标签组件
        TaskItemStatusLabel.hpp
        TaskItemStatusLabel.cpp

        # 应用配置管理
        YouTransAppConfig.hpp
        YouTransAppConfig.cpp

        # 加载界面
        YouTransLoading.hpp
        YouTransLoading.cpp

        # 登录界面
        YouTransLogin.hpp
        YouTransLogin.cpp

        # 转码任务线程
        TranscodeTaskThread.hpp
        TranscodeTaskThread.cpp

        # 转码参数管理
        YouTranscoderParams.hpp
        YouTranscoderParams.cpp
)

# Qt 资源文件列表
SET(Qrc_Files
    you-trans.qrc
    )

# macOS Bundle 配置
SET(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})        # Bundle 标识符
SET(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})         # Bundle 名称

SET(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})              # 版权信息

SET(MACOSX_BUNDLE_SHORT_VERSION_STRING ${spectral_VERSION})  # 短版本字符串
SET(MACOSX_BUNDLE_BUNDLE_VERSION ${spectral_VERSION})        # Bundle 版本

# 设置应用图标
SET(ICON_NAME "YouTransIcon.icns")
SET(${PROJECT_NAME}_MAC_ICON "${PROJECT_SOURCE_DIR}/icons/${ICON_NAME}")
SET(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
SET_PROPERTY(SOURCE "${${PROJECT_NAME}_MAC_ICON}" PROPERTY
    MACOSX_PACKAGE_LOCATION Resources)

# 将 Qt 资源文件转换为 C++ 代码
QT_ADD_RESOURCES(QRC_FILES ${Qrc_Files})

# 根据平台创建可执行文件
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    # Windows 平台：创建普通可执行文件（带图标）
    ADD_EXECUTABLE(YouTranser
        ${${PROJECT_NAME}_MAC_ICON}
        ${PROJECT_SOURCES}
        ${QRC_FILES}
        "win_icon.rc"          # Windows 图标资源文件
    )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    MESSAGE(STATUS "current platform: Darwin")
    # macOS 平台：创建 Bundle 应用程序
    QT_ADD_EXECUTABLE(YouTranser
        MACOSX_BUNDLE
        ${${PROJECT_NAME}_MAC_ICON}
        ${PROJECT_SOURCES}
        ${QRC_FILES}
    )
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")

# 设置 macOS Info.plist 文件路径
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY
      MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/macOS/Info.plist.in")

# 根据平台配置链接库
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    # Windows 平台链接库配置
    TARGET_LINK_LIBRARIES(YouTranser PRIVATE
        # 自定义库
        YouCommon              # 公共组件库

        # EyerLib 音视频处理库
        EyerAVTranscoder       # 转码模块
        EyerAV                 # 音视频处理模块
        EyerMath               # 数学计算模块
        EyerCore               # 核心库
        EyerThread             # 线程管理库

        # FFmpeg 相关库
        avformat               # 封装格式处理
        avcodec                # 编解码器
        swscale                # 图像缩放
        swresample             # 音频重采样
        avutil                 # 工具库

        # 编码器库
        x264                   # H.264 编码器
        x265                   # H.265/HEVC 编码器
        vpx                    # VP8/VP9 编码器
        mp3lame                # MP3 编码器
        fdk-aac                # AAC 编码器

        # 加密库
        ssl                    # OpenSSL
        crypto                 # OpenSSL 加密

        # Windows 系统库
        Strmiids               # DirectShow 相关
        Mfplat                 # Media Foundation 平台
        Bcrypt                 # 加密 API
        Secur32                # 安全支持提供程序接口
        Crypt32                # 加密 API
        ws2_32                 # Windows Sockets 2

        # 其他工具库
        iconv                  # 字符编码转换
        lzma                   # LZMA 压缩
        z                      # zlib 压缩
        bz2                    # bzip2 压缩

        # Qt 库
        Qt${QT_VERSION_MAJOR}::Widgets   # Qt Widgets 模块
        Qt${QT_VERSION_MAJOR}::Network   # Qt Network 模块
    )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Android")
    MESSAGE(STATUS "current platform: Android")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    MESSAGE(STATUS "current platform: Darwin")
    # macOS 平台链接库配置
    TARGET_LINK_LIBRARIES(YouTranser PRIVATE
        # 自定义库
        YouCommon              # 公共组件库

        # EyerLib 音视频处理库
        EyerAVTranscoder       # 转码模块
        EyerAV                 # 音视频处理模块
        EyerMath               # 数学计算模块
        EyerCore               # 核心库
        EyerThread             # 线程管理库

        # FFmpeg 相关库
        avformat               # 封装格式处理
        avcodec                # 编解码器
        swscale                # 图像缩放
        swresample             # 音频重采样
        avutil                 # 工具库

        # 编码器库
        x264                   # H.264 编码器
        x265                   # H.265/HEVC 编码器
        vpx                    # VP8/VP9 编码器
        mp3lame                # MP3 编码器
        fdk-aac                # AAC 编码器

        # 加密库
        ssl                    # OpenSSL
        crypto                 # OpenSSL 加密

        # 其他工具库
        iconv                  # 字符编码转换
        lzma                   # LZMA 压缩
        z                      # zlib 压缩
        bz2                    # bzip2 压缩

        # Qt 库
        Qt${QT_VERSION_MAJOR}::Widgets   # Qt Widgets 模块
        Qt${QT_VERSION_MAJOR}::Network   # Qt Network 模块
    )

    # 为 macOS 设置 RPATH，使运行时能找到 Qt 框架
    SET_TARGET_PROPERTIES(YouTranser PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks;${CMAKE_PREFIX_PATH}/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "iOS")
    ADD_DEFINITIONS(-D EYER_PLATFORM_IOS)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    MESSAGE(STATUS "current platform: Emscripten")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

# 设置目标属性
SET_TARGET_PROPERTIES(YouTranser PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER youtrans.zzsin.com                                          # macOS Bundle 标识符
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}                                          # Bundle 版本号
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}    # 短版本字符串
    MACOSX_BUNDLE TRUE             # 启用 macOS Bundle
    WIN32_EXECUTABLE TRUE          # Windows 下创建 GUI 应用（非控制台）
)

# 安装配置
INSTALL(TARGETS YouTranser
    BUNDLE DESTINATION .                           # macOS Bundle 安装到根目录
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})   # 库文件安装路径

# Qt6 特定配置
IF(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(YouTranser)  # 完成 Qt6 可执行文件配置
ENDIF()

# CMake 配置示例命令（用于 MinGW）
# $CMAKE_HOME/bin/cmake ../YouPicker/ -DCMAKE_PREFIX_PATH=C:/Qt/6.2.4/mingw_64/ -G "MinGW Makefiles"