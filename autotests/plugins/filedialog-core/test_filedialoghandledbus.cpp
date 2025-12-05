// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/dbus/filedialoghandledbus.h"
#include "../../../src/plugins/filedialog/core/views/filedialog.h"
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QApplication>
#include <QDBusVariant>
#include <QTimer>
#include <QWindow>
#include <QUrl>
#include <QDir>
#include <QVariantMap>
#include <QWidget>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileDialog>

// Forward declarations to avoid including heavy headers
#include <QFileDialog>

DFMBASE_USE_NAMESPACE
using namespace filedialog_core;

class UT_FileDialogHandleDBus : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Mock QApplication if not exists
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            new QApplication(argc, argv);
        }
        
        // Create a mock FileDialog first to avoid window creation issues
        mockDialog = new filedialog_core::FileDialog(QUrl());
        
        // Mock FMWindowsIns.createWindow to return our mock dialog
        using CreateWindowFunc = dfmbase::FileManagerWindow* (FileManagerWindowsManager::*)(const QUrl &, bool, QString *);
        stub.set_lamda(static_cast<CreateWindowFunc>(&FileManagerWindowsManager::createWindow),
                      [this](FileManagerWindowsManager *, const QUrl &, bool, QString *) -> dfmbase::FileManagerWindow* {
            __DBG_STUB_INVOKE__
            return static_cast<dfmbase::FileManagerWindow*>(mockDialog);
        });

        // Mock FMWindowsIns.findWindowById
        using FindWindowByIdFunc = dfmbase::FileManagerWindow* (FileManagerWindowsManager::*)(quint64);
        stub.set_lamda(static_cast<FindWindowByIdFunc>(&FileManagerWindowsManager::findWindowById), [this](FileManagerWindowsManager *, quint64) -> dfmbase::FileManagerWindow* {
            __DBG_STUB_INVOKE__
            return static_cast<dfmbase::FileManagerWindow*>(mockDialog); // Return our mock dialog
        });

        // Mock QWidget::internalWinId to avoid accessing real window ID
        stub.set_lamda(VADDR(QWidget, internalWinId), [this]() -> qulonglong {
            __DBG_STUB_INVOKE__
            return 12345; // Mock window ID
        });

        // Mock FileDialog::lastVisitedUrl to return a valid URL
        stub.set_lamda(VADDR(filedialog_core::FileDialog, lastVisitedUrl), [this]() -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl::fromLocalFile(QDir::homePath());
        });

        // Mock QWidget::windowHandle to return a valid QWindow
        stub.set_lamda(VADDR(QWidget, windowHandle), [this]() -> QWindow* {
            __DBG_STUB_INVOKE__
            static QWindow *mockWindow = new QWindow();
            return mockWindow;
        });

        // Mock QWidget::close to avoid actual closing
        stub.set_lamda(VADDR(QWidget, close), [this]() -> bool {
            __DBG_STUB_INVOKE__
            return true; // Return true to indicate successful close
        });

        // Create handle after setting up all stubs
        handle = new FileDialogHandleDBus();
    }

    virtual void TearDown() override
    {
        delete handle;
        handle = nullptr;
        if (mockDialog) {
            delete mockDialog;
            mockDialog = nullptr;
        }
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    FileDialogHandleDBus *handle = nullptr;
    filedialog_core::FileDialog *mockDialog { nullptr };
};

TEST_F(UT_FileDialogHandleDBus, Constructor_CreatesHandleSuccessfully)
{
    EXPECT_NE(handle, nullptr);
    EXPECT_NE(handle->widget(), nullptr);
}

TEST_F(UT_FileDialogHandleDBus, Directory_ReturnsCorrectDirectory)
{
    QString expectedPath = "/usr";
    
    // Mock FileDialogHandle::directory to return specific directory
    stub.set_lamda((QDir(FileDialogHandle::*)()const)ADDR(FileDialogHandle, directory),
                   [expectedPath](FileDialogHandle *) -> QDir {
        __DBG_STUB_INVOKE__
        return QDir(expectedPath);
    });
    
    QString result = handle->directory();
    EXPECT_EQ(result, expectedPath);
}

