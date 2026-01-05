# EyerLib 技术架构文档

## 1. 项目概述

EyerLib 是一个基于 FFmpeg 的跨平台音视频处理库，采用 C++17 标准开发。它提供了完整的音视频编解码、转码、处理等功能，是 YouTranser 视频转码工具的核心底层库。

### 1.1 核心特性
- **跨平台支持**: Windows、macOS、Linux
- **基于 FFmpeg**: 利用 FFmpeg 的强大功能
- **模块化设计**: 清晰的模块划分，便于维护和扩展
- **高性能**: 支持硬件加速和多线程处理
- **易用 API**: 封装了复杂的 FFmpeg 操作，提供简洁的接口

### 1.2 技术栈
- **语言**: C++17
- **构建系统**: CMake
- **核心依赖**: FFmpeg (libavcodec, libavformat, libavutil, libswscale, libswresample)
- **编码器支持**: x264, x265, libvpx, fdk-aac, lame 等

---

## 2. 整体架构

### 2.1 模块划分

EyerLib 采用分层模块化架构，主要分为以下几个核心模块：

```
EyerLib
├── EyerCore          # 核心基础库（工具类、数据结构）
├── EyerAV            # 音视频处理模块（编解码、读写）
├── EyerAVTranscoder  # 转码模块（高级转码接口）
├── EyerThread        # 线程管理模块
└── EyerMath          # 数学计算模块（矩阵、向量等）
```

### 2.2 模块依赖关系

```
EyerAVTranscoder
    ↓ 依赖
EyerAV + EyerCore
    ↓ 依赖
EyerCore (基础工具)
    ↓ 可选依赖
EyerThread (多线程支持)
EyerMath (图像处理计算)
```

---

## 3. 核心模块详解

### 3.1 EyerCore 模块

**定位**: 提供基础工具类和数据结构，是整个库的基础设施。

#### 3.1.1 主要组件

| 组件 | 功能描述 |
|------|---------|
| `EyerString` | 字符串处理类，封装字符串操作 |
| `EyerLog` | 日志系统，支持不同级别的日志输出 |
| `EyerTime` | 时间处理工具，获取时间戳、格式化时间 |
| `EyerBuffer` | 缓冲区管理，内存数据管理 |
| `EyerQueue` | 队列数据结构（FIFO） |
| `EyerLinkedList` | 链表数据结构 |
| `EyerMap` | 映射表数据结构 |
| `EyerLockQueue` | 线程安全的队列 |
| `EyerLRUCache` | LRU 缓存实现 |
| `EyerSmartPtr` | 智能指针实现 |
| `EyerMD5` | MD5 哈希计算 |
| `EyerURLUtil` | URL 工具类 |
| `EyerFileReader` | 文件读取工具 |
| `EyerObserver` | 观察者模式实现 |
| `EyerBitStream` | 位流处理工具 |

#### 3.1.2 设计特点
- **RAII 原则**: 自动资源管理
- **线程安全**: 关键数据结构提供线程安全版本
- **跨平台**: 抽象了平台差异

---

### 3.2 EyerAV 模块

**定位**: 音视频处理的核心模块，封装了 FFmpeg 的主要功能。

#### 3.2.1 核心类层次结构

```
EyerAV 模块
│
├── 输入/输出层
│   ├── EyerAVReader          # 音视频文件读取器
│   ├── EyerAVWriter           # 音视频文件写入器
│   ├── EyerAVReaderCustomIO   # 自定义 IO 接口
│   └── EyerAVVideoWriter      # 视频写入器
│
├── 编解码层
│   ├── EyerAVDecoder          # 解码器（Packet → Frame）
│   ├── EyerAVEncoder          # 编码器（Frame → Packet）
│   ├── EyerAVDecoderBox       # 解码器盒子（单流解码）
│   ├── EyerAVDecoderBoxGroup  # 解码器组（多流解码）
│   ├── EyerAVDecoderLine      # 解码流水线
│   └── EyerAVDecoderLineAudio # 音频解码流水线
│
├── 数据容器层
│   ├── EyerAVPacket           # 编码后的数据包（压缩数据）
│   ├── EyerAVFrame            # 解码后的帧（原始数据）
│   ├── EyerAVStream           # 流信息（编解码器参数、时间基准等）
│   └── EyerAVPixelFrame       # 像素帧
│
├── 处理工具层
│   ├── EyerAVResample         # 音频重采样
│   ├── EyerAVBitstreamFilter  # 码流过滤器
│   ├── EyerAVSnapshot        # 截图功能
│   └── EyerImageUtil          # 图像处理工具
│
└── 辅助类
    ├── EyerAVCodecID          # 编解码器 ID 枚举
    ├── EyerAVPixelFormat       # 像素格式枚举
    ├── EyerAVSampleFormat     # 采样格式枚举
    ├── EyerAVChannelLayout    # 声道布局枚举
    ├── EyerAVFileFmt          # 文件格式枚举
    ├── EyerAVRational         # 有理数（时间基准）
    └── EyerMediaInfo           # 媒体信息解析
```

