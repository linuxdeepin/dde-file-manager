#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <memory>

#include "dfm-test-base.h"

/**
 * @brief DDE文件管理器测试环境设置类
 * 
 * 继承GTest的TestEventListener，在测试运行过程中进行环境配置和清理。
 */
class DFMTestEnvironment : public ::testing::TestEventListener {
public:
    /**
     * @brief 测试程序开始前的环境设置
     */
    void OnTestProgramStart(const ::testing::UnitTest& /*unit_test*/) override {
        setupTestEnvironment();
        printEnvironmentInfo();
    }
    
    /**
     * @brief 测试程序结束后的清理
     */
    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        printTestSummary(unit_test);
        cleanupTestEnvironment();
    }
    
    /**
     * @brief 测试用例开始前的准备
     */
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        current_test_name_ = QString("%1.%2")
                               .arg(test_info.test_case_name())
                               .arg(test_info.name());
        qDebug() << "开始测试:" << current_test_name_;
    }
    
    /**
     * @brief 测试用例结束后的清理
     */
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        if (test_info.result()->Failed()) {
            qWarning() << "测试失败:" << current_test_name_;
            // 在这里可以添加失败时的特殊处理，如截图、日志收集等
        } else {
            qDebug() << "测试通过:" << current_test_name_;
        }
    }

private:
    QString current_test_name_;
    
    /**
     * @brief 设置测试环境
     */
    void setupTestEnvironment() {
        // 设置Qt应用程序相关环境变量
        qputenv("QT_QPA_PLATFORM", "offscreen");
        qputenv("QT_LOGGING_RULES", "*.debug=true");
        qputenv("DISPLAY", ":99");  // 虚拟显示器
        
        // 设置测试数据目录
        QString testDataDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                            + "/dfm-tests2";
        QDir().mkpath(testDataDir);
        qputenv("DFM_TEST_DATA_DIR", testDataDir.toLocal8Bit());
        
        // 设置超时时间
        qputenv("QTEST_FUNCTION_TIMEOUT", "30000");  // 30秒超时
        
        // 禁用文件系统监视器（避免测试干扰）
        qputenv("DFM_DISABLE_FILE_WATCHER", "1");
        
        // 启用调试输出
        QLoggingCategory::setFilterRules("dfm.*.debug=true");
        
        qInfo() << "测试环境设置完成";
    }
    
    /**
     * @brief 打印环境信息
     */
    void printEnvironmentInfo() {
        qInfo() << "========================================";
        qInfo() << "DDE文件管理器测试环境信息";
        qInfo() << "========================================";
        qInfo() << "Qt版本:" << QT_VERSION_STR;
        qInfo() << "平台:" << QGuiApplication::platformName();
        qInfo() << "测试数据目录:" << qgetenv("DFM_TEST_DATA_DIR");
        qInfo() << "========================================";
    }
    
    /**
     * @brief 打印测试摘要
     */
    void printTestSummary(const ::testing::UnitTest& unit_test) {
        qInfo() << "========================================";
        qInfo() << "测试执行摘要";
        qInfo() << "========================================";
        qInfo() << "总测试数:" << unit_test.total_test_count();
        qInfo() << "通过测试数:" << unit_test.successful_test_count();
        qInfo() << "失败测试数:" << unit_test.failed_test_count();
        qInfo() << "跳过测试数:" << unit_test.skipped_test_count();
        qInfo() << "运行时间:" << unit_test.elapsed_time() << "ms";
        qInfo() << "========================================";
        
        if (unit_test.failed_test_count() > 0) {
            qWarning() << "有测试失败，请检查上方日志";
        }
    }
    
    /**
     * @brief 清理测试环境
     */
    void cleanupTestEnvironment() {
        // 清理测试数据目录
        QString testDataDir = qgetenv("DFM_TEST_DATA_DIR");
        if (!testDataDir.isEmpty()) {
            QDir dir(testDataDir);
            if (dir.exists()) {
                dir.removeRecursively();
                qDebug() << "清理测试数据目录:" << testDataDir;
            }
        }
        
        qInfo() << "测试环境清理完成";
    }
};

