// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/dbus/filedialoghandle.h"
#include "../../../src/plugins/filedialog/core/views/filedialog.h"
#include "../../../src/plugins/filedialog/core/utils/corehelper.h"
#include "../../../src/plugins/filedialog/core/events/coreeventscaller.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/standardpaths.h>
#include <QApplication>
#include <QDir>
#include <QUrl>
#include <QDBusVariant>
#include <QFileDialog>
#include <QPointer>
#include <QTimer>
#include <QEventLoop>

DFMBASE_USE_NAMESPACE
using namespace filedialog_core;

class UT_FileDialogHandle : public testing::Test
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
        
        // Mock FMWindowsIns.createWindow first to avoid window creation issues
        mockDialog = new FileDialog(QUrl());
        stub.set_lamda(&FileManagerWindowsManager::createWindow, [&] {
            __DBG_STUB_INVOKE__
            return mockDialog;
        });

        // Mock FMWindowsIns.findWindowById
        stub.set_lamda(&FileManagerWindowsManager::findWindowById, [&] {
            __DBG_STUB_INVOKE__
            return mockDialog;
        });
        
        // Mock FileDialog constructor to avoid "Create window failed" error
        // This is a simplified approach to avoid complex Qt GUI initialization issues
        try {
            handle = new FileDialogHandle();
        } catch (...) {
            // If constructor fails, we'll still have a valid handle for testing
            handle = nullptr;
        }
    }

    virtual void TearDown() override
    {
        delete handle;
        handle = nullptr;
        mockDialog = nullptr;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    FileDialogHandle *handle = nullptr;
    QPointer<FileDialog> mockDialog;
};

TEST_F(UT_FileDialogHandle, Constructor_CreatesDialogSuccessfully)
{
    EXPECT_NE(handle, nullptr);
    EXPECT_NE(handle->widget(), nullptr);
}

TEST_F(UT_FileDialogHandle, SetParent_SetsParentCorrectly)
{
    QWidget parent;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setParent
    bool dialogSetParentCalled = false;
    QWidget *receivedParent = nullptr;
    testStub.set_lamda((void(FileDialog::*)(QWidget*))ADDR(FileDialog, setParent), [&dialogSetParentCalled, &receivedParent](FileDialog *, QWidget *parent) {
        __DBG_STUB_INVOKE__
        dialogSetParentCalled = true;
        receivedParent = parent;
    });
    
    // Mock QObject::setParent
    bool qobjectSetParentCalled = false;
    QWidget *qobjectReceivedParent = nullptr;
    testStub.set_lamda((void(QObject::*)(QObject*))ADDR(QObject, setParent), [&qobjectSetParentCalled, &qobjectReceivedParent](QObject *, QObject *parent) {
        __DBG_STUB_INVOKE__
        qobjectSetParentCalled = true;
        qobjectReceivedParent = qobject_cast<QWidget*>(parent);
    });
    
    handle->setParent(&parent);
    
    EXPECT_TRUE(dialogSetParentCalled);
    EXPECT_EQ(receivedParent, &parent);
    EXPECT_TRUE(qobjectSetParentCalled);
    EXPECT_EQ(qobjectReceivedParent, &parent);
}

TEST_F(UT_FileDialogHandle, SetDirectory_String_SetsDirectoryCorrectly)
{
    QString testDir = "/home/test";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setDirectory
    bool setDirectoryCalled = false;
    QString receivedDir;
    testStub.set_lamda((void(FileDialog::*)(const QString&))ADDR(FileDialog, setDirectory),
                       [&setDirectoryCalled, &receivedDir](FileDialog *, const QString &directory) {
        __DBG_STUB_INVOKE__
        setDirectoryCalled = true;
        receivedDir = directory;
    });

    handle->setDirectory(testDir);
    EXPECT_TRUE(setDirectoryCalled);
    EXPECT_EQ(receivedDir, testDir);
}

TEST_F(UT_FileDialogHandle, SetDirectory_QDir_SetsDirectoryCorrectly)
{
    QDir testDir("/home/test");
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setDirectory
    bool setDirectoryCalled = false;
    QDir receivedDir;
    testStub.set_lamda((void(FileDialog::*)(const QDir&))ADDR(FileDialog, setDirectory),
                       [&setDirectoryCalled, &receivedDir](FileDialog *, const QDir &directory) {
        __DBG_STUB_INVOKE__
        setDirectoryCalled = true;
        receivedDir = directory;
    });

    handle->setDirectory(testDir);
    EXPECT_TRUE(setDirectoryCalled);
    EXPECT_EQ(receivedDir, testDir);
}

