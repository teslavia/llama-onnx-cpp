# Llama-2 C++ Inference on Mac (ONNX Runtime GenAI)

本项目记录了在 Apple Silicon 环境下，如何将 Llama-2-7B-Chat 模型导出为 ONNX 格式，并基于微软 **ONNX Runtime GenAI (C++)** 库开发高性能、支持多轮对话的本地推理程序。

## ✨ 项目亮点

- **高性能**: 针对 M4 芯片优化，使用 Int4 量化，显著降低显存占用并提升推理速度。
- **纯 C++ 实现**: 摆脱 Python 依赖，利用底层 C API 实现高效推理。
- **多轮对话**: 实现了 KV Cache 的持久化管理，支持连续对话（Chat Mode）。
- **最新 API 适配**: 解决了 onnxruntime-genai 最新版本 (v0.6.0+) API 变动带来的兼容性问题。

## 🛠 前置要求

- **硬件**: Mac Mini M4 Ultra (或 M1/M2/M3 系列 Mac)
- **系统**: macOS Sequoia (或更新版本)
- **工具**:
  - Xcode Command Line Tools
  - CMake (brew install cmake)
  - Python 3.10+ (用于模型导出)
  - Hugging Face Access Token (用于下载 Llama-2)

## 📂 项目结构

```bash
.
├── lib/                        # 依赖库目录
│   ├── include/                # 头文件 (ort_genai_c.h)
│   ├── libonnxruntime-genai.dylib
│   └── libonnxruntime.dylib    # 核心运行时库
├── model-onnx/                 # 导出的 Int4 ONNX 模型
├── src/
│   └── main.cpp                # 主程序源码 (支持多轮对话)
├── CMakeLists.txt              # 构建配置
└── README.md
```

## 🚀 步骤一：模型导出与量化

使用 onnxruntime-genai 的 Python 工具将 Hugging Face 格式的模型转换为优化的 ONNX 格式。

1. **安装依赖**:

   ```bash
   pip install numpy onnxruntime-genai huggingface_hub
   ```

2. **登录 Hugging Face** (Llama-2 是门控模型，需先申请权限):

   ```bash
   huggingface-cli login
   ```

3. **导出并量化 (Int4)**:

   ```bash
   # -p int4: 使用 4-bit 量化 (推荐 Apple Silicon 使用)
   # -e cpu: ARM CPU 支持 NEON 指令集加速
   python3 -m onnxruntime_genai.models.builder -m meta-llama/Llama-2-7b-chat-hf -o ./model-onnx -p int4 -e cpu
   ```

## 💻 步骤二：C++ 环境配置

1. 从 [ONNX Runtime GenAI GitHub](https://www.google.com/url?sa=E&q=https%3A%2F%2Fgithub.com%2Fmicrosoft%2Fonnxruntime-genai) 下载或编译适用于 macOS ARM64 的库文件。
2. 将编译好的 libonnxruntime-genai.dylib 和依赖的 libonnxruntime.dylib 放入项目的 lib/ 目录。
3. 将头文件 ort_genai_c.h 放入 lib/include/。

## ⚙️ 步骤三：编译与构建

使用了适配最新版 API (AppendTokenSequences 接口) 的代码逻辑。

```bash
mkdir build && cd build
cmake ..
make
```

## ▶️ 步骤四：运行推理

运行生成的可执行文件，并指定模型目录路径：

```bash
# 确保在 build 目录下运行
./llama_inference ../model-onnx
```

### 交互示例

```bash
Loading model from: ../model-onnx...
Model loaded! Type '/exit' to quit.

>>> User: Hello, explain quantum computing like I'm 5.
>>> Llama: Sure! Imagine you have a magic coin that can be heads and tails at the same time...

>>> User: What was the first thing I asked you?
>>> Llama: You asked me to explain quantum computing simply.
```

## ⚠️ 常见问题与解决方案

### 1. 动态库加载失败 (Image not found / abort)

**现象**: 运行时提示 dyld: Library not loaded 或直接 abort。
**原因**: 程序找不到依赖的 .dylib 文件。
**解决**:

- 确保 libonnxruntime.dylib 和 libonnxruntime-genai.dylib 都在同一目录下。
- 在 CMake 中配置了 RPATH，或者手动将库文件复制到 build 目录或 /usr/local/lib。

### 2. API 编译错误 (SetInputSequences undefined)

**现象**: 编译时报错 OgaGeneratorParamsSetInputSequences 未定义。
**原因**: 微软更新了 API，将输入处理从 Params 中分离。
**解决**: 本项目已适配新版 API，使用 OgaGenerator_AppendTokenSequences(generator, sequences) 来处理输入。

### 3. 多轮对话不记忆上下文

**原因**: 每轮对话重新创建了 Generator。
**解决**: Generator 对象在循环外创建，循环内仅使用 Append 追加新的 Token，从而保持 KV Cache 状态。

## 🔗 参考资料

- [Microsoft ONNX Runtime GenAI](https://www.google.com/url?sa=E&q=https%3A%2F%2Fgithub.com%2Fmicrosoft%2Fonnxruntime-genai)
- [Meta Llama 2](https://www.google.com/url?sa=E&q=https%3A%2F%2Fhuggingface.co%2Fmeta-llama)