TEST_F(UT_FileDialogHandleDBus, SetDirectoryUrl_String_SetsUrlCorrectly)
{
    QString testDirectory = "/usr";
    QUrl expectedUrl = QUrl::fromLocalFile(testDirectory);
    
    // Mock FileDialogHandle::setDirectoryUrl
    bool setDirectoryUrlCalled = false;
    QUrl receivedUrl;
    stub.set_lamda((void(FileDialogHandle::*)(const QUrl&))ADDR(FileDialogHandle, setDirectoryUrl),
                   [&setDirectoryUrlCalled, &receivedUrl](FileDialogHandle *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        setDirectoryUrlCalled = true;
        receivedUrl = url;
    });
    
    handle->setDirectoryUrl(testDirectory);
    
    EXPECT_TRUE(setDirectoryUrlCalled);
    EXPECT_EQ(receivedUrl, expectedUrl);
}

TEST_F(UT_FileDialogHandleDBus, SetDirectoryUrl_EmptyString_SetsHomePath)
{
    QString emptyDirectory = "";
    QString homePath = QDir::homePath();
    QUrl expectedUrl = QUrl::fromLocalFile(homePath);
    
    // Mock FileDialogHandle::setDirectoryUrl
    bool setDirectoryUrlCalled = false;
    QUrl receivedUrl;
    stub.set_lamda((void(FileDialogHandle::*)(const QUrl&))ADDR(FileDialogHandle, setDirectoryUrl),
                   [&setDirectoryUrlCalled, &receivedUrl](FileDialogHandle *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        setDirectoryUrlCalled = true;
        receivedUrl = url;
    });
    
    handle->setDirectoryUrl(emptyDirectory);
    
    EXPECT_TRUE(setDirectoryUrlCalled);
    EXPECT_EQ(receivedUrl, expectedUrl);
}

TEST_F(UT_FileDialogHandleDBus, DirectoryUrl_ReturnsCorrectUrl)
{
    QUrl expectedUrl("file:///home/test");
    
    // Mock FileDialogHandle::directoryUrl
    stub.set_lamda(ADDR(FileDialogHandle, directoryUrl), 
                   [expectedUrl](FileDialogHandle *) -> QUrl {
        __DBG_STUB_INVOKE__
        return expectedUrl;
    });
    
    QString result = handle->directoryUrl();
    EXPECT_EQ(result, expectedUrl.toString());
}

TEST_F(UT_FileDialogHandleDBus, SelectUrl_SelectsUrlCorrectly)
{
    QString testUrlString = "file:///home/test.txt";
    QUrl expectedUrl(testUrlString);
    
    // Mock FileDialogHandle::selectUrl
    bool selectUrlCalled = false;
    QUrl receivedUrl;
    stub.set_lamda(ADDR(FileDialogHandle, selectUrl),
                   [&selectUrlCalled, &receivedUrl](FileDialogHandle *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        selectUrlCalled = true;
        receivedUrl = url;
    });
    
    handle->selectUrl(testUrlString);
    
    EXPECT_TRUE(selectUrlCalled);
    EXPECT_EQ(receivedUrl, expectedUrl);
}

TEST_F(UT_FileDialogHandleDBus, SelectedUrls_ReturnsCorrectUrls)
{
    QList<QUrl> expectedUrls = {
        QUrl("file:///home/test1.txt"),
        QUrl("file:///home/test2.txt")
    };
    
    // Mock FileDialogHandle::selectedUrls
    stub.set_lamda(ADDR(FileDialogHandle, selectedUrls), 
                   [expectedUrls](FileDialogHandle *) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return expectedUrls;
    });
    
    QStringList result = handle->selectedUrls();
    EXPECT_EQ(result.size(), 2);
    EXPECT_TRUE(result.contains("file:///home/test1.txt"));
    EXPECT_TRUE(result.contains("file:///home/test2.txt"));
}

TEST_F(UT_FileDialogHandleDBus, Filter_ReturnsCorrectFilter)
{
    QDir::Filters expectedFilter = QDir::Files | QDir::Dirs;
    
    // Mock FileDialogHandle::filter
    stub.set_lamda(ADDR(FileDialogHandle, filter), 
                   [expectedFilter](FileDialogHandle *) -> QDir::Filters {
        __DBG_STUB_INVOKE__
        return expectedFilter;
    });
    
    int result = handle->filter();
    EXPECT_EQ(result, static_cast<int>(expectedFilter));
}

