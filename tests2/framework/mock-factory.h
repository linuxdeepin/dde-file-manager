#pragma once

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QSignalSpy>
#include <QTest>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QPoint>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QElapsedTimer>
#include <QScopeGuard>
#include <memory>
#include <functional>
#include <unordered_map>
#include <typeinfo>

/**
 * @brief DDE文件管理器Mock工厂和测试辅助工具
 * 
 * 提供Qt对象的模拟创建功能，支持信号/槽测试和各种事件模拟。
 */
namespace DFMTest {

/**
 * @brief 信号测试器增强版
 * 
 * 提供更丰富的信号测试功能，支持条件等待和参数验证。
 */
class AdvancedSignalTester {
public:
    /**
     * @brief 构造函数
     * @param sender 信号发送者
     * @param signal 信号（使用SIGNAL宏格式）
     */
    template<typename T>
    AdvancedSignalTester(const T* sender, const char* signal) 
        : spy_(sender, signal), sender_name_(sender->objectName()) {
        if (sender_name_.isEmpty()) {
            sender_name_ = QString("Object_%1").arg(reinterpret_cast<quintptr>(sender), 0, 16);
        }
    }
    
    /**
     * @brief 等待信号发射
     * @param timeout 超时时间（毫秒）
     * @return 是否在超时前收到信号
     */
    bool waitForSignal(int timeout = 5000) {
        return spy_.wait(timeout);
    }
    
    /**
     * @brief 等待信号发射指定次数
     * @param count 期望的信号次数
     * @param timeout 超时时间（毫秒）
     * @return 是否达到期望次数
     */
    bool waitForSignals(int count, int timeout = 5000) {
        QElapsedTimer timer;
        timer.start();
        
        while (spy_.count() < count && timer.elapsed() < timeout) {
            QTest::qWait(10);
        }
        
        return spy_.count() >= count;
    }
    
    /**
     * @brief 验证信号发射次数
     * @param expectedCount 期望次数
     * @return 是否匹配
     */
    bool verifyCount(int expectedCount) const {
        return spy_.count() == expectedCount;
    }
    
    /**
     * @brief 验证信号参数
     * @param index 信号索引
     * @param expectedArgs 期望的参数列表
     * @return 是否匹配
     */
    bool verifyArguments(int index, const QList<QVariant>& expectedArgs) const {
        if (index >= spy_.count()) {
            return false;
        }
        
        QList<QVariant> actualArgs = spy_.at(index);
        return actualArgs == expectedArgs;
    }
    
    /**
     * @brief 获取信号参数
     * @param index 信号索引
     * @return 参数列表
     */
    QList<QVariant> getArguments(int index = 0) const {
        if (index < spy_.count()) {
            return spy_.at(index);
        }
        return {};
    }
    
    /**
     * @brief 获取第一个参数
     * @param index 信号索引
     * @return 第一个参数
     */
    template<typename T>
    T getFirstArgument(int index = 0) const {
        auto args = getArguments(index);
        if (!args.isEmpty()) {
            return args.first().value<T>();
        }
        return T{};
    }
    
    /**
     * @brief 清空收集的信号
     */
    void clear() {
        spy_.clear();
    }
    
    /**
     * @brief 获取收集的信号数量
     */
    int count() const {
        return spy_.count();
    }
    
    /**
     * @brief 打印调试信息
     */
    void printDebugInfo() const {
        qDebug() << QString("SignalTester for %1: %2 signals collected")
                      .arg(sender_name_).arg(spy_.count());
        for (int i = 0; i < spy_.count(); ++i) {
            qDebug() << QString("  Signal %1:").arg(i) << spy_.at(i);
        }
    }

private:
    QSignalSpy spy_;
    QString sender_name_;
};

/**
 * @brief 事件模拟器
 * 
 * 提供各种Qt事件的模拟功能，支持鼠标、键盘、定时器等事件。
 */
class EventSimulator {
public:
    /**
     * @brief 模拟鼠标点击
     * @param widget 目标控件
     * @param position 点击位置（相对于控件）
     * @param button 鼠标按钮
     * @param modifiers 修饰键
     */
    static void simulateMouseClick(QWidget* widget, const QPoint& position = QPoint(),
                                 Qt::MouseButton button = Qt::LeftButton,
                                 Qt::KeyboardModifiers modifiers = Qt::NoModifier) {
        if (!widget) return;
        
        QPoint pos = position.isNull() ? widget->rect().center() : position;
        
        // 鼠标按下
        QMouseEvent pressEvent(QEvent::MouseButtonPress, pos, widget->mapToGlobal(pos),
                               button, button, modifiers);
        QApplication::sendEvent(widget, &pressEvent);
        
        // 短暂延迟
        QTest::qWait(10);
        
        // 鼠标释放
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, pos, widget->mapToGlobal(pos),
                                 button, Qt::NoButton, modifiers);
        QApplication::sendEvent(widget, &releaseEvent);
    }
    
