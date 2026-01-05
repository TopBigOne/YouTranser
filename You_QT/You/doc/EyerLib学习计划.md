# EyerLib 学习计划

## 学习目标

通过系统学习 EyerLib 源码，掌握：
1. 音视频处理的基本原理和流程
2. FFmpeg 库的使用方法
3. C++ 音视频编程实践
4. 转码系统的设计和实现

---

## 学习路径

### 阶段一：基础准备（1-2周）

#### 1.1 前置知识
- [ ] **C++17 基础**
  - 智能指针（`std::shared_ptr`, `std::unique_ptr`）
  - 移动语义和右值引用
  - Lambda 表达式
  - 模板编程基础

- [ ] **音视频基础概念**
  - 视频编码原理（H.264/H.265）
  - 音频编码原理（AAC/MP3）
  - 容器格式（MP4/MKV/AVI）
  - 像素格式（YUV/RGB）
  - 采样率、声道、位深

- [ ] **FFmpeg 基础**
  - FFmpeg 架构和组件
  - 编解码流程
  - 数据包（Packet）和帧（Frame）的概念
  - 时间基准（Timebase）和 PTS/DTS

**推荐资源**:
- 《FFmpeg 从入门到精通》
- FFmpeg 官方文档：https://ffmpeg.org/documentation.html
- 雷霄骅的博客：https://blog.csdn.net/leixiaohua1020

#### 1.2 环境搭建
- [ ] 编译 EyerLib
- [ ] 配置开发环境（IDE、调试器）
- [ ] 运行测试用例

---

### 阶段二：核心模块学习（3-4周）

#### 2.1 EyerCore 模块（1周）

**学习目标**: 理解基础工具类的实现和使用

**重点内容**:
- [ ] `EyerString`: 字符串处理
- [ ] `EyerBuffer`: 缓冲区管理
- [ ] `EyerQueue` / `EyerLinkedList`: 数据结构
- [ ] `EyerLockQueue`: 线程安全队列
- [ ] `EyerSmartPtr`: 智能指针实现
- [ ] `EyerLog`: 日志系统

**实践任务**:
1. 阅读源码，理解每个类的设计
2. 编写简单的测试程序
3. 分析内存管理和线程安全实现

**推荐学习顺序**:
```
EyerString → EyerBuffer → EyerQueue → EyerLockQueue → EyerSmartPtr
```

---

#### 2.2 EyerAV 模块 - 基础类（1周）

**学习目标**: 理解音视频处理的基础数据结构

**重点内容**:
- [ ] `EyerAVPacket`: 数据包（编码后的压缩数据）
  - 数据结构和内存管理
  - 时间戳处理
  - 流索引

- [ ] `EyerAVFrame`: 帧（解码后的原始数据）
  - 视频帧：像素数据（YUV/RGB）
  - 音频帧：PCM 数据
  - 内存对齐和缓冲区管理

- [ ] `EyerAVStream`: 流信息
  - 编解码器参数
  - 时间基准（Timebase）
  - 流属性

- [ ] `EyerAVRational`: 有理数（时间基准）
- [ ] `EyerAVCodecID`: 编解码器 ID 枚举
- [ ] `EyerAVPixelFormat`: 像素格式枚举
- [ ] `EyerAVSampleFormat`: 采样格式枚举

**实践任务**:
1. 阅读源码，理解数据结构
2. 编写程序创建和操作 Packet/Frame
3. 理解时间戳的转换和计算

**推荐学习顺序**:
```
EyerAVStream → EyerAVPacket → EyerAVFrame → 枚举类
```

---

#### 2.3 EyerAV 模块 - 读取器（3-4天）

**学习目标**: 理解如何读取音视频文件

**重点内容**:
- [ ] `EyerAVReader`: 文件读取器
  - `Open()`: 打开文件
  - `Read()`: 读取数据包
  - `GetStream()`: 获取流信息
  - `Seek()`: 跳转
  - `GetDuration()`: 获取时长

- [ ] `EyerAVReaderCustomIO`: 自定义 IO 接口
  - 实现自定义数据源
  - 加密文件读取
  - 内存数据读取