TEST_F(UT_FileDialogHandle, Directory_ReturnsCorrectDirectory)
{
    QDir expectedDir("/home/test");
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::directory
    testStub.set_lamda(ADDR(FileDialog, directory), [expectedDir](FileDialog *) -> QDir {
        __DBG_STUB_INVOKE__
        return expectedDir;
    });

    QDir result = handle->directory();
    EXPECT_EQ(result, expectedDir);
}

TEST_F(UT_FileDialogHandle, SetDirectoryUrl_SetsUrlCorrectly)
{
    QUrl testUrl("file:///home/test");
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setDirectoryUrl
    bool setDirectoryUrlCalled = false;
    QUrl receivedUrl;
    testStub.set_lamda(ADDR(FileDialog, setDirectoryUrl),
                       [&setDirectoryUrlCalled, &receivedUrl](FileDialog *, const QUrl &directory) {
        __DBG_STUB_INVOKE__
        setDirectoryUrlCalled = true;
        receivedUrl = directory;
    });

    handle->setDirectoryUrl(testUrl);
    EXPECT_TRUE(setDirectoryUrlCalled);
    EXPECT_EQ(receivedUrl, testUrl);
}

TEST_F(UT_FileDialogHandle, DirectoryUrl_ReturnsCorrectUrl)
{
    QUrl expectedUrl("file:///home/test");
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::directoryUrl
    testStub.set_lamda(ADDR(FileDialog, directoryUrl), [expectedUrl](FileDialog *) -> QUrl {
        __DBG_STUB_INVOKE__
        return expectedUrl;
    });

    QUrl result = handle->directoryUrl();
    EXPECT_EQ(result, expectedUrl);
}

TEST_F(UT_FileDialogHandle, SelectFile_SelectsFileCorrectly)
{
    QString testFile = "test.txt";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock CoreHelper::delayInvokeProxy
    bool delayInvokeCalled = false;
    QString receivedFile;
    testStub.set_lamda(&CoreHelper::delayInvokeProxy,
                       [&delayInvokeCalled, &receivedFile](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        delayInvokeCalled = true;
        // Execute the function to test the selectFile call
        func();
    });
    
    // Mock FileDialog::selectFile
    bool selectFileCalled = false;
    testStub.set_lamda(ADDR(FileDialog, selectFile),
                       [&selectFileCalled, &receivedFile](FileDialog *, const QString &filename) {
        __DBG_STUB_INVOKE__
        selectFileCalled = true;
        receivedFile = filename;
    });

    handle->selectFile(testFile);
    EXPECT_TRUE(delayInvokeCalled);
    EXPECT_TRUE(selectFileCalled);
    EXPECT_EQ(receivedFile, testFile);
}

TEST_F(UT_FileDialogHandle, SelectedFiles_ReturnsCorrectFiles)
{
    QStringList expectedFiles = { "test1.txt", "test2.txt" };
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::selectedFiles
    testStub.set_lamda(ADDR(FileDialog, selectedFiles), [expectedFiles](FileDialog *) -> QStringList {
        __DBG_STUB_INVOKE__
        return expectedFiles;
    });

    QStringList result = handle->selectedFiles();
    EXPECT_EQ(result, expectedFiles);
}

TEST_F(UT_FileDialogHandle, SelectUrl_SelectsUrlCorrectly)
{
    QUrl testUrl("file:///home/test.txt");
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock CoreHelper::delayInvokeProxy
    bool delayInvokeCalled = false;
    QUrl receivedUrl;
    testStub.set_lamda(&CoreHelper::delayInvokeProxy,
                       [&delayInvokeCalled, &receivedUrl](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        delayInvokeCalled = true;
        // Execute the function to test selectUrl call
        func();
    });
    
    // Mock FileDialog::selectUrl
    bool selectUrlCalled = false;
    testStub.set_lamda(ADDR(FileDialog, selectUrl),
                       [&selectUrlCalled, &receivedUrl](FileDialog *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        selectUrlCalled = true;
        receivedUrl = url;
    });

    handle->selectUrl(testUrl);
    EXPECT_TRUE(delayInvokeCalled);
    EXPECT_TRUE(selectUrlCalled);
    EXPECT_EQ(receivedUrl, testUrl);
}

TEST_F(UT_FileDialogHandle, SelectedUrls_ReturnsCorrectUrls)
{
    QList<QUrl> expectedUrls = {
        QUrl("file:///home/test1.txt"),
        QUrl("file:///home/test2.txt")
    };
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::selectedUrls
    testStub.set_lamda(ADDR(FileDialog, selectedUrls), [expectedUrls](FileDialog *) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return expectedUrls;
    });

    QList<QUrl> result = handle->selectedUrls();
    EXPECT_EQ(result, expectedUrls);
}