    /**
     * @brief 模拟鼠标双击
     * @param widget 目标控件
     * @param position 点击位置
     * @param button 鼠标按钮
     */
    static void simulateMouseDoubleClick(QWidget* widget, const QPoint& position = QPoint(),
                                       Qt::MouseButton button = Qt::LeftButton) {
        if (!widget) return;
        
        QPoint pos = position.isNull() ? widget->rect().center() : position;
        
        QMouseEvent doubleClickEvent(QEvent::MouseButtonDblClick, pos, widget->mapToGlobal(pos),
                                     button, button, Qt::NoModifier);
        QApplication::sendEvent(widget, &doubleClickEvent);
    }
    
    /**
     * @brief 模拟键盘按键
     * @param widget 目标控件
     * @param key 按键
     * @param modifiers 修饰键
     * @param text 文字（对于字符键）
     */
    static void simulateKeyPress(QWidget* widget, Qt::Key key,
                               Qt::KeyboardModifiers modifiers = Qt::NoModifier,
                               const QString& text = QString()) {
        if (!widget) return;
        
        QKeyEvent keyEvent(QEvent::KeyPress, key, modifiers, text);
        QApplication::sendEvent(widget, &keyEvent);
        
        QTest::qWait(10);
        
        QKeyEvent keyReleaseEvent(QEvent::KeyRelease, key, modifiers, text);
        QApplication::sendEvent(widget, &keyReleaseEvent);
    }
    
    /**
     * @brief 模拟文本输入
     * @param widget 目标控件
     * @param text 要输入的文本
     */
    static void simulateTextInput(QWidget* widget, const QString& text) {
        if (!widget || text.isEmpty()) return;
        
        for (const QChar& ch : text) {
            QKeyEvent keyEvent(QEvent::KeyPress, 0, Qt::NoModifier, ch);
            QApplication::sendEvent(widget, &keyEvent);
            QTest::qWait(5);
        }
    }
    
    /**
     * @brief 模拟定时器超时
     * @param timer 定时器对象
     */
    static void simulateTimerTimeout(QTimer* timer) {
        if (timer && timer->isActive()) {
            emit timer->timeout();
        }
    }
    
    /**
     * @brief 等待并处理事件
     * @param ms 等待时间（毫秒）
     */
    static void processEventsFor(int ms) {
        QElapsedTimer timer;
        timer.start();
        
        while (timer.elapsed() < ms) {
            QApplication::processEvents();
            QTest::qWait(10);
        }
    }
};

/**
 * @brief 通用Mock工厂
 * 
 * 为各种Qt对象创建配置好的模拟实例。
 */
template<typename QtObject>
class MockFactory {
public:
    /**
     * @brief 创建Mock对象
     * @param parent 父对象
     * @return 配置好的Mock对象
     */
    static std::unique_ptr<QtObject> createMock(QObject* parent = nullptr) {
        auto mock = std::make_unique<QtObject>(parent);
        configureForTesting(mock.get());
        return mock;
    }
    
    /**
     * @brief 创建具名Mock对象
     * @param name 对象名称
     * @param parent 父对象
     * @return 配置好的Mock对象
     */
    static std::unique_ptr<QtObject> createNamedMock(const QString& name, QObject* parent = nullptr) {
        auto mock = createMock(parent);
        mock->setObjectName(name);
        return mock;
    }

private:
    /**
     * @brief 为测试配置对象
     */
    static void configureForTesting(QtObject* object) {
        if constexpr (std::is_base_of_v<QObject, QtObject>) {
            // 设置对象名称便于调试
            if (object->objectName().isEmpty()) {
                object->setObjectName(QString("Mock_%1_%2")
                                    .arg(typeid(QtObject).name())
                                    .arg(reinterpret_cast<quintptr>(object), 0, 16));
            }
        }
        
        if constexpr (std::is_base_of_v<QWidget, QtObject>) {
            // 为Widget设置测试友好的属性
            QWidget* widget = static_cast<QWidget*>(object);
            widget->setAttribute(Qt::WA_DontShowOnScreen, true);
            widget->resize(100, 100);  // 默认大小
        }
    }
};

/**
 * @brief DFM特定的Mock工厂
 * 
 * 为DFM项目中的特定类型提供专门的Mock创建方法。
 * 注意：这些方法需要在相应的类可用时才能使用。
 */
class DFMSpecificMocks {
public:
    /**
     * @brief 创建Mock EventChannel
     * 注意：需要在EventChannel类可用时使用
     */
    template<typename EventChannelType>
    static std::unique_ptr<EventChannelType> createMockEventChannel() {
        auto mock = MockFactory<EventChannelType>::createNamedMock("MockEventChannel");
        // 这里可以添加EventChannel特定的配置
        return mock;
    }
    