- [ ] `EyerMediaInfo`: 媒体信息解析

**实践任务**:
1. 编写程序读取视频文件
2. 打印流的详细信息
3. 实现简单的 Seek 功能
4. 尝试实现自定义 IO（如读取加密文件）

**推荐学习顺序**:
```
EyerAVReader → EyerMediaInfo → EyerAVReaderCustomIO
```

**参考代码**:
- `EyerAVTest/EyerAVReaderTest.hpp`
- `EyerAVTest/EyerAVReaderGetInfoTest.hpp`

---

#### 2.4 EyerAV 模块 - 解码器（3-4天）

**学习目标**: 理解如何解码音视频数据

**重点内容**:
- [ ] `EyerAVDecoder`: 解码器
  - `Init()`: 初始化解码器
  - `SendPacket()`: 发送数据包
  - `RecvFrame()`: 接收解码后的帧
  - 多线程解码
  - 硬件加速（D3D11VA）

- [ ] `EyerAVDecoderBox`: 解码器盒子（单流）
- [ ] `EyerAVDecoderBoxGroup`: 解码器组（多流）
- [ ] `EyerAVDecoderLine`: 解码流水线

**实践任务**:
1. 编写程序解码视频文件
2. 保存解码后的帧为图片
3. 理解解码流程：Packet → Frame
4. 测试多线程解码性能

**推荐学习顺序**:
```
EyerAVDecoder → EyerAVDecoderBox → EyerAVDecoderBoxGroup
```

**参考代码**:
- `EyerAVTest/EyerAVDecoderBoxTest.hpp`
- `EyerAVTest/EyerAVDecoderLineTest.hpp`

---

#### 2.5 EyerAV 模块 - 编码器（3-4天）

**学习目标**: 理解如何编码音视频数据

**重点内容**:
- [ ] `EyerAVEncoder`: 编码器
  - `Init()`: 初始化编码器
  - `SendFrame()`: 发送帧
  - `RecvPacket()`: 接收编码后的数据包
  - 编码参数设置

- [ ] `EyerAVEncoderParam`: 编码参数
  - 视频参数：分辨率、码率、CRF、GOP
  - 音频参数：采样率、声道、码率

**实践任务**:
1. 编写程序编码视频
2. 测试不同编码参数的效果
3. 理解编码流程：Frame → Packet
4. 对比不同编码器的性能和质量

**推荐学习顺序**:
```
EyerAVEncoderParam → EyerAVEncoder
```

**参考代码**:
- `EyerAVTest/EyerAVEncoderTest.hpp`

---

#### 2.6 EyerAV 模块 - 写入器（2-3天）

**学习目标**: 理解如何写入音视频文件

**重点内容**:
- [ ] `EyerAVWriter`: 文件写入器
  - `Open()`: 打开输出文件
  - `AddStream()`: 添加流
  - `WritePacket()`: 写入数据包
  - `WriteHeader()` / `WriteTrailer()`: 写入文件头尾

**实践任务**:
1. 编写程序将编码后的数据包写入文件
2. 理解容器格式的结构
3. 实现简单的转封装（Remux）

**推荐学习顺序**:
```
EyerAVWriter
```

**参考代码**:
- `EyerAVTest/EyerAVRemuxTest.hpp`

---

#### 2.7 EyerAV 模块 - 处理工具（1周）

**学习目标**: 理解音视频处理工具

**重点内容**:
- [ ] `EyerAVResample`: 音频重采样
  - 采样率转换
  - 声道转换
  - 格式转换

- [ ] `EyerAVBitstreamFilter`: 码流过滤器
  - H.264 处理
  - 添加/删除 SEI

- [ ] `EyerAVSnapshot`: 截图功能
- [ ] `EyerImageUtil`: 图像处理工具
- [ ] `EyerAVFrame::Scale()`: 视频缩放

**实践任务**:
1. 实现音频重采样
2. 实现视频缩放
3. 实现截图功能
4. 理解像素格式转换

