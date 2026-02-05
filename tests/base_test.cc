#include <gtest/gtest.h>
#include <dotenv.h>

class BaseTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        dotenv::init(".env.example");
    }
};

TEST_F(BaseTestFixture, ReadUndefinedVariableWithDefaultValue) {
    const auto _value = dotenv::getenv("UNDEFINED_VAR", "EHLO");
    ASSERT_EQ(_value, "EHLO");
}

TEST_F(BaseTestFixture, ReadDefinedVariableWithDefaultValue) {
    const auto _value = dotenv::getenv("DEFINED_VAR", "EHLO");
    ASSERT_EQ(_value, "OLHE");
}

TEST_F(BaseTestFixture, VariableReferenceExpansion) {
    // 需要先在.env.example中添加测试数据
    const auto base = std::getenv("BASE");
    const auto expanded = std::getenv("EXPANDED");

    ASSERT_STREQ(base, "hello");
    ASSERT_STREQ(expanded, "hello world");
}

TEST(DotenvPreserveTest, PreserveExistingVariable) {
    // 设置已有环境变量
    _putenv("PRESERVE_TEST=original");

    // 创建临时.env文件
    std::ofstream env_file(".env.preserve_test");
    env_file << "PRESERVE_TEST=from_file\n";
    env_file.close();

    // 使用Preserve模式加载
    dotenv::init(dotenv::Preserve, ".env.preserve_test");

    // 应该保留原始值
    const char* value = std::getenv("PRESERVE_TEST");
    ASSERT_STREQ(value, "original");

    // 清理
    remove(".env.preserve_test");
}

TEST(DotenvErrorTest, InvalidFileDoesNotCrash) {
    // 加载不存在的文件不应该崩溃
    ASSERT_NO_THROW(dotenv::init("nonexistent.env"));
}

TEST(DotenvErrorTest, MalformedLineIgnored) {
    // 创建包含错误格式的.env文件
    std::ofstream env_file(".env.malformed");
    env_file << "VALID_VAR=value\n";
    env_file << "MALFORMED LINE WITHOUT EQUALS\n";  // 错误行
    env_file << "ANOTHER_VALID=value2\n";
    env_file.close();

    // 应该能加载（忽略错误行）
    ASSERT_NO_THROW(dotenv::init(".env.malformed"));

    // 有效的变量应该被加载
    ASSERT_STREQ(std::getenv("VALID_VAR"), "value");
    ASSERT_STREQ(std::getenv("ANOTHER_VALID"), "value2");

    remove(".env.malformed");
}