    /**
     * @brief 创建Mock FileInfo
     * 注意：需要在FileInfo类可用时使用
     */
    template<typename FileInfoType>
    static std::unique_ptr<FileInfoType> createMockFileInfo(const QString& filePath = "/tmp/test") {
        auto mock = MockFactory<FileInfoType>::createNamedMock("MockFileInfo");
        // 这里可以添加FileInfo特定的配置
        return mock;
    }
    
    /**
     * @brief 创建Mock Plugin
     * 注意：需要在Plugin类可用时使用
     */
    template<typename PluginType>
    static std::unique_ptr<PluginType> createMockPlugin(const QString& pluginName = "TestPlugin") {
        auto mock = MockFactory<PluginType>::createNamedMock(QString("Mock_%1").arg(pluginName));
        // 这里可以添加Plugin特定的配置
        return mock;
    }
};

/**
 * @brief 测试辅助工具集合
 */
class TestUtils {
public:
    /**
     * @brief 创建临时文件
     * @param suffix 文件后缀
     * @param content 文件内容
     * @return 临时文件路径
     */
    static QString createTempFile(const QString& suffix = ".txt", const QString& content = "") {
        QString tempDir = qgetenv("DFM_TEST_DATA_DIR");
        if (tempDir.isEmpty()) {
            tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        }
        
        QString fileName = QString("test_%1_%2%3")
                            .arg(QDateTime::currentMSecsSinceEpoch())
                            .arg(qrand())
                            .arg(suffix);
        
        QString filePath = QDir(tempDir).absoluteFilePath(fileName);
        
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << content;
        }
        
        return filePath;
    }
    
    /**
     * @brief 创建临时目录
     * @param prefix 目录前缀
     * @return 临时目录路径
     */
    static QString createTempDir(const QString& prefix = "test_dir") {
        QString tempBase = qgetenv("DFM_TEST_DATA_DIR");
        if (tempBase.isEmpty()) {
            tempBase = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        }
        
        QString dirName = QString("%1_%2_%3")
                           .arg(prefix)
                           .arg(QDateTime::currentMSecsSinceEpoch())
                           .arg(qrand());
        
        QString dirPath = QDir(tempBase).absoluteFilePath(dirName);
        QDir().mkpath(dirPath);
        
        return dirPath;
    }
    
    /**
     * @brief 清理临时文件或目录
     * @param path 路径
     */
    static void cleanupTempPath(const QString& path) {
        QFileInfo info(path);
        if (info.isFile()) {
            QFile::remove(path);
        } else if (info.isDir()) {
            QDir(path).removeRecursively();
        }
    }
    
    /**
     * @brief 比较两个文件内容
     * @param file1 文件1路径
     * @param file2 文件2路径
     * @return 内容是否相同
     */
    static bool compareFiles(const QString& file1, const QString& file2) {
        QFile f1(file1), f2(file2);
        
        if (!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly)) {
            return false;
        }
        
        return f1.readAll() == f2.readAll();
    }
};

} // namespace DFMTest

// 便利宏定义

/**
 * @brief 创建信号监听器的便利宏
 */
#define ADVANCED_SIGNAL_SPY(object, signal) \
    DFMTest::AdvancedSignalTester(object, signal)

/**
 * @brief 验证信号发射次数的便利宏
 */
#define VERIFY_SIGNAL_COUNT(spy, count) \
    EXPECT_TRUE(spy.verifyCount(count)) << "Expected " << count << " signals, got " << spy.count()

/**
 * @brief 等待信号发射的便利宏
 */
#define WAIT_FOR_SIGNAL_TIMEOUT(spy, timeout) \
    EXPECT_TRUE(spy.waitForSignal(timeout)) << "Signal not emitted within " << timeout << "ms"

/**
 * @brief 模拟鼠标点击的便利宏
 */
#define SIMULATE_CLICK(widget, pos) \
    DFMTest::EventSimulator::simulateMouseClick(widget, pos)

/**
 * @brief 模拟键盘输入的便利宏
 */
#define SIMULATE_KEY(widget, key) \
    DFMTest::EventSimulator::simulateKeyPress(widget, key)

/**
 * @brief 模拟文本输入的便利宏
 */
#define SIMULATE_TEXT(widget, text) \
    DFMTest::EventSimulator::simulateTextInput(widget, text)

/**
 * @brief 创建临时文件的便利宏
 */
#define CREATE_TEMP_FILE(suffix, content) \
    DFMTest::TestUtils::createTempFile(suffix, content)

/**
 * @brief 创建临时目录的便利宏
 */
#define CREATE_TEMP_DIR(prefix) \
    DFMTest::TestUtils::createTempDir(prefix)

/**
 * @brief RAII临时路径清理器
 */
#define TEMP_PATH_CLEANER(path) \
    auto cleaner_##__LINE__ = [&](){ DFMTest::TestUtils::cleanupTempPath(path); }; \
    QScopeGuard guard_##__LINE__(cleaner_##__LINE__) 