#### 3.2.2 核心数据流

**解码流程**:
```
文件/流 → EyerAVReader → EyerAVPacket → EyerAVDecoder → EyerAVFrame
```

**编码流程**:
```
EyerAVFrame → EyerAVEncoder → EyerAVPacket → EyerAVWriter → 文件/流
```

**转码流程**:
```
输入文件 → Reader → Decoder → [处理] → Encoder → Writer → 输出文件
```

#### 3.2.3 关键类详解

##### EyerAVReader（读取器）
- **功能**: 打开音视频文件，读取编码后的数据包
- **支持**: 本地文件、网络流、自定义 IO
- **主要方法**:
  - `Open()`: 打开文件
  - `Read(packet)`: 读取数据包
  - `GetStream(index)`: 获取流信息
  - `Seek()`: 跳转到指定时间

##### EyerAVDecoder（解码器）
- **功能**: 将编码数据包解码为原始帧
- **支持**: 多线程解码、硬件加速（D3D11VA）
- **主要方法**:
  - `Init(stream, threadnum)`: 初始化解码器
  - `SendPacket(packet)`: 发送数据包
  - `RecvFrame(frame)`: 接收解码后的帧

##### EyerAVEncoder（编码器）
- **功能**: 将原始帧编码为数据包
- **支持**: H.264/H.265/VP8/VP9/AAC/MP3 等
- **主要方法**:
  - `Init(param)`: 初始化编码器
  - `SendFrame(frame)`: 发送帧
  - `RecvPacket(packet)`: 接收编码后的数据包

##### EyerAVFrame（帧）
- **功能**: 存储解码后的原始音视频数据
- **视频**: 像素数据（YUV/RGB）
- **音频**: PCM 数据
- **主要方法**:
  - `GetData()`: 获取数据指针
  - `Scale()`: 缩放
  - `Resample()`: 重采样（音频）

##### EyerAVWriter（写入器）
- **功能**: 将编码后的数据包写入文件
- **支持**: MP4/MOV/MKV/AVI 等格式
- **主要方法**:
  - `Open()`: 打开输出文件
  - `AddStream(encoder)`: 添加流
  - `WritePacket(packet)`: 写入数据包

---

### 3.3 EyerAVTranscoder 模块

**定位**: 高级转码接口，封装了完整的转码流程。

#### 3.3.1 核心类

##### EyerAVTranscoder（转码器）
- **功能**: 提供完整的转码功能
- **特点**:
  - 支持视频/音频转码
  - 支持时间范围裁剪
  - 支持进度回调
  - 支持中断控制

**主要方法**:
```cpp
// 设置输入输出路径
SetOutputPath(path)
SetParams(params)

// 执行转码
Transcode(interrupt, customIO)

// 监听器接口
SetListener(listener)  // 进度、成功、失败回调
```

##### EyerAVTranscoderParams（转码参数）
- **视频参数**:
  - 编码器（H.264/H.265/VP8/VP9）
  - 分辨率（宽高）
  - 像素格式
  - CRF（质量参数）
- **音频参数**:
  - 编码器（AAC/MP3）
  - 采样率
  - 声道布局
- **其他参数**:
  - 输出格式（MP4/MOV/MKV）
  - 时间范围（开始/结束时间）
  - 线程数（解码/编码）