TEST_F(UT_FileDialogHandle, AddDisableUrlScheme_DisablesSchemeCorrectly)
{
    QString testScheme = "ftp";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock CoreHelper::delayInvokeProxy
    bool delayInvokeCalled = false;
    QString receivedScheme;
    testStub.set_lamda(&CoreHelper::delayInvokeProxy,
                       [&delayInvokeCalled, &receivedScheme](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        delayInvokeCalled = true;
        // Execute the function to test urlSchemeEnable call
        func();
    });
    
    // Mock FileDialog::urlSchemeEnable
    bool urlSchemeEnableCalled = false;
    testStub.set_lamda(ADDR(FileDialog, urlSchemeEnable),
                       [&urlSchemeEnableCalled, &receivedScheme](FileDialog *, const QString &scheme, bool enable) {
        __DBG_STUB_INVOKE__
        urlSchemeEnableCalled = true;
        receivedScheme = scheme;
        EXPECT_FALSE(enable); // Should be disabled
    });

    handle->addDisableUrlScheme(testScheme);
    EXPECT_TRUE(delayInvokeCalled);
    EXPECT_TRUE(urlSchemeEnableCalled);
    EXPECT_EQ(receivedScheme, testScheme);
}

TEST_F(UT_FileDialogHandle, SetNameFilters_SetsFiltersCorrectly)
{
    QStringList filters = { "Text Files (*.txt)", "Images (*.png *.jpg)" };
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setNameFilters
    bool setNameFiltersCalled = false;
    QStringList receivedFilters;
    testStub.set_lamda(ADDR(FileDialog, setNameFilters),
                       [&setNameFiltersCalled, &receivedFilters](FileDialog *, const QStringList &filters) {
        __DBG_STUB_INVOKE__
        setNameFiltersCalled = true;
        receivedFilters = filters;
    });

    handle->setNameFilters(filters);
    EXPECT_TRUE(setNameFiltersCalled);
    EXPECT_EQ(receivedFilters, filters);
}

TEST_F(UT_FileDialogHandle, NameFilters_ReturnsCorrectFilters)
{
    QStringList expectedFilters = { "Text Files (*.txt)", "Images (*.png *.jpg)" };
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::nameFilters
    testStub.set_lamda(ADDR(FileDialog, nameFilters), [expectedFilters](FileDialog *) -> QStringList {
        __DBG_STUB_INVOKE__
        return expectedFilters;
    });

    QStringList result = handle->nameFilters();
    EXPECT_EQ(result, expectedFilters);
}

TEST_F(UT_FileDialogHandle, SelectNameFilter_SelectsFilterCorrectly)
{
    QString filter = "Text Files (*.txt)";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::selectNameFilter
    bool selectNameFilterCalled = false;
    QString receivedFilter;
    testStub.set_lamda(ADDR(FileDialog, selectNameFilter),
                       [&selectNameFilterCalled, &receivedFilter](FileDialog *, const QString &filter) {
        __DBG_STUB_INVOKE__
        selectNameFilterCalled = true;
        receivedFilter = filter;
    });

    handle->selectNameFilter(filter);
    EXPECT_TRUE(selectNameFilterCalled);
    EXPECT_EQ(receivedFilter, filter);
}

TEST_F(UT_FileDialogHandle, SelectedNameFilter_ReturnsCorrectFilter)
{
    QString expectedFilter = "Text Files (*.txt)";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::selectedNameFilter
    testStub.set_lamda(ADDR(FileDialog, selectedNameFilter), [expectedFilter](FileDialog *) -> QString {
        __DBG_STUB_INVOKE__
        return expectedFilter;
    });

    QString result = handle->selectedNameFilter();
    EXPECT_EQ(result, expectedFilter);
}

TEST_F(UT_FileDialogHandle, SelectNameFilterByIndex_SelectsFilterCorrectly)
{
    int index = 1;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::selectNameFilterByIndex
    bool selectNameFilterByIndexCalled = false;
    int receivedIndex;
    testStub.set_lamda(ADDR(FileDialog, selectNameFilterByIndex),
                       [&selectNameFilterByIndexCalled, &receivedIndex](FileDialog *, int index) {
        __DBG_STUB_INVOKE__
        selectNameFilterByIndexCalled = true;
        receivedIndex = index;
    });

    handle->selectNameFilterByIndex(index);
    EXPECT_TRUE(selectNameFilterByIndexCalled);
    EXPECT_EQ(receivedIndex, index);
}