**推荐学习顺序**:
```
EyerAVResample → EyerAVFrame::Scale → EyerAVSnapshot
```

**参考代码**:
- `EyerAVTest/EyerAVFrameTest.hpp`
- `EyerAVTest/EyerAVFrameImage.hpp`

---

### 阶段三：高级功能学习（2-3周）

#### 3.1 EyerAVTranscoder 模块（1-2周）

**学习目标**: 理解完整的转码流程

**重点内容**:
- [ ] `EyerAVTranscoder`: 转码器
  - 转码流程设计
  - 多流处理
  - 进度回调
  - 中断控制

- [ ] `EyerAVTranscoderParams`: 转码参数
  - 视频参数配置
  - 音频参数配置
  - 时间范围裁剪

- [ ] `EyerAVTranscodeStream`: 转码流管理
  - 解码器/编码器管理
  - 重采样器管理
  - 流同步

**实践任务**:
1. 阅读完整的转码源码
2. 理解转码流程的每个步骤
3. 实现自定义转码参数
4. 添加转码进度显示
5. 实现转码中断功能

**推荐学习顺序**:
```
EyerAVTranscoderParams → EyerAVTranscodeStream → EyerAVTranscoder
```

**参考代码**:
- `EyerAVTranscoder/EyerAVTranscoder.cpp`
- `EyerAVTest/EyerAVTranscodeTest.hpp`

**重点理解**:
1. 转码的完整流程
2. 多流同步机制
3. 时间戳处理
4. 错误处理和恢复

---

#### 3.2 EyerThread 模块（3-4天）

**学习目标**: 理解多线程处理

**重点内容**:
- [ ] `EyerThread`: 线程类
  - 线程生命周期管理
  - 事件循环
  - 任务队列

- [ ] `EyerConditionVariableBox`: 条件变量
- [ ] 线程同步机制

**实践任务**:
1. 理解线程模型
2. 实现多线程转码
3. 理解线程同步

**推荐学习顺序**:
```
EyerThread → EyerConditionVariableBox
```

---

#### 3.3 EyerMath 模块（可选，2-3天）

**学习目标**: 理解数学计算在图像处理中的应用

**重点内容**:
- [ ] 矩阵运算（`Eatrix`）
- [ ] 向量运算（`Eector`）
- [ ] 插值计算（`EnterPolation`）
- [ ] 2D 裁剪（`Eyer2DCrop`）

**实践任务**:
1. 理解矩阵在图像变换中的应用
2. 实现简单的图像变换

---

### 阶段四：实战项目（2-3周）

#### 4.1 项目一：简单转码工具（1周）

**目标**: 实现一个简单的命令行转码工具

**功能要求**:
- [ ] 读取视频文件
- [ ] 转码为指定格式
- [ ] 显示转码进度
- [ ] 支持基本参数配置

**技术要点**:
- 使用 `EyerAVTranscoder`
- 实现进度回调
- 错误处理

---

#### 4.2 项目二：视频处理工具（1-2周）

**目标**: 实现一个功能更丰富的视频处理工具

**功能要求**:
- [ ] 视频转码
- [ ] 视频裁剪（时间范围）
- [ ] 视频缩放
- [ ] 截图功能
- [ ] 音频提取

**技术要点**:
- 使用 `EyerAVReader` / `EyerAVWriter`
- 使用 `EyerAVDecoder` / `EyerAVEncoder`
- 使用 `EyerAVSnapshot`
- 实现自定义处理流程

---

#### 4.3 项目三：性能优化（1周）

**目标**: 优化转码性能

**优化方向**:
- [ ] 多线程优化
- [ ] 内存优化
- [ ] 硬件加速
- [ ] 缓冲区优化

**技术要点**:
- 分析性能瓶颈
- 使用性能分析工具
- 优化关键路径

---

## 学习方法建议

### 1. 阅读源码
- **从简单到复杂**: 先看基础类，再看复杂功能
- **理解设计意图**: 思考为什么这样设计
- **关注接口**: 理解公共接口的设计

