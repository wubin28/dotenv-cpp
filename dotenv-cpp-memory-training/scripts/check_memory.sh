#!/bin/bash
#
# 内存检测脚本 - Linux/Mac版本
# 用于编译和检测 dotenv-cpp 内存练习项目的内存问题
#
# 使用方法:
#   ./check_memory.sh v1    # 检测问题1
#   ./check_memory.sh v2    # 检测问题2
#   ./check_memory.sh v3    # 检测问题3
#   ./check_memory.sh v4    # 检测问题4
#

set -e  # 遇到错误时退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ============================================
# 函数：打印带颜色的消息
# ============================================
print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# ============================================
# 函数：检查环境
# ============================================
check_env() {
    print_info "检查开发环境..."

    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake未安装，请先安装CMake (>= 3.10)"
        exit 1
    fi
    CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
    print_success "CMake版本: $CMAKE_VERSION"

    # 检查编译器
    if command -v g++ &> /dev/null; then
        GCC_VERSION=$(g++ --version | head -n1)
        print_success "编译器: $GCC_VERSION"
        COMPILER="g++"
    elif command -v clang++ &> /dev/null; then
        CLANG_VERSION=$(clang++ --version | head -n1)
        print_success "编译器: $CLANG_VERSION"
        COMPILER="clang++"
    else
        print_error "未找到C++编译器（g++或clang++）"
        exit 1
    fi

    # 检查Valgrind（可选）
    if command -v valgrind &> /dev/null; then
        VALGRIND_VERSION=$(valgrind --version)
        print_success "Valgrind: $VALGRIND_VERSION (可选工具已安装)"
        HAS_VALGRIND=1
    else
        print_warning "Valgrind未安装（可选，建议安装以获得更详细的报告）"
        HAS_VALGRIND=0
    fi

    echo ""
}

# ============================================
# 函数：编译代码
# ============================================
compile_code() {
    print_info "编译代码..."

    # 创建build目录
    if [ ! -d "build" ]; then
        mkdir build
        print_info "创建build目录"
    fi

    cd build

    # 运行CMake配置
    print_info "运行CMake配置（启用AddressSanitizer）..."
    cmake .. -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug > /dev/null

    # 编译指定版本
    print_info "编译 config_${VERSION}_buggy..."
    make config_${VERSION}_buggy 2>&1 | grep -E "(error|warning)" || true

    if [ -f "config_${VERSION}_buggy" ]; then
        print_success "编译成功"
    else
        print_error "编译失败"
        exit 1
    fi

    cd ..
    echo ""
}

# ============================================
# 函数：运行AddressSanitizer测试
# ============================================
run_asan_test() {
    print_info "运行AddressSanitizer检测..."

    # 设置ASan环境变量
    export ASAN_OPTIONS=detect_leaks=1:symbolize=1:abort_on_error=0

    # 运行程序并捕获输出
    cd build
    OUTPUT=$(./config_${VERSION}_buggy 2>&1 || true)
    cd ..

    # 检查是否有内存问题
    if echo "$OUTPUT" | grep -q "ERROR: AddressSanitizer"; then
        print_error "检测到内存问题！"
        echo ""
        echo "=== AddressSanitizer 报告 ==="
        echo "$OUTPUT" | grep -A 20 "ERROR: AddressSanitizer"
        echo ""
        return 1
    elif echo "$OUTPUT" | grep -q "ERROR: LeakSanitizer"; then
        print_error "检测到内存泄露！"
        echo ""
        echo "=== LeakSanitizer 报告 ==="
        echo "$OUTPUT" | grep -A 20 "ERROR: LeakSanitizer"
        echo ""
        return 1
    else
        print_success "AddressSanitizer未检测到问题"
        return 0
    fi
}

# ============================================
# 函数：运行Valgrind测试（可选）
# ============================================
run_valgrind_test() {
    if [ $HAS_VALGRIND -eq 0 ]; then
        print_warning "跳过Valgrind测试（未安装）"
        return 0
    fi

    print_info "运行Valgrind检测（可能需要较长时间）..."

    cd build
    VALGRIND_OUTPUT=$(valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./config_${VERSION}_buggy 2>&1 || true)
    cd ..

    # 检查是否有内存泄露
    if echo "$VALGRIND_OUTPUT" | grep -q "definitely lost:"; then
        LEAK_SIZE=$(echo "$VALGRIND_OUTPUT" | grep "definitely lost:" | head -n1)

        if echo "$LEAK_SIZE" | grep -q "0 bytes in 0 blocks"; then
            print_success "Valgrind未检测到明确的内存泄露"
            return 0
        else
            print_error "Valgrind检测到内存泄露！"
            echo ""
            echo "=== Valgrind 摘要 ==="
            echo "$VALGRIND_OUTPUT" | grep -A 10 "LEAK SUMMARY"
            echo ""
            return 1
        fi
    else
        print_success "Valgrind检测完成"
        return 0
    fi
}

# ============================================
# 函数：生成报告
# ============================================
print_report() {
    local asan_result=$1
    local valgrind_result=$2

    echo ""
    echo "========================================="
    echo "           检测报告 - 版本 ${VERSION}"
    echo "========================================="

    if [ $asan_result -eq 0 ]; then
        print_success "AddressSanitizer: 通过"
    else
        print_error "AddressSanitizer: 发现问题"
    fi

    if [ $HAS_VALGRIND -eq 1 ]; then
        if [ $valgrind_result -eq 0 ]; then
            print_success "Valgrind: 通过"
        else
            print_error "Valgrind: 发现问题"
        fi
    fi

    echo ""
    if [ $asan_result -ne 0 ] || [ $valgrind_result -ne 0 ]; then
        print_warning "建议修复步骤："
        echo "  1. 查看上方的详细报告，定位问题代码行号"
        echo "  2. 参考文档: docs/0${VERSION:1}-problem-*.md"
        echo "  3. 使用GitHub Copilot分析和修复问题"
        echo "  4. 重新运行此脚本验证修复效果"
    else
        print_success "恭喜！未检测到内存问题"
        echo ""
        echo "  如果这是问题代码（buggy版本），请确认："
        echo "  - 是否使用了正确的可执行文件？"
        echo "  - 是否已经修复了代码？"
    fi

    echo "========================================="
    echo ""
}

# ============================================
# 主程序
# ============================================

# 检查参数
if [ $# -eq 0 ]; then
    print_error "使用方法: $0 <version>"
    echo ""
    echo "示例:"
    echo "  $0 v1    # 检测问题1"
    echo "  $0 v2    # 检测问题2"
    echo "  $0 v3    # 检测问题3"
    echo "  $0 v4    # 检测问题4"
    exit 1
fi

VERSION=$1

# 验证版本号
if [[ ! "$VERSION" =~ ^v[1-4]$ ]]; then
    print_error "无效的版本号: $VERSION"
    echo "有效的版本号: v1, v2, v3, v4"
    exit 1
fi

echo ""
echo "========================================="
echo "  内存检测工具 - dotenv-cpp 练习"
echo "========================================="
echo ""

# 执行检查流程
check_env
compile_code

asan_result=0
valgrind_result=0

run_asan_test || asan_result=$?
run_valgrind_test || valgrind_result=$?

print_report $asan_result $valgrind_result

# 返回状态码
if [ $asan_result -ne 0 ] || [ $valgrind_result -ne 0 ]; then
    exit 1
else
    exit 0
fi