##### EyerAVTranscodeStream（转码流）
- **功能**: 管理单个流的转码状态
- **包含**:
  - `decoder`: 解码器实例
  - `encoder`: 编码器实例
  - `resample`: 重采样器（音频）
  - `readStreamId`: 输入流索引
  - `writeStreamId`: 输出流索引

#### 3.3.2 转码流程

```
1. 打开输入文件 (EyerAVReader)
   ↓
2. 解析流信息
   ↓
3. 为每个流创建解码器和编码器
   ↓
4. 打开输出文件 (EyerAVWriter)
   ↓
5. 循环处理:
   - 读取数据包 (Reader.Read)
   - 解码 (Decoder.RecvFrame)
   - 处理帧 (缩放、重采样等)
   - 编码 (Encoder.RecvPacket)
   - 写入 (Writer.WritePacket)
   ↓
6. 刷新缓冲区 (Flush)
   ↓
7. 关闭文件，释放资源
```

#### 3.3.3 监听器接口

```cpp
class EyerAVTranscoderListener {
    virtual int OnProgress(float progress);  // 进度回调 (0.0-1.0)
    virtual int OnFail(EyerAVTranscoderError & error);  // 失败回调
    virtual int OnSuccess();  // 成功回调
};
```

---

### 3.4 EyerThread 模块

**定位**: 提供线程管理功能。

#### 3.4.1 核心类

##### EyerThread（线程类）
- **功能**: 封装线程操作
- **特点**:
  - 事件循环支持
  - 任务队列
  - 线程同步

**主要方法**:
```cpp
Start()              // 启动线程
Stop()               // 停止线程
Run()                // 线程执行函数（虚函数）
PushEvent(runnable)  // 添加任务
StartEventLoop()     // 启动事件循环
StopEventLoop()      // 停止事件循环
```

##### EyerConditionVariableBox（条件变量）
- **功能**: 线程同步工具

---

### 3.5 EyerMath 模块

**定位**: 提供数学计算功能，主要用于图像处理。

#### 3.5.1 核心组件

| 组件 | 功能 |
|------|------|
| `Eatrix` | 矩阵类（模板类） |
| `Eatrix4x4` | 4x4 矩阵（用于 3D 变换） |
| `Eatrix3x3` | 3x3 矩阵 |
| `EectorF2/F3/F4` | 2D/3D/4D 向量 |
| `Eect` | 矩形类 |
| `EyerRectI` | 整数矩形 |
| `EyerColor` | 颜色类 |
| `EnterPolation` | 插值计算 |
| `Eyer2DCrop` | 2D 裁剪工具 |

---

## 4. 设计模式与架构原则

### 4.1 设计模式

1. **PIMPL (Pointer to Implementation)**
   - 使用 `*Private` 类隐藏实现细节
   - 减少头文件依赖，提高编译速度

2. **RAII (Resource Acquisition Is Initialization)**
   - 自动资源管理
   - 析构函数自动释放资源

3. **观察者模式**
   - `EyerObserver` / `EyerSubject`
   - 用于事件通知

4. **工厂模式**
   - 编解码器创建
   - 格式检测

### 4.2 架构原则

1. **单一职责**: 每个类职责明确
2. **依赖倒置**: 高层模块不依赖低层模块，都依赖抽象
3. **接口隔离**: 提供细粒度的接口
4. **开闭原则**: 对扩展开放，对修改关闭

---

## 5. 数据流与处理流程

### 5.1 完整转码数据流

```
输入文件
  ↓
[EyerAVReader] 读取数据包
  ↓
EyerAVPacket (压缩数据)
  ↓
[EyerAVDecoder] 解码
  ↓
EyerAVFrame (原始数据)
  ↓
[可选处理]
  - EyerAVResample (音频重采样)
  - Scale (视频缩放)
  - 其他处理
  ↓
[EyerAVEncoder] 编码
  ↓
EyerAVPacket (压缩数据)
  ↓
[EyerAVWriter] 写入文件
  ↓
输出文件
```

### 5.2 多流处理

EyerAVTranscoder 支持同时处理多个流（视频流、音频流、字幕流等）:
- 每个流有独立的解码器和编码器
- 通过 `EyerAVTranscodeStream` 管理每个流的状态
- 同步处理多个流，保证音视频同步

---

## 6. 关键数据结构