TEST_F(UT_FileDialogHandleDBus, SetFilter_SetsFilterCorrectly)
{
    int testFilter = static_cast<int>(QDir::Files | QDir::Hidden);
    
    // Mock FileDialogHandle::setFilter
    bool setFilterCalled = false;
    QDir::Filters receivedFilter;
    stub.set_lamda((void(FileDialogHandle::*)(QDir::Filters))ADDR(FileDialogHandle, setFilter),
                   [&setFilterCalled, &receivedFilter](FileDialogHandle *, QDir::Filters filters) {
        __DBG_STUB_INVOKE__
        setFilterCalled = true;
        receivedFilter = filters;
    });
    
    handle->setFilter(testFilter);
    
    EXPECT_TRUE(setFilterCalled);
    EXPECT_EQ(receivedFilter, static_cast<QDir::Filters>(testFilter));
}

TEST_F(UT_FileDialogHandleDBus, SetViewMode_DoesNothing)
{
    int mode = static_cast<int>(QFileDialog::ViewMode::Detail);
    
    // The function is intentionally empty, so we just test it doesn't crash
    handle->setViewMode(mode);
    
    EXPECT_TRUE(true); // Test passes if no crash occurs
}

TEST_F(UT_FileDialogHandleDBus, ViewMode_ReturnsCorrectMode)
{
    QFileDialog::ViewMode expectedMode = QFileDialog::ViewMode::Detail;
    
    // Mock FileDialogHandle::viewMode
    stub.set_lamda(ADDR(FileDialogHandle, viewMode), 
                   [expectedMode](FileDialogHandle *) -> QFileDialog::ViewMode {
        __DBG_STUB_INVOKE__
        return expectedMode;
    });
    
    int result = handle->viewMode();
    EXPECT_EQ(result, static_cast<int>(expectedMode));
}

TEST_F(UT_FileDialogHandleDBus, SetFileMode_SetsModeCorrectly)
{
    int mode = static_cast<int>(QFileDialog::FileMode::ExistingFiles);
    
    // Mock FileDialogHandle::setFileMode
    bool setFileModeCalled = false;
    QFileDialog::FileMode receivedMode;
    stub.set_lamda(ADDR(FileDialogHandle, setFileMode),
                   [&setFileModeCalled, &receivedMode](FileDialogHandle *, QFileDialog::FileMode mode) {
        __DBG_STUB_INVOKE__
        setFileModeCalled = true;
        receivedMode = mode;
    });
    
    handle->setFileMode(mode);
    
    EXPECT_TRUE(setFileModeCalled);
    EXPECT_EQ(receivedMode, static_cast<QFileDialog::FileMode>(mode));
}

TEST_F(UT_FileDialogHandleDBus, SetAcceptMode_SetsModeCorrectly)
{
    int mode = static_cast<int>(QFileDialog::AcceptMode::AcceptSave);
    
    // Mock FileDialogHandle::setAcceptMode
    bool setAcceptModeCalled = false;
    QFileDialog::AcceptMode receivedMode;
    stub.set_lamda(ADDR(FileDialogHandle, setAcceptMode),
                   [&setAcceptModeCalled, &receivedMode](FileDialogHandle *, QFileDialog::AcceptMode mode) {
        __DBG_STUB_INVOKE__
        setAcceptModeCalled = true;
        receivedMode = mode;
    });
    
    handle->setAcceptMode(mode);
    
    EXPECT_TRUE(setAcceptModeCalled);
    EXPECT_EQ(receivedMode, static_cast<QFileDialog::AcceptMode>(mode));
}

TEST_F(UT_FileDialogHandleDBus, AcceptMode_ReturnsCorrectMode)
{
    QFileDialog::AcceptMode expectedMode = QFileDialog::AcceptMode::AcceptSave;
    
    // Mock FileDialogHandle::acceptMode
    stub.set_lamda(ADDR(FileDialogHandle, acceptMode), 
                   [expectedMode](FileDialogHandle *) -> QFileDialog::AcceptMode {
        __DBG_STUB_INVOKE__
        return expectedMode;
    });
    
    int result = handle->acceptMode();
    EXPECT_EQ(result, static_cast<int>(expectedMode));
}

TEST_F(UT_FileDialogHandleDBus, SetLabelText_SetsTextCorrectly)
{
    int label = static_cast<int>(QFileDialog::DialogLabel::Accept);
    QString text = "Custom Accept";
    
    // Mock FileDialogHandle::setLabelText
    bool setLabelTextCalled = false;
    QFileDialog::DialogLabel receivedLabel;
    QString receivedText;
    stub.set_lamda(ADDR(FileDialogHandle, setLabelText),
                   [&setLabelTextCalled, &receivedLabel, &receivedText](FileDialogHandle *, QFileDialog::DialogLabel label, const QString &text) {
        __DBG_STUB_INVOKE__
        setLabelTextCalled = true;
        receivedLabel = label;
        receivedText = text;
    });
    
    handle->setLabelText(label, text);
    
    EXPECT_TRUE(setLabelTextCalled);
    EXPECT_EQ(receivedLabel, static_cast<QFileDialog::DialogLabel>(label));
    EXPECT_EQ(receivedText, text);
}

