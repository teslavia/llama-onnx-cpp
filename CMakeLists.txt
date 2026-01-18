cmake_minimum_required(VERSION 3.10)
project(LlamaOnnxCpp)

set(CMAKE_CXX_STANDARD 17)

# 设置库路径
set(LIB_DIR "${CMAKE_SOURCE_DIR}/lib")
include_directories(${LIB_DIR}/include)
link_directories(${LIB_DIR})

add_executable(llama_inference src/main.cpp)

# 链接 onnxruntime-genai 库
target_link_libraries(llama_inference onnxruntime-genai)

# 设置 RPATH，以便运行时能找到动态库
if(APPLE)
    set_target_properties(llama_inference PROPERTIES INSTALL_RPATH "@loader_path/../lib")
endif()