TEST_F(UT_FileDialogHandle, SelectedNameFilterIndex_ReturnsCorrectIndex)
{
    int expectedIndex = 2;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::selectedNameFilterIndex
    testStub.set_lamda(ADDR(FileDialog, selectedNameFilterIndex), [expectedIndex](FileDialog *) -> int {
        __DBG_STUB_INVOKE__
        return expectedIndex;
    });

    int result = handle->selectedNameFilterIndex();
    EXPECT_EQ(result, expectedIndex);
}

TEST_F(UT_FileDialogHandle, WinId_ReturnsCorrectId)
{
    quint64 expectedWinId = 12345;
    stub.set_lamda(ADDR(FileDialog, internalWinId), [&]() -> quint64 {
        __DBG_STUB_INVOKE__;
        return expectedWinId;
    });
    quint64 result = handle->winId();
    EXPECT_EQ(result, expectedWinId);
}

TEST_F(UT_FileDialogHandle, Filter_ReturnsCorrectFilter)
{
    QDir::Filters expectedFilter = QDir::Files | QDir::Dirs;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::filter
    testStub.set_lamda(ADDR(FileDialog, filter), [expectedFilter](FileDialog *) -> QDir::Filters {
        __DBG_STUB_INVOKE__
        return expectedFilter;
    });

    QDir::Filters result = handle->filter();
    EXPECT_EQ(result, expectedFilter);
}

TEST_F(UT_FileDialogHandle, SetFilter_SetsFilterCorrectly)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock CoreHelper::delayInvokeProxy
    bool delayInvokeCalled = false;
    QDir::Filters receivedFilter;
    testStub.set_lamda(&CoreHelper::delayInvokeProxy,
                       [&delayInvokeCalled, &receivedFilter](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        delayInvokeCalled = true;
        // Execute the function to test setFilter call
        func();
    });
    
    // Mock FileDialog::setFilter
    bool setFilterCalled = false;
    testStub.set_lamda(ADDR(FileDialog, setFilter),
                       [&setFilterCalled, &receivedFilter](FileDialog *, QDir::Filters filters) {
        __DBG_STUB_INVOKE__
        setFilterCalled = true;
        receivedFilter = filters;
    });

    QDir::Filters testFilter = QDir::Files | QDir::Hidden;
    handle->setFilter(testFilter);
    EXPECT_TRUE(delayInvokeCalled);
    EXPECT_TRUE(setFilterCalled);
    EXPECT_EQ(receivedFilter, testFilter);
}

TEST_F(UT_FileDialogHandle, SetViewMode_Detail_SendsListMode)
{
    // Mock CoreEventsCaller::sendViewMode
    bool sendViewModeCalled = false;
    DFMBASE_NAMESPACE::Global::ViewMode receivedMode;
    stub.set_lamda(&CoreEventsCaller::sendViewMode, [&](QWidget *obj, DFMBASE_NAMESPACE::Global::ViewMode mode) {
        __DBG_STUB_INVOKE__;
        sendViewModeCalled = true;
        receivedMode = mode;
    });
    handle->setViewMode(QFileDialog::ViewMode::Detail);
    EXPECT_TRUE(sendViewModeCalled);
    EXPECT_EQ(receivedMode, DFMBASE_NAMESPACE::Global::ViewMode::kListMode);
}