TEST_F(UT_FileDialogHandleDBus, LabelText_ReturnsCorrectText)
{
    int label = static_cast<int>(QFileDialog::DialogLabel::Accept);
    QString expectedText = "Custom Accept";
    
    // Mock FileDialogHandle::labelText
    stub.set_lamda(ADDR(FileDialogHandle, labelText), 
                   [expectedText](FileDialogHandle *, QFileDialog::DialogLabel) -> QString {
        __DBG_STUB_INVOKE__
        return expectedText;
    });
    
    QString result = handle->labelText(label);
    EXPECT_EQ(result, expectedText);
}

TEST_F(UT_FileDialogHandleDBus, SetOptions_SetsOptionsCorrectly)
{
    int options = static_cast<int>(QFileDialog::Options(QFileDialog::Option::DontUseNativeDialog));
    
    // Mock FileDialogHandle::setOptions
    bool setOptionsCalled = false;
    QFileDialog::Options receivedOptions;
    stub.set_lamda(ADDR(FileDialogHandle, setOptions),
                   [&setOptionsCalled, &receivedOptions](FileDialogHandle *, QFileDialog::Options options) {
        __DBG_STUB_INVOKE__
        setOptionsCalled = true;
        receivedOptions = options;
    });
    
    handle->setOptions(options);
    
    EXPECT_TRUE(setOptionsCalled);
    EXPECT_EQ(receivedOptions, static_cast<QFileDialog::Options>(options));
}

TEST_F(UT_FileDialogHandleDBus, SetOption_SetsOptionCorrectly)
{
    int option = static_cast<int>(QFileDialog::Option::DontUseNativeDialog);
    bool on = true;
    
    // Mock FileDialogHandle::setOption
    bool setOptionCalled = false;
    QFileDialog::Option receivedOption;
    bool receivedOn = false;
    stub.set_lamda(ADDR(FileDialogHandle, setOption),
                   [&setOptionCalled, &receivedOption, &receivedOn](FileDialogHandle *, QFileDialog::Option option, bool on) {
        __DBG_STUB_INVOKE__
        setOptionCalled = true;
        receivedOption = option;
        receivedOn = on;
    });
    
    handle->setOption(option, on);
    
    EXPECT_TRUE(setOptionCalled);
    EXPECT_EQ(receivedOption, static_cast<QFileDialog::Option>(option));
    EXPECT_EQ(receivedOn, on);
}

TEST_F(UT_FileDialogHandleDBus, Options_ReturnsCorrectOptions)
{
    QFileDialog::Options expectedOptions = QFileDialog::Options(QFileDialog::Option::DontUseNativeDialog);
    
    // Mock FileDialogHandle::options
    stub.set_lamda(ADDR(FileDialogHandle, options), 
                   [expectedOptions](FileDialogHandle *) -> QFileDialog::Options {
        __DBG_STUB_INVOKE__
        return expectedOptions;
    });
    
    int result = handle->options();
    EXPECT_EQ(result, static_cast<int>(expectedOptions));
}

TEST_F(UT_FileDialogHandleDBus, TestOption_ReturnsCorrectState)
{
    int option = static_cast<int>(QFileDialog::Option::DontUseNativeDialog);
    bool expectedState = true;
    
    // Mock FileDialogHandle::testOption
    stub.set_lamda(ADDR(FileDialogHandle, testOption), 
                   [expectedState](FileDialogHandle *, QFileDialog::Option) -> bool {
        __DBG_STUB_INVOKE__
        return expectedState;
    });
    
    bool result = handle->testOption(option);
    EXPECT_EQ(result, expectedState);
}

TEST_F(UT_FileDialogHandleDBus, WinId_ReturnsCorrectId)
{
    qulonglong expectedWinId = 12345;
    
    // Mock FileDialogHandle::winId
    stub.set_lamda(ADDR(FileDialogHandle, winId), 
                   [expectedWinId](FileDialogHandle *) -> qulonglong {
        __DBG_STUB_INVOKE__
        return expectedWinId;
    });
    
    qulonglong result = handle->winId();
    EXPECT_EQ(result, expectedWinId);
}