### 2. 实践编程
- **编写测试程序**: 每个模块都要写测试
- **调试源码**: 使用调试器跟踪执行流程
- **修改源码**: 尝试修改和扩展功能

### 3. 文档记录
- **记录学习笔记**: 记录重要概念和实现细节
- **绘制流程图**: 理解数据流和处理流程
- **总结设计模式**: 识别使用的设计模式

### 4. 参考资源
- **FFmpeg 文档**: 理解底层实现
- **测试用例**: `EyerAVTest/` 目录下的测试代码
- **示例代码**: YouTranser 项目中的使用示例

---

## 学习检查点

### 检查点 1：基础理解（阶段一完成后）
- [ ] 能够解释音视频编码的基本概念
- [ ] 理解 FFmpeg 的基本架构
- [ ] 能够编译和运行 EyerLib

### 检查点 2：核心模块（阶段二完成后）
- [ ] 能够使用 `EyerAVReader` 读取文件
- [ ] 能够使用 `EyerAVDecoder` 解码视频
- [ ] 能够使用 `EyerAVEncoder` 编码视频
- [ ] 能够使用 `EyerAVWriter` 写入文件

### 检查点 3：高级功能（阶段三完成后）
- [ ] 能够使用 `EyerAVTranscoder` 进行转码
- [ ] 理解转码的完整流程
- [ ] 能够实现自定义处理功能

### 检查点 4：实战能力（阶段四完成后）
- [ ] 能够独立开发音视频处理工具
- [ ] 能够优化性能
- [ ] 能够解决实际问题

---

## 常见问题与解答

### Q1: 如何理解 Packet 和 Frame 的区别？
**A**: Packet 是编码后的压缩数据，Frame 是解码后的原始数据。解码流程：Packet → Frame，编码流程：Frame → Packet。

### Q2: 时间戳（PTS/DTS）如何理解？
**A**: PTS（Presentation Time Stamp）是显示时间戳，DTS（Decode Time Stamp）是解码时间戳。由于 B 帧的存在，解码顺序和显示顺序可能不同。

### Q3: 如何实现音视频同步？
**A**: 通过比较音频和视频的 PTS，调整播放速度或丢弃/重复帧来实现同步。

### Q4: 转码性能如何优化？
**A**: 
- 使用多线程解码/编码
- 使用硬件加速
- 优化内存分配
- 减少数据拷贝

### Q5: 如何处理错误？
**A**: 
- 检查函数返回值
- 使用监听器接收错误回调
- 查看日志输出
- 理解错误码含义

---

## 推荐学习资源

### 书籍
- 《FFmpeg 从入门到精通》
- 《音视频开发进阶指南》
- 《C++ Primer》

### 在线资源
- FFmpeg 官方文档：https://ffmpeg.org/documentation.html
- FFmpeg Wiki：https://trac.ffmpeg.org/wiki
- 雷霄骅的博客：https://blog.csdn.net/leixiaohua1020

### 工具
- **调试器**: GDB / LLDB
- **性能分析**: Valgrind / Instruments
- **代码阅读**: Source Insight / CLion

---

## 学习时间规划

| 阶段 | 内容 | 预计时间 | 累计时间 |
|------|------|---------|---------|
| 阶段一 | 基础准备 | 1-2周 | 1-2周 |
| 阶段二 | 核心模块 | 3-4周 | 4-6周 |
| 阶段三 | 高级功能 | 2-3周 | 6-9周 |
| 阶段四 | 实战项目 | 2-3周 | 8-12周 |

**总计**: 约 2-3 个月（每天 2-3 小时）

---

## 总结

EyerLib 是一个功能强大的音视频处理库，学习它需要：
1. **扎实的基础**: C++ 和音视频基础知识
2. **系统的方法**: 按照模块逐步学习
3. **大量的实践**: 通过编程加深理解
4. **持续的学习**: 音视频技术更新快，需要持续学习

通过系统学习 EyerLib，你将掌握：
- 音视频处理的核心技术
- FFmpeg 的使用方法
- C++ 音视频编程实践
- 转码系统的设计和实现

祝你学习顺利！