TEST_F(UT_FileDialogHandle, SetViewMode_List_SendsIconMode)
{
    // Mock CoreEventsCaller::sendViewMode
    bool sendViewModeCalled = false;
    DFMBASE_NAMESPACE::Global::ViewMode receivedMode;
    stub.set_lamda(&CoreEventsCaller::sendViewMode, [&](QWidget *obj, DFMBASE_NAMESPACE::Global::ViewMode mode) {
        __DBG_STUB_INVOKE__;
        sendViewModeCalled = true;
        receivedMode = mode;
    });
    handle->setViewMode(QFileDialog::ViewMode::List);
    EXPECT_TRUE(sendViewModeCalled);
    EXPECT_EQ(receivedMode, DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
}

TEST_F(UT_FileDialogHandle, ViewMode_ReturnsCorrectMode)
{
    QFileDialog::ViewMode expectedMode = QFileDialog::ViewMode::Detail;
    stub.set_lamda(ADDR(FileDialog, currentViewMode), [&](FileDialog *) -> QFileDialog::ViewMode {
        __DBG_STUB_INVOKE__;
        return expectedMode;
    });
    QFileDialog::ViewMode result = handle->viewMode();
    EXPECT_EQ(result, expectedMode);
}

TEST_F(UT_FileDialogHandle, SetFileMode_SetsModeCorrectly)
{
    QFileDialog::FileMode mode = QFileDialog::FileMode::ExistingFiles;
    bool setFileModeCalled = false;
    stub.set_lamda(ADDR(FileDialog, setFileMode), [&](FileDialog *, QFileDialog::FileMode m) {
        __DBG_STUB_INVOKE__;
        setFileModeCalled = (m == mode);
    });
    handle->setFileMode(mode);
    EXPECT_TRUE(setFileModeCalled);
}

TEST_F(UT_FileDialogHandle, SetAcceptMode_SetsModeCorrectly)
{
    QFileDialog::AcceptMode mode = QFileDialog::AcceptMode::AcceptSave;
    bool setAcceptModeCalled = false;
    stub.set_lamda(ADDR(FileDialog, setAcceptMode), [&](FileDialog *, QFileDialog::AcceptMode m) {
        __DBG_STUB_INVOKE__;
        setAcceptModeCalled = (m == mode);
    });
    handle->setAcceptMode(mode);
    EXPECT_TRUE(setAcceptModeCalled);
}

TEST_F(UT_FileDialogHandle, AcceptMode_ReturnsCorrectMode)
{
    QFileDialog::AcceptMode expectedMode = QFileDialog::AcceptMode::AcceptSave;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::acceptMode
    testStub.set_lamda(ADDR(FileDialog, acceptMode), [expectedMode](FileDialog *) -> QFileDialog::AcceptMode {
        __DBG_STUB_INVOKE__
        return expectedMode;
    });

    QFileDialog::AcceptMode result = handle->acceptMode();
    EXPECT_EQ(result, expectedMode);
}

TEST_F(UT_FileDialogHandle, SetLabelText_SetsTextCorrectly)
{
    QFileDialog::DialogLabel label = QFileDialog::DialogLabel::LookIn;
    QString text = "Custom Label";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setLabelText
    bool setLabelTextCalled = false;
    QFileDialog::DialogLabel receivedLabel;
    QString receivedText;
    testStub.set_lamda(ADDR(FileDialog, setLabelText),
                       [&setLabelTextCalled, &receivedLabel, &receivedText](FileDialog *, QFileDialog::DialogLabel label, const QString &text) {
        __DBG_STUB_INVOKE__
        setLabelTextCalled = true;
        receivedLabel = label;
        receivedText = text;
    });

    handle->setLabelText(label, text);
    EXPECT_TRUE(setLabelTextCalled);
    EXPECT_EQ(receivedLabel, label);
    EXPECT_EQ(receivedText, text);
}

TEST_F(UT_FileDialogHandle, LabelText_ReturnsCorrectText)
{
    QFileDialog::DialogLabel label = QFileDialog::DialogLabel::LookIn;
    QString expectedText = "Custom Label";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::labelText
    testStub.set_lamda(ADDR(FileDialog, labelText), [expectedText](FileDialog *, QFileDialog::DialogLabel label) -> QString {
        __DBG_STUB_INVOKE__
        return expectedText;
    });

    QString result = handle->labelText(label);
    EXPECT_EQ(result, expectedText);
}

TEST_F(UT_FileDialogHandle, SetOptions_SetsOptionsCorrectly)
{
    QFileDialog::Options options = QFileDialog::Options(QFileDialog::Option::DontUseNativeDialog);
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setOptions
    bool setOptionsCalled = false;
    QFileDialog::Options receivedOptions;
    testStub.set_lamda(ADDR(FileDialog, setOptions),
                       [&setOptionsCalled, &receivedOptions](FileDialog *, QFileDialog::Options options) {
        __DBG_STUB_INVOKE__
        setOptionsCalled = true;
        receivedOptions = options;
    });

    handle->setOptions(options);
    EXPECT_TRUE(setOptionsCalled);
    EXPECT_EQ(receivedOptions, options);
}

TEST_F(UT_FileDialogHandle, SetOption_SetsOptionCorrectly)
{
    QFileDialog::Option option = QFileDialog::Option::DontUseNativeDialog;
    bool on = true;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setOption
    bool setOptionCalled = false;
    QFileDialog::Option receivedOption;
    bool receivedOn = false;
    testStub.set_lamda(ADDR(FileDialog, setOption),
                       [&setOptionCalled, &receivedOption, &receivedOn](FileDialog *, QFileDialog::Option option, bool on) {
        __DBG_STUB_INVOKE__
        setOptionCalled = true;
        receivedOption = option;
        receivedOn = on;
    });

    handle->setOption(option, on);
    EXPECT_TRUE(setOptionCalled);
    EXPECT_EQ(receivedOption, option);
    EXPECT_EQ(receivedOn, on);
}

TEST_F(UT_FileDialogHandle, Options_ReturnsCorrectOptions)
{
    QFileDialog::Options expectedOptions = QFileDialog::Options(QFileDialog::Option::DontUseNativeDialog);
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::options
    testStub.set_lamda(ADDR(FileDialog, options), [expectedOptions](FileDialog *) -> QFileDialog::Options {
        __DBG_STUB_INVOKE__
        return expectedOptions;
    });

    QFileDialog::Options result = handle->options();
    EXPECT_EQ(result, expectedOptions);
}

TEST_F(UT_FileDialogHandle, TestOption_ReturnsCorrectState)
{
    QFileDialog::Option option = QFileDialog::Option::DontUseNativeDialog;
    bool expectedState = true;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::testOption
    testStub.set_lamda(ADDR(FileDialog, testOption), [expectedState](FileDialog *, QFileDialog::Option option) -> bool {
        __DBG_STUB_INVOKE__
        return expectedState;
    });

    bool result = handle->testOption(option);
    EXPECT_EQ(result, expectedState);
}

TEST_F(UT_FileDialogHandle, SetCurrentInputName_SetsNameCorrectly)
{
    QString name = "test.txt";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setCurrentInputName
    bool setCurrentInputNameCalled = false;
    QString receivedName;
    testStub.set_lamda(ADDR(FileDialog, setCurrentInputName),
                       [&setCurrentInputNameCalled, &receivedName](FileDialog *, const QString &name) {
        __DBG_STUB_INVOKE__
        setCurrentInputNameCalled = true;
        receivedName = name;
    });

    handle->setCurrentInputName(name);
    EXPECT_TRUE(setCurrentInputNameCalled);
    EXPECT_EQ(receivedName, name);
}

TEST_F(UT_FileDialogHandle, AddCustomWidget_AddsWidgetCorrectly)
{
    int type = 0; // kLineEditType
    QString data = "custom data";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::addCustomWidget
    bool addCustomWidgetCalled = false;
    int receivedType = -1;
    QString receivedData;
    testStub.set_lamda(ADDR(FileDialog, addCustomWidget),
                       [&addCustomWidgetCalled, &receivedType, &receivedData](FileDialog *, FileDialog::CustomWidgetType type, const QString &data) {
        __DBG_STUB_INVOKE__
        addCustomWidgetCalled = true;
        receivedType = static_cast<int>(type);
        receivedData = data;
    });

    handle->addCustomWidget(type, data);
    EXPECT_TRUE(addCustomWidgetCalled);
    EXPECT_EQ(receivedType, type);
    EXPECT_EQ(receivedData, data);
}

TEST_F(UT_FileDialogHandle, GetCustomWidgetValue_ReturnsCorrectValue)
{
    int type = 0; // kLineEditType
    QString text = "test text";
    QVariant expectedValue = "custom value";
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::getCustomWidgetValue
    testStub.set_lamda(ADDR(FileDialog, getCustomWidgetValue), [expectedValue](FileDialog *, FileDialog::CustomWidgetType type, const QString &text) -> QVariant {
        __DBG_STUB_INVOKE__
        return expectedValue;
    });

    QDBusVariant result = handle->getCustomWidgetValue(type, text);
    EXPECT_EQ(result.variant(), expectedValue);
}

TEST_F(UT_FileDialogHandle, AllCustomWidgetsValue_ReturnsCorrectValues)
{
    int type = 0; // kLineEditType
    QVariantMap expectedValues = { { "key1", "value1" }, { "key2", "value2" } };
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::allCustomWidgetsValue
    testStub.set_lamda(ADDR(FileDialog, allCustomWidgetsValue), [expectedValues](FileDialog *, FileDialog::CustomWidgetType type) -> QVariantMap {
        __DBG_STUB_INVOKE__
        return expectedValues;
    });

    QVariantMap result = handle->allCustomWidgetsValue(type);
    EXPECT_EQ(result, expectedValues);
}

TEST_F(UT_FileDialogHandle, BeginAddCustomWidget_CallsDialogMethod)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::beginAddCustomWidget
    bool beginAddCustomWidgetCalled = false;
    testStub.set_lamda(ADDR(FileDialog, beginAddCustomWidget),
                       [&beginAddCustomWidgetCalled](FileDialog *) {
        __DBG_STUB_INVOKE__
        beginAddCustomWidgetCalled = true;
    });

    handle->beginAddCustomWidget();
    EXPECT_TRUE(beginAddCustomWidgetCalled);
}

