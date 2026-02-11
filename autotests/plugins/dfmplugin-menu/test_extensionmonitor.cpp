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
#include <QStandardPaths>

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

TEST_F(UT_ExtensionMonitor, Start)
{
    bool called = false;
    typedef void (*FuncType)(int, Qt::TimerType, const QObject *, QtPrivate::QSlotObjectBase *);
    stub.set_lamda(static_cast<FuncType>(&QTimer::singleShotImpl),
                   [&]() {
                       __DBG_STUB_INVOKE__
                       called = true;
                   });

    ExtensionMonitor::instance()->start();
    EXPECT_TRUE(called);
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

TEST_F(UT_ExtensionMonitor, OnFileAdded_SourceNotExists_DoesNotCopy)
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
