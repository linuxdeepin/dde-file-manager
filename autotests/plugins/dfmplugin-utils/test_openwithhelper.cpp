// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/openwith/openwithhelper.h"
#include "plugins/common/dfmplugin-utils/openwith/openwithwidget.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-framework/dpf.h>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_OpenWithHelper : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_OpenWithHelper, Constructor_CreatesObject)
{
    OpenWithHelper *helper = new OpenWithHelper();
    EXPECT_NE(helper, nullptr);
    delete helper;
}

TEST_F(UT_OpenWithHelper, Constructor_WithParent_CreatesObject)
{
    QObject parent;
    OpenWithHelper *helper = new OpenWithHelper(&parent);
    EXPECT_NE(helper, nullptr);
    EXPECT_EQ(helper->parent(), &parent);
}

TEST_F(UT_OpenWithHelper, createOpenWithWidget_InvalidUrl_ReturnsNull)
{
    QUrl invalidUrl;

    QWidget *result = OpenWithHelper::createOpenWithWidget(invalidUrl);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_OpenWithHelper, createOpenWithWidget_NullFileInfo_ReturnsNull)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    QWidget *result = OpenWithHelper::createOpenWithWidget(url);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_OpenWithHelper, createOpenWithWidget_IsDirectory_ReturnsNull)
{
    QUrl url = QUrl::fromLocalFile("/tmp/testdir");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType type) -> bool {
                       __DBG_STUB_INVOKE__
                       return type == OptInfoType::kIsDir;
                   });

    QWidget *result = OpenWithHelper::createOpenWithWidget(url);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_OpenWithHelper, createOpenWithWidget_HookDisabled_ReturnsNull)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl, bool *&&);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run),
                   [](EventSequenceManager *, const QString &, const QString &,
                      const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = true;
                       return true;
                   });

    QWidget *result = OpenWithHelper::createOpenWithWidget(url);

    EXPECT_EQ(result, nullptr);
}

TEST_F(UT_OpenWithHelper, createOpenWithWidget_ValidFile_ReturnsWidget)
{
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, isAttributes),
                   [](FileInfo *, const OptInfoType) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    typedef bool (EventSequenceManager::*RunFunc)(const QString &, const QString &, QUrl, bool *&&);
    stub.set_lamda(static_cast<RunFunc>(&EventSequenceManager::run),
                   [](EventSequenceManager *, const QString &, const QString &,
                      const QUrl &, bool *result) -> bool {
                       __DBG_STUB_INVOKE__
                       *result = false;
                       return false;
                   });

    stub.set_lamda(ADDR(OpenWithWidget, selectFileUrl),
                   [](OpenWithWidget *, const QUrl &) {
                       __DBG_STUB_INVOKE__
                   });

    QWidget *result = OpenWithHelper::createOpenWithWidget(url);

    EXPECT_NE(result, nullptr);
    if (result)
        delete result;
}