/**
 * @brief 解析命令行参数
 * @param argc 参数个数
 * @param argv 参数数组
 * @return 是否需要GUI支持
 */
bool parseCommandLineArgs(int argc, char** argv) {
    bool needGui = false;
    
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        
        if (arg == "--gui" || arg == "-g") {
            needGui = true;
            qInfo() << "启用GUI支持";
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "DDE文件管理器测试程序\n"
                      << "用法: " << argv[0] << " [选项] [GTest选项]\n"
                      << "\n选项:\n"
                      << "  --gui, -g     启用GUI支持（默认使用offscreen平台）\n"
                      << "  --help, -h    显示此帮助信息\n"
                      << "\n常用GTest选项:\n"
                      << "  --gtest_filter=<pattern>  运行匹配模式的测试\n"
                      << "  --gtest_list_tests        列出所有测试\n"
                      << "  --gtest_repeat=<count>    重复运行测试\n"
                      << "  --gtest_output=xml:<file> 输出XML格式报告\n"
                      << std::endl;
            return false;
        }
    }
    
    return needGui;
}

/**
 * @brief 设置异常处理
 */
void setupExceptionHandling() {
    // 设置未处理异常的处理器
    std::set_terminate([]() {
        qCritical() << "程序遇到未处理的异常，即将终止";
        
        // 尝试获取异常信息
        try {
            if (auto eptr = std::current_exception()) {
                std::rethrow_exception(eptr);
            }
        } catch (const std::exception& e) {
            qCritical() << "异常类型: std::exception";
            qCritical() << "异常信息:" << e.what();
        } catch (...) {
            qCritical() << "未知异常类型";
        }
        
        std::abort();
    });
}

/**
 * @brief 主函数
 * 
 * 支持GTest和Qt6Test的统一测试入口点，提供完整的测试环境配置。
 * 
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 程序退出码，0表示成功
 */
int main(int argc, char** argv) {
    // 设置异常处理
    setupExceptionHandling();
    
    // 解析命令行参数
    bool needGui = parseCommandLineArgs(argc, argv);
    if (argc > 1 && (QString::fromLocal8Bit(argv[1]) == "--help" || 
                     QString::fromLocal8Bit(argv[1]) == "-h")) {
        return 0;
    }
    
    // 创建Qt应用程序
    std::unique_ptr<QCoreApplication> app;
    if (needGui) {
        app = std::make_unique<QApplication>(argc, argv);
        qInfo() << "创建GUI应用程序";
    } else {
        app = std::make_unique<QCoreApplication>(argc, argv);
        qInfo() << "创建控制台应用程序";
    }
    
    // 设置应用程序信息
    app->setApplicationName("DFM-Tests2");
    app->setApplicationVersion("1.0.0");
    app->setOrganizationName("Deepin");
    app->setOrganizationDomain("deepin.org");
    
    // 初始化GTest
    ::testing::InitGoogleTest(&argc, argv);
    
    // 添加测试环境监听器
    ::testing::TestEventListeners& listeners = 
        ::testing::UnitTest::GetInstance()->listeners();
    
    // 移除默认的打印监听器（避免重复输出）
    delete listeners.Release(listeners.default_result_printer());
    
    // 添加我们自定义的监听器
    listeners.Append(new DFMTestEnvironment);
    
    // 添加简化的结果打印器
    listeners.Append(new ::testing::PrettyUnitTestResultPrinter);
    
    try {
        qInfo() << "开始运行测试...";
        
        // 运行所有测试
        int result = RUN_ALL_TESTS();
        
        // 处理Qt事件队列中的剩余事件
        app->processEvents();
        
        qInfo() << "测试运行完成，退出码:" << result;
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "测试运行期间发生异常:" << e.what();
        return -1;
    } catch (...) {
        qCritical() << "测试运行期间发生未知异常";
        return -1;
    }
} 