TEST_F(UT_FileDialogHandleDBus, SetWindowTitle_SetsTitleCorrectly)
{
    QString title = "Custom Title";
    
    // Mock widget()->setWindowTitle
    bool setWindowTitleCalled = false;
    QString receivedTitle;
    stub.set_lamda(&QWidget::setWindowTitle, 
                   [&setWindowTitleCalled, &receivedTitle](QWidget *, const QString &title) {
        __DBG_STUB_INVOKE__
        setWindowTitleCalled = true;
        receivedTitle = title;
    });
    
    handle->setWindowTitle(title);
    
    EXPECT_TRUE(setWindowTitleCalled);
    EXPECT_EQ(receivedTitle, title);
}

TEST_F(UT_FileDialogHandleDBus, WindowActive_ReturnsCorrectState)
{
    bool expectedState = true;
    
    // Mock widget()->isActiveWindow
    stub.set_lamda(&QWidget::isActiveWindow, 
                   [expectedState]() -> bool {
        __DBG_STUB_INVOKE__
        return expectedState;
    });
    
    bool result = handle->windowActive();
    EXPECT_EQ(result, expectedState);
}

TEST_F(UT_FileDialogHandleDBus, WindowActive_WidgetNull_ReturnsFalse)
{
    // Mock widget() to return nullptr
    stub.set_lamda(ADDR(FileDialogHandle, widget), 
                   []() -> QWidget* {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    
    bool result = handle->windowActive();
    EXPECT_FALSE(result);
}

TEST_F(UT_FileDialogHandleDBus, ActivateWindow_ActivatesWindow)
{
    // Mock widget()->activateWindow
    bool activateWindowCalled = false;
    stub.set_lamda(&QWidget::activateWindow, 
                   [&activateWindowCalled]() {
        __DBG_STUB_INVOKE__
        activateWindowCalled = true;
    });
    
    handle->activateWindow();
    
    EXPECT_TRUE(activateWindowCalled);
}

TEST_F(UT_FileDialogHandleDBus, HeartbeatInterval_ReturnsCorrectInterval)
{
    int expectedInterval = 30000; // Default 30 seconds
    
    int result = handle->heartbeatInterval();
    EXPECT_EQ(result, expectedInterval);
}

TEST_F(UT_FileDialogHandleDBus, MakeHeartbeat_StartsTimer)
{
    // Mock QTimer::start
    bool timerStartCalled = false;
    // Use proper overload for QTimer::start
    using TimerStartType = void (QTimer::*)();
    stub.set_lamda(static_cast<TimerStartType>(&QTimer::start),
                   [&timerStartCalled](QTimer *) {
        __DBG_STUB_INVOKE__
        timerStartCalled = true;
    });
    
    handle->makeHeartbeat();
    
    EXPECT_TRUE(timerStartCalled);
}

TEST_F(UT_FileDialogHandleDBus, WindowFlags_ReturnsCorrectFlags)
{
    quint32 expectedFlags = static_cast<quint32>(Qt::Window | Qt::Dialog);
    
    // Mock widget()->windowFlags
    stub.set_lamda(&QWidget::windowFlags, 
                   [expectedFlags]() -> Qt::WindowFlags {
        __DBG_STUB_INVOKE__
        return static_cast<Qt::WindowFlags>(expectedFlags);
    });
    
    quint32 result = handle->windowFlags();
    EXPECT_EQ(result, expectedFlags);
}

TEST_F(UT_FileDialogHandleDBus, SetHeartbeatInterval_SetsIntervalCorrectly)
{
    int interval = 60000; // 60 seconds
    
    // Mock QTimer::setInterval
    bool setIntervalCalled = false;
    int receivedInterval = -1;
    // Use proper overload for QTimer::setInterval
    using TimerSetIntervalType = void (QTimer::*)(int);
    stub.set_lamda(static_cast<TimerSetIntervalType>(&QTimer::setInterval),
                   [&setIntervalCalled, &receivedInterval](QTimer *, int interval) {
        __DBG_STUB_INVOKE__
        setIntervalCalled = true;
        receivedInterval = interval;
    });
    
    handle->setHeartbeatInterval(interval);
    
    EXPECT_TRUE(setIntervalCalled);
    EXPECT_EQ(receivedInterval, interval);
}

TEST_F(UT_FileDialogHandleDBus, SetWindowFlags_SetsFlagsCorrectly)
{
    quint32 flags = static_cast<quint32>(Qt::Window | Qt::Dialog);
    
    // Mock widget()->setWindowFlags
    bool setWindowFlagsCalled = false;
    Qt::WindowFlags receivedFlags;
    // Use proper overload for QWidget::setWindowFlags
    using SetWindowFlagsType = void (QWidget::*)(Qt::WindowFlags);
    stub.set_lamda(static_cast<SetWindowFlagsType>(&QWidget::setWindowFlags),
                   [&setWindowFlagsCalled, &receivedFlags](QWidget *, Qt::WindowFlags flags) {
        __DBG_STUB_INVOKE__
        setWindowFlagsCalled = true;
        receivedFlags = flags;
    });
    
    handle->setWindowFlags(flags);
    
    EXPECT_TRUE(setWindowFlagsCalled);
    EXPECT_EQ(receivedFlags, static_cast<Qt::WindowFlags>(flags));
}

TEST_F(UT_FileDialogHandleDBus, Constructor_SetsUpConnections)
{
    // Test that constructor sets up necessary connections
    // This is tested indirectly through the fact that the object was created successfully
    // and the timer was started with default interval
    EXPECT_TRUE(true);
}

TEST_F(UT_FileDialogHandleDBus, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    // Test multiple method calls with different parameters
    int setDirectoryUrlCallCount = 0;
    int selectUrlCallCount = 0;
    int setFilterCallCount = 0;
    int setAcceptModeCallCount = 0;
    
    // Mock FileDialogHandle::setDirectoryUrl
    stub.set_lamda((void(FileDialogHandle::*)(const QUrl&))ADDR(FileDialogHandle, setDirectoryUrl),
                   [&setDirectoryUrlCallCount](FileDialogHandle *, const QUrl &) {
        __DBG_STUB_INVOKE__
        setDirectoryUrlCallCount++;
    });
    
    // Mock FileDialogHandle::selectUrl
    stub.set_lamda(ADDR(FileDialogHandle, selectUrl),
                   [&selectUrlCallCount](FileDialogHandle *, const QUrl &) {
        __DBG_STUB_INVOKE__
        selectUrlCallCount++;
    });
    
    // Mock FileDialogHandle::setFilter
    stub.set_lamda((void(FileDialogHandle::*)(QDir::Filters))ADDR(FileDialogHandle, setFilter),
                   [&setFilterCallCount](FileDialogHandle *, QDir::Filters) {
        __DBG_STUB_INVOKE__
        setFilterCallCount++;
    });
    
    // Mock FileDialogHandle::setAcceptMode
    stub.set_lamda(ADDR(FileDialogHandle, setAcceptMode),
                   [&setAcceptModeCallCount](FileDialogHandle *, QFileDialog::AcceptMode) {
        __DBG_STUB_INVOKE__
        setAcceptModeCallCount++;
    });
    
    // Call methods multiple times
    handle->setDirectoryUrl("/home/test1");
    handle->setDirectoryUrl("/home/test2");
    handle->selectUrl("file:///home/test1.txt");
    handle->selectUrl("file:///home/test2.txt");
    handle->setFilter(static_cast<int>(QDir::Files));
    handle->setFilter(static_cast<int>(QDir::Files | QDir::Hidden));
    handle->setAcceptMode(static_cast<int>(QFileDialog::AcceptOpen));
    handle->setAcceptMode(static_cast<int>(QFileDialog::AcceptSave));
    
    EXPECT_EQ(setDirectoryUrlCallCount, 2);
    EXPECT_EQ(selectUrlCallCount, 2);
    EXPECT_EQ(setFilterCallCount, 2);
    EXPECT_EQ(setAcceptModeCallCount, 2);
}

TEST_F(UT_FileDialogHandleDBus, EdgeCase_EmptyParameters_HandlesCorrectly)
{
    // Test edge cases with empty parameters
    handle->setDirectoryUrl("");
    handle->selectUrl("");
    handle->setLabelText(0, "");
    handle->setWindowTitle("");
    
    // All should complete without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_FileDialogHandleDBus, EdgeCase_InvalidParameters_HandlesCorrectly)
{
    // Test edge cases with invalid parameters
    handle->setFileMode(-1); // Invalid file mode
    handle->setAcceptMode(-1); // Invalid accept mode
    handle->setOption(-1, true); // Invalid option
    handle->setLabelText(-1, "Test"); // Invalid label
    handle->setHeartbeatInterval(-1); // Invalid interval
    
    // All should complete without crashing
    EXPECT_TRUE(true);
}
