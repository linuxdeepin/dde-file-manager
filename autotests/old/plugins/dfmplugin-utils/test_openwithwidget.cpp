// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-utils/openwith/openwithwidget.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/mimetype/mimesappsmanager.h>

#include <DRadioButton>
#include <QListWidget>

#include <gtest/gtest.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

class UT_OpenWithWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(ADDR(MimesAppsManager, initMimeTypeApps),
                       [] {
                           __DBG_STUB_INVOKE__
                       });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(UT_OpenWithWidget, Constructor_CreatesWidget)
{
    OpenWithWidget *widget = new OpenWithWidget();

    EXPECT_NE(widget, nullptr);
    EXPECT_NE(widget->openWithListWidget, nullptr);
    EXPECT_NE(widget->openWithBtnGroup, nullptr);

    delete widget;
}

TEST_F(UT_OpenWithWidget, selectFileUrl_SetsCurrentUrl)
{
    OpenWithWidget *widget = new OpenWithWidget();
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");

    widget->selectFileUrl(url);

    EXPECT_EQ(widget->currentFileUrl, url);

    delete widget;
}

TEST_F(UT_OpenWithWidget, slotExpandChange_FalseState_DoesNothing)
{
    OpenWithWidget *widget = new OpenWithWidget();

    widget->slotExpandChange(false);

    delete widget;
}

TEST_F(UT_OpenWithWidget, slotExpandChange_InvalidUrl_Returns)
{
    OpenWithWidget *widget = new OpenWithWidget();

    widget->slotExpandChange(true);

    delete widget;
}

TEST_F(UT_OpenWithWidget, slotExpandChange_NullFileInfo_Returns)
{
    OpenWithWidget *widget = new OpenWithWidget();
    widget->currentFileUrl = QUrl::fromLocalFile("/tmp/test.txt");

    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    widget->slotExpandChange(true);

    delete widget;
}

TEST_F(UT_OpenWithWidget, slotExpandChange_ValidUrl_LoadsApps)
{
    OpenWithWidget *widget = new OpenWithWidget();
    QUrl url = QUrl::fromLocalFile("/tmp/test.txt");
    widget->currentFileUrl = url;

    auto mockFileInfo = QSharedPointer<FileInfo>(new FileInfo(url));
    stub.set_lamda(&InfoFactory::create<FileInfo>,
                   [&mockFileInfo](const QUrl &, const Global::CreateFileInfoType, QString *) -> FileInfoPointer {
                       __DBG_STUB_INVOKE__
                       return mockFileInfo;
                   });

    stub.set_lamda(VADDR(FileInfo, fileMimeType),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QMimeType();
                   });

    stub.set_lamda(qOverload<const QMimeType &>(&MimesAppsManager::getDefaultAppByMimeType),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QString();
                   });

    stub.set_lamda(ADDR(MimesAppsManager, getRecommendedAppsByQio),
                   [] {
                       __DBG_STUB_INVOKE__
                       return QStringList();
                   });

    widget->slotExpandChange(true);

    delete widget;
}

TEST_F(UT_OpenWithWidget, openWithBtnChecked_WithButton_SetsDefault)
{
    OpenWithWidget *widget = new OpenWithWidget();
    bool setDefaultCalled = false;

    stub.set_lamda(ADDR(MimesAppsManager, setDefautlAppForTypeByGio),
                   [&setDefaultCalled] {
                       __DBG_STUB_INVOKE__
                       setDefaultCalled = true;
                       return true;
                   });

    Dtk::Widget::DRadioButton btn;
    btn.setProperty("mimeTypeName", "text/plain");
    btn.setProperty("appPath", "/usr/share/applications/test.desktop");

    widget->openWithBtnChecked(&btn);

    EXPECT_TRUE(setDefaultCalled);

    delete widget;
}

TEST_F(UT_OpenWithWidget, openWithBtnChecked_NullButton_DoesNothing)
{
    OpenWithWidget *widget = new OpenWithWidget();
    bool setDefaultCalled = false;

    stub.set_lamda(ADDR(MimesAppsManager, setDefautlAppForTypeByGio),
                   [&setDefaultCalled] {
                       __DBG_STUB_INVOKE__
                       setDefaultCalled = true;
                       return true;
                   });

    widget->openWithBtnChecked(nullptr);

    EXPECT_FALSE(setDefaultCalled);

    delete widget;
}
