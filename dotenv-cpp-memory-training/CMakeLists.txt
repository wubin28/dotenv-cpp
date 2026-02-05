cmake_minimum_required(VERSION 3.10)
project(dotenv_memory_training CXX)

# 设置C++标准（与dotenv-cpp兼容）
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用所有警告
add_compile_options(-Wall -Wextra -Wpedantic)

# 可选：启用AddressSanitizer
# 使用方法：cmake .. -DENABLE_ASAN=ON
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

# 包含路径
# 假设dotenv.h位于上层目录的include路径
include_directories(${CMAKE_SOURCE_DIR}/../include/laserpants/dotenv)
include_directories(${CMAKE_SOURCE_DIR}/src/common)

# ============================================
# 问题代码可执行文件（buggy版本）
# ============================================

# 问题1：基本内存泄露
add_executable(config_v1_buggy
    src/buggy/config_manager_v1.cpp
)

# 问题2：double-free
add_executable(config_v2_buggy
    src/buggy/config_manager_v2.cpp
)

# 问题3：悬挂指针
add_executable(config_v3_buggy
    src/buggy/config_manager_v3.cpp
)

# 问题4：异常安全
add_executable(config_v4_buggy
    src/buggy/config_manager_v4.cpp
)

# ============================================
# 修复代码可执行文件（fixed版本）
# ============================================

# 问题1修复
add_executable(config_v1_fixed
    src/fixed/config_manager_v1_fixed.cpp
)

# 问题2修复
add_executable(config_v2_fixed
    src/fixed/config_manager_v2_fixed.cpp
)

# 问题3修复
add_executable(config_v3_fixed
    src/fixed/config_manager_v3_fixed.cpp
)

# 问题4修复
add_executable(config_v4_fixed
    src/fixed/config_manager_v4_fixed.cpp
)

# ============================================
# 测试可执行文件
# ============================================

# 测试1
add_executable(test_v1
    tests/test_v1.cpp
)

# 测试2
add_executable(test_v2
    tests/test_v2.cpp
)

# 测试3
add_executable(test_v3
    tests/test_v3.cpp
)

# 测试4
add_executable(test_v4
    tests/test_v4.cpp
)

# ============================================
# 安装规则（可选）
# ============================================

# 如果需要安装，可以取消注释以下内容
# install(TARGETS
#     config_v1_buggy config_v2_buggy config_v3_buggy config_v4_buggy
#     config_v1_fixed config_v2_fixed config_v3_fixed config_v4_fixed
#     DESTINATION bin
# )

# ============================================
# 打印配置信息
# ============================================

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