TEST_F(UT_FileDialogHandle, EndAddCustomWidget_CallsDialogMethod)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::endAddCustomWidget
    bool endAddCustomWidgetCalled = false;
    testStub.set_lamda(ADDR(FileDialog, endAddCustomWidget),
                       [&endAddCustomWidgetCalled](FileDialog *) {
        __DBG_STUB_INVOKE__
        endAddCustomWidgetCalled = true;
    });

    handle->endAddCustomWidget();
    EXPECT_TRUE(endAddCustomWidgetCalled);
}

TEST_F(UT_FileDialogHandle, SetAllowMixedSelection_SetsSelectionCorrectly)
{
    bool on = true;
    
    // Mock FileDialog::setAllowMixedSelection
    bool setAllowMixedSelectionCalled = false;
    using SetAllowMixedSelectionFunc = void (FileDialog::*)(bool);
    stub.set_lamda(static_cast<SetAllowMixedSelectionFunc>(&FileDialog::setAllowMixedSelection), [&](FileDialog *, bool arg) {
        __DBG_STUB_INVOKE__;
        setAllowMixedSelectionCalled = arg;
    });

    handle->setAllowMixedSelection(on);
    EXPECT_TRUE(setAllowMixedSelectionCalled);
}

TEST_F(UT_FileDialogHandle, SetHideOnAccept_SetsHideCorrectly)
{
    bool enable = true;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setHideOnAccept
    bool setHideOnAcceptCalled = false;
    bool receivedEnable = false;
    testStub.set_lamda(ADDR(FileDialog, setHideOnAccept),
                       [&setHideOnAcceptCalled, &receivedEnable](FileDialog *, bool enable) {
        __DBG_STUB_INVOKE__
        setHideOnAcceptCalled = true;
        receivedEnable = enable;
    });

    handle->setHideOnAccept(enable);
    EXPECT_TRUE(setHideOnAcceptCalled);
    EXPECT_EQ(receivedEnable, enable);
}

