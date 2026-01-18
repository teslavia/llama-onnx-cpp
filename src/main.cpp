#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "ort_genai_c.h"

// 错误检查辅助函数
void CheckResult(OgaResult* result) {
    if (result) {
        const char* msg = OgaResultGetError(result);
        std::string error_msg = msg ? msg : "Unknown error";
        OgaDestroyResult(result);
        throw std::runtime_error(error_msg);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model_path>" << std::endl;
        return 1;
    }

    const char* model_path = argv[1];
    
    // 初始化资源指针
    OgaModel* model = nullptr;
    OgaTokenizer* tokenizer = nullptr;
    OgaTokenizerStream* tokenizer_stream = nullptr;
    OgaGeneratorParams* params = nullptr;
    OgaGenerator* generator = nullptr;

    try {
        std::cout << "Loading model from: " << model_path << "..." << std::endl;

        // 1. 加载模型与分词器
        CheckResult(OgaCreateModel(model_path, &model));
        CheckResult(OgaCreateTokenizer(model, &tokenizer));
        CheckResult(OgaCreateTokenizerStream(tokenizer, &tokenizer_stream));

        // 2. 设置生成参数
        CheckResult(OgaCreateGeneratorParams(model, &params));
        CheckResult(OgaGeneratorParamsSetSearchNumber(params, "max_length", 2048)); // 总上下文长度
        CheckResult(OgaGeneratorParamsSetSearchNumber(params, "temperature", 0.7)); // 增加一点创造性

        // 3. 创建生成器 (Generator 维护对话状态)
        CheckResult(OgaCreateGenerator(model, params, &generator));

        std::cout << "Model loaded! Type '/exit' to quit.\n" << std::endl;

        // --- 对话主循环 ---
        while (true) {
            // 4. 获取用户输入
            std::cout << "\n>>> User: ";
            std::string user_input;
            if (!std::getline(std::cin, user_input)) break; // 处理 EOF

            if (user_input == "/exit" || user_input == "exit") {
                break;
            }

            if (user_input.empty()) continue;

            // 5. Llama-2 对话格式化
            // 格式: [INST] 用户内容 [/INST]
            std::string prompt = "[INST] " + user_input + " [/INST]";

            // 6. 编码输入 (Encode)
            OgaSequences* sequences = nullptr;
            CheckResult(OgaCreateSequences(&sequences));
            CheckResult(OgaTokenizerEncode(tokenizer, prompt.c_str(), sequences));

            // 7. 将编码后的 Token 追加到生成器 (Append)
            // 这会更新 KV Cache，让模型“记得”之前的对话
            CheckResult(OgaGenerator_AppendTokenSequences(generator, sequences));

            // 用完 sequences 就可以释放了，数据已经进 generator 了
            OgaDestroySequences(sequences); 

            // 8. 生成回复 (Generate Loop)
            std::cout << ">>> Llama: " << std::flush;
            
            while (!OgaGenerator_IsDone(generator)) {
                // 计算下一个 Token
                CheckResult(OgaGenerator_GenerateNextToken(generator));

                // 获取刚刚生成的 Token
                size_t seq_len = OgaGenerator_GetSequenceCount(generator, 0);
                const int32_t* seq_data = OgaGenerator_GetSequenceData(generator, 0);
                
                if (seq_len > 0) {
                    int32_t new_token = seq_data[seq_len - 1];
                    
                    // 解码并打印
                    const char* token_str;
                    CheckResult(OgaTokenizerStreamDecode(tokenizer_stream, new_token, &token_str));
                    std::cout << token_str << std::flush;
                }
            }
            std::cout << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "\n❌ Error: " << e.what() << std::endl;
    }

    // 9. 清理资源
    if (generator) OgaDestroyGenerator(generator);
    if (params) OgaDestroyGeneratorParams(params);
    if (tokenizer_stream) OgaDestroyTokenizerStream(tokenizer_stream);
    if (tokenizer) OgaDestroyTokenizer(tokenizer);
    if (model) OgaDestroyModel(model);

    return 0;
}