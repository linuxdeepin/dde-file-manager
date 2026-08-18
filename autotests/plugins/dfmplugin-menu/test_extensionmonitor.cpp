// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "oemmenuscene/extensionmonitor.h"

#include <dfm-base/file/local/localfilewatcher.h>

#include <gtest/gtest.h>

#include <QFile>
#include <QDir>
#include <QUrl>
#include <QTimer>
#include <QTemporaryDir>
#include <QApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <chrono>

namespace QtPrivate {
class QSlotObjectBase;
}

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_menu;

class UT_ExtensionMonitor : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_ExtensionMonitor, Instance_ReturnsSingleton)
{
    auto instance1 = ExtensionMonitor::instance();
    auto instance2 = ExtensionMonitor::instance();
    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

// rewritten: on Qt >= 6.8 the int-msec overload of QTimer::singleShotImpl is
// inlined, the call made by start() resolves to the nanoseconds overload,
// so that is the one to stub here.
TEST_F(UT_ExtensionMonitor, Start_SchedulesDeferredInitialization)
{
    bool scheduled = false;
    typedef void (*FuncType)(std::chrono::nanoseconds, Qt::TimerType, const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(&QTimer::singleShotImpl), [&scheduled]() {
        __DBG_STUB_INVOKE__
        scheduled = true;
    });

    EXPECT_NO_FATAL_FAILURE(ExtensionMonitor::instance()->start());
    EXPECT_TRUE(scheduled);
}

TEST_F(UT_ExtensionMonitor, SetupFileWatchers)
{
    bool called = false;
    stub.set_lamda(VADDR(LocalFileWatcher, startWatcher), [&called] {
        __DBG_STUB_INVOKE__
        called = true;
        return true;
    });

    ExtensionMonitor::instance()->extensionMap.insert("/tmp", "/test");
    ExtensionMonitor::instance()->setupFileWatchers();
    EXPECT_TRUE(called);
}

TEST_F(UT_ExtensionMonitor, ProcessExtensionDirectory_SourceNotExists_DoesNotCrash)
{
    stub.set_lamda(static_cast<bool (QDir::*)() const>(&QDir::exists), [](QDir *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    EXPECT_NO_FATAL_FAILURE(ExtensionMonitor::instance()->start());
}

TEST_F(UT_ExtensionMonitor, ProcessExtensionDirectory_SourceExists_CopiesFiles)
{
    bool copyFileCalled = false;
    stub.set_lamda(static_cast<bool (QDir::*)() const>(&QDir::exists), [](QDir *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<QStringList (QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const>(&QDir::entryList),
                   [](QDir *, const QStringList &, QDir::Filters, QDir::SortFlags) -> QStringList {
                       __DBG_STUB_INVOKE__
                       return { "test.desktop" };
                   });

    stub.set_lamda(static_cast<bool (*)(const QString &, const QString &)>(&QFile::copy), [&copyFileCalled](const QString &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        copyFileCalled = true;
        return true;
    });

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(&ExtensionMonitor::checkAndMkpath, [](ExtensionMonitor *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    ExtensionMonitor::instance()->extensionMap.insert("test1", "test2");
    ExtensionMonitor::instance()->copyInitialFiles();
    EXPECT_TRUE(copyFileCalled);
}

TEST_F(UT_ExtensionMonitor, OnFileAdded_DesktopFile_CopiesFile)
{
    bool copyFileCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QString &)>(&QFile::copy), [&copyFileCalled](const QString &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        copyFileCalled = true;
        return true;
    });

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &path) -> bool {
        __DBG_STUB_INVOKE__
        return path.endsWith(".desktop");
    });

    QUrl url = QUrl::fromLocalFile("/usr/share/deepin/dde-file-manager/oem-menuextensions/test.desktop");
    ExtensionMonitor::instance()->onFileAdded(url);
    EXPECT_TRUE(copyFileCalled);
}

TEST_F(UT_ExtensionMonitor, OnFileAdded_NonDesktopFile_DoesNotCopy)
{
    bool copyFileCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QString &)>(&QFile::copy), [&copyFileCalled](const QString &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        copyFileCalled = true;
        return true;
    });

    QUrl url = QUrl::fromLocalFile("/usr/share/deepin/dde-file-manager/oem-menuextensions/test.txt");
    ExtensionMonitor::instance()->onFileAdded(url);
    EXPECT_FALSE(copyFileCalled);
}

// renamed: onFileAdded never checks whether the source file exists,
// the copy attempt is always performed for *.desktop files.
TEST_F(UT_ExtensionMonitor, OnFileAdded_AnyFile_TriggersCopy)
{
    bool copyFileCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QString &, const QString &)>(&QFile::copy), [&copyFileCalled](const QString &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        copyFileCalled = true;
        return true;
    });

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl url = QUrl::fromLocalFile("/usr/share/deepin/dde-file-manager/oem-menuextensions/test.desktop");
    ExtensionMonitor::instance()->onFileAdded(url);
    EXPECT_TRUE(copyFileCalled);
}