TEST_F(UT_FileDialogHandle, HideOnAccept_ReturnsCorrectState)
{
    bool expectedState = true;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::hideOnAccept
    testStub.set_lamda(ADDR(FileDialog, hideOnAccept), [expectedState](FileDialog *) -> bool {
        __DBG_STUB_INVOKE__
        return expectedState;
    });

    bool result = handle->hideOnAccept();
    EXPECT_EQ(result, expectedState);
}

TEST_F(UT_FileDialogHandle, Show_ShowsDialogCorrectly)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::updateAsDefaultSize
    bool updateAsDefaultSizeCalled = false;
    testStub.set_lamda(ADDR(FileDialog, updateAsDefaultSize),
                       [&updateAsDefaultSizeCalled](FileDialog *) {
        __DBG_STUB_INVOKE__
        updateAsDefaultSizeCalled = true;
    });
    
    // Mock FileDialog::moveCenter (inherited from FileManagerWindow)
    bool moveCenterCalled = false;
    testStub.set_lamda(ADDR(FileManagerWindow, moveCenter),
                       [&moveCenterCalled](FileManagerWindow *) {
        __DBG_STUB_INVOKE__
        moveCenterCalled = true;
    });

    // Mock FMWindowsIns.showWindow
    bool showWindowCalled = false;
    testStub.set_lamda(&FileManagerWindowsManager::showWindow,
                       [&showWindowCalled](/*FileManagerWindowsManager::FMWindow *window*/) {
        __DBG_STUB_INVOKE__
        showWindowCalled = true;
    });

    handle->show();
    EXPECT_TRUE(updateAsDefaultSizeCalled);
    EXPECT_TRUE(moveCenterCalled);
    EXPECT_TRUE(showWindowCalled);
}

TEST_F(UT_FileDialogHandle, Hide_HidesDialogCorrectly)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::hide
    bool hideCalled = false;
    testStub.set_lamda(ADDR(FileDialog, hide),
                       [&hideCalled](QWidget *) {
        __DBG_STUB_INVOKE__
        hideCalled = true;
    });

    handle->hide();
    EXPECT_TRUE(hideCalled);
}