### 6.1 EyerAVPacket（数据包）
- **用途**: 存储编码后的压缩数据
- **包含**: 数据指针、大小、时间戳、流索引

### 6.2 EyerAVFrame（帧）
- **用途**: 存储解码后的原始数据
- **视频**: 像素数据（YUV/RGB 格式）
- **音频**: PCM 数据
- **包含**: 数据指针、宽高、像素格式、时间戳

### 6.3 EyerAVStream（流信息）
- **用途**: 描述流的属性
- **包含**: 编解码器参数、时间基准、流索引、媒体类型

---

## 7. 性能优化

### 7.1 多线程支持
- **解码线程**: `EyerAVDecoder::Init(stream, threadnum)`
- **编码线程**: `EyerAVTranscoderParams::SetEncodeThreadNum()`
- **并行处理**: 多个流可以并行处理

### 7.2 硬件加速
- **视频解码**: 支持 D3D11VA（Windows）
- **视频编码**: 支持硬件编码器（通过 FFmpeg）

### 7.3 内存管理
- **智能指针**: `EyerSmartPtr` 自动管理内存
- **缓冲区复用**: 减少内存分配
- **LRU 缓存**: `EyerLRUCache` 缓存常用数据

---

## 8. 错误处理

### 8.1 错误码
- `EyerAVTranscoderError`: 转码错误枚举
- `EyerAVErrorCode`: AV 模块错误码

### 8.2 错误处理机制
- **返回值**: 函数返回 0 表示成功，非 0 表示失败
- **异常**: 关键错误通过监听器回调通知
- **日志**: `EyerLog` 记录详细错误信息

---

## 9. 扩展性

### 9.1 自定义 IO
- `EyerAVReaderCustomIO`: 支持自定义数据源
- 可用于加密文件、内存数据、网络流等

### 9.2 插件化
- 编解码器通过 FFmpeg 动态加载
- 支持添加新的编解码器

### 9.3 格式扩展
- 通过 FFmpeg 支持新格式
- `EyerAVFileFmt` 枚举可扩展

---

## 10. 依赖关系

### 10.1 外部依赖
- **FFmpeg**: 核心音视频处理库
- **x264/x265**: H.264/H.265 编码器
- **libvpx**: VP8/VP9 编码器
- **fdk-aac**: AAC 编码器
- **lame**: MP3 编码器
- **OpenSSL**: 加密库（用于 HTTPS 流）

### 10.2 内部依赖
```
EyerAVTranscoder → EyerAV → EyerCore
                ↓
            EyerThread (可选)
            EyerMath (可选)
```

---

## 11. 使用示例

### 11.1 基本转码流程

```cpp
// 1. 创建转码器
EyerAVTranscoder transcoder("input.mp4");

// 2. 设置输出路径
transcoder.SetOutputPath("output.mp4");

// 3. 设置转码参数
EyerAVTranscoderParams params;
params.SetVideoCodecId(EyerAVCodecID::CODEC_ID_H265);
params.SetWidthHeight(1920, 1080);
params.SetCRF(23);
params.SetAudioCodecId(EyerAVCodecID::CODEC_ID_AAC);
transcoder.SetParams(params);

// 4. 设置监听器（可选）
MyListener listener;
transcoder.SetListener(&listener);

// 5. 执行转码
MyInterrupt interrupt;
transcoder.Transcode(&interrupt);
```

### 11.2 读取媒体信息

```cpp
EyerAVReader reader("video.mp4");
reader.Open();

int streamCount = reader.GetStreamCount();
for (int i = 0; i < streamCount; i++) {
    EyerAVStream stream = reader.GetStream(i);
    EyerLog("Stream %d: Type=%d\n", i, stream.GetType());
}

reader.Close();
```

---

## 12. 总结

EyerLib 是一个设计良好的音视频处理库，具有以下特点：

1. **模块化**: 清晰的模块划分，职责明确
2. **易用性**: 封装了复杂的 FFmpeg 操作
3. **高性能**: 支持多线程和硬件加速
4. **可扩展**: 支持自定义 IO 和插件化
5. **跨平台**: 支持 Windows、macOS、Linux

通过合理的架构设计，EyerLib 为上层应用（如 YouTranser）提供了强大而易用的音视频处理能力。