TEST_F(UT_ExtensionMonitor, OnFileDeleted_ExistingFile_RemovesFile)
{
    bool removeFileCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::remove), [&removeFileCalled](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        removeFileCalled = true;
        return true;
    });

    QUrl url = QUrl::fromLocalFile("/usr/share/deepin/dde-file-manager/oem-menuextensions/test.desktop");
    ExtensionMonitor::instance()->onFileDeleted(url);
    EXPECT_TRUE(removeFileCalled);
}

TEST_F(UT_ExtensionMonitor, OnFileDeleted_NonExistingFile_DoesNotRemove)
{
    bool removeFileCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::remove), [&removeFileCalled](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        removeFileCalled = true;
        return true;
    });

    QUrl url = QUrl::fromLocalFile("/usr/share/deepin/dde-file-manager/oem-menuextensions/test.desktop");
    ExtensionMonitor::instance()->onFileDeleted(url);
    EXPECT_FALSE(removeFileCalled);
}

TEST_F(UT_ExtensionMonitor, OnFileAdded_UpdatesExtensionMap)
{
    stub.set_lamda(static_cast<bool (*)(const QString &, const QString &)>(&QFile::copy), [](const QString &, const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<bool (*)(const QString &)>(&QFile::exists), [](const QString &path) -> bool {
        __DBG_STUB_INVOKE__
        return path.endsWith(".desktop");
    });

    stub.set_lamda(&ExtensionMonitor::checkAndMkpath, [](ExtensionMonitor *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    QUrl url = QUrl::fromLocalFile("/usr/share/deepin/dde-file-manager/oem-menuextensions/test.desktop");
    EXPECT_NO_FATAL_FAILURE(ExtensionMonitor::instance()->onFileAdded(url));
}

// extended: direct call with real directories
TEST_F(UT_ExtensionMonitor, ProcessExtensionDirectory_RealDirs_CopiesDesktopFilesOnly)
{
    QTemporaryDir sourceDir;
    QTemporaryDir targetDir;
    ASSERT_TRUE(sourceDir.isValid());
    ASSERT_TRUE(targetDir.isValid());

    auto writeFile = [](const QString &path) {
        QFile f(path);
        bool ok = f.open(QIODevice::WriteOnly | QIODevice::Text);
        if (ok) {
            f.write("[Desktop Entry]\nType=Application\nName=T\n");
            f.close();
        }
        return ok;
    };

    ASSERT_TRUE(writeFile(QDir(sourceDir.path()).filePath("a.desktop")));
    ASSERT_TRUE(writeFile(QDir(sourceDir.path()).filePath("b.desktop")));
    ASSERT_TRUE(writeFile(QDir(sourceDir.path()).filePath("c.txt")));

    EXPECT_NO_FATAL_FAILURE(
        ExtensionMonitor::instance()->processExtensionDirectory(sourceDir.path(), targetDir.path()));

    EXPECT_TRUE(QFile::exists(QDir(targetDir.path()).filePath("a.desktop")));
    EXPECT_TRUE(QFile::exists(QDir(targetDir.path()).filePath("b.desktop")));
    EXPECT_FALSE(QFile::exists(QDir(targetDir.path()).filePath("c.txt")));

    // second pass: existing targets are skipped without error
    EXPECT_NO_FATAL_FAILURE(
        ExtensionMonitor::instance()->processExtensionDirectory(sourceDir.path(), targetDir.path()));
}

// extended: target path missing is created by checkAndMkpath
TEST_F(UT_ExtensionMonitor, ProcessExtensionDirectory_MissingTarget_CreatesTargetDir)
{
    QTemporaryDir baseDir;
    ASSERT_TRUE(baseDir.isValid());

    QString sourcePath = baseDir.path();
    QString targetPath = QDir(baseDir.path()).filePath("nested/target");
    ASSERT_FALSE(QDir(targetPath).exists());

    ExtensionMonitor::instance()->processExtensionDirectory(sourcePath, targetPath);
    EXPECT_TRUE(QDir(targetPath).exists());
}

// 不打桩 QTimer：真实等待 5 秒超时，执行 start() 中注册的延迟初始化 lambda。
TEST_F(UT_ExtensionMonitor, Start_DeferredInitLambda_RunsAfterTimeout)
{
    auto *monitor = ExtensionMonitor::instance();

    bool copyCalled = false;
    stub.set_lamda(ADDR(ExtensionMonitor, copyInitialFiles), [&copyCalled](ExtensionMonitor *) {
        __DBG_STUB_INVOKE__
        copyCalled = true;
    });
    bool setupCalled = false;
    stub.set_lamda(ADDR(ExtensionMonitor, setupFileWatchers), [&setupCalled](ExtensionMonitor *) {
        __DBG_STUB_INVOKE__
        setupCalled = true;
    });

    monitor->start();

    QElapsedTimer elapsed;
    elapsed.start();
    while (elapsed.elapsed() < 5200)
        QApplication::processEvents(QEventLoop::AllEvents, 100);

    EXPECT_TRUE(copyCalled);
    EXPECT_TRUE(setupCalled);
}