TEST_F(UT_FileDialogHandle, Accept_AcceptsDialogCorrectly)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::accept
    bool acceptCalled = false;
    testStub.set_lamda(ADDR(FileDialog, accept),
                       [&acceptCalled](FileDialog *) {
        __DBG_STUB_INVOKE__
        acceptCalled = true;
    });

    handle->accept();
    EXPECT_TRUE(acceptCalled);
}

TEST_F(UT_FileDialogHandle, Done_DoesDialogCorrectly)
{
    int result = 1;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::done
    bool doneCalled = false;
    int receivedResult = -1;
    testStub.set_lamda(ADDR(FileDialog, done),
                       [&doneCalled, &receivedResult](FileDialog *, int r) {
        __DBG_STUB_INVOKE__
        doneCalled = true;
        receivedResult = r;
    });

    handle->done(result);
    EXPECT_TRUE(doneCalled);
    EXPECT_EQ(receivedResult, result);
}

TEST_F(UT_FileDialogHandle, Exec_ExecsDialogCorrectly)
{
    int expectedResult = 1;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::exec
    testStub.set_lamda(ADDR(FileDialog, exec), [expectedResult](FileDialog *) -> int {
        __DBG_STUB_INVOKE__
        return expectedResult;
    });

    int result = handle->exec();
    EXPECT_EQ(result, expectedResult);
}

TEST_F(UT_FileDialogHandle, Open_OpensDialogCorrectly)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::open
    bool openCalled = false;
    testStub.set_lamda(ADDR(FileDialog, open),
                       [&openCalled](FileDialog *) {
        __DBG_STUB_INVOKE__
        openCalled = true;
    });

    handle->open();
    EXPECT_TRUE(openCalled);
}

TEST_F(UT_FileDialogHandle, Reject_RejectsDialogCorrectly)
{
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::reject
    bool rejectCalled = false;
    testStub.set_lamda(ADDR(FileDialog, reject),
                       [&rejectCalled](FileDialog *) {
        __DBG_STUB_INVOKE__
        rejectCalled = true;
    });

    handle->reject();
    EXPECT_TRUE(rejectCalled);
}

TEST_F(UT_FileDialogHandle, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
{
    // Test multiple method calls with different parameters
    int setDirectoryCallCount = 0;
    int selectFileCallCount = 0;
    int setNameFiltersCallCount = 0;
    int setViewModeCallCount = 0;
    
    // Create a new stub for this test
    stub_ext::StubExt testStub;
    
    // Mock FileDialog::setDirectory
    testStub.set_lamda((void(FileDialog::*)(const QString&))ADDR(FileDialog, setDirectory),
                       [&setDirectoryCallCount](FileDialog *, const QString &directory) {
        __DBG_STUB_INVOKE__
        setDirectoryCallCount++;
    });
    
    // Mock CoreHelper::delayInvokeProxy for selectFile
    testStub.set_lamda(&CoreHelper::delayInvokeProxy,
                       [](std::function<void()> func, quint64 winId, QObject *sender) {
        __DBG_STUB_INVOKE__
        // Execute function to test selectFile call
        func();
    });
    
    // Mock FileDialog::selectFile
    testStub.set_lamda(ADDR(FileDialog, selectFile),
                       [&selectFileCallCount](FileDialog *, const QString &filename) {
        __DBG_STUB_INVOKE__
        selectFileCallCount++;
    });
    
    // Mock FileDialog::setNameFilters
    testStub.set_lamda(ADDR(FileDialog, setNameFilters),
                       [&setNameFiltersCallCount](FileDialog *, const QStringList &filters) {
        __DBG_STUB_INVOKE__
        setNameFiltersCallCount++;
    });
    
    // Mock CoreEventsCaller::sendViewMode
    testStub.set_lamda(&CoreEventsCaller::sendViewMode, [&setViewModeCallCount](QWidget *obj, DFMBASE_NAMESPACE::Global::ViewMode mode) {
        __DBG_STUB_INVOKE__;
        setViewModeCallCount++;
    });

    // Call methods multiple times
    handle->setDirectory("/home/test1");
    handle->setDirectory("/home/test2");
    handle->selectFile("test1.txt");
    handle->selectFile("test2.txt");
    handle->setNameFilters({"*.txt"});
    handle->setNameFilters({"*.png"});
    handle->setViewMode(QFileDialog::ViewMode::Detail);
    handle->setViewMode(QFileDialog::ViewMode::List);
    
    EXPECT_EQ(setDirectoryCallCount, 2);
    EXPECT_EQ(selectFileCallCount, 2);
    EXPECT_EQ(setNameFiltersCallCount, 2);
    EXPECT_EQ(setViewModeCallCount, 